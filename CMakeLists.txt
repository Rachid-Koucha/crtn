cmake_minimum_required(VERSION 2.6)

# To accept VERSION token VERSION in PROJECT directive
cmake_policy(SET CMP0048 NEW)

# This sets CMAKE_PROJECT_NAME
#           PROJECT_SOURCE_DIR, crtn_SOURCE_DIR
#           PROJECT_BINARY_DIR, crtn_BINARY_DIR
#           PROJECT_VERSION, crtn_VERSION
#           PROJECT_VERSION_MAJOR, crtn_VERSION_MAJOR
#           PROJECT_VERSION_MINOR, crtn_VERSION_MINOR
#           PROJECT_VERSION_PATCH, crtn_VERSION_PATCH
#           PROJECT_DESCRIPTION, crtn_DESCRIPTION
#           PROJECT_HOMEPAGE_URL, crtn_HOMEPAGE_URL
PROJECT(crtn
        VERSION 0.2.2
        DESCRIPTION "CoRouTiNe API for C language"
        HOMEPAGE_URL "https://github.com/Rachid-Koucha/crtn"
        LANGUAGES C)

SET(CRTN_VERSION "${crtn_VERSION_MAJOR}.${crtn_VERSION_MINOR}.${crtn_VERSION_PATCH}")


# Define GNU standard installation directories
#
# This defines several variables:
#
# CMAKE_INSTALL_<dir>      - destination for files of a given type
# CMAKE_INSTALL_FULL_<dir> - corresponding absolute path
#
# where <dir> is:
#
#   BINDIR           - user executables (bin)
#   SBINDIR          - system admin executables (sbin)
#   LIBEXECDIR       - program executables (libexec)
#   SYSCONFDIR       - read-only single-machine data (etc)
#   SHAREDSTATEDIR   - modifiable architecture-independent data (com)
#   LOCALSTATEDIR    - modifiable single-machine data (var)
#   LIBDIR           - object code libraries (lib or lib64 or
#                      lib/<multiarch-tuple> on Debian)
#   INCLUDEDIR       - C header files (include)
#   OLDINCLUDEDIR    - C header files for non-gcc (/usr/include)
#   DATAROOTDIR      - read-only architecture-independent data root (share)
#   DATADIR          - read-only architecture-independent data (DATAROOTDIR)
#   INFODIR          - info documentation (DATAROOTDIR/info)
#   LOCALEDIR        - locale-dependent data (DATAROOTDIR/locale)
#   MANDIR           - man documentation (DATAROOTDIR/man)
#   DOCDIR           - documentation root (DATAROOTDIR/doc/PROJECT_NAME)
INCLUDE(GNUInstallDirs)

#
# Forbid builds in the source tree
#
IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

    MESSAGE(FATAL_ERROR "###########################################################################\n"
                        "You are attempting to build in the source directory.\n"
                        "Run cmake from an alternate build directory to avoid source tree pollution.\n"
                        "###########################################################################")

ENDIF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

MESSAGE(STATUS "Configuring CRTN version ${CRTN_VERSION}")

# Size of the coroutine name
SET(CFG_CRTN_NAME_SZ 24)
SET(CFG_CRTN_STACK_SIZE 16384)
SET(CFG_CRTN_MAX 20)
SET(CFG_CRTN_MBX_MAX 64)
SET(CFG_CRTN_SEM_MAX 64)
OPTION(HAVE_CRTN_MBX "Mailbox service" OFF)
OPTION(HAVE_CRTN_SEM "Semaphore service" OFF)
CONFIGURE_FILE(config.h.in config.h)

SET(VERSION ${CRTN_VERSION})
CONFIGURE_FILE(README.md.in README.md @ONLY)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

ADD_DEFINITIONS(-g -O2 -fsigned-char -freg-struct-return -Wall -W -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror)


if (CMAKE_COVERAGE)

  include(CodeCoverage)

  APPEND_COVERAGE_COMPILER_FLAGS()

endif()

# Get the user identifier
EXECUTE_PROCESS(COMMAND id -u
                OUTPUT_VARIABLE USER_ID)

MESSAGE("The user id is ${USER_ID}")

