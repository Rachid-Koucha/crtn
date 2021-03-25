// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : wc_cc.c
// Description : Line, word, char counter (caller/callee programming model)
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
//     09-Feb-2021 R. Koucha      - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <sys/select.h>
#include <unistd.h>


static size_t w_offset, r_offset;

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



#define SPACE 0
#define WORD  1
#define LINE  2
#define END   3
#define EMPTY 4



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


static int fill_buffer(void)
{
  int rc;
  unsigned long to;

  to = 0;

  do {

    rc = nb_read(0, buffer, BUFFER_SIZE, to);

    switch(rc) {

      case -1: {
        // Error
        return -1;
      }
      break;

      case -2: {
        // Timeout
        to = 250;  // Polling 250 ms
      }
      break;

      case -3: {
        // EOF
        buffer[0] = EOF;
        w_offset = 1;
        return 1;
      }
      break;

      default: {
        // New data
        w_offset = rc;
        return rc;
      }
      break;
    } // End switch

  } while(1);

} // fill_buffer


static int read_buffer(char *c)
{
  if (r_offset == w_offset) {
    r_offset = 0;
    return 0;
  }

  *c = buffer[r_offset ++];
  if (*c != EOF) {
    cnts.nb_chars ++;
  }

  return 1;
} // read_buffer


#define unread_buffer(c) do {           \
                  assert(r_offset > 0); \
                  -- r_offset;          \
                  cnts.nb_chars --;     \
                } while(0)


static int get_spaces(void)
{
  int rc;
  char c;

  rc = read_buffer(&c);

  while(rc) {

    if (isspace(c)) {
      if (c == '\n') {
        unread_buffer(c);
        return LINE;
      }

      cnts.nb_spaces ++;
    } else if (c == EOF) {
      return END;
    } else {
      unread_buffer(c);
      cnts.nb_words ++;
      return WORD;
    }

    rc = read_buffer(&c);

  } // End while

  return EMPTY;

} // get_spaces


static int get_word(void)
{
  int rc;
  char c;

  rc = read_buffer(&c);

  while(rc) {

    if (isspace(c)) {
      unread_buffer(c);
      if (c == '\n') {
        return LINE;
      } else {
        return SPACE;
      }
    } else if (c == EOF) {
       return END;
    }

    rc = read_buffer(&c);
  } // End while

  return EMPTY;

} // get_word


static int get_lines(void)
{
  int rc;
  char c;

  rc = read_buffer(&c);

  while(rc) {

    if (isspace(c)) {
      if (c == '\n') {
        cnts.nb_lines ++;
      } else {
        unread_buffer(c);
        return SPACE;
      }
    } else if (c == EOF) {
      return END;
    } else {
      unread_buffer(c);
      cnts.nb_words ++;
      return WORD;
    }

    rc = read_buffer(&c);
  } // End while

  return EMPTY;

} // get_lines


static int counter(void)
{
  static int state = SPACE;
  int rc = SPACE;

  do {

    switch(state) {

      case SPACE: {
        rc = get_spaces();
      }
      break;

      case WORD: {
        rc = get_word();
      }
      break;

      case LINE: {
        rc = get_lines();
      }
      break;
    } // End switch

    if (rc == END || rc == EMPTY) {
      break;
    }

    state = rc;

  } while(1);

  return rc;
  
} // counter


int main(void)
{
  int rc;
  int exit_code = 0;

  while(1) {

    rc = fill_buffer();
    if (rc > 0) {
      rc = counter();
      if (rc == END) {
        break;
      }
    } else {
      // Error
      exit_code = 1;
      goto err;
    }
  } // End while

  printf("Lines: %zu / Words: %zu / Spaces: %zu / Characters: %zu\n"
         ,
         cnts.nb_lines, cnts.nb_words, cnts.nb_spaces, cnts.nb_chars
        );

err:

  return exit_code;

} // main
