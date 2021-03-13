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
//     14-Feb-2021 R. Koucha      - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "crtn.h"

static size_t w_offset, r_offset;
static char buffer[128];

struct counter_t
{
  size_t nb_chars;
  size_t nb_words;
  size_t nb_lines;
};

#define SPACE 0
#define WORD  1
#define LINE  2
#define END   3


static int read_buffer(struct counter_t *counters)
{
  if (r_offset == w_offset) {
    crtn_yield(counters);
    r_offset = 0;
  }

  return buffer[r_offset ++];
} // read_buffer


static int get_spaces(int c, struct counter_t *counters)
{
  while(1) {

    if (isspace(c)) {
      counters->nb_chars ++;
      if (c == '\n') {
        counters->nb_lines ++;
        return LINE;
      }
    } else if (c == EOF) {
      counters->nb_lines ++;
      return END;
    } else {
      counters->nb_chars ++;
      counters->nb_words ++;
      return WORD;
    }

    c = read_buffer(counters);
  } // End while

} // get_spaces


static int get_word(int c, struct counter_t *counters)
{
  while(1) {

    if (isspace(c)) {
      counters->nb_chars ++;
      if (c == '\n') {
        counters->nb_lines ++;
        return LINE;
      } else {
        return SPACE;
      }
    } else if (c == EOF) {
      counters->nb_lines ++;
      return END;
    } else {
      counters->nb_chars ++;
    }

    c = read_buffer(counters);
  } // End while

} // get_word


static int get_lines(int c, struct counter_t *counters)
{
  while(1) {

    if (isspace(c)) {
      counters->nb_chars ++;
      if (c == '\n') {
        counters->nb_lines ++;
      } else {
        return SPACE;
      }
    } else if (c == EOF) {
      return END;
    } else {
      counters->nb_chars ++;
      counters->nb_words ++;
      return WORD;
    }

    c = read_buffer(counters);
  } // End while

} // get_lines


static int counter(void *param)
{
  int state = SPACE;
  int c;
  struct counter_t counters;

  (void)param;

  counters.nb_chars =
  counters.nb_words =
  counters.nb_lines = 0;

  while(state != END) {

    c = read_buffer(&counters);

    switch(state) {

      case SPACE: {
        state = get_spaces(c, &counters);
      }
      break;

      case WORD: {
        state = get_word(c, &counters);
      }
      break;

      case LINE: {
        state = get_lines(c, &counters);
      }
      break;
    } // End switch

  } // End while

  // Provide the last counters
  crtn_yield(&counters);

  return 0;
  
} // counter


int main(void)
{
  int c;
  crtn_t cid;
  int rc;
  int status;
  crtn_attr_t attr;
  struct counter_t *cnts;

  attr = crtn_attr_new();
  if (!attr) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_attr_new(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_set_attr_type(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_spawn(&cid, "Counter", counter, 0, attr);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_attr_delete(attr);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_attr_delete(): error '%m' (%d)\n", errno);
    return 1;
  }

  while(1) {

    c = getchar();
    buffer[w_offset++] = c;

    if (c == EOF) {

      rc = crtn_wait(cid, (void **)&cnts);
      if (rc != 0) {
        errno = crtn_errno();
        fprintf(stderr, "crtn_wait(): error '%m' (%d)\n", errno);
        return 1;
      }

      break;
    }

    if (w_offset == sizeof(buffer)) {

      rc = crtn_wait(cid, (void **)&cnts);
      if (rc != 0) {
        errno = crtn_errno();
        fprintf(stderr, "crtn_wait(): error '%m' (%d)\n", errno);
        return 1;
      }

      w_offset = 0;
    }

  } // End while

  printf("Lines: %zu / Words: %zu / Characters: %zu\n"
         ,
         cnts->nb_lines, cnts->nb_words, cnts->nb_chars
        );

  // Finish the coroutine
  rc = crtn_wait(cid, 0);
  if (rc != CRTN_DEAD) {
    fprintf(stderr, "Unexpect rc=%d from crtn_wait(), error '%m' (%d)\n", rc, errno);
    return 1;
  }

  rc = crtn_join(cid, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    return 1;
  }

  // Counter no more valid here as the counter coroutine is finished

  return status;

} // main
