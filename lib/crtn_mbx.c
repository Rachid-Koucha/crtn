// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : crtn_mbx.c
// Description : Mailbox management
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
#include <sys/types.h>
#include <stdlib.h>

#include "crtn.h"
#include "crtn_list.h"
#include "crtn_ccb.h"


static struct
{
  int         busy;
  size_t      nb_msgs;
  crtn_link_t msgs;
  crtn_link_t crtns;
} crtn_mbx[CRTN_MBX_MAX];


/*
  Number of active mailboxes
*/
static int crtn_mbx_nb;
static int crtn_next_free_mbxid;


static crtn_mbx_t crtn_get_mbxid(void)
{
  int i;
  int count = CRTN_MBX_MAX;

  for (i = crtn_next_free_mbxid; count; count --, i = (i + 1) % CRTN_MBX_MAX) {
    if (!(crtn_mbx[i].busy)) {
      // Mark the context busy
      crtn_mbx[i].busy = 1;
      CRTN_LIST_INIT(&(crtn_mbx[i].msgs));
      CRTN_LIST_INIT(&(crtn_mbx[i].crtns));
      crtn_mbx[i].nb_msgs = 0;
      crtn_next_free_mbxid = (i + 1) % CRTN_MBX_MAX;
      crtn_mbx_nb ++;
      return i;
    }
  }

  crtn_set_errno(EAGAIN);
  return -1;

} // crtn_get_mbxid


static void crtn_free_mbxid(crtn_mbx_t mbx)
{

  crtn_mbx[mbx].busy = 0;
  crtn_next_free_mbxid = mbx;
  crtn_mbx_nb --;

} // crtn_free_mbxid


int crtn_mbx_new(crtn_mbx_t *mbx)
{
  crtn_mbx_t id;

  if (!mbx) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  *mbx = -1;

  id = crtn_get_mbxid();
  if (id >= 0) {
    *mbx = id;
    return 0;
  }

  return -1;
} // crtn_mbx_new


int crtn_mbx_delete(crtn_mbx_t mbx)
{
  if (mbx < 0 || mbx >= CRTN_MBX_MAX) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  crtn_free_mbxid(mbx);

  return 0;
} // crtn_mbx_delete


int crtn_mbx_post(
                  crtn_mbx_t  mbx,
                  void       *msg
                 )
{
  crtn_link_t *link;

  if (mbx < 0 || mbx >= CRTN_MBX_MAX) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  if (!msg) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  link = ((crtn_link_t *)msg) - 1;

  CRTN_LIST_ADD_TAIL(&(crtn_mbx[mbx].msgs), link);
  crtn_mbx[mbx].nb_msgs ++;

  // Wake up the coroutines waiting on the mailbox if any
  link = CRTN_LIST_FRONT(&(crtn_mbx[mbx].crtns));
  if (link) {

    do {
      CRTN_LIST_DEL(link);
      crtn_make_runnable(link);
      link = CRTN_LIST_FRONT(&(crtn_mbx[mbx].crtns));
    } while (link);
  }

  return 0;
} // crtn_mbx_post


int crtn_mbx_get(
                  crtn_mbx_t   mbx,
                  void       **msg
                )
{
  crtn_link_t *link;

  if (mbx < 0 || mbx >= CRTN_MBX_MAX) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  if (!msg) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  do {

    if (!(crtn_mbx[mbx].nb_msgs)) {
      crtn_make_waiting(&(crtn_mbx[mbx].crtns), &(crtn_current->link));
      crtn_yield(0);
    }

    link = CRTN_LIST_FRONT(&(crtn_mbx[mbx].msgs));

    // If multiple coroutines are were waiting on the mailbox,
    // perhaps the available message has already been removed
    if (link) {
      CRTN_LIST_DEL(link);
      crtn_mbx[mbx].nb_msgs --;
      *msg = (void *)(link + 1);
    }

  } while(!link);

  return 0;
} // crtn_mbx_get


int crtn_mbx_tryget(
                    crtn_mbx_t   mbx,
                    void       **msg
                   )
{
  crtn_link_t *link;

  if (mbx < 0 || mbx >= CRTN_MBX_MAX) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  if (!msg) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  if (!(crtn_mbx[mbx].nb_msgs)) {
    *msg = (void *)0;
    crtn_set_errno(EAGAIN);
    return -1;
  }

  link = CRTN_LIST_FRONT(&(crtn_mbx[mbx].msgs));
  CRTN_LIST_DEL(link);
  crtn_mbx[mbx].nb_msgs --;
  *msg = (void *)(link + 1);

  return 0;
} // crtn_mbx_tryget


#define CRTN_MSG_HDR_SZ                                    \
  ((sizeof(crtn_link_t) + __alignof__(crtn_link_t) - 1) &  \
   ~(__alignof__(crtn_link_t) - 1))

void *crtn_mbx_alloc(
                     size_t size
                    )
{
  crtn_link_t *p;

  p = (void *)malloc(CRTN_MSG_HDR_SZ + size);

  return (void *)(p + 1);

} // crtn_mbx_alloc



int crtn_mbx_free(
                   void *msg
                  )
{
  crtn_link_t *p;

  if (!msg) {
    crtn_set_errno(EINVAL);
    return -1;
  }

  p = ((crtn_link_t *)msg) - 1;

  free(p);

  return 0;

} // crtn_mbx_free


void *crtn_mbx_format(
                      char   *buffer,
                      size_t  buffer_size,
                      size_t *data_size
                     )
{
  crtn_link_t *p;

  if (!buffer || !data_size) {
    crtn_set_errno(EINVAL);
    return (void *)0;
  }

  if (buffer_size < CRTN_MSG_HDR_SZ) {
    crtn_set_errno(EINVAL);
    return (void *)0;
  }

  // Check the alignment of the buffer to make sure
  // that the internal header can be placed at the
  // beginning of the space
  if ((size_t)buffer & (__alignof__(crtn_link_t) - 1)) {
    crtn_set_errno(EINVAL);
    return (void *)0;
  }

  p = ((crtn_link_t *)buffer) + 1;
  *data_size = buffer_size - CRTN_MSG_HDR_SZ;

  return p;

} // crtn_mbx_format

