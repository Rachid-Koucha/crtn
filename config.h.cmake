// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// File        : config.h
// Description : Configuration of CRTN
// License     :
//
//  Copyright (C) 2020 Rachid Koucha <rachid dot koucha at gmail dot com>
//
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//
// Evolutions  :
//
//     26-Feb-2021  R. Koucha           - Creation
//
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef CONFIG_H
#define CONFIG_H


//---------------------------------------------------------------------------
// Name : __CRTN_FALLTHROUGH
// Usage: For switch/cases without breaks to avoid compiler errors/warnings
//        The attribute is available from GCC version 7
//----------------------------------------------------------------------------
#define CRTN_GCC_VERSION (__GNUC__ * 10000 \
                         + __GNUC_MINOR__ * 100 \
                         + __GNUC_PATCHLEVEL__)

#if CRTN_GCC_VERSION >= 70000

#define __CRTN_FALLTHROUGH __attribute__ ((fallthrough));

#else

#define __CRTN_FALLTHROUGH

#endif // CRTN_GCC_VERSION

//---------------------------------------------------------------------------
// Name : CRTN_BUILD_DIR
// Usage: Pathname of the build directory
//----------------------------------------------------------------------------
#define CRTN_BUILD_DIR "@CMAKE_BINARY_DIR@"


//---------------------------------------------------------------------------
// Name : CRTN_VERSION
// Usage: Version of CRTN
//----------------------------------------------------------------------------
#define CRTN_VERSION "@CRTN_VERSION@"


//---------------------------------------------------------------------------
// Name : CRTN_MAX
// Usage: Maximum number of coroutines
//----------------------------------------------------------------------------
#define CRTN_MAX @CFG_CRTN_MAX@


//---------------------------------------------------------------------------
// Name : CRTN_MBX
// Usage: Include mailbox services
//----------------------------------------------------------------------------
#cmakedefine HAVE_CRTN_MBX


//---------------------------------------------------------------------------
// Name : CRTN_MBX_MAX
// Usage: Maximum number of mailboxes
//----------------------------------------------------------------------------
#define CRTN_MBX_MAX @CFG_CRTN_MBX_MAX@



//---------------------------------------------------------------------------
// Name : CRTN_SEM
// Usage: Include semaphore services
//----------------------------------------------------------------------------
#cmakedefine HAVE_CRTN_SEM


//---------------------------------------------------------------------------
// Name : CRTN_SEM_MAX
// Usage: Maximum number of semaphores
//----------------------------------------------------------------------------
#define CRTN_SEM_MAX @CFG_CRTN_SEM_MAX@


#endif // CONFIG_H
