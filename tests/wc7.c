// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : wc7.c
// Description : Line, word, spaces, char counter
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
//     22-Mar-2021 R. Koucha      - Creation
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
//               -2, timeout
//               -3, EOF
//----------------------------------------------------------------------------
static int nb_read(int fd, char *buf, size_t bufsz, unsigned long to_ms)
{
int            rc;
fd_set         fdset;
int            nfds = fd + 1;
struct timeval to;

  to.tv_sec  = 0;
  to.tv_usec = to_ms * 1000;

  while(1) {
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    rc = select(nfds, &fdset, NULL, NULL, &to);
    switch (rc) {

      // Error
      case -1 : {
        // Interrupted system call ?
        if (EINTR != errno) {
          return -1;
        }
      }
      break;

      // Timeout
      case 0: {
        return -2;        
      }
      break;

      // Incoming data
      default : {
        rc = read(fd, buf, bufsz);

        // Error ?
        if (rc < 0) {
          return -1;
        }

        // EOF ?
        if (0 == rc) {
          return -3;
        }

        // Data
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

static int fill_buffer(void)
{
  int rc;
  unsigned long to;
  size_t size;

  to = 0;
  while(1) {

    do {

      if (w_offset) {
        // If buffer is empty, reset the pointers
        if (w_offset == r_offset) {
          w_offset = r_offset = 0;
        }
      }

      size = BUFFER_SIZE - w_offset;

      // There is still some space behind the write pointer, tries to fill
      // it with additional input data
      if (size) {
        rc = nb_read(0, &(buffer[w_offset]), size, to);
        to = 0;
      } else {
        // The wrtie pointer is at the end of the buffer and there
        // are still remaining data to read
        crtn_yield(0);
      }

    } while(size == 0);

    switch(rc) {
      case -1: {
        // Error
        buffer[w_offset] = EOF; // Trigger the end of the coroutines  
        w_offset ++;
        crtn_yield(0);
        return -1;
      }
      break;

      case -2: {
        // Timeout
        if (w_offset > r_offset) {
          // There are still data to read in the buffer
          crtn_yield(0);
        } else {

          // Buffer empty, wait for more input data
          to = 250;  // Polling 250 ms
        }
      }
      break;

      case -3: {
        // EOF
        buffer[w_offset] = EOF; // Trigger the end of the coroutines  
        w_offset ++;
        crtn_yield(0);
        return 0;
      }
      break;

      default: {
        // New data
        w_offset += rc;
        crtn_yield(0);
      }
      break;
    } // End switch
  } // End while

} // fill_buffer


static int get_spaces(void *p)
{
  int c;

  (void)p;

  do {

    c = read_buffer();
    while(isspace(c) && (c != '\n') && (c != EOF)) {
      cnts.nb_spaces ++;
      c = read_buffer();
    }
    unread_buffer(c);

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

  (void)p;

  do {

    count = cnts.nb_chars;
    c = read_buffer();
    while(!isspace(c) && (c != EOF)) {
      c = read_buffer();
    }
    unread_buffer(c);
    if (cnts.nb_chars > count) {
      cnts.nb_words ++;
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
  int c;

  (void)p;

  do {

    c = read_buffer();
    while((c == '\n') && (c != EOF)) {
      cnts.nb_lines ++;
      c = read_buffer();
    }
    unread_buffer(c);

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
  int exit_code;

  rc = crtn_spawn(&cid_word, "word", get_word, 0, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_spawn(&cid_lines, "lines", get_lines, 0, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_spawn(&cid_spaces, "space", get_spaces, 0, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  exit_code = 0;

  rc = fill_buffer();
  if (rc != 0) {
    fprintf(stderr, "Input error '%m' (%d)\n", errno);
    exit_code = 1;
  }

  rc = crtn_join(cid_word, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    exit_code = 1;
  }

  rc = crtn_join(cid_spaces, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    exit_code = 1;
  }

  rc = crtn_join(cid_lines, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    exit_code = 1;
  }

  printf("Lines: %zu / Words: %zu / Spaces: %zu / Characters: %zu\n"
         ,
         cnts.nb_lines, cnts.nb_words, cnts.nb_spaces, cnts.nb_chars
        );

  return exit_code;

} // main
