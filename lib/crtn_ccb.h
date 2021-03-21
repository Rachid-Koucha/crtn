// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : crtn_ccb.h
// Description : Coroutines Control Block
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
//
// Evolutions  :
//
//     08-Feb-2021 R. Koucha      - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef CRTN_CCB_H
#define CRTN_CCB_H

#include <stddef.h>
#include <ucontext.h>

#include "crtn.h"
#include "crtn_list.h"


/*
  Entry point for makecontext()
*/
typedef void (* mkctx_func_t)(void);



/*
  Coroutine's attribute
*/
typedef struct
{
  unsigned int type;

  size_t stack_size;

} crtn_ccb_attr_t;


/*
           CCB

  Coroutine Control Block

*/
typedef struct crtn_ccb
{
  int cid;

  int state;
#define CRTN_STATE_ALLOCATED  0
#define CRTN_STATE_READY      1
#define CRTN_STATE_RUNNABLE   2
#define CRTN_STATE_RUNNING    3
#define CRTN_STATE_WAITING    4
#define CRTN_STATE_ZOMBIE     5

  crtn_ccb_attr_t  attr;

  // Entry point (function, parameter and return code)
  crtn_entry_t  entry;
  void         *param;
  int           status;

  char *stack;
  char *cancel_stack; // For stackless coroutines
  size_t cancel_stack_size;

  // Saved registers and signal mask
  ucontext_t ctx;

  // Error on last library/system call
  int err_num;

  // Link into the current list
  crtn_link_t link;

  // Joining corouting
  struct crtn_ccb *joining;
  struct crtn_ccb *joining_on;

  // Waiting corouting
  struct crtn_ccb *waiting;
  struct crtn_ccb *waiting_on;

  void *yielded_data;

  char name[CRTN_NAME_SZ];

  int flags;
#define CRTN_CCB_FLAG_STATIC     0x1
#define CRTN_CCB_FLAG_CANCELLED  0x2
} crtn_ccb_t;



#define CRTN_LINK2CCB(l) ((crtn_ccb_t *)((char *)(l) - offsetof(crtn_ccb_t, link)))


extern crtn_ccb_t *crtn_current;

#define crtn_set_errno(e) crtn_current->err_num = e

extern void crtn_make_runnable(crtn_link_t *link);

extern void crtn_make_waiting(
                           crtn_link_t *list,
                           crtn_link_t *link
                          );

extern void crtn_get_size_env(
                       const char *name,
                       size_t *value,
                       size_t default_value
                       );

#endif // CRTN_CCB_H
