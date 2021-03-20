// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : check_crtn_api.c
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
#include <stdio.h>

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


static int entry(void *p)
{
  (void)p;
  return 0;
}


static int entry1(void *p)
{
  int rc;

  (void)p;

  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  return 0;
}


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_spawn)

int rc;
crtn_t cid;
//crtn_t cid1;
int status;
crtn_attr_t attr;
crtn_t cid_tab[CRTN_MAX];
unsigned int i;

  // ------- Overflow of the maximum number of coroutines

  // One coroutine already allocated for Main
  for (i = 0; i < (CRTN_MAX - 1); i ++) {
    rc = crtn_spawn(&(cid_tab[i]), "foo", entry, 0, 0);
    ck_assert_int_eq(rc, 0);
  }

  // Maximum number of coroutines reached
  rc = crtn_spawn(&(cid_tab[i]), "foo", entry, 0, 0);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EAGAIN);

  for (i = 0; i < (CRTN_MAX - 1); i ++) {
    rc = crtn_join(cid_tab[i], 0);
    ck_assert_int_eq(rc, 0);
  }


  // ------- Standalone coroutine not started
  rc = crtn_spawn(&cid, "foo", entry, 0, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);


  // ------- Standalone coroutine started and finished
  rc = crtn_spawn(&cid, "foo", entry, 0, 0);
  ck_assert_int_eq(rc, 0);

  // Make foo start and finish
  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);

  
  // ------- Standalone coroutine started
  rc = crtn_spawn(&cid, "foo", entry1, 0, 0);
  ck_assert_int_eq(rc, 0);

  // Make foo start: it calls crtn_yield()
  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);


  // ------- Standalone coroutine with default attributes
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_spawn(&cid, "foo", entry, 0, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);


  // ------- Stackful + custom stack
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STACKFUL);
  ck_assert_int_eq(rc, 0);

  rc = crtn_set_attr_stack_size(attr, CRTN_DEFAULT_STACK_SIZE + 4096);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "foo", entry, 0, attr);
  ck_assert_int_eq(rc, 0);

  // Start coroutine
  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);


  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);


  // ------- Stackless coroutine not started
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STACKLESS);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "foo", entry, 0, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);


  // -------
  rc = crtn_spawn(&cid, "foo", entry1, 0, 0);
  ck_assert_int_eq(rc, 0);

  // Start coroutine
  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  // The standalone coroutine is in the runnable list
  // suspended on crtn_yield()

  rc = crtn_cancel(cid);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, CRTN_STATUS_CANCELLED);


  // -------
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "foo", entry, 0, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_cancel(cid);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, CRTN_STATUS_CANCELLED);


END_TEST


static int entry5(void *p)
{
  (void)p;

  crtn_exit(5);

  return 0;
}


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_exit)

int rc;
crtn_t cid;
int status;
crtn_attr_t attr;

  // ------- Started through crtn_join()
  rc = crtn_spawn(&cid, "foo", entry5, 0, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

  // ------- Started and finished before crtn_join()
  rc = crtn_spawn(&cid, "foo", entry5, 0, 0);
  ck_assert_int_eq(rc, 0);

  // Start the coroutine
  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

#if 0

  // ------- Not started
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "foo_5", entry5, 0, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

  // Program error as a stepper coroutine will not start
  // without a crtn_wait()

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);
#endif // 0

  // ------- Started
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "foo", entry5, 0, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_wait(cid, 0);
  ck_assert_int_eq(rc, CRTN_DEAD);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

END_TEST




static int entry20(void *p)
{
  int rc;
  int value;

  (void)p;

  value = 1;
  rc = crtn_yield(&value);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  value = 2;
  rc = crtn_yield(&value);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  crtn_exit(-2);

  return 0;
}

static int entry21(void *p)
{
  int rc;
  int value;

  (void)p;

  value = 1;
  rc = crtn_yield(&value);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  return 0;
}


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_wait)

int rc;
crtn_t cid;
int status;
crtn_attr_t attr;
int *pvalue;

  // -------
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "foo_20", entry20, 0, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_wait(cid, (void **)&pvalue);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(*pvalue, 1);

  rc = crtn_wait(cid, (void **)&pvalue);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(*pvalue, 2);

  rc = crtn_wait(cid, 0);
  ck_assert_int_eq(rc, CRTN_DEAD);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, -2);


  // -------
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "foo1", entry21, 0, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_wait(cid, (void **)&pvalue);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(*pvalue, 1);

  // This makes the coroutine finish its entry point
  rc = crtn_wait(cid, (void **)&pvalue);
  ck_assert_int_eq(rc, CRTN_DEAD);
  ck_assert_ptr_eq(pvalue, 0);

  rc = crtn_wait(cid, (void **)&pvalue);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EPERM);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 0);

