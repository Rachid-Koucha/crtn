// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : wc4.c
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
    r_offset = 0;
  }

  return buffer[r_offset ++];
} // read_buffer

#define unread_buffer(c) do { \
                  assert(r_offset > 0); \
                  -- r_offset; \
                } while(0)

static void fill_buffer(void)
{
  do {

    w_offset = nb_read(0, buffer, BUFFER_SIZE);

    // If no error, there is at least an EOF in the buffer
    if (w_offset > 0) {
      crtn_yield(0);
    }

  } while (w_offset > 0 && buffer[w_offset-1] != EOF);

} // fill_buffer


static void get_spaces(void)
{
  int c;

  c = read_buffer();
  while(isspace(c) && (c != '\n') && (c != EOF)) {
    cnts.nb_chars ++;
    c = read_buffer();
  }

  unread_buffer(c);
  return;

} // get_spaces

static void get_word(void)
{
  int c;

  cnts.nb_words ++;

  c = read_buffer();
  while(!isspace(c) && (c != EOF)) {
    cnts.nb_chars ++;
    c = read_buffer();
  }

  unread_buffer(c);
  return;

} // get_word


static void get_lines(void)
{
  int c;

  c = read_buffer();
  while(c == '\n') {
    cnts.nb_chars ++;
    cnts.nb_lines ++;
    c = read_buffer();
  }

  unread_buffer(c);
  return;

} // get_lines


static int counter(void *param)
{
  int c;

  (void)param;

  do {

    c = read_buffer();
    unread_buffer(c);
    if (c == '\n') {
      get_lines();
    } else if (isspace(c)) {
      get_spaces();
    } else if (c != EOF) {
      get_word();
    }

  } while (c != EOF);

  return 0;
  
} // counter


int main(void)
{
  crtn_t cid;
  int rc;
  int status;

  rc = crtn_spawn(&cid, "Counter", counter, 0, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  fill_buffer();

  rc = crtn_join(cid, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    return 1;
  }

  printf("Lines: %zu / Words: %zu / Characters: %zu\n"
         ,
         cnts.nb_lines, cnts.nb_words, cnts.nb_chars
        );

  return status;

} // main
