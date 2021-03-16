// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : wc5.c
// Description : Line, word, char counter
// License     :
//
//  Copyright (C) 2021 Rachid Koucha <rachid dot koucha at gmail dot com>
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to:
//  the Free Software Foundation, Inc.,
//  51 Franklin Street, Fifth Floor,
//  Boston, MA  02110-1301  USA
//
// Evolutions  :
//
//     25-Feb-2021 R. Koucha      - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <sys/select.h>
#include <unistd.h>

#include "crtn.h"

static int w_offset, r_offset;

#define BUFFER_SIZE 128
static char buffer[BUFFER_SIZE];

struct counter_t
{
  size_t nb_chars;
  size_t nb_spaces;
  size_t nb_words;
  size_t nb_lines;
};

struct counter_t cnts;



//----------------------------------------------------------------------------
// Name        : nb_read
// Description : Non blocking read
// Return      : Number of read bytes if OK
//               -1, if error
//----------------------------------------------------------------------------
int nb_read(int fd, char *buf, size_t bufsz)
{
int             rc;
fd_set          fdset;
int             nfds = fd + 1;
struct timeval  to;

  to.tv_sec  = 0;
  to.tv_usec = 0;

  while(1) {
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    rc = select(nfds, &fdset, NULL, NULL, &to);
    switch (rc) {

      // Error
      case -1 : {
        // Interrupted system call ?
        if (EINTR == errno) {
          to.tv_sec  = 0;
          to.tv_usec = 0;
        } else {
          return -1;
        }
      }
      break;

      // Timeout
      case 0: {
        // No data ==> Retry with a timeout 5 ms
        to.tv_usec = 5000;
      }
      break;

      // Incoming data
      default : {
        rc = read(fd, buf, bufsz);
        if (rc < 0) {
          return -1;
        }

        if (0 == rc) {
          buf[0] = EOF;
          return 1;
        }

        return rc;
      }
      break;
    } // End switch
  } // End while
} // nb_read

static int read_buffer(void)
{
  if (r_offset == w_offset) {
    crtn_yield(0);
  }

  cnts.nb_chars ++;
  return buffer[r_offset ++];
} // read_buffer

#define unread_buffer(c) do {           \
                  assert(r_offset > 0); \
                  -- r_offset;          \
                  cnts.nb_chars --;     \
                } while(0)

static void fill_buffer(void)
{
  do {

    // Upon EOF, nb_read() returns 1 with EOF in buffer[0]
    w_offset = nb_read(0, buffer, BUFFER_SIZE - 1);

    // If no error, there is at least an EOF in the buffer
    if (w_offset > 0) {
      buffer[w_offset] = 0;
      r_offset = 0;
      while (r_offset != w_offset && buffer[w_offset-1] != EOF) {
        crtn_yield(0);
      }
    }

  } while (w_offset > 0 && buffer[w_offset-1] != EOF);

} // fill_buffer


static int get_spaces(void *p)
{
  int c;
  crtn_sem_t sem = *((crtn_sem_t *)p);
  int rc;

  do {

    rc = crtn_sem_p(sem);
    if (rc != 0) {
      fprintf(stderr, "crtn_sem_p(): error %d\n", crtn_errno());
      return -1;
    }

    c = read_buffer();
    while(isspace(c) && (c != '\n') && (c != EOF)) {
      cnts.nb_spaces ++;
      c = read_buffer();
    }
    unread_buffer(c);

    rc = crtn_sem_v(sem);
    if (rc != 0) {
      fprintf(stderr, "crtn_sem_v(): error %d\n", crtn_errno());
      return -1;
    }

    if (c == EOF) {
      break;
    }

    crtn_yield(0);

  } while(1);

  return 0;

} // get_spaces

static int get_word(void *p)
{
  int c;
  size_t count;
  crtn_sem_t sem = *((crtn_sem_t *)p);
  int rc;

  do {

    rc = crtn_sem_p(sem);
    if (rc != 0) {
      fprintf(stderr, "crtn_sem_p(): error %d\n", crtn_errno());
      return -1;
    }

    count = cnts.nb_chars;
    c = read_buffer();
    while(!isspace(c) && (c != EOF)) {
      c = read_buffer();
    }
    unread_buffer(c);
    if (cnts.nb_chars > count) {
      cnts.nb_words ++;
    }

    rc = crtn_sem_v(sem);
    if (rc != 0) {
      fprintf(stderr, "crtn_sem_v(): error %d\n", crtn_errno());
      return -1;
    }

    if (c == EOF) {
      break;
    }

    crtn_yield(0);

  } while(1);

  return 0;

} // get_word


static int get_lines(void *p)
{
  crtn_sem_t sem = *((crtn_sem_t *)p);
  int c;
  int rc;

  do {

    rc = crtn_sem_p(sem);
    if (rc != 0) {
      fprintf(stderr, "crtn_sem_p(): error %d\n", crtn_errno());
      return -1;
    }

    c = read_buffer();
    while((c == '\n') && (c != EOF)) {
      cnts.nb_lines ++;
      c = read_buffer();
    }
    unread_buffer(c);

    rc = crtn_sem_v(sem);
    if (rc != 0) {
      fprintf(stderr, "crtn_sem_v(): error %d\n", crtn_errno());
      return -1;
    }

    if (c == EOF) {
      break;
    }

    crtn_yield(0);

  } while(1);

  return 0;

} // get_lines


int main(void)
{
  crtn_t cid_word, cid_spaces, cid_lines;
  int rc;
  int status;
  crtn_sem_t sem;

  rc = crtn_sem_new(&sem, 1);
  if (rc != 0) {
    fprintf(stderr, "Error %d\n", crtn_errno());
    return 1;
  }

  rc = crtn_spawn(&cid_word, "word", get_word, &sem, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_spawn(&cid_lines, "lines", get_lines, &sem, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_spawn(&cid_spaces, "space", get_spaces, &sem, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  fill_buffer();

  rc = crtn_join(cid_word, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_join(cid_spaces, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_join(cid_lines, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_sem_delete(sem);
  if (rc != 0) {
    fprintf(stderr, "Error %d\n", crtn_errno());
    return 1;
  }

  printf("Lines: %zu / Words: %zu / Spaces: %zu / Characters: %zu\n"
         ,
         cnts.nb_lines, cnts.nb_words, cnts.nb_spaces, cnts.nb_chars
        );

  return status;

} // main
