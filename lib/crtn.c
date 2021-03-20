// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : crtn.c
// Description : Coroutines
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
//     08-Feb-2021 R. Koucha      - Creation
//     20-Mar-2021 R. Koucha      - Fixed FIFO scheduling
//                                - Return value for crtn_yield()
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "../config.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "crtn.h"
#include "crtn_ccb.h"
#include "crtn_list.h"



/*
  Default attributes
*/
static crtn_ccb_attr_t crtn_default_attr = {

  CRTN_TYPE_STANDALONE | CRTN_TYPE_STACKFUL,

  CRTN_DEFAULT_STACK_SIZE

};

/*
  Runnable list
*/
static crtn_link_t crtn_runnable_list;


/*
  Table of CCB
*/
static crtn_ccb_t *crtn_tab[CRTN_MAX];

/*
  Number of active CCB
*/
static int crtn_nb;
static int crtn_next_free_id;



/*
  Running CCB
*/
crtn_ccb_t *crtn_current;


static crtn_ccb_t crtn_ccb_main;


#define CRTN_EXIST(id) (((id) >= 0)       && \
                        ((id) < CRTN_MAX) && \
                        crtn_tab[(id)])



/*
  Stack for the stackless coroutines
*/
static char *crtn_stackless;

/*
  Size of the termination stack for the stackless coroutines
*/
#define CRTN_CANCEL_STACK_SIZE (4 * 1024)


int crtn_errno(void)
{
  return crtn_current->err_num;
}


static crtn_t crtn_get_id(crtn_ccb_t *ccb)
{
  int i;
  int count = CRTN_MAX;

  for (i = crtn_next_free_id; count; count --, i = (i + 1) % CRTN_MAX) {
    if (!(crtn_tab[i])) {
      // Mark the context busy
      crtn_tab[i] = ccb;
      crtn_next_free_id = (i + 1) % CRTN_MAX;
      crtn_nb ++;
      return i;
    }
  }

  // Impossible as controls are done before calling this function
  // to make sure that there is space
  assert(0);
  crtn_set_errno(EAGAIN);
  return -1;

} // crtn_get_id


static void crtn_free_id(crtn_t cid)
{

  crtn_tab[cid] = (crtn_ccb_t *)0;
  crtn_next_free_id = cid;
  crtn_nb --;

} // crtn_free_id


static void crtn_free(crtn_ccb_t *ccb)
{
  crtn_free_id(ccb->cid);

  if (!(ccb->attr.type & CRTN_TYPE_STACKLESS)) {

    if (!(ccb->flags & CRTN_CCB_FLAG_STATIC)) {

      // The CCB is in the stack
      free(ccb->stack);
    }

  } else {

    // The CCB is in the cancel stack area
    free(ccb->cancel_stack);

  }
} // crtn_free


crtn_t crtn_self(void)
{
  return crtn_current->cid;
} // crtn_self


void crtn_make_runnable(crtn_link_t *link)
{
  CRTN_LINK2CCB(link)->state = CRTN_STATE_RUNNABLE;
  CRTN_LIST_ADD_TAIL(&crtn_runnable_list, link);
}


void crtn_make_waiting(
                    crtn_link_t *list,
                    crtn_link_t *link
                   )
{
  CRTN_LIST_DEL(link);
  CRTN_LINK2CCB(link)->state = CRTN_STATE_WAITING;
  if (list) {
    CRTN_LIST_ADD_FRONT(list, link);
  }
}