# "uninstall" target (as cmake does not provide "make uninstall" by default)
# Reference: https://gitlab.kitware.com/cmake/community/wikis/FAQ#can-i-do-make-uninstall-with-cmake
#
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# Add "make dist" like target. The following generates a "make package" target
SET(CRTN_HOME_PAGE ${PROJECT_HOMEPAGE_URL})
SET(CPACK_PACKAGE_VERSION_MAJOR ${CRTN_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${CRTN_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${CRTN_VERSION_PATCH})
SET(CPACK_PACKAGE_VENDOR "Rachid Koucha")
SET(CPACK_PACKAGE_CONTACT "Rachid Koucha <rachid dot koucha at gmail dot com>")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}) # Otherwise it defaults to "/usr"
SET(CPACK_PACKAGE_HOMEPAGE_URL ${CRTN_HOME_PAGE})
SET(CPACK_COMPONENTS_GROUPING "IGNORE")

# Packages type:
#   . Debian (DEB)
#   . Red-Hat Package Manager (RPM)
#   . Tar GZipped (TGZ)
#   . Self extracting Tar GZipped (STGZ)
#
# We set it on cmake's command line (e.g. -DCPACK_GENERATOR="DEB;STGZ;RPM")
#SET(CPACK_GENERATOR "DEB" "RPM" "TGZ" "STGZ")

SET(CPACK_COMPONENTS_ALL crtn)
SET(CPACK_COMPONENTS_IGNORE_GROUP 1)

# This makes the archive types (TGZ and STGZ) generate one file per
# component
SET(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

# To avoid TGZ include "crtn-xxx-Linux/" in the pathnames 
SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY "OFF")


SET(PKG_NAME crtn)
SET(PKG_SUMMARY "CRTN (CoRouTiNe API for C language)")
SET(CPACK_COMPONENT_CRTN_DESCRIPTION ${crtn_DESCRIPTION})


#
# TGZ package
#
SET(CPACK_TGZ_COMPONENT_INSTALL "ON")
#SET(CPACK_TGZ_CRTN_PACKAGE_NAME "${PKG_NAME}")

#
# STGZ package
#
SET(CPACK_STGZ_COMPONENT_INSTALL "ON")
#SET(CPACK_STGZ_CRTN_PACKAGE_NAME "${PKG_NAME}")


#
# CRTN DEB package
#
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${CRTN_HOME_PAGE})
SET(CPACK_DEB_COMPONENT_INSTALL "ON")
SET(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS "ON")
SET(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
SET(CPACK_DEBIAN_CRTN_PACKAGE_NAME "${PKG_NAME}")
SET(CPACK_DEBIAN_CRTN_PACKAGE_CONTROL_EXTRA ${CMAKE_BINARY_DIR}/crtn.postinst)

#
# CRTN RPM package
#
SET(CPACK_RPM_COMPONENT_INSTALL "ON")
SET(CPACK_RPM_CRTN_PACKAGE_NAME "${PKG_NAME}")
SET(CPACK_RPM_FILE_NAME "RPM-DEFAULT")
SET(CPACK_RPM_PACKAGE_URL ${CRTN_HOME_PAGE})
SET(CPACK_RPM_CRTN_PACKAGE_SUMMARY "${PKG_SUMMARY}")
SET(CPACK_RPM_PACKAGE_LICENSE "GPL/LGPL")
SET(CPACK_RPM_CRTN_POST_INSTALL_SCRIPT_FILE ${CMAKE_BINARY_DIR}/crtn.postinst)


#
# To set before "ADD_SUBDIRECTORY" to propagate the variable into the sub-
# directories
#
#SET(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
CONFIGURE_FILE(crtn.postinst.in crtn.postinst @ONLY)

#
# Configure the header file
#
CONFIGURE_FILE(include/crtn.h.in include/crtn.h @ONLY)

#
# Configure the manuals
#
SET(ONLINE_MANUALS man/crtn.7 man/crtn.3)

if (${HAVE_CRTN_MBX} STREQUAL ON)
  SET(ONLINE_MANUALS ${ONLINE_MANUALS} man/crtn_mbx.3)
endif()

if (${HAVE_CRTN_SEM} STREQUAL ON)
  SET(ONLINE_MANUALS ${ONLINE_MANUALS} man/crtn_sem.3)
endif()

FOREACH(man ${ONLINE_MANUALS})

  CONFIGURE_FILE(${man}.in ${man} @ONLY)

ENDFOREACH()

ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(man)

IF (NOT CMAKE_CROSSCOMPILING)
  INCLUDE(FindCheck)
ENDIF (NOT CMAKE_CROSSCOMPILING)

if (CHECK_FOUND)

  # Add the 'test' target
  ENABLE_TESTING()

else(CHECK_FOUND)

  MESSAGE("Check package not found ==> No unitary test will be built")

endif(CHECK_FOUND)

ADD_SUBDIRECTORY(tests)

# Must be in last position to take in account the preceding
# (even ADD_SUBDIRECTORY to get the component names)
INCLUDE(CPack)
