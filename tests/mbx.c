// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : mbx.c
// Description : Program demonstrating the mailboxes
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

#include <stdio.h>
#include "crtn.h"

#define NB_MSG 5

#define MSG_SZ 100

static int sender(void *p)
{
  void *msg;
  int rc;
  crtn_mbx_t mbx = *((crtn_mbx_t *)p);
  int i;

  for (i = 0; i < NB_MSG; i ++) {

    msg = crtn_mbx_alloc(MSG_SZ);
    if (!msg) {
      fprintf(stderr, "Error %d\n", crtn_errno());
      return -1;
    }

    snprintf((char *)msg, MSG_SZ, "Message#%d", i);

    rc = crtn_mbx_post(mbx, msg);
    if (rc != 0) {
      fprintf(stderr, "Error %d\n", crtn_errno());
      return -1;
    }

    crtn_yield(0);
  }

  return i;
} // sender


static int receiver(void *p)
{
  void *msg;
  int rc;
  crtn_mbx_t mbx = *((crtn_mbx_t *)p);
  int i;

  for (i = 0; i < NB_MSG; i ++) {

    rc = crtn_mbx_get(mbx, &msg);
    if (rc != 0) {
      fprintf(stderr, "Error %d\n", crtn_errno());
      return -1;
    }

    printf("Received: '%s'\n", (char *)msg);

    rc = crtn_mbx_free(msg);
    if (rc != 0) {
      fprintf(stderr, "Error %d\n", crtn_errno());
      return -1;
    }

    crtn_yield(0);
  }

  return i;
} // receiver


int main(void)
{
  int rc;
  crtn_mbx_t mbx;
  crtn_t cid, cid1;
  int status, status1;

  rc = crtn_mbx_new(&mbx);
  if (rc != 0) {
    fprintf(stderr, "Error %d\n", crtn_errno());
    return 1;
  }

  rc = crtn_spawn(&cid, "Sender", sender, &mbx, 0);
  if (rc != 0) {
    fprintf(stderr, "Error %d\n", crtn_errno());
    return 1;
  }

  rc = crtn_spawn(&cid1, "Receiver", receiver, &mbx, 0);
  if (rc != 0) {
    fprintf(stderr, "Error %d\n", crtn_errno());
    return 1;
  }

  rc = crtn_join(cid, &status);
  if (rc != 0) {
    fprintf(stderr, "Error %d\n", crtn_errno());
    return 1;
  }

  rc = crtn_join(cid1, &status1);
  if (rc != 0) {
    fprintf(stderr, "Error %d\n", crtn_errno());
    return 1;
  }

  rc = crtn_mbx_delete(mbx);
  if (rc != 0) {
    fprintf(stderr, "Error %d\n", crtn_errno());
    return 1;
  }

  return 0;
} // main