int crtn_yield(void *data)
{
  int rc;
  crtn_link_t *plink;
  crtn_ccb_t *next_ccb;
  crtn_ccb_t *old_ccb;

  switch(crtn_current->state) {

    case CRTN_STATE_RUNNING: {

      // The current coroutine will stay runnable only if it is not a stepper
      if (crtn_current->attr.type & CRTN_TYPE_STEPPER) {

        // If some coroutine is waiting on the current coroutine
        if (crtn_current->waiting) {

          // Pass the data
          crtn_current->yielded_data = data;

          // Put the waiting coroutine in the runnable list
          crtn_make_runnable(&(crtn_current->waiting->link));
        }

        // Put current coroutine in the READY state
        CRTN_LIST_DEL(&(crtn_current->link));
        crtn_current->state = CRTN_STATE_READY;

        // Get the link of the schedulable coroutine
        plink = CRTN_LIST_FRONT(&crtn_runnable_list);

        // If there no schedulable coroutines, this is a dead end!
        assert(plink);

        // Context switch
        next_ccb = CRTN_LINK2CCB(plink);
        assert(next_ccb->state == CRTN_STATE_RUNNABLE);
        old_ccb = crtn_current;
        crtn_current = next_ccb;
        crtn_current->state = CRTN_STATE_RUNNING;
        swapcontext(&(old_ccb->ctx), &(next_ccb->ctx));

        rc = CRTN_SCHED_OTHER;

      } else {

        // The coroutine is STANDALONE, it decided to give the processor

        // The current coroutine stays RUNNABLE

        // Put the current CCB at the end of the runnable list if it is not
        // already the last. Brand new coroutines from calls to crtn_spawn()
        // may be located before the running coroutine in the runnable list.
        if (crtn_runnable_list.prev != &(crtn_current->link))  {
          CRTN_LIST_DEL(&(crtn_current->link));
          CRTN_LIST_ADD_TAIL(&crtn_runnable_list, &(crtn_current->link));
        }

        // Get the link of the schedulable coroutine (1st of the list)
        plink = CRTN_LIST_FRONT(&crtn_runnable_list);

        // The list can't be empty (otherwise it is an internal bug!)
        assert(plink);

        // Context switch if it is not the current one
        if (plink != &(crtn_current->link)) {
          crtn_current->state = CRTN_STATE_RUNNABLE;
          next_ccb = CRTN_LINK2CCB(plink);
          assert(next_ccb->state == CRTN_STATE_RUNNABLE);
          old_ccb = crtn_current;
          crtn_current = next_ccb;
          crtn_current->state = CRTN_STATE_RUNNING;
          swapcontext(&(old_ccb->ctx), &(next_ccb->ctx));

          rc = CRTN_SCHED_OTHER;
        } else {
          rc = CRTN_SCHED_SELF;
        }
      }
    }
    break;

    case CRTN_STATE_ZOMBIE: {

      // The current coroutine reached the end of its entry point
      // or it called crtn_exit()

      // If a coroutine is joining or waiting
      if (crtn_current->joining) {

        // Put the joining coroutine in the ready list
        crtn_make_runnable(&(crtn_current->joining->link));

      } else if (crtn_current->waiting) {

        // If a coroutine is waiting
        assert(crtn_current->attr.type & CRTN_TYPE_STEPPER);

        // No data
        crtn_current->yielded_data = 0;

        // Put the joining coroutine in the ready list
        crtn_make_runnable(&(crtn_current->waiting->link));

      }

      // Get the link of the schedulable coroutine
      plink = CRTN_LIST_FRONT(&crtn_runnable_list);

      // If there are no schedulable coroutines, this is a dead end!
      assert(plink);

      // Context switch
      next_ccb = CRTN_LINK2CCB(plink);
      old_ccb = crtn_current;
      crtn_current = next_ccb;
      crtn_current->state = CRTN_STATE_RUNNING;
      swapcontext(&(old_ccb->ctx), &(next_ccb->ctx));

      rc = CRTN_SCHED_OTHER;
    }
    break;

    case CRTN_STATE_WAITING: {

      // The current coroutine called either crtn_wait() or crtn_join()

      // Get the link of the schedulable coroutine
      plink = CRTN_LIST_FRONT(&crtn_runnable_list);

      // If there no schedulable coroutines, this is a dead end!
      assert(plink);

      // Context switch
      next_ccb = CRTN_LINK2CCB(plink);
      old_ccb = crtn_current;
      crtn_current = next_ccb;
      crtn_current->state = CRTN_STATE_RUNNING;
      swapcontext(&(old_ccb->ctx), &(next_ccb->ctx));

      rc = CRTN_SCHED_OTHER;
    }
    break;

    default: {

      assert(0);

      crtn_set_errno(ENOSYS);
      rc = -1;

    }
    break;

  } // End switch

  return rc;

} // crtn_yield


static void crtn_end(int status)
{
  crtn_ccb_t *ccb = crtn_current;

  ccb->status = status;

  // Change the state
  ccb->state = CRTN_STATE_ZOMBIE;

  // Get out from the runnable list
  CRTN_LIST_DEL(&(ccb->link));

  // Give back the processor
  crtn_yield(0);

  // We should never come back here as the state
  // of the coroutine being ZOMBIE, it must not
  // be scheduled again
  assert(0);
  
} // crtn_end


static void crtn_entry(void)
{
  crtn_ccb_t *ccb = crtn_current;
  int status;

  // Call the entry point
  status = ccb->entry(ccb->param);

  // Handle termination (cancellation, exit)
  crtn_end(status);

} // crtn_entry


