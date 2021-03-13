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


struct msg_hdr
{
  size_t data_len;
  char data[0];
};


static crtn_mbx_t mbx;

static unsigned int nb_chars, nb_words, nb_lines;

#define SPACE 0
#define WORD  1
#define LINE  2
#define END   3


static int read_buffer(void)
{
  int rc;
  static char *msg, *data;
  static size_t data_len;
  static size_t r_offset;

  if (r_offset == data_len) {

    struct msg_hdr *header;

    rc = crtn_mbx_get(mbx, (void *)&msg);
    if (0 != rc) {
      errno = crtn_errno();
      fprintf(stderr, "crtn_mbx_get(): error '%m' (%d)\n", errno);
      return EOF;
    }

    header = (struct msg_hdr *)msg;
    data = header->data;
    data_len = header->data_len;
    r_offset = 0;
  }

  return data[r_offset ++];
} // read_buffer


static int get_spaces(int c)
{
  while(1) {

    if (isspace(c)) {
      nb_chars ++;
      if (c == '\n') {
        nb_lines ++;
        return LINE;
      }
    } else if (c == EOF) {
      nb_lines ++;
      return END;
    } else {
      nb_chars ++;
      nb_words ++;
      return WORD;
    }

    c = read_buffer();
  } // End while

} // get_spaces


static int get_word(int c)
{
  while(1) {

    if (isspace(c)) {
      nb_chars ++;
      if (c == '\n') {
        nb_lines ++;
        return LINE;
      } else {
        return SPACE;
      }
    } else if (c == EOF) {
      nb_lines ++;
      return END;
    } else {
      nb_chars ++;
    }

    c = read_buffer();
  } // End while

} // get_word


static int get_lines(int c)
{
  while(1) {

    if (isspace(c)) {
      nb_chars ++;
      if (c == '\n') {
        nb_lines ++;
      } else {
        return SPACE;
      }
    } else if (c == EOF) {
      return END;
    } else {
      nb_chars ++;
      nb_words ++;
      return WORD;
    }

    c = read_buffer();
  } // End while

} // get_lines


static int counter(void *param)
{
  int state = SPACE;
  int c;

  (void)param;

  while(state != END) {

    c = read_buffer();

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
  char buffer[128];
  char *msg;
  char *data;
  size_t data_size;
  struct msg_hdr *header;
  unsigned int w_offset;

  rc = crtn_mbx_new(&mbx);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_mbx_create(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_spawn(&cid, "Counter", counter, 0, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  msg = crtn_mbx_format(buffer, sizeof(buffer), &data_size);
  if (!msg) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_mbx_format(): error '%m' (%d)\n", errno);
    return 1;
  }

  header = (struct msg_hdr *)msg;
  data = header->data;
  data_size = sizeof(buffer) - (data - buffer);
  w_offset = 0;

  while(1) {

    c = getchar();
    data[w_offset++] = c;

    if (c == EOF) {
      header->data_len = w_offset;
      rc = crtn_mbx_post(mbx, msg);
      if (rc != 0) {
        errno = crtn_errno();
        fprintf(stderr, "crtn_mbx_post(): error '%m' (%d)\n", errno);
        return 1;
      }

      crtn_yield(0);

      break;
    }

    if (w_offset == data_size) {
      header->data_len = w_offset;
      rc = crtn_mbx_post(mbx, msg);
      if (rc != 0) {
        errno = crtn_errno();
        fprintf(stderr, "crtn_mbx_post(): error '%m' (%d)\n", errno);
        return 1;
      }
      w_offset = 0;

      crtn_yield(0);
    }

  } // End while

  rc = crtn_join(cid, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_mbx_delete(mbx);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_mbx_delete(): error '%m' (%d)\n", errno);
    return 1;
  }

  printf("Lines: %u / Words: %u / Characters: %u\n"
         ,
         nb_lines, nb_words, nb_chars
        );

  return status;

} // main
