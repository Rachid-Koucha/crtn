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


TMPDIR="/tmp/crtn_$$"



cleanup()
{
  if [ -d ${TMPDIR} ]
  then
    rm -rf ${TMPDIR}
  fi
}

trap 'cleanup' HUP INT EXIT TERM QUIT


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


uid=`id -u`


# User manual
help()
{
PLIST="RPM|DEB|TGZ|STGZ"
OPTLIST="MBX|SEM"

  {
    echo
    echo Usage:
    echo
    echo '  '`basename $1` "[-b browser] [-c] [-T|-C] [-d install_dir] [-o ${OPTLIST}]"
    echo "                  [-B] [-I] [-U] [-A] [-P ${PLIST}] [-h]"
    echo
    echo "    -b    : Browser's pathname to display the test coverage HTML results"
    echo "    -c    : Cleanup built objects"
    echo "    -C    : Launch test the coverage measurement"
    echo "    -T    : Launch the regression tests"
    echo "    -d    : Installation directory (default: ${INST_DIR})"
    echo "    -P (*): Generate ${PLIST} package"
    echo "    -B    : Build the software"
    echo "    -I (*): Install the software"
    echo "    -U (*): Uninstall the software"
    echo "    -A    : Generate an archive of the software (sources)"
    echo "    -o    : Add ${OPTLIST} service"
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


# Parse the command line
while getopts cd:P:BIUATCb:o:h arg
do
  case ${arg} in
    c) CLEANUP=1;;
    d) INST_DIR=${OPTARG};;
    P) OPTARG=`echo ${OPTARG} | tr [:lower:] [:upper:]`
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
    I) INSTALL_IT=1;;
    U) UNINSTALL_IT=1;;
    A) ARCHIVE_IT=1;;
    T) TEST_IT=1;;
    C) COV_IT=1;;
    b) BROWSER=${OPTARG};;
    h) help $0
       exit 0;;
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
fi

# Make sure that we are running as root user for some options
if [ -n "${CPACK_GENERATOR}" -o ${INSTALL_IT} -eq 1 -o ${UNINSTALL_IT} -eq 1 ]
then
  if [ ${uid} -ne 0 ]
  then
    echo "Those script options need super user rights (try at least sudo)" >&2
    exit 1
  fi
fi


if [ ${CLEANUP} -eq 1 ]
then

  echo Cleanup...

  echo ">" Launching cmake cleanup
  make clean > /dev/null 2>&1

  echo ">" Deleting test coverage stuff
  rm -rf all_coverage
  find . -type f -name \*.gcda -exec rm -f {} \; -print
  find . -type f -name \*.gcno -exec rm -f {} \; -print

  echo ">" Deleting core files
  find . -type f -name core -exec rm -f {} \; -print

  echo ">" Deleting emacs backup files
  find . -type f -name \*~ -exec rm -f {} \; -print

  # Delete the packages
  find . -type f \( -name \*.tgz -o -name \*.tar.gz -o -name \*.tar.Z -o -name \*.deb -o -name \*.rpm -o -name crtn-\*-Linux-\*.sh \) -exec rm -f {} \; -print

  echo ">" Deleting packaging of cmake
  rm -rf _CPack_Packages CPackConfig.cmake CPackSourceConfig.cmake

  echo ">" Deleting the pkg-config files
  find . -type f -name \*.pc -exec rm -f {} \; -print

  echo ">" Deleting the files generated by cmake
  rm -f CMakeCache.txt config.h install_manifest*.txt
  find . -type f -name Makefile -exec rm -f {} \; -print
  find . -type f -name cmake_install.cmake -exec rm -f {} \; -print
  find . -type d -name CMakeFiles -exec rm -rf {} \; -print 2>/dev/null
  find . -type f -name CMakeCache.txt -exec rm -f {} \; -print

  echo ">" Deleting compressed mans
  find . -type f \( -name crtn\*.gz \) -exec rm -f {} \; -print

  # Delete the generated mans
  rm -f man/crtn.3

  # Delete various common objects
  find . -type f -name a.out -exec rm -f {} \; -print
  rm -f src/crtn
  rm -f tests/check_all

  echo ">" Deleting config files generated by cmake
  find . -type f -name \*.in | while read file
  do
    base=${file%.in}; base=${base##*/}
    dir=${file%/*}
    if [ -f ${dir}/${base} ]
    then
      echo "   >" erasing ${dir}/${base}
      rm -f ${dir}/${base}
    fi
  done

fi

# Make sure that cmake is installed if build or installation or 
# packaging is requested
if [ ${BUILD_IT} -eq 1 -o ${ARCHIVE_IT} -eq 1 -o ${INSTALL_IT} -eq 1 -o ${UNINSTALL_IT} -eq 1 -o ${TEST_IT} -eq 1 -o ${COV_IT} -eq 1 -o -n "${CPACK_GENERATOR}" ]
then
  which cmake > /dev/null 2>&1
  if [ $? -ne 0 ]
  then
    echo To be able to compile/install CRTN, you must install cmake and/or update the PATH variable >&2
    exit 1
  fi

  # Launch cmake
  echo Configuring CRTN installation in ${INST_DIR}...
  cmake ${CONFIG_DEFINES} -DCMAKE_INSTALL_PREFIX=${INST_DIR} .
fi

# If archive is requested
if [ ${ARCHIVE_IT} -eq 1 ]
then

  which tar > /dev/null 2>&1
  if [ $? -ne 0 ]
  then
    echo "To be able to generate a CRTN archive, you must install 'tar' and/or update the PATH variable" >&2
    exit 1
  fi

  # Launch the build to get CRTN config file
  make

  # Get CRTN's version
  CRTN_VERSION=`cat config.h | grep -E "^#define CRTN_VERSION" | cut -d' ' -f3 | cut -d\" -f2`
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

  mkdir -p ${TMPDIR}/${ARCHIVE_DIR}

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
      mkdir -p ${TMPDIR}/${ARCHIVE_DIR}/${dir}
    fi

    cp ${comment} ${TMPDIR}/${ARCHIVE_DIR}/${dir}

  done

  echo Building archive ${ARCHIVE_NAME}...
  tar cvfz ${ARCHIVE_NAME} -C ${TMPDIR} ${ARCHIVE_DIR} > /dev/null 2>&1

  rm -rf ${TMPDIR}/${ARCHIVE_DIR}
fi

# If build is requested
if [ ${BUILD_IT} -eq 1 ]
then
  make
fi

# If installation is requested
if [ ${INSTALL_IT} -eq 1 ]
then
  make
  make install
fi

# If uninstallation is requested
if [ ${UNINSTALL_IT} -eq 1 ]
then
  make uninstall
fi

# If test is requested
if [ ${TEST_IT} -eq 1 ]
then
  make
  tests/check_all
fi

# If test coverage is requested
if [ ${COV_IT} -eq 1 ]
then
  make clean
  cmake ${CONFIG_DEFINES} -DCMAKE_COVERAGE=1 -DCMAKE_BUILD_TYPE=Debug .
  make
  make all_coverage
  if [ -n "${BROWSER}" ]
  then
    ${BROWSER} ./all_coverage/index.html
  fi
fi

if [ -n "${CPACK_GENERATOR}" ]
then

  # Configure CMAKE
  cmake ${CONFIG_DEFINES} -DCPACK_GENERATOR=${CPACK_GENERATOR} -DCMAKE_INSTALL_PREFIX=${INST_DIR} .

  # Launch the build
  make clean
  make
  make package

fi
