// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : wc.c
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
//     09-Feb-2021 R. Koucha      - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "crtn.h"

static size_t w_offset, r_offset;
static char buffer[128];

static unsigned int nb_chars, nb_words, nb_lines;

#define SPACE 0
#define WORD  1
#define LINE  2
#define END   3


static int read_buffer(void)
{
  if (r_offset == w_offset) {
    crtn_yield(0);
    r_offset = 0;
  }

  nb_chars ++;
  return buffer[r_offset ++];
} // read_buffer


#define unread_buffer(c) do {           \
                  assert(r_offset > 0); \
                  -- r_offset;          \
                  nb_chars --;          \
                } while(0)


static int get_spaces(int c)
{
  while(1) {

    c = read_buffer();

    if (isspace(c)) {
      if (c == '\n') {
        unread_buffer(c);
        return LINE;
      }
    } else if (c == EOF) {
      unread_buffer(c);
      return END;
    } else {
      nb_words ++;
      unread_buffer(c);
      return WORD;
    }
  } // End while

} // get_spaces


static int get_word(int c)
{
  while(1) {

    c = read_buffer();

    if (isspace(c)) {
      if (c == '\n') {
        unread_buffer(c);
        return LINE;
      } else {
        unread_buffer(c);
        return SPACE;
      }
    } else if (c == EOF) {
      unread_buffer(c);
      return END;
    }

  } // End while

} // get_word


static int get_lines(int c)
{
  while(1) {

    c = read_buffer();

    if (isspace(c)) {
      if (c == '\n') {
        nb_lines ++;
      } else {
        unread_buffer(c);
        return SPACE;
      }
    } else if (c == EOF) {
      unread_buffer(c);
      return END;
    } else {
      unread_buffer(c);
      nb_words ++;
      return WORD;
    }

  } // End while

} // get_lines


static int counter(void *param)
{
  int state = SPACE;
  int c;

  (void)param;

  while(state != END) {

    switch(state) {

      case SPACE: {
        state = get_spaces(c);
      }
      break;

      case WORD: {
        state = get_word(c);
      }
      break;

      case LINE: {
        state = get_lines(c);
      }
      break;
    } // End switch

  } // End while

  return 0;
  
} // counter


int main(void)
{
  int c;
  crtn_t cid;
  int rc;
  int status;

  rc = crtn_spawn(&cid, "Counter", counter, 0, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  while(1) {

    c = getchar();
    buffer[w_offset++] = c;

    if (c == EOF) {
      crtn_yield(0);
      break;
    }

    if (w_offset == sizeof(buffer)) {
      crtn_yield(0);
      w_offset = 0;
    }

  } // End while

  rc = crtn_join(cid, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    return 1;
  }

  printf("Lines: %u / Words: %u / Characters: %u\n"
         ,
         nb_lines, nb_words, nb_chars
        );

  return status;

} // main
