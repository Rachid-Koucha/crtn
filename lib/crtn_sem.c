// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : crtn_sem.c
// Description : Semaphore management
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
//     11-Feb-2021 R. Koucha      - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "../config.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#include "crtn.h"
#include "crtn_list.h"
#include "crtn_ccb.h"



static size_t crtn_sem_max;

static struct crtn_sem_t
{
  int          busy;
  unsigned int counter;
  crtn_link_t  crtns;
} *crtn_sem;


/*
  Number of active semaphores
*/
static int crtn_sem_nb;
static int crtn_next_free_semid;


static crtn_sem_t crtn_get_semid(void)
{
  int i;
  int count = crtn_sem_max;

  for (i = crtn_next_free_semid; count; count --, i = (i + 1) % crtn_sem_max) {
    if (!(crtn_sem[i].busy)) {
      // Mark the context busy
      crtn_sem[i].busy = 1;
      CRTN_LIST_INIT(&(crtn_sem[i].crtns));
      crtn_sem[i].counter = 0;
      crtn_next_free_semid = (i + 1) % crtn_sem_max;
      crtn_sem_nb ++;
      return i;
    }
  }

  crtn_set_errno(EAGAIN);
  return -1;

} // crtn_get_semid


static void crtn_free_semid(crtn_sem_t sem)
{

  crtn_sem[sem].busy = 0;
  crtn_next_free_semid = sem;
  crtn_sem_nb --;

} // crtn_free_mbxid


int crtn_sem_new(
                 crtn_sem_t *sem,
                 unsigned int initval
                )
{
  crtn_sem_t id;

  if (!sem) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  *sem = -1;

  id = crtn_get_semid();
  if (id >= 0) {
    crtn_sem[id].counter = initval;
    *sem = id;
    return 0;
  }

  return -1;
} // crtn_sem_new


int crtn_sem_delete(crtn_sem_t sem)
{
  if (sem < 0 || (size_t)sem >= crtn_sem_max) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  crtn_free_semid(sem);

  return 0;
} // crtn_sem_delete


int crtn_sem_v(
                crtn_sem_t sem
               )
{
  crtn_link_t *link;

  if (sem < 0 || (size_t)sem >= crtn_sem_max) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  if (crtn_sem[sem].counter ++ == 0) {

    // Wake up the coroutines waiting
    // on the semaphore if any
    link = CRTN_LIST_FRONT(&(crtn_sem[sem].crtns));
    if (link) {
      do {
        CRTN_LIST_DEL(link);
        crtn_make_runnable(link);
        link = CRTN_LIST_FRONT(&(crtn_sem[sem].crtns));
      } while (link);
    }
  }

  return 0;
} // crtn_sem_v


int crtn_sem_p(
               crtn_sem_t sem
              )
{
  if (sem < 0 || (size_t)sem >= crtn_sem_max) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  while (crtn_sem[sem].counter == 0) {
    crtn_make_waiting(&(crtn_sem[sem].crtns), &(crtn_current->link));
    crtn_yield(0);
  }

  crtn_sem[sem].counter --;

  return 0;
} // crtn_sem_p


void crtn_lib_sem_init(void)
{
  crtn_get_size_env("CRTN_SEM_MAX", &crtn_sem_max, CRTN_SEM_MAX);
  crtn_sem = (struct crtn_sem_t *)malloc(crtn_sem_max * sizeof(struct crtn_sem_t));
  if (!crtn_sem) {
    fprintf(stderr, "malloc(%zu): %m (%d)\n", crtn_sem_max * sizeof(crtn_sem_t), errno);
    return;
  }
} // crtn_lib_sem_init


void crtn_lib_sem_exit(void)
{
  free(crtn_sem);
} // crtn_lib_sem_exit