END_TEST




static int entry30(void *p)
{
  int rc;
  int value;

  (void)p;

  value = 1;
  rc = crtn_yield(&value);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  value = 2;
  rc = crtn_yield(&value);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  crtn_exit(-4);

  return 0;
}

static int entry31(void *p)
{
  int cid = *((int *)p);
  int status;
  int rc;

  (void)p;

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 20);

  return 0;
}

static int entry32(void *p)
{
  int rc;
  (void)p;

  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  return 20;
}


static int entry_foo6(void *p)
{
  int cid = *((int *)p);
  int rc;

  (void)p;

  rc = crtn_wait(cid, 0);
  ck_assert_int_eq(rc, CRTN_DEAD);

  return 0;
}

static int entry_sub(void *p)
{
  (void)p;

  return 28;
}

static int entry_foo5(void *p)
{
  crtn_t cid;
  int status;
  int rc;

  (void)p;

  rc = crtn_spawn(&cid, "sub", entry_sub, 0, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 28);

  return 20;
}


static int entry39(void *p)
{
  int rc;
  crtn_t cid = *((crtn_t *)p);

  rc = crtn_cancel(cid);
  ck_assert_int_eq(rc, 0);

  return -45;
}


static crtn_t cid39;

static int entry38(void *p)
{
  int rc;
  int mycid = crtn_self();
  crtn_attr_t attr;

  (void)p;

  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid39, "foo_39", entry39, &mycid, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

  // Trigger foo_39 which will cancel me :-(
  rc = crtn_wait(cid39, 0);
  ck_assert_int_eq(rc, 0);

  return 26;
}



// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_cancel)

int rc;
crtn_t cid, cid1;
int status;
crtn_attr_t attr;
int *pvalue;

  // ------- Cancel stepper/stackless coroutine (in ready state)
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER|CRTN_TYPE_STACKLESS);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "foo1", entry30, 0, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

  // This makes foo1 start
  rc = crtn_wait(cid, (void **)&pvalue);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(*pvalue, 1);

  rc = crtn_cancel(cid);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, CRTN_STATUS_CANCELLED);



  // ------- Cancel on terminated stepper/stackless coroutine
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER|CRTN_TYPE_STACKLESS);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "_foo2", entry30, 0, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_wait(cid, (void **)&pvalue);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(*pvalue, 1);

  rc = crtn_wait(cid, (void **)&pvalue);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(*pvalue, 2);

  // This makes the coroutine finish its entry point and enter ZOMBIE state
  rc = crtn_wait(cid, (void **)&pvalue);
  ck_assert_int_eq(rc, CRTN_DEAD);
  ck_assert_ptr_eq(pvalue, 0);

  rc = crtn_cancel(cid);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, -4);


  // ------- Cancel a stepper coroutine in READY state
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "foo3", entry30, 0, attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_wait(cid, (void **)&pvalue);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(*pvalue, 1);

  rc = crtn_cancel(cid);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, CRTN_STATUS_CANCELLED);
  

  // ------- Cancel a coroutine suspended on crtn_join()
  rc = crtn_spawn(&cid, "foo5", entry32, 0, 0);
  ck_assert_int_eq(rc, 0);

  // Suspend foo5 on crtn_yield()
  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  rc = crtn_spawn(&cid1, "foo6", entry31, &cid, 0);
  ck_assert_int_eq(rc, 0);

  // Suspend foo6 on crtn_join(): this wakes up foo5
  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  // foo5 suspends on the second crtn_yield()

  // Cancel the joining foo6 coroutine
  rc = crtn_cancel(cid1);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid1, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, CRTN_STATUS_CANCELLED);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 20);


  // ------- Multiple coroutines waiting each other...
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER);
  ck_assert_int_eq(rc, 0);

  // Suspended in ready state
  rc = crtn_spawn(&cid, "foo5", entry_foo5, 0, attr);
  ck_assert_int_eq(rc, 0);

  // foo5 ready

  rc = crtn_attr_delete(attr);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid1, "foo6", entry_foo6, &cid, 0);
  ck_assert_int_eq(rc, 0);

  // foo6 runnable

  crtn_yield(0);

  // main runnable (foo6 --> main)
  // foo6 running
  // foo6 triggers foo5
  // foo5 runnable (foo6 --> main --> foo5)
  // foo6 waiting on foo5 (main --> foo5)
  // main is running (main --> foo5)
  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  // main runnable
  // foo5 is running (foo5 --> main)
  // sub is spawned (foo5 --> main --> sub)
  // foo5 is joining sub (main --> sub)

  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  // sub is running
  // main is runnable (sub --> main)
  // sub finishes
  // foo5 is runnable
  // main is running (main -->foo5)

  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  // foo5 is running (it joins sub)
  // main is runnable (foo5 --> main)
  // foo5 finishes, foo6 is runnable (main --> foo6)
  // main is running

  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  // foo6 is running
  // main is runnable (foo6 --> main)
  // foo6 finnishes
  // main is running

  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_SELF);

  // Cancel the foo6 coroutine which is finished
  rc = crtn_cancel(cid1);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EINVAL);

  rc = crtn_join(cid1, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 20);


  // ------- Cancel a coroutine in waiting state
  attr = crtn_attr_new();
  ck_assert_ptr_ne(attr, NULL);

  rc = crtn_spawn(&cid, "foo_38", entry38, 0, attr);
  ck_assert_int_eq(rc, 0);

  // Trigger foo_38
  rc = crtn_yield(0);
  ck_assert_int_eq(rc, CRTN_SCHED_OTHER);

  // foo_39 cancels foo_38 which waits on it
  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, CRTN_STATUS_CANCELLED);

  rc = crtn_join(cid39, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, -45);