static void crtn_fill_ccb(
                          crtn_ccb_t   *ccb,
                          const char   *name,
                          char         *stack,
                          size_t        stack_sz,
                          crtn_entry_t  entry,
                          void         *param,
                          crtn_ccb_attr_t *iattr
                         )
{
  ccb->state = CRTN_STATE_ALLOCATED;
  ccb->entry = entry;
  ccb->param = param;
  ccb->stack = stack;
  ccb->joining = 0;
  snprintf(ccb->name, CRTN_NAME_SZ, "%s", name);
  ccb->flags = 0;
  if (iattr) {
    ccb->attr = *iattr;
  } else {
    ccb->attr = crtn_default_attr;
  }
  ccb->waiting = 0;
  ccb->yielded_data = 0;
  CRTN_LINK_INIT(&(ccb->link));

  // Initialize the stack in the context
  ccb->ctx.uc_stack.ss_sp = stack;
  ccb->ctx.uc_stack.ss_size = stack_sz;
  ccb->ctx.uc_stack.ss_flags = 0;
  ccb->ctx.uc_link = 0;

} // crtn_fill_ccb


int crtn_spawn(
               crtn_t       *cid,
               const char   *name,
               crtn_entry_t  entry,
               void         *param,
               crtn_attr_t   attr
              )
{
  char            *stack;
  size_t           stack_sz;
  crtn_ccb_t      *ccb;
  char            *p;
  crtn_ccb_attr_t *iattr;

  // Check the parameters
  if (!cid || !entry || !name) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  *cid = -1;

  if (crtn_nb >= CRTN_MAX) {
    crtn_set_errno(EAGAIN);
    return -1;
  }

  if (attr) {
    iattr = (crtn_ccb_attr_t *)attr;
  } else {
    iattr = &crtn_default_attr;
  }

  if (iattr->type & CRTN_TYPE_STACKLESS) {

    // This is a stackless coroutine

    // The CCB is allocated globally but to be able to set
    // a termination context in crtn_cancel(), a termination
    // stack must be set up otherwise, the cancel stack frame
    // can be clobbered by other running stackless coroutines

    // Allocate the CCB
    stack = (char *)malloc(CRTN_CANCEL_STACK_SIZE);
    if (!stack) {
      crtn_set_errno(errno);
      return -1;
    }

    // The CCB is aligned at the bottom of the stack
    p = stack + CRTN_CANCEL_STACK_SIZE - sizeof(crtn_ccb_t); 
    p = (char *)((unsigned long)p & ~(__alignof__(crtn_ccb_t) - 1));
    ccb = (crtn_ccb_t *)p;
    ccb->cancel_stack = stack;
    ccb->cancel_stack_size = (size_t)(p - stack);

    // At runtime, the stackless coroutines actually all use the same
    // stack to avoid the pollution of the caller's stack

    // If the stackless stack is not yet allocated, allocate it
    if (!crtn_stackless) {
      crtn_stackless = (char *)malloc(CRTN_DEFAULT_STACK_SIZE);
      if (!crtn_stackless) {
        crtn_set_errno(errno);
        free(ccb);
        return -1;
      }
    }

    stack = crtn_stackless;
    stack_sz = CRTN_DEFAULT_STACK_SIZE;

  } else {

    // The CCB will be at the bottom of the stack

    // Dynamic allocation of the stack
    stack_sz = iattr->stack_size;
    stack = (char *)malloc(stack_sz);
    if (!stack) {
      crtn_set_errno(errno);
      return -1;
    }

    // The CCB is aligned at the bottom of the stack
    p = stack + stack_sz - sizeof(crtn_ccb_t); 
    p = (char *)((unsigned long)p & ~(__alignof__(crtn_ccb_t) - 1));
    ccb = (crtn_ccb_t *)p;
    stack_sz = (size_t)(p - stack);
  }

  ccb->cid = crtn_get_id(ccb);
  assert(ccb->cid >= 0);
  crtn_fill_ccb(ccb, name, stack, stack_sz, entry, param, iattr);

  *cid = ccb->cid;

  if (ccb->attr.type & CRTN_TYPE_STEPPER) {
    ccb->state = CRTN_STATE_READY;
  } else {
    crtn_make_runnable(&(ccb->link));
  }

#if 1

  getcontext(&(ccb->ctx));

#else

  // We remove this check as the compiler complains with:
  //  "variable 'ccb' might be clobbered by 'longjmp' or 'vfork' [-Werror=clobbered]"

  // On x86_64 machines, getcontext() may fail only if the rt_sigprocmask() fails
  {
  int rc = getcontext(&(ccb->ctx));

    if (0 != rc) {
      crtn_set_errno(errno);
      crtn_free_id(ccb->cid);
      free(stack);
      return -1;
    }
  }

#endif // 0

  makecontext(&(ccb->ctx), (mkctx_func_t)crtn_entry, 0);

  return 0;
} // crtn_spawn


