// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : crtn_list.h
// Description : List management
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


#ifndef CRTN_LIST_H
#define CRTN_LIST_H


typedef struct crtn_link {

  struct crtn_link *next;
  struct crtn_link *prev;

} crtn_link_t;


#define CRTN_LINK_INIT(e) (e)->next = (e)->prev = (e)
#define CRTN_LIST_INIT(l) CRTN_LINK_INIT(l)


#define CRTN_LIST_ADD_FRONT(l, e)         \
                 do {                     \
                   (l)->next->prev = (e); \
                   (e)->next = (l)->next; \
                   (e)->prev = (l);       \
                   (l)->next = (e);       \
                 } while(0)


#define CRTN_LIST_ADD_TAIL(l, e)          \
                 do {                     \
                   (l)->prev->next = (e); \
                   (e)->next = (l);       \
                   (e)->prev = (l)->prev; \
                   (l)->prev = (e);       \
                  } while(0)


#define CRTN_LIST_INSERT_BEFORE(b, e)     \
                 do {                     \
                   (b)->next = (e);       \
                   (b)->prev = (e)->prev; \
                   (e)->prev->next = (b); \
                   (e)->prev = (b);       \
                 } while(0)


#define CRTN_IS_LINKED(e) ((e)->next != (e))


#define CRTN_LIST_DEL(e)                          \
                 do {                             \
                   if (CRTN_IS_LINKED(e)) {       \
                     (e)->next->prev = (e)->prev; \
                     (e)->prev->next = (e)->next; \
                     CRTN_LINK_INIT(e);           \
                   }                              \
                 } while(0)


#define CRTN_LIST_EMPTY(l) ((l)->next == (l))


#define CRTN_LIST_FRONT(l) (!CRTN_LIST_EMPTY(l) ? (l)->next : NULL)


#define CRTN_LIST_TAIL(l) (!CRTN_LIST_EMPTY(l) ? (l)->tail : NULL)


#endif // CRTN_LIST_H