END_TEST


static int entry40(void *p)
{
  (void)p;

  crtn_yield(0);

  return 20;
}

static int entry41(void *p)
{
  crtn_t cid = *((crtn_t *)p);
  int status;
  int rc;

  (void)p;

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 20);

  return 25;
}



// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_join)

int rc;
crtn_t cid, cid1;
int status;

  // ------- Concurrent join on the same coroutine
  rc = crtn_spawn(&cid, "foo_40", entry40, 0, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid1, "foo_41", entry41, &cid, 0);
  ck_assert_int_eq(rc, 0);

  // Trigger foo_41
  crtn_yield(0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EBUSY);

  // Trigger foo_40
  crtn_yield(0);

  // foo_41 runs
  rc = crtn_join(cid1, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 25);
  

END_TEST



#ifdef HAVE_CRTN_MBX


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_new)

int rc;
int i;
crtn_mbx_t mbx_tab[CRTN_MBX_MAX + 1];

  // ------- Overflow of the maximum number of mailboxes

  // One coroutine already allocated for Main
  for (i = 0; i < CRTN_MBX_MAX; i ++) {
    rc = crtn_mbx_new(&(mbx_tab[i]));
    ck_assert_int_eq(rc, 0);
  }

  // Maximum number of mailboxes reached
  rc = crtn_mbx_new(&(mbx_tab[i]));
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EAGAIN);

  for (i = 0; i < (CRTN_MBX_MAX - 1); i ++) {
    rc = crtn_mbx_delete(mbx_tab[i]);
    ck_assert_int_eq(rc, 0);
  }

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_delete)

int rc;
crtn_mbx_t mbx;

  // -------

  rc = crtn_mbx_new(&mbx);
  ck_assert_int_eq(rc, 0);
  rc = crtn_mbx_delete(mbx);
  ck_assert_int_eq(rc, 0);

END_TEST




static int entry_post_01(void *p)
{
  void *msg;
  int rc;
  crtn_mbx_t mbx = *((crtn_mbx_t *)p);

  msg = (void *)0;
  rc = crtn_mbx_get(mbx, &msg);
  ck_assert_int_eq(rc, 0);
  ck_assert_ptr_ne(msg, 0);

  rc = crtn_mbx_free(msg);
  ck_assert_int_eq(rc, 0);

  return 5;
}


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_post)