crtn_attr_t crtn_attr_new(void)
{
  crtn_ccb_attr_t *iattr;

  iattr = (crtn_ccb_attr_t *)malloc(sizeof(crtn_ccb_attr_t));
  *iattr = crtn_default_attr;
  return (crtn_attr_t)iattr;

} // crtn_attr_new


int crtn_attr_delete(
                     crtn_attr_t attr
                    )
{
  if (!attr) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  free(attr);

  return 0;

} // crtn_attr_delete


int crtn_set_attr_type(
                       crtn_attr_t attr,
                       unsigned int type
                      )
{
crtn_ccb_attr_t *iattr;

#define CRTN_TYPE_MASK 0x3

  if (!attr ||
      (type & ~CRTN_TYPE_MASK)) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  iattr = (crtn_ccb_attr_t *)attr;
  if (type & CRTN_TYPE_STACKLESS) {
    iattr->type |= CRTN_TYPE_STACKLESS;
    iattr->stack_size = 0;
  } else {
    if (!(iattr->stack_size)) {
      iattr->stack_size = CRTN_DEFAULT_STACK_SIZE;
    }
  }

  if (type & CRTN_TYPE_STEPPER) {
    iattr->type |= CRTN_TYPE_STEPPER;
  }

  return 0;
} // crtn_set_attr_type


int crtn_set_attr_stack_size(
                       crtn_attr_t attr,
                       size_t stack_size
                      )
{
crtn_ccb_attr_t *iattr;

  if (!attr ||
      (stack_size < CRTN_DEFAULT_STACK_SIZE)) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  iattr = (crtn_ccb_attr_t *)attr;
  if (iattr->type & CRTN_TYPE_STACKLESS) {
    crtn_set_errno(EINVAL);
    return -1;
  }
  iattr->stack_size = stack_size;

  return 0;
} // crtn_set_attr_stack_size


void crtn_exit(int status)
{

  // Handle termination (cancellation, exit)
  crtn_end(status);  

} // crtn_exit


int crtn_join(crtn_t cid, int *status)
{
crtn_ccb_t *ccb;

  if (!CRTN_EXIST(cid)) {
    crtn_set_errno(ENOENT);
    return -1;
  }

  if (cid == crtn_current->cid) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  ccb = crtn_tab[cid];

  if (ccb->joining) {
    crtn_set_errno(EBUSY);
    return -1;
  }

  switch(ccb->state) {

    case CRTN_STATE_ZOMBIE: {
      ccb->joining = crtn_current;
      crtn_current->joining_on = ccb;
    }
    break;

    case CRTN_STATE_RUNNABLE:
    case CRTN_STATE_WAITING:
    case CRTN_STATE_READY: {

      crtn_make_waiting(0, &(crtn_current->link));

      ccb->joining = crtn_current;
      crtn_current->joining_on = ccb;

      // Give back the processor
      crtn_yield(0);
    }
    break;

    default: {

      assert(0);
    }
    break;

  } // End switch

  // For stackless coroutines, the local variables are clobbered here
  // ==> Reload 'ccb'
  ccb = crtn_current->joining_on;

  if (status) {
    *status = ccb->status;
  }

  crtn_current->joining_on = 0;
  ccb->joining = 0;

  // Free the coroutine
  crtn_free(ccb);

  return 0;
} // crtn_join


