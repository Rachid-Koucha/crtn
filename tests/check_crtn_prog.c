// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : check_crtn_prog.c
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
//     21-Mar-2021  R. Koucha    - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "../config.h"
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <limits.h>

#include "crtn.h"

#include "check_all.h"
#include "check_crtn.h"


// This is run once in the main process before each test cases
static void crtn_setup_unchecked_fixture(void)
{


} // crtn_setup_unchecked_fixture


// This is run once inside each test case processes
static void crtn_setup_checked_fixture(void)
{

} // crtn_setup_checked_fixture


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_fibonacci)

  int rc;
  char *av[30];
  char pathname[256];
  pid_t pid;
  int status;
  char max_long[100];

  snprintf(pathname, sizeof(pathname), "%s/tests/fibonacci", CRTN_ROOT_SRC);

  // -------

  av[0] = pathname;
  av[1] = (char *)0;
  rc = ck_exec_prog(av);
  ck_assert_int_gt(rc, 0);
  pid = rc;
  sleep(10);
  rc = kill(pid, SIGINT);
  ck_assert_int_eq(rc, 0);
  rc = waitpid(pid, &status, 0);
  ck_assert_int_eq(rc, pid);
  // The programs returns the status of the cancelled coroutine
  ck_assert_exited(status, CRTN_STATUS_CANCELLED);

  // ------- Config environment variables
  rc = setenv("CRTN_MAX", "10", 1);
  ck_assert_int_eq(rc, 0);

  av[0] = pathname;
  av[1] = (char *)0;
  rc = ck_exec_prog(av);
  ck_assert_int_gt(rc, 0);
  pid = rc;
  sleep(2);
  rc = kill(pid, SIGINT);
  ck_assert_int_eq(rc, 0);
  rc = waitpid(pid, &status, 0);
  ck_assert_int_eq(rc, pid);
  // The programs returns the status of the cancelled coroutine
  ck_assert_exited(status, CRTN_STATUS_CANCELLED);

  // ------- Environment variable too big to trigger an error in strtol()
  rc = snprintf(max_long, sizeof(max_long), "%ld", LONG_MAX);
  if (rc < (int)(sizeof(max_long) - 1)) {
    max_long[rc] = '9';
    max_long[rc + 1] = '\0';
    rc = setenv("CRTN_MAX", max_long, 1);
    ck_assert_int_eq(rc, 0);

    av[0] = pathname;
    av[1] = (char *)0;
    rc = ck_exec_prog(av);
    ck_assert_int_gt(rc, 0);
    pid = rc;
    sleep(2);
    rc = kill(pid, SIGINT);
    ck_assert_int_eq(rc, 0);
    rc = waitpid(pid, &status, 0);
    ck_assert_int_eq(rc, pid);
    // The programs returns the status of the cancelled coroutine
    ck_assert_exited(status, CRTN_STATUS_CANCELLED);
  }

  // ------- Environment variable too big to trigger a malloc() error
  snprintf(max_long, sizeof(max_long), "%ld", LONG_MAX - 1);
  rc = setenv("CRTN_MAX", max_long, 1);
  ck_assert_int_eq(rc, 0);

  av[0] = pathname;
  av[1] = (char *)0;
  rc = ck_exec_prog(av);
  ck_assert_int_gt(rc, 0);
  pid = rc;
  sleep(2);
  rc = kill(pid, SIGINT);
  ck_assert_int_eq(rc, 0);
  rc = waitpid(pid, &status, 0);
  ck_assert_int_eq(rc, pid);
  // The programs will crash as the table of coroutines is not allocated
  // The coverage data is available as we installed a signal handler on
  // SIGSEGV to exit gracefully with exit code 7
  ck_assert_exited(status, 7);
  rc = unsetenv("CRTN_MAX");
  ck_assert_int_eq(rc, 0);

#ifdef HAVE_CRTN_MBX
  // ------- Environment variables too big to trigger a malloc error
  rc = setenv("CRTN_MBX_MAX", max_long, 1);
  ck_assert_int_eq(rc, 0);

  av[0] = pathname;
  av[1] = (char *)0;
  rc = ck_exec_prog(av);
  ck_assert_int_gt(rc, 0);
  pid = rc;
  sleep(2);
  rc = kill(pid, SIGINT);
  ck_assert_int_eq(rc, 0);
  rc = waitpid(pid, &status, 0);
  ck_assert_int_eq(rc, pid);
  // The programs returns the status of the cancelled coroutine
  ck_assert_exited(status, CRTN_STATUS_CANCELLED);
  rc = unsetenv("CRTN_MBX_MAX");
  ck_assert_int_eq(rc, 0);
#endif // HAVE_CRTN_MBX


#ifdef HAVE_CRTN_SEM
  // ------- Environment variables too big to trigger a malloc error
  rc = setenv("CRTN_SEM_MAX", max_long, 1);
  ck_assert_int_eq(rc, 0);

  av[0] = pathname;
  av[1] = (char *)0;
  rc = ck_exec_prog(av);
  ck_assert_int_gt(rc, 0);
  pid = rc;
  sleep(2);
  rc = kill(pid, SIGINT);
  ck_assert_int_eq(rc, 0);
  rc = waitpid(pid, &status, 0);
  ck_assert_int_eq(rc, pid);
  // The programs returns the status of the cancelled coroutine
  ck_assert_exited(status, CRTN_STATUS_CANCELLED);
  rc = unsetenv("CRTN_SEM_MAX");
  ck_assert_int_eq(rc, 0);
#endif // HAVE_CRTN_SEM


END_TEST



TCase *crtn_prog_tests(void)
{
TCase *tc_prog;

  /* Error test case */
  tc_prog = tcase_create("CRTN PROGRAMS");

  // Increase the timeout for the test case as we use lots of timeouts
  tcase_set_timeout(tc_prog, 60);

  tcase_add_unchecked_fixture(tc_prog, crtn_setup_unchecked_fixture, 0);
  tcase_add_checked_fixture(tc_prog, crtn_setup_checked_fixture, 0);
  tcase_add_test(tc_prog, test_crtn_fibonacci);

  return tc_prog;
} // crtn_prog_tests