int rc;
crtn_mbx_t mbx;
char *msg;
crtn_t cid, cid1;
int status;

  // -------

  rc = crtn_mbx_new(&mbx);
  ck_assert_int_eq(rc, 0);

  msg = crtn_mbx_alloc(30);
  ck_assert_ptr_ne(msg, 0);

  rc = crtn_mbx_post(mbx, msg);
  ck_assert_int_eq(rc, 0);

  rc = crtn_mbx_delete(mbx);
  ck_assert_int_eq(rc, 0);

  rc = crtn_mbx_free(msg);
  ck_assert_int_eq(rc, 0);


  // ------- Post in a mbx on which a coroutine is not waiting yet
  rc = crtn_mbx_new(&mbx);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "post_01", entry_post_01, &mbx, 0);
  ck_assert_int_eq(rc, 0);

  msg = crtn_mbx_alloc(30);
  ck_assert_ptr_ne(msg, 0);

  rc = crtn_mbx_post(mbx, msg);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

  rc = crtn_mbx_delete(mbx);
  ck_assert_int_eq(rc, 0);


  // ------- Post in a mbx on which a coroutine is waiting
  rc = crtn_mbx_new(&mbx);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "post_01", entry_post_01, &mbx, 0);
  ck_assert_int_eq(rc, 0);

  msg = crtn_mbx_alloc(30);
  ck_assert_ptr_ne(msg, 0);

  // Make the coroutine wait on the mailbox
  crtn_yield(0);

  rc = crtn_mbx_post(mbx, msg);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

  rc = crtn_mbx_delete(mbx);
  ck_assert_int_eq(rc, 0);


  // ------- Post in a mbx on which 2 coroutines are waiting
  rc = crtn_mbx_new(&mbx);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "post_01", entry_post_01, &mbx, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid1, "post_02", entry_post_01, &mbx, 0);
  ck_assert_int_eq(rc, 0);

  // Make the coroutines wait on the mailbox (the FIFO scheduler
  // will start the two coroutines before going back here)
  crtn_yield(0);

  msg = crtn_mbx_alloc(30);
  ck_assert_ptr_ne(msg, 0);

  // This puts the coroutines in the runnable state
  // One will get the message and the other goes back in waiting state
  rc = crtn_mbx_post(mbx, msg);
  ck_assert_int_eq(rc, 0);

  msg = crtn_mbx_alloc(30);
  ck_assert_ptr_ne(msg, 0);

  rc = crtn_mbx_post(mbx, msg);
  ck_assert_int_eq(rc, 0);

  // Launch the coroutines
  crtn_yield(0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

  rc = crtn_join(cid1, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

  rc = crtn_mbx_delete(mbx);
  ck_assert_int_eq(rc, 0);


  // ------- Post one message at a time in a mbx on which 2 coroutines are waiting
  rc = crtn_mbx_new(&mbx);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "post_01", entry_post_01, &mbx, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid1, "post_02", entry_post_01, &mbx, 0);
  ck_assert_int_eq(rc, 0);

  // Make the coroutines wait on the mailbox (the FIFO scheduler
  // will start the two coroutines before going back here)
  crtn_yield(0);

  msg = crtn_mbx_alloc(30);
  ck_assert_ptr_ne(msg, 0);

  // This puts the coroutines in the runnable state
  // One will get the message and the other goes back in waiting state
  rc = crtn_mbx_post(mbx, msg);
  ck_assert_int_eq(rc, 0);

  msg = crtn_mbx_alloc(30);
  ck_assert_ptr_ne(msg, 0);

  rc = crtn_mbx_post(mbx, msg);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

  rc = crtn_join(cid1, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

  rc = crtn_mbx_delete(mbx);
  ck_assert_int_eq(rc, 0);


END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_get)

int rc;
crtn_mbx_t mbx;
char *msg;
crtn_t cid;
int status;

  // -------

  rc = crtn_mbx_new(&mbx);
  ck_assert_int_eq(rc, 0);

  msg = crtn_mbx_alloc(30);
  ck_assert_ptr_ne(msg, 0);

  rc = crtn_mbx_post(mbx, msg);
  ck_assert_int_eq(rc, 0);

  msg = (char *)0;
  rc = crtn_mbx_get(mbx, (void **)&msg);
  ck_assert_int_eq(rc, 0);
  ck_assert_ptr_ne(msg, 0);

  rc = crtn_mbx_delete(mbx);
  ck_assert_int_eq(rc, 0);

  rc = crtn_mbx_free(msg);
  ck_assert_int_eq(rc, 0);


  // ------- Cancel a coroutine waiting on a mailbox
  rc = crtn_mbx_new(&mbx);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "post_01", entry_post_01, &mbx, 0);
  ck_assert_int_eq(rc, 0);

  // Make the coroutines wait on the mailbox (the FIFO scheduler
  // will start the two coroutines before going back here)
  crtn_yield(0);

  // Cancel the waiting coroutine
  rc = crtn_cancel(cid);
  ck_assert_int_eq(rc, 0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, CRTN_STATUS_CANCELLED);

  rc = crtn_mbx_delete(mbx);
  ck_assert_int_eq(rc, 0);