int crtn_wait(crtn_t cid, void **ret)
{
  crtn_ccb_t *ccb;

  if (!CRTN_EXIST(cid)) {
    crtn_set_errno(ENOENT);
    return -1;
  }

  if (cid == crtn_current->cid) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  ccb = crtn_tab[cid];

  if (!(ccb->attr.type & CRTN_TYPE_STEPPER)) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  if (ccb->state != CRTN_STATE_READY) {
    crtn_set_errno(EPERM);
    return -1;
  }

  // As it is ready, no join/wait has been done on it
  assert(!(ccb->joining || ccb->waiting));

  crtn_make_runnable(&(ccb->link));

  crtn_make_waiting(0, &(crtn_current->link));

  ccb->waiting = crtn_current;
  crtn_current->waiting_on = ccb;

  // Give back the processor
  crtn_yield(0);

  // For stackless coroutines, the local variables are clobbered here
  // ==> Reload 'ccb'
  ccb = crtn_current->waiting_on;

  ccb->waiting = 0;
  crtn_current->waiting_on = 0;

  if (ret) {
    *ret = ccb->yielded_data;
  }

  // If the call to crtn_wait() trigger the termination of the target
  // coroutine or if it has been cancelled
  if (ccb->state != CRTN_STATE_READY) {

    assert(ccb->state == CRTN_STATE_ZOMBIE);
    return CRTN_DEAD;

  }

  return 0;
} // crtn_wait


int crtn_cancel(crtn_t cid)
{
crtn_ccb_t *ccb, *ccb1;

  if (!CRTN_EXIST(cid)) {
    crtn_set_errno(ENOENT);
    return -1;
  }

  if (cid == crtn_current->cid) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  if (cid == CRTN_CID_MAIN) {
    crtn_set_errno(EPERM);
    return -1;
  }

  ccb = crtn_tab[cid];

  if (ccb->flags & CRTN_CCB_FLAG_CANCELLED) {
    crtn_set_errno(EBUSY);
    return -1;
  }

  switch(ccb->state) {

    case CRTN_STATE_ZOMBIE: {
      crtn_set_errno(EINVAL);
      return -1;
    }
    break;

    case CRTN_STATE_WAITING: {

      // The target coroutine is waiting on some resource
      // ==> Disable the wait
      if (ccb->waiting_on) {
        ccb1 = ccb->waiting_on;
        ccb1->waiting = 0;
        ccb->waiting_on = 0;
      } else if (ccb->joining_on) {
        ccb1 = ccb->joining_on;
        ccb1->joining = 0;
        ccb->joining_on = 0;
      }

      // Unlink the coroutine from the list of the object
      // it is waiting on (e.g. semaphore, mailbox)
      // If it is not linked, the macro does not change the links
      CRTN_LIST_DEL(&(ccb->link));
    }
    __CRTN_FALLTHROUGH

    case CRTN_STATE_READY: {

      // Put the target coroutine at the beginning of
      // the runnable queue and change its state to runnable
      crtn_make_runnable(&(ccb->link));

    }
    break;

    default: {

      // The target coroutine is in runnable state
      assert(ccb->state == CRTN_STATE_RUNNABLE);

    }
    break;

  } // End switch

  ccb->flags |= CRTN_CCB_FLAG_CANCELLED;

  // Change the context of the target coroutine to make it call the
  // termination routine
  if (ccb->attr.type & CRTN_TYPE_STACKLESS) {
    // For stackless coroutines, we use the dedicated cancel stack otherwise
    // the stack frame could be clobbered
    ccb->ctx.uc_stack.ss_sp = ccb->cancel_stack;
    ccb->ctx.uc_stack.ss_size = ccb->cancel_stack_size;
    ccb->ctx.uc_stack.ss_flags = 0;

    makecontext(&(ccb->ctx), (mkctx_func_t)crtn_end, 1, CRTN_STATUS_CANCELLED);
  } else {
    makecontext(&(ccb->ctx), (mkctx_func_t)crtn_end, 1, CRTN_STATUS_CANCELLED);
  }

  return 0;
} // crtn_cancel


void __attribute__ ((constructor)) crtn_lib_init(void);

void crtn_lib_init(void)
{
  crtn_ccb_t *ccb;

  // Initialize the list
  CRTN_LIST_INIT(&crtn_runnable_list);

  // Make the CCB of the main thread, no entry point
  ccb = crtn_current = &crtn_ccb_main;
  ccb->cid = crtn_get_id(ccb);
  assert(ccb->cid == CRTN_CID_MAIN);
  crtn_fill_ccb(ccb, "Main", 0, 0, 0, 0, 0);

  // Put the MAIN coroutine in the runnable list
  crtn_make_runnable(&(ccb->link));

  // Main is the running coroutine
  ccb->state = CRTN_STATE_RUNNING;
} // crtn_lib_init



void __attribute__ ((destructor)) crtn_lib_exit(void);

void crtn_lib_exit(void)
{
  // Stop all the coroutines

  // Free the CCBs

  // Free the stack of the stackless coroutines
  if (crtn_stackless) {
    free(crtn_stackless);
    crtn_stackless = 0;
  }

} // crtn_lib_exit
