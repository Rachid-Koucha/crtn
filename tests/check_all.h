// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : check_all.h
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


#ifndef CHECK_ALL_H
#define CHECK_ALL_H

#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <check.h>
#include <sys/wait.h>


// ----------------------------------------------------------------------------
// Name   : ck_assert_signaled
// Usage  : checks the termination status is a signal reception
// Return : None
// ----------------------------------------------------------------------------
#define ck_assert_signaled(status, sig) \
                   do { ck_assert_int_ne(WIFSIGNALED(status), 0); \
                        ck_assert_int_eq(WTERMSIG(status), (sig)); } while(0)

// ----------------------------------------------------------------------------
// Name   : ck_assert_exited
// Usage  : checks the termination status is an exit code
// Return : None
// ----------------------------------------------------------------------------
#define ck_assert_exited(status, code) \
                 do { ck_assert_int_ne(WIFEXITED(status), 0); \
                      ck_assert_int_eq(WEXITSTATUS(status), (code)); } while(0)

// ----------------------------------------------------------------------------
// Name   : ck_assert_errno_eq
// Usage  : checks the value of errno
// Return : None
// ----------------------------------------------------------------------------
#define ck_assert_errno_eq(e) ck_assert_int_eq(errno, (e))


#define _ck_assert_size(X, OP, Y) do { \
  size_t _ck_x = (X); \
  size_t _ck_y = (Y); \
  ck_assert_msg(_ck_x OP _ck_y, "Assertion '%s' failed: %s == %jd, %s == %jd", #X" "#OP" "#Y, #X, _ck_x, #Y, _ck_y); \
} while (0)


#define ck_assert_size_eq(X, Y) _ck_assert_size(X, ==, Y)

#define ck_assert_size_ne(X, Y) _ck_assert_size(X, !=, Y)

#define ck_assert_size_gt(X, Y) _ck_assert_size(X, >, Y)

#define ck_assert_size_ge(X, Y) _ck_assert_size(X, >=, Y)

#define ck_assert_size_lt(X, Y) _ck_assert_size(X, <, Y)

#define ck_assert_size_le(X, Y) _ck_assert_size(X, <=, Y)


// ----------------------------------------------------------------------------
// Name   : ck_crtn_assert
// Usage  : Assert which prints a formatted message
// ----------------------------------------------------------------------------
#define ck_crtn_assert(e, format, ...) do { if (!(e)) {    \
           fprintf(stderr, \
                  "!!! CRTN_ASSERT_%d(%s/%s#%d) '%s' !!! ==> " format, \
                   getpid(), basename(__FILE__), __FUNCTION__, __LINE__, #e, ## __VA_ARGS__); \
       abort();           \
    } } while(0);


extern int ck_exec_prog(
                        char *av[]
                       );


#endif // CHECK_ALL_H
