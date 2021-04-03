#!/bin/bash
#
# Simple script to build/install/test/package CRTN
#
#  Copyright (C) 2021 Rachid Koucha <rachid dot koucha at gmail dot com>
#
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


SW_NAME=CRTN
TMP_DIR="/tmp/crtn_$$"
BUILD_DIR_DEFAULT="build"
BUILD_DIR=${BUILD_DIR_DEFAULT}

cleanup_exit()
{
  if [ -d ${TMP_DIR} ]
  then
    rm -rf ${TMP_DIR}
  fi
}

trap 'cleanup_exit' HUP INT EXIT TERM QUIT


# Default values
INST_DIR=/usr/local
BUILD_IT=0
INSTALL_IT=0
UNINSTALL_IT=0
TEST_IT=0
COV_IT=0
BROWSER=
ARCHIVE_IT=0
CLEANUP=0
CPACK_GENERATOR=
TOOLCHAIN=


# User manual
help()
{
PLIST="RPM|DEB|TGZ|STGZ"
OPTLIST="MBX|SEM"
TOOL_CHAINS=$(ls cmake/toolchains)

  {
    echo
    echo Usage:
    echo
    echo '  '`basename $1` "[-c] [-T|-C [browser]] [-d install_dir] [-o ${OPTLIST}] [-I] [-U]"
    echo "                  [-B] [-A] [-P ${PLIST}] [-b build_dir] [-X toolchain] [-h]"
    echo
    echo "    -c    : Cleanup built objects"
    echo "    -C [browser]: Launch test the coverage measurement (results are displayed with 'browser')"
    echo "    -T    : Launch the regression tests"
    echo "    -d    : Installation directory (default: ${INST_DIR})"
    echo "    -P ${PLIST}: Generate packages"
    echo "    -B    : Build the software"
    echo "    -b build_dir: Build directory (default: ${BUILD_DIR})"
    echo "    -I (*): Install the software"
    echo "    -U (*): Uninstall the software"
    echo "    -A    : Generate an archive of the software (sources)"
    echo "    -o    : Add ${OPTLIST} service"
    echo "    -X toolchain: Cross-build with a given toolchain file"
    echo "    -h    : this help"
    echo
    echo "   (*) Super user rights required"
    echo
  } 1>&2
}



# If no arguments ==> Display help
if [ $# -eq 0 ]
then
  help $0
  exit 1
fi


check_tool()
{
  which $1 > /dev/null 2>&1
  if [ $? -ne 0 ]
  then
    echo $1 tool is required \(is it installed or in the PATH variable?\) >&2
    exit 1
  fi
}


OPTSTRING=":cd:P:BIUATC:o:b:X:h"

manage_optional_arg()
{
  
  case $1 in
    C) COV_IT=1;;
    *) echo Missing argument for option -$1 >&2
       help $0
       exit 1;;
  esac

}

check_optional_arg()
{
  ARG=$1

  if [ ${ARG:0:1} = "-" ]
  then
     return 1
  else
     return 0
  fi

}

clean_build()
{
  if [ -d ${BUILD_DIR} ]
  then
    echo Removing \'${BUILD_DIR}\' directory
    rm -rf ${BUILD_DIR}
  fi
}

check_build_dir()
{
  # To avoid the destruction of existing directories and files
  # not related to crtn
  if [ -d $1 ]
  then
     files=`ls $1 | wc -l`
     if [ "$files" != "0" ]
     then
       [ ! -f $1/.${SW_NAME} ] && echo "Directory '$1' does not seem to belong to ${SW_NAME}" >&2 && exit 1
     fi
  fi
}

create_build_dir()
{
  # Make the build directory
  if [ ! -d ${BUILD_DIR} ]
  then
      mkdir -p ${BUILD_DIR}
      if [ $? -ne 0 ]
      then 
        echo Unable to create build directory \'${BUILD_DIR}\' >&2
        exit 1
      fi
      > ${BUILD_DIR}/.${SW_NAME}
  fi
}