END_TEST



// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_tryget)

int rc;
crtn_mbx_t mbx;
char *msg;

  // -------

  rc = crtn_mbx_new(&mbx);
  ck_assert_int_eq(rc, 0);

  msg = (char *)0;
  rc = crtn_mbx_tryget(mbx, (void **)&msg);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EAGAIN);
  ck_assert_ptr_eq(msg, 0);

  msg = crtn_mbx_alloc(30);
  ck_assert_ptr_ne(msg, 0);

  rc = crtn_mbx_post(mbx, msg);
  ck_assert_int_eq(rc, 0);

  msg = (char *)0;
  rc = crtn_mbx_tryget(mbx, (void **)&msg);
  ck_assert_int_eq(rc, 0);
  ck_assert_ptr_ne(msg, 0);

  rc = crtn_mbx_delete(mbx);
  ck_assert_int_eq(rc, 0);

  rc = crtn_mbx_free(msg);
  ck_assert_int_eq(rc, 0);

END_TEST



// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_mbx_format)

int rc;
crtn_mbx_t mbx;
char buffer[50];
char *msg;
size_t msg_size;

  // -------

  rc = crtn_mbx_new(&mbx);
  ck_assert_int_eq(rc, 0);

  msg = (char *)0;
  rc = crtn_mbx_tryget(mbx, (void **)&msg);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EAGAIN);
  ck_assert_ptr_eq(msg, 0);

  msg_size = 0;
  msg = crtn_mbx_format(buffer, sizeof(buffer), &msg_size);
  ck_assert_ptr_ne(msg, 0);
  ck_assert_size_gt(msg_size, 0);

#define MESSAGE "Message"
snprintf(msg, msg_size, "%s", MESSAGE);

  rc = crtn_mbx_post(mbx, msg);
  ck_assert_int_eq(rc, 0);

  msg = (char *)0;
  rc = crtn_mbx_tryget(mbx, (void **)&msg);
  ck_assert_int_eq(rc, 0);
  ck_assert_ptr_ne(msg, 0);

  rc = strcmp(msg, MESSAGE);
  ck_assert_int_eq(rc, 0);

  rc = crtn_mbx_delete(mbx);
  ck_assert_int_eq(rc, 0);

END_TEST


#endif // HAVE_CRTN_MBX




#ifdef HAVE_CRTN_SEM


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_sem_new)

int rc;
int i;
crtn_mbx_t sem_tab[CRTN_SEM_MAX + 1];

  // ------- Overflow of the maximum number of semaphores

  // One coroutine already allocated for Main
  for (i = 0; i < CRTN_SEM_MAX; i ++) {
    rc = crtn_sem_new(&(sem_tab[i]), 1);
    ck_assert_int_eq(rc, 0);
  }

  // Maximum number of semaphores reached
  rc = crtn_sem_new(&(sem_tab[i]), 1);
  ck_assert_int_eq(rc, -1);
  ck_assert_int_eq(crtn_errno(), EAGAIN);

  for (i = 0; i < (CRTN_SEM_MAX - 1); i ++) {
    rc = crtn_sem_delete(sem_tab[i]);
    ck_assert_int_eq(rc, 0);
  }

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_sem_delete)

int rc;
crtn_sem_t sem;

  // -------

  rc = crtn_sem_new(&sem, 1);
  ck_assert_int_eq(rc, 0);
  rc = crtn_sem_delete(sem);
  ck_assert_int_eq(rc, 0);

END_TEST



static int entry_sem_01(void *p)
{
  int rc;
  crtn_sem_t sem = *((crtn_sem_t *)p);

  rc = crtn_sem_p(sem);
  ck_assert_int_eq(rc, 0);

  return 5;
}

