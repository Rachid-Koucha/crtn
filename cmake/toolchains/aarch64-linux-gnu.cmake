#
# CMake defines to cross-compile for Raspberry Pi (ARM64/Linux when
# crossbuild-essential-arm64 is installed)
#

# This sets CMAKE_CROSSCOMPILING
SET(CMAKE_SYSTEM_NAME Linux)

SET(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
SET(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)
SET(CMAKE_SYSTEM_PROCESSOR arm64)

#add_definitions("-mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm")

# rdynamic means the backtrace should work
IF (CMAKE_BUILD_TYPE MATCHES "Debug")
   add_definitions(-rdynamic)
ENDIF()

# avoids annoying and pointless warnings from gcc
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE")
#SET(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -c")

# Search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