# Parse the command line
while getopts ${OPTSTRING} arg
do
  case ${arg} in
    c) CLEANUP=1;;
    d) check_optional_arg ${OPTARG}
       if [ $? -ne 0 ]
       then echo Missing argument for option -${arg} >&2
            help $0
            exit 1
       fi
       INST_DIR=${OPTARG};;
    P) OPTARG=`echo ${OPTARG} | tr [:lower:] [:upper:]`
       check_optional_arg ${OPTARG}
       if [ $? -ne 0 ]
       then echo Missing argument for option -${arg} >&2
            help $0
            exit 1
       fi
       if [ ${OPTARG} = "DEB" ]
       then if [ -n "${CPACK_GENERATOR}" ]
            then CPACK_GENERATOR="${CPACK_GENERATOR};DEB"
            else CPACK_GENERATOR=DEB
            fi
       elif [ ${OPTARG} = "RPM" ]
       then if [ -n "${CPACK_GENERATOR}" ]
            then CPACK_GENERATOR="${CPACK_GENERATOR};RPM"
            else CPACK_GENERATOR=RPM
            fi
       elif [ ${OPTARG} = "TGZ" ]
       then if [ -n "${CPACK_GENERATOR}" ]
            then CPACK_GENERATOR="${CPACK_GENERATOR};TGZ"
            else CPACK_GENERATOR=TGZ
            fi
       elif [ ${OPTARG} = "STGZ" ]
       then if [ -n "${CPACK_GENERATOR}" ]
            then CPACK_GENERATOR="${CPACK_GENERATOR};STGZ"
            else CPACK_GENERATOR=STGZ
            fi
       else echo Unknown package type \'${OPTARG}\' >&2
            help $0
            exit 1
       fi;;
    o) OPTARG=`echo ${OPTARG} | tr [:lower:] [:upper:]`
       check_optional_arg ${OPTARG}
       if [ $? -ne 0 ]
       then echo Missing argument for option -${arg} >&2
            help $0
            exit 1
       fi
       if [ ${OPTARG} = "MBX" ]
       then if [ -n "${CONFIG_DEFINES}" ]
            then CONFIG_DEFINES="${CONFIG_DEFINES} -DHAVE_CRTN_MBX=ON"
            else CONFIG_DEFINES="-DHAVE_CRTN_MBX=ON"
            fi
       elif [ ${OPTARG} = "SEM" ]
       then if [ -n "${CONFIG_DEFINES}" ]
            then CONFIG_DEFINES="${CONFIG_DEFINES} -DHAVE_CRTN_SEM=ON"
            else CONFIG_DEFINES="-DHAVE_CRTN_SEM=ON"
            fi
       else echo Unknown service \'${OPTARG}\' >&2
            help $0
            exit 1
       fi;;
    B) BUILD_IT=1;;
    b) check_optional_arg ${OPTARG}
       if [ $? -ne 0 ]
       then echo Missing argument for option -${arg} >&2
            help $0
            exit 1
       fi
       check_build_dir ${OPTARG}
       BUILD_DIR=${OPTARG};;
    I) INSTALL_IT=1;;
    U) UNINSTALL_IT=1;;
    A) ARCHIVE_IT=1;;
    T) TEST_IT=1;;
    C) COV_IT=1;
       check_optional_arg ${OPTARG}
       if [ $? -eq 0 ]
       then BROWSER=${OPTARG}
       else OPTIND=$((OPTIND - 1))
       fi;;
    X) check_optional_arg ${OPTARG}
       if [ $? -ne 0 ]
       then echo Missing argument for option -${arg} >&2
            help $0
            exit 1
       fi
       TOOLCHAIN=${OPTARG};;
    h) help $0
       exit 0;;
    :) manage_optional_arg ${OPTARG};;
    \?) echo Invalid option -${OPTARG} >&2
        help $0
        exit 1;;
    *) help $0
       exit 1;;
  esac
done

shift $((${OPTIND} - 1))

# Check the arguments
if [ -n "$1" ]
then
  echo Too many arguments >&2
  help $0
  exit 1
fi

# Make sure that we are running in the source directory
if [ ! -f `basename $0` ]
then
  echo This script must be run in the source directory of CRTN >&2
  exit 1
else
  SRC_DIR=`pwd`
fi

# Make sure that we are running as root user for some options
if [ ${INSTALL_IT} -eq 1 -o ${UNINSTALL_IT} -eq 1 ]
then
  uid=`id -u`
  if [ ${uid} -ne 0 ]
  then
    echo "At least one of the specified script option needs super user rights (try at least sudo)" >&2
    exit 1
  fi
fi

check_tool cmake

if [ ${CLEANUP} -eq 1 ]
then
  clean_build
fi

