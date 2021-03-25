// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : fibonacci_cc.c
// Description : Fibonacci sequence (caller/callee programming model)
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
//     24-Mar-2021 R. Koucha      - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <stdlib.h>

static int signaled;

static void hdl_sigint(int sig)
{

  printf("Signal %d...\n", sig);
  signaled = 1;

} // hdl_sigint


static  unsigned long long fibonacci(void)
{
  static unsigned long long prevn_1 = 0;
  static unsigned long long prevn = 1;
  static int i = 0;

  if (i == 0) {
    i ++;
    return 0;
  } else if (i == 1) {
    i ++;
    return 1;
  } else {
    unsigned long long cur = 0;

    // Check overflow
    if ((ULLONG_MAX - prevn_1) < prevn) {
      prevn_1 = 0;
      prevn = 1;
      i = 1;
      return 0;
    }

    cur = prevn + prevn_1;
    prevn_1 = prevn;
    prevn = cur;
    return cur;
  }
  
} // fibonacci


int main(void)
{
  unsigned long long seq;
  unsigned int i;

  signal(SIGINT, hdl_sigint);

  i = 0;
  while(1) {

    seq = fibonacci();
    printf("seq[%u]=%llu\n", i, seq);
    i ++;
    sleep(1);

    if (signaled) {
      break;
    }
  } // End while

  return 0;

} // main