static int entry_sem_02(void *p)
{
  int rc;
  crtn_sem_t sem = *((crtn_sem_t *)p);

  rc = crtn_sem_p(sem);
  ck_assert_int_eq(rc, 0);

  rc = crtn_sem_v(sem);
  ck_assert_int_eq(rc, 0);

  return 5;
}



// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_sem_p)

int rc;
crtn_sem_t sem;
crtn_t cid, cid1;
int status;

  // -------
  rc = crtn_sem_new(&sem, 1);
  ck_assert_int_eq(rc, 0);

  rc = crtn_sem_p(sem);
  ck_assert_int_eq(rc, 0);

  rc = crtn_sem_delete(sem);
  ck_assert_int_eq(rc, 0);

  // ------- One coroutine blocked on the semaphore
  rc = crtn_sem_new(&sem, 1);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "sem_01", entry_sem_01, &sem, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_sem_p(sem);
  ck_assert_int_eq(rc, 0);

  // Make the coroutine wait on the semaphore
  crtn_yield(0);

  rc = crtn_sem_v(sem);
  ck_assert_int_eq(rc, 0);

  // Make the coroutine wake up
  crtn_yield(0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

  rc = crtn_sem_delete(sem);
  ck_assert_int_eq(rc, 0);


  // ------- Two coroutines blocked on the semaphore
  rc = crtn_sem_new(&sem, 1);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid, "sem_01", entry_sem_02, &sem, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_spawn(&cid1, "sem_02", entry_sem_02, &sem, 0);
  ck_assert_int_eq(rc, 0);

  rc = crtn_sem_p(sem);
  ck_assert_int_eq(rc, 0);

  // Make the coroutines wait on the semaphore
  crtn_yield(0);

  rc = crtn_sem_v(sem);
  ck_assert_int_eq(rc, 0);

  // Make the coroutines wake up
  crtn_yield(0);

  rc = crtn_join(cid, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

  rc = crtn_join(cid1, &status);
  ck_assert_int_eq(rc, 0);
  ck_assert_int_eq(status, 5);

  rc = crtn_sem_delete(sem);
  ck_assert_int_eq(rc, 0);

END_TEST


// The unitary tests are run in separate processes
// START_TEST is a "static" definition of a function
START_TEST(test_crtn_sem_v)

int rc;
crtn_sem_t sem;

  // -------

  rc = crtn_sem_new(&sem, 1);
  ck_assert_int_eq(rc, 0);

  rc = crtn_sem_v(sem);
  ck_assert_int_eq(rc, 0);

  rc = crtn_sem_delete(sem);
  ck_assert_int_eq(rc, 0);

END_TEST


#endif // HAVE_CRTN_SEM



TCase *crtn_api_tests(void)
{
TCase *tc_api;

  /* Error test case */
  tc_api = tcase_create("CRTN API");

  // Increase the timeout for the test case as we use lots of timeouts
  tcase_set_timeout(tc_api, 60);

  tcase_add_unchecked_fixture(tc_api, crtn_setup_unchecked_fixture, 0);
  tcase_add_checked_fixture(tc_api, crtn_setup_checked_fixture, 0);
  tcase_add_test(tc_api, test_crtn_spawn);
  tcase_add_test(tc_api, test_crtn_exit);
  tcase_add_test(tc_api, test_crtn_wait);
  tcase_add_test(tc_api, test_crtn_cancel);
  tcase_add_test(tc_api, test_crtn_join);

#ifdef HAVE_CRTN_MBX
  tcase_add_test(tc_api, test_crtn_mbx_new);
  tcase_add_test(tc_api, test_crtn_mbx_delete);
  tcase_add_test(tc_api, test_crtn_mbx_post);
  tcase_add_test(tc_api, test_crtn_mbx_get);
  tcase_add_test(tc_api, test_crtn_mbx_tryget);
  tcase_add_test(tc_api, test_crtn_mbx_format);
#endif // HAVE_CRTN_MBX

#ifdef HAVE_CRTN_SEM
  tcase_add_test(tc_api, test_crtn_sem_new);
  tcase_add_test(tc_api, test_crtn_sem_delete);
  tcase_add_test(tc_api, test_crtn_sem_p);
  tcase_add_test(tc_api, test_crtn_sem_v);
#endif // HAVE_CRTN_SEM

  return tc_api;
} // crtn_api_tests