# If archive is requested
if [ ${ARCHIVE_IT} -eq 1 ]
then

  check_tool tar

  # Launch the configuration to get CRTN config file
  clean_build
  create_build_dir
  cd ${BUILD_DIR}
  cmake ${CONFIG_DEFINES} -DCMAKE_INSTALL_PREFIX=${INST_DIR} ${SRC_DIR}
  cd -

  # Get CRTN's version
  CRTN_VERSION=`cat ${BUILD_DIR}/config.h | grep -E "^#define CRTN_VERSION" | cut -d' ' -f3 | cut -d\" -f2`
  if [ -z "${CRTN_VERSION}" ]
  then
    echo Unable to get the version of the software >&2
    exit 1
  fi

  ARCHIVE_DIR=crtn_src-${CRTN_VERSION}
  ARCHIVE_NAME=${ARCHIVE_DIR}.tgz

  # Make sure that the list of file exists
  FILE_LIST=file_list.txt

  if test ! -f ${FILE_LIST}
  then
    echo ${FILE_LIST} does not exist >&2
    exit 1
  fi

  mkdir -p ${TMP_DIR}/${ARCHIVE_DIR}

  # Copy the files into the temporary directory
  cat ${FILE_LIST} | while read comment line
  do

    # Skip empty lines
    if  [ -z "${comment}" ]
    then continue
    fi

    # Skip the comments
    if [ ${comment} = "#" ]
    then continue
    fi

    dir=${comment%/*};dir=${dir#./}

    if [ -n "${dir}" ]
    then
      mkdir -p ${TMP_DIR}/${ARCHIVE_DIR}/${dir}
    fi

    cp ${comment} ${TMP_DIR}/${ARCHIVE_DIR}/${dir}

  done

  echo Building archive ${BUILD_DIR}/${ARCHIVE_NAME}...
  tar cvfz ${BUILD_DIR}/${ARCHIVE_NAME} -C ${TMP_DIR} ${ARCHIVE_DIR} > /dev/null 2>&1

  rm -rf ${TMP_DIR}/${ARCHIVE_DIR}
fi

# If build is requested
if [ ${BUILD_IT} -eq 1 ]
then
  clean_build
  create_build_dir
  cd ${BUILD_DIR}
  cmake ${CONFIG_DEFINES} -DCMAKE_INSTALL_PREFIX=${INST_DIR} ${SRC_DIR}
  make
  cd -
fi

# If cross-build is requested
if [ -n "${TOOLCHAIN}" ]
then
  clean_build
  create_build_dir
  cd ${BUILD_DIR}
  if [ ! -f ${TOOLCHAIN} ]
  then echo The toolchain file '${TOOLCHAIN}' is not accessible from '${BUILD_DIR}' directory >&2
       exit 1
  fi
  cmake -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} ${CONFIG_DEFINES} -DCMAKE_INSTALL_PREFIX=${INST_DIR} ${SRC_DIR}
  make
  cd -
fi

# If installation is requested
if [ ${INSTALL_IT} -eq 1 ]
then

  # Make the build directory
  create_build_dir

  cd ${BUILD_DIR}
  cmake ${CONFIG_DEFINES} -DCMAKE_INSTALL_PREFIX=${INST_DIR} ${SRC_DIR}
  make
  make install
  cd -
fi

# If uninstallation is requested
if [ ${UNINSTALL_IT} -eq 1 ]
then
  if [ ! -d ${BUILD_DIR} ]
  then echo Build directory \(${BUILD_DIR}\) is not created, uninstallation not possible... >&2
       exit 1
  fi
  cd ${BUILD_DIR}
  make uninstall
  cd -
fi

# If test is requested
if [ ${TEST_IT} -eq 1 ]
then
  clean_build
  create_build_dir
  cd ${BUILD_DIR}
  cmake ${CONFIG_DEFINES} -DCMAKE_INSTALL_PREFIX=${INST_DIR} ${SRC_DIR}
  make
  tests/check_all
  cd -
fi

# If test coverage is requested
if [ ${COV_IT} -eq 1 ]
then
  clean_build
  create_build_dir
  cd ${BUILD_DIR}
  cmake ${CONFIG_DEFINES} -DCMAKE_COVERAGE=1 -DCMAKE_BUILD_TYPE=Debug ${SRC_DIR}
  make
  make all_coverage
  cd -
  if [ -n "${BROWSER}" ]
  then
    ${BROWSER} ${BUILD_DIR}/all_coverage/index.html
  fi
fi

if [ -n "${CPACK_GENERATOR}" ]
then

  clean_build
  create_build_dir

  cd ${BUILD_DIR}

  # Configure CMAKE
  cmake ${CONFIG_DEFINES} -DCPACK_GENERATOR=${CPACK_GENERATOR} -DCMAKE_INSTALL_PREFIX=${INST_DIR} ${SRC_DIR}

  # Launch the build
  make
  make package

  cd -

fi
