// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : wc1.c
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
//     26-Feb-2021 R. Koucha      - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>



#define BUFFER_SIZE 128
static char buffer[BUFFER_SIZE];

static size_t w_offset, r_offset = BUFFER_SIZE;

struct counter_t
{
  size_t nb_chars;
  size_t nb_words;
  size_t nb_lines;
};

struct counter_t cnts;

#define SPACE 0
#define WORD  1
#define LINE  2
#define END   3



static int read_buffer(void)
{
  if (r_offset == BUFFER_SIZE) {
    r_offset = 0;
    return -1;
  }

  return buffer[r_offset ++];
} // read_buffer


static void fill_buffer(void)
{
  int c;

  w_offset = 0;
  do {
    c = getchar();
    buffer[w_offset ++] = c;
  } while ((c != EOF) && (w_offset < BUFFER_SIZE));

} // fill_buffer


static int read_char(void)
{
  int c;

  c = read_buffer();
  if (r_offset == 0) {
    fill_buffer();
    c = read_buffer();
  }

  return c;
} // read_char


#define unread_char(c) do { \
                  assert(r_offset > 0); \
                  -- r_offset; \
                } while(0)


static void get_spaces(void)
{
  int c;

  cnts.nb_chars ++;

  do {
    c = read_char();
    cnts.nb_chars ++;
  } while(isspace(c) && (c != '\n') && (c != EOF));

  cnts.nb_chars --;
  unread_char(c);
  return;

} // get_spaces

static void get_word(void)
{
  int c;

  cnts.nb_words ++;
  cnts.nb_chars ++;

  do {
    c = read_char();
    cnts.nb_chars ++;
  } while(!isspace(c) && (c != EOF));

  cnts.nb_chars --;
  unread_char(c);
  return;

} // get_word


static void get_lines(void)
{
  int c;

  cnts.nb_chars ++;

  do {
    cnts.nb_lines ++;
    c = read_char();
    cnts.nb_chars ++;
  } while(c == '\n');

  cnts.nb_chars --;
  unread_char(c);
  return;

} // get_lines


static int counter(void)
{
  int c;

  do {

    c = read_char();

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
  counter();

  printf("Lines: %zu / Words: %zu / Characters: %zu\n"
         ,
         cnts.nb_lines, cnts.nb_words, cnts.nb_chars
        );

  return 0;

} // main
