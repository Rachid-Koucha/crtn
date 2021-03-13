// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : check_crtn_errcodes.c
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


static int dummy_entry(void *p)
{
  (void)p;
  return 0;
}

// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_spawn)

int rc;
crtn_t cid;

  rc = crtn_spawn(0, 0, 0, 0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_spawn(0, "foo", dummy_entry, 0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_spawn(&cid, 0, dummy_entry, 0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_spawn(&cid, "foo", 0, 0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);


END_TEST



static int dummy_entry1(void *p)
{
int rc;

  (void)p;

  rc = crtn_join(crtn_self(), 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  return rc;
}

// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_join)

int rc;
crtn_t cid;
int status;

  rc = crtn_join(0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_join(56000, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), ENOENT);

  rc = crtn_spawn(&cid, "foo", dummy_entry1, 0, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, -1);

END_TEST



static int dummy_entry2(void *p)
{
  int rc;

  (void)p;

  rc = crtn_wait(CRTN_CID_MAIN, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  return 1;
}


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_wait)

int rc;
crtn_t cid;
int status;

  rc = crtn_wait(0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_wait(56000, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), ENOENT);

  rc = crtn_spawn(&cid, "foo", dummy_entry, 0, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_wait(cid, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 0);

  // -------
  rc = crtn_spawn(&cid, "foo", dummy_entry2, 0, 0);
  ck_assert_int_eq(rc, 0);

  // The standalone coroutine tried to wait on
  // Main (no stepper coroutine)

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 1);


END_TEST


static int dummy_entry3(void *p)
{
int rc;

  (void)p;

  rc = crtn_cancel(CRTN_CID_MAIN);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EPERM);

  return rc;
}

static int dummy_entry4(void *p)
{
  (void)p;

  crtn_yield(0);

  return 0;
}

// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_cancel)

int rc;
crtn_t cid;
int status;

  rc = crtn_cancel(0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_cancel(56000);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), ENOENT);

  rc = crtn_spawn(&cid, "foo", dummy_entry3, 0, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, -1);


  rc = crtn_spawn(&cid, "foo", dummy_entry, 0, 0);
  ck_assert_int_eq(rc, 0);

  // Trigger the coroutine
  crtn_yield(0);

  // As the coroutine is STANDALONE, it run and exited
  rc = crtn_cancel(cid);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 0);


  rc = crtn_spawn(&cid, "foo", dummy_entry4, 0, 0);
  ck_assert_int_eq(rc, 0);

  // As the coroutine is STANDALONE, it run and called crtn_yield()
  rc = crtn_cancel(cid);
  ck_assert_int_eq(rc, 0);

  rc = crtn_cancel(cid);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EBUSY);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, CRTN_STATUS_CANCELLED);


END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_attr)

int rc;
crtn_attr_t attr;

  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_attr_delete(0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_set_attr_type(0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_set_attr_type(attr, 0xFFFFFFFF);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_set_attr_stack_size(0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_set_attr_stack_size(attr, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STACKLESS);
  ck_assert_int_eq(rc, 0);

  rc = crtn_set_attr_stack_size(attr, 30*1024);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

END_TEST


#ifdef HAVE_CRTN_MBX

// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_new)

int rc;

  rc = crtn_mbx_new(0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_delete)

int rc;

  rc = crtn_mbx_delete(-1);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_mbx_delete(CRTN_MBX_MAX);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_post)

int rc;

  rc = crtn_mbx_post(-1, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_mbx_post(CRTN_MBX_MAX, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_mbx_post(0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_get)

int rc;

  rc = crtn_mbx_get(-1, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_mbx_get(CRTN_MBX_MAX, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_mbx_get(0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_tryget)

int rc;

  rc = crtn_mbx_tryget(-1, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_mbx_tryget(CRTN_MBX_MAX, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_mbx_tryget(0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_free)

int rc;

  rc = crtn_mbx_free(0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_format)

void *p;
char buffer[50];
size_t data_size;

  p = crtn_mbx_format(0, 0, 0);
  ck_assert_ptr_eq(p, 0);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  p = crtn_mbx_format(buffer, 0, &data_size);
  ck_assert_ptr_eq(p, 0);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  // Alignment problem
  p = crtn_mbx_format(&(buffer[1]), sizeof(buffer) - 1, &data_size);
  ck_assert_ptr_eq(p, 0);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST



#endif // HAVE_CRTN_MBX



#ifdef HAVE_CRTN_SEM

// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_sem_new)

int rc;

  rc = crtn_sem_new(0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_sem_delete)

int rc;

  rc = crtn_sem_delete(-1);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_sem_delete(CRTN_SEM_MAX);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_sem_v)

int rc;

  rc = crtn_sem_v(-1);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_sem_v(CRTN_SEM_MAX);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_sem_p)

int rc;

  rc = crtn_sem_p(-1);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_sem_p(CRTN_SEM_MAX);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

END_TEST


#endif // HAVE_CRTN_MBX


TCase *crtn_errcodes_tests(void)
{
TCase *tc_err_code;

  /* Error test case */
  tc_err_code = tcase_create("CRTN error codes");

  // Increase the timeout for the test case as we use lots of timeouts
  tcase_set_timeout(tc_err_code, 60);

  tcase_add_unchecked_fixture(tc_err_code, crtn_setup_unchecked_fixture, 0);
  tcase_add_checked_fixture(tc_err_code, crtn_setup_checked_fixture, 0);
  tcase_add_test(tc_err_code, test_crtn_spawn);
  tcase_add_test(tc_err_code, test_crtn_join);
  tcase_add_test(tc_err_code, test_crtn_wait);
  tcase_add_test(tc_err_code, test_crtn_cancel);
  tcase_add_test(tc_err_code, test_crtn_attr);

#ifdef HAVE_CRTN_MBX
  tcase_add_test(tc_err_code, test_crtn_mbx_new);
  tcase_add_test(tc_err_code, test_crtn_mbx_delete);
  tcase_add_test(tc_err_code, test_crtn_mbx_post);
  tcase_add_test(tc_err_code, test_crtn_mbx_get);
  tcase_add_test(tc_err_code, test_crtn_mbx_tryget);
  tcase_add_test(tc_err_code, test_crtn_mbx_free);
  tcase_add_test(tc_err_code, test_crtn_mbx_format);
#endif // HAVE_CRTN_MBX

#ifdef HAVE_CRTN_SEM
  tcase_add_test(tc_err_code, test_crtn_sem_new);
  tcase_add_test(tc_err_code, test_crtn_sem_delete);
  tcase_add_test(tc_err_code, test_crtn_sem_v);
  tcase_add_test(tc_err_code, test_crtn_sem_p);
#endif // HAVE_CRTN_SEM

  return tc_err_code;
} // crtn_errcodes_tests
