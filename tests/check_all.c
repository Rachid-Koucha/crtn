// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : check_all.c
// Description : Unitary test for CRTN
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
//     02-Mar-2021  R. Koucha    - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "../config.h"

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#include "check_all.h"
#include "check_crtn.h"


int main(void)
{
int      number_failed = 0;
Suite   *s1;
SRunner *sr;

  // Create the suites
  s1 = suite_create("CRTN tests");

  // Add the tests cases in the suites
  suite_add_tcase(s1, crtn_errcodes_tests());
  suite_add_tcase(s1, crtn_api_tests());

  sr = srunner_create(s1);

  srunner_run_all(sr, CK_ENV);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (0 == number_failed) ? EXIT_SUCCESS : EXIT_FAILURE;
} // main
