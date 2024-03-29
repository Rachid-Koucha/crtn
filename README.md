# CoRouTiNe API (CRTN)

[1 Introduction](#1_Introduction)  
[2 Maintainers](#2_Maintainers)  
[3 Download ](#3_Download)  
[4 Administration with cmake ](#4_Adm_cmake)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.1 Configuration](#4_1_Cfg)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.2 Build](#4_2_Build)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.3 Installation](#4_3_Installation)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.4 Tests](#4_4_Tests)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.5 Tests coverage](#4_5_Tests_coverage)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.6 Packaging](#4_6_Packaging)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.7 Cross-compiling](#4_7_Cross_compiling)  
[5 Administration with crtn_install.sh](#5_Adm_script)  
&nbsp;&nbsp;&nbsp;&nbsp;[5.1 crtn_install.sh script](#5_1_crtn_install_sh_scritpt)  
&nbsp;&nbsp;&nbsp;&nbsp;[5.2 Build, installation, cleanup](#5_2_Build_installation_cleanup)  
&nbsp;&nbsp;&nbsp;&nbsp;[5.3 Tests](#5_3_Tests)  
&nbsp;&nbsp;&nbsp;&nbsp;[5.4 Tests coverage](#5_4_Tests_coverage)  
&nbsp;&nbsp;&nbsp;&nbsp;[5.5 Packaging](#5_5_Packaging)  
&nbsp;&nbsp;&nbsp;&nbsp;[5.6 Cross-compiling](#5_6_Cross_compiling)  
[6 Usage](#6_Usage)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.1 Online manuals](#6_1_Online_man)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.2 Overview of the API](#6_2_API_overw)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.3 Examples](#6_3_Examples)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[6.3.1 Generator](#6_3_1_Generator)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[6.3.2 Producer/Consumer](#6_3_2_Prodcons)  
&nbsp;&nbsp;&nbsp;&nbsp;[6.4 Configuration environment variables](#6_4_Cfg_env_var)  
[7 Performance considerations](#7_Perf_cons)  
[Annexes](#Annexes)  
&nbsp;&nbsp;&nbsp;&nbsp;[A.1 Notes about RPM package](#A_1_Notes_rpm)  
&nbsp;&nbsp;&nbsp;&nbsp;[A.2 Notes about DEB package](#A_2_Notes_deb)  


## <a name="1_Introduction"></a>1 Introduction

CoRouTiNe (`crtn`) is an API providing [coroutines](https://en.wikipedia.org/wiki/Coroutine)
in C language programs. They are concurrent execution flows that can be suspended or resumed
under the control of the user. The underlying operating system has no idea of their existence.

The service is an abstraction layer implemented as a shared library on top of GLIBC:

<p align="center"><img src="doc/crtn_layers.png"></p>

`crtn` is distributed under the GNU LGPL license.

The current document concerns `crtn` version **0.2.5**.

Two articles have been published in french in the issues number [251](https://connect.ed-diamond.com/gnu-linux-magazine/glmf-251) and [253](https://connect.ed-diamond.com/gnu-linux-magazine/glmf-253) of GNU Linux Magazine France:

<p align="center"><a href="https://connect.ed-diamond.com/gnu-linux-magazine/glmf-251/execution-concurrente-avec-les-coroutines"><img src="doc/cover_01.png"></a></p>

<p align="center"><a href="https://connect.ed-diamond.com/gnu-linux-magazine/glmf-253/une-api-de-coroutines-pour-le-langage-c"><img src="doc/cover_02.png"></a></p>


## <a name="2_Maintainers"></a>2 Maintainers

To report bugs or suggestions, please contact [me](mailto:rachid.koucha@gmail.com)

## <a name="3_Download"></a>3 Download

The source code is available on [github](https://github.com/Rachid-Koucha/crtn).
To clone it:
```
$ git clone https://github.com/Rachid-Koucha/crtn.git
```
To get the source code of the 0.2.5 version:
```
$ cd crtn
crtn$ git checkout tags/v0.2.5
```

The source tree is:

<p align="center"><img src="doc/crtn_source_tree.png"></p>

## <a name="4_Adm_cmake"></a>4 Administration with cmake

In the following paragraphs, we suppose that the source code tree is located in _$HOME/crtn_.

To avoid the pollution of the source code tree with the build artifacts, it is
preferable to make an out of source configuration and build. So, we create a
build directory from which all the configuration/build operations will be done.
In the the following sections, we use _/tmp/crtn_build_:
```
$ mkdir /tmp/crtn_build
$ cd /tmp/crtn_build
/tmp/crtn_build$
```

### <a name="4_1_Cfg"></a>4.1 Configuration

To configure the build:

```
/tmp/crtn_build$ ls
/tmp/crtn_build$ cmake $HOME/crtn
[...]
-- Configuring CRTN version 0.2.5
[...]
-- Build files have been written to: /tmp/crtn_build
/tmp/crtn_build$ ls
CMakeCache.txt  CMakeFiles  CPackConfig.cmake [...]
```

Some optional services can be set by cmake variables:
```
/tmp/crtn_build$ cmake -LH
[...]
// Mailbox service
HAVE_CRTN_MBX:BOOL=OFF

// Semaphore service
HAVE_CRTN_SEM:BOOL=OFF
```

To configure the package with the optional mailbox and semaphore services:
```
/tmp/crtn_build$ cmake -DHAVE_CRTN_MBX=ON -DHAVE_CRTN_SEM=ON $HOME/crtn
-- Configuring CRTN version 0.2.5
[...]
-- Build files have been written to: /tmp/crtn_build
/tmp/crtn_build$ cmake -LH
[...]
// Mailbox service
HAVE_CRTN_MBX:BOOL=ON

// Semaphore service
HAVE_CRTN_SEM:BOOL=ON
```

### <a name="4_2_Build"></a>4.2 Build

To build the software:
```
/tmp/crtn_build$ make
[  1%] Building C object lib/CMakeFiles/crtn.dir/crtn.c.o
[  3%] Building C object lib/CMakeFiles/crtn.dir/crtn_mbx.c.o
[...]
/tmp/crtn_build$ ls lib
[...]libcrtn.so  libcrtn.so.0  libcrtn.so.0.2.5
```

To clean the built files:
```
/tmp/crtn_build$ make clean
/tmp/crtn_build$ ls lib
CMakeFiles  Makefile  cmake_install.cmake
```

### <a name="4_3_Installation"></a>4.3 Installation

To install the software in the default _/usr/local_ directory:
```
/tmp/crtn_build$ sudo make install
/tmp/crtn_build$ ls -l /usr/local/lib/libcrtn.so*
lrwxrwxrwx 1 root root    12 mars   21 12:04 /usr/local/lib/libcrtn.so -> libcrtn.so.0
lrwxrwxrwx 1 root root    16 mars   21 12:04 /usr/local/lib/libcrtn.so.0 -> libcrtn.so.0.2.5
-r--r--r-- 1 root root 60040 mars   21 12:04 /usr/local/lib/libcrtn.so.0.2.5
/tmp/crtn_build$ ls -l /usr/local/share/man/man3/crtn*
-r--r--r-- 1 root root 2786 mars   21 12:04 /usr/local/share/man/man3/crtn.3.gz
-r--r--r-- 1 root root   55 mars   21 12:04 /usr/local/share/man/man3/crtn_attr_delete.3.gz
-r--r--r-- 1 root root   52 mars   21 12:04 /usr/local/share/man/man3/crtn_attr_new.3.gz
-r--r--r-- 1 root root   50 mars   21 12:04 /usr/local/share/man/man3/crtn_cancel.3.gz
[...]
/tmp/crtn_build$ ls -l /usr/local/share/man/man7/crtn*
-r--r--r-- 1 root root 1436 mars   21 12:04 /usr/local/share/man/man7/crtn.7.gz
```
To uninstall the software:
```
/tmp/crtn_build$ sudo make uninstall
/tmp/crtn_build$ ls -l /usr/local/share/man/man3/crtn*
ls: cannot access '/usr/local/share/man/man3/crtn*': No such file or directory
```

### <a name="4_4_Tests"></a>4.4 Tests

To launch the regression tests, [check](https://libcheck.github.io/check/) package must be available on the system.

The tests can be triggered through cmake's _test_ target:
```
/tmp/crtn_build$ make test
Running tests...
Test project /tmp/crtn_build
    Start 1: crtn_tests
1/1 Test #1: crtn_tests .......................   Passed   22.03 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =  22.03 sec
```
The logs are located in _Testing_ sub-directory:
```
/tmp/crtn_build$ ls Testing
Temporary
/tmp/crtn_build$ ls Testing/Temporary
CTestCostData.txt  LastTest.log
```
The tests can also be triggered by calling directly the executable in _tests_ sub-directory:
```
/tmp/crtn_build$ tests/check_all
Running suite(s): CRTN tests
[...]
100%: Checks: 32, Failures: 0, Errors: 0
```

### <a name="4_5_Tests_coverage"></a>4.5 Tests coverage

To measure the tests coverage, the `gcov/lcov` packages are required. For example, here is the test coverage with the semaphore/mailbox optional services:
```
/tmp/crtn_build$ make clean
/tmp/crtn_build$ cmake -DHAVE_CRTN_MBX=ON -DHAVE_CRTN_SEM=ON -DCMAKE_COVERAGE=1 -DCMAKE_BUILD_TYPE=Debug $HOME/crtn
-- Configuring CRTN version 0.2.5
CMAKE_C_COMPILER_ID=GNU
-- Appending code coverage compiler flags: -g -O0 --coverage -fprofile-arcs -ftest-coverage
[...]
/tmp/crtn_build$ make
-- Configuring CRTN version 0.2.5
CMAKE_C_COMPILER_ID=GNU
-- Appending code coverage compiler flags: -g -O0 --coverage -fprofile-arcs -ftest-coverage
[...]
/tmp/crtn_build$ make all_coverage
[...]
Running suite(s): CRTN tests
100%: Checks: 31, Failures: 0, Errors: 0
Capturing coverage data from .
[...]
Overall coverage rate:
  lines......: 95.8% (527 of 550 lines)
  functions..: 100.0% (40 of 40 functions)
Open .../all_coverage/index.html in your browser to view the coverage report.
[100%] Built target all_coverage
```
The resulting _/tmp/crtn_build/all_coverage/index.html_ file can be viewed in a browser
to get something like this:

<p align="center"><img src="doc/crtn_coverage.png"></p>


### <a name="4_6_Packaging"></a>4.6 Packaging

To generate the Debian (_deb_), Red-Hat Package Manager (_rpm_), Tar GZipped (_tgz_) and
Self Extracting Tar GZipped (_stgz_) binary packages with the semaphore/mailbox optional services:

```
/tmp/crtn_build$ make clean
/tmp/crtn_build$ cmake -DHAVE_CRTN_MBX=ON -DHAVE_CRTN_SEM=ON -DCPACK_GENERATOR="DEB;RPM;TGZ;STGZ" -DCMAKE_INSTALL_PREFIX=/usr/local $HOME/crtn
/tmp/crtn_build$ make
/tmp/crtn_build$ make package
[...]
CPack: - package: .../crtn/crtn_0.2.5_amd64.deb generated.
[...]
Pack: - package: .../crtn/crtn-0.2.5-1.x86_64.rpm generated.
[...]
CPack: - package: .../crtn/crtn-0.2.5-Linux-crtn.tar.gz generated.
[...]
CPack: - package: .../crtn/crtn-0.2.5-Linux-crtn.sh generated.
```

### <a name="4_7_Cross_compiling"></a>4.7 Cross-compiling

To cross-compile with `cmake`, a toolchain file is required. Some examples are
provided in _cmake/toolchains_ from the top level of the source tree.

For an ARM 32 bits build when **crossbuild-essential-armhf** is installed::
```
/tmp/crtn_build$ make clean
/tmp/crtn_build$ rm CMakeCache.txt
/tmp/crtn_build$ cmake -DCMAKE_TOOLCHAIN_FILE=$HOME/crtn/cmake/toolchains/arm-linux-gnueabihf.cmake -DHAVE_CRTN_MBX=ON -DHAVE_CRTN_SEM=ON $HOME/crtn
-- The C compiler identification is GNU 9.3.0
-- Check for working C compiler: /usr/bin/arm-linux-gnueabihf-gcc
-- Check for working C compiler: /usr/bin/arm-linux-gnueabihf-gcc -- works
[...]
/tmp/crtn_build$ make
[  1%] Building C object lib/CMakeFiles/crtn.dir/crtn.c.o
[  3%] Building C object lib/CMakeFiles/crtn.dir/crtn_mbx.c.o
[...]
/tmp/crtn_build$ file lib/libcrtn.so.0.2.5
lib/libcrtn.so.0.2.5: ELF 32-bit LSB shared object, ARM, EABI5 version 1 (SYSV), dynamically linked, BuildID[sha1]=9a719ea0ab44ecffe6100f146f08fb6cf6b57e63, with debug_info, not stripped
```
For an ARM 64 bits build when **crossbuild-essential-arm64** is installed:
```
/tmp/crtn_build$ make clean
/tmp/crtn_build$ rm CMakeCache.txt
/tmp/crtn_build$ cmake -DCMAKE_TOOLCHAIN_FILE=$HOME/crtn/cmake/toolchains/aarch64-linux-gnu.cmake -DHAVE_CRTN_MBX=ON -DHAVE_CRTN_SEM=ON $HOME/crtn
-- The C compiler identification is GNU 9.3.0
-- Check for working C compiler: /usr/bin/aarch64-linux-gnu-gcc
-- Check for working C compiler: /usr/bin/aarch64-linux-gnu-gcc -- works
[...]
/tmp/crtn_build$ make
Scanning dependencies of target crtn
[  1%] Building C object lib/CMakeFiles/crtn.dir/crtn.c.o
[  3%] Building C object lib/CMakeFiles/crtn.dir/crtn_mbx.c.o
[...]
/tmp/crtn_build$ file lib/libcrtn.so.0.2.5
lib/libcrtn.so.0.2.5: ELF 64-bit LSB shared object, ARM aarch64, version 1 (SYSV), dynamically linked, BuildID[sha1]=6a60c0908cb8f4b94827418dbfa17eebf556b9fa, with debug_info, not stripped
```

## <a name="5_Adm_script"></a>5 Administration with crtn_install.sh


### <a name="5_1_crtn_install_sh_scritpt"></a>5.1 crtn_install.sh script

This shell script is a swiss army knife to make several things. It implicitly uses `cmake`.
**The tool is to be used from the top level of the source tree**. In the following paragraphs,
we suppose that the source code tree is located in _$HOME/crtn_.

To display the help, use the `-h` option:
```
$ cd $HOME/crtn
$HOME/crtn$ ./crtn_install.sh -h

Usage:

  crtn_install.sh [-c] [-T|-C [browser]] [-d install_dir] [-o MBX|SEM] [-I] [-U]
                  [-B] [-A] [-P RPM|DEB|TGZ|STGZ] [-b build_dir] [-X toolchain] [-h]

    -c    : Cleanup built objects
    -C [browser]: Measure the test coverage (results are displayed with 'browser')
    -T    : Launch the regression tests
    -d    : Installation directory (default: /usr/local)
    -P RPM|DEB|TGZ|STGZ: Generate packages
    -B    : Build the software
    -b build_dir: Build directory (default: build)
    -I (*): Install the software
    -U (*): Uninstall the software
    -A    : Generate an archive of the software (sources)
    -o    : Add MBX|SEM service
    -X toolchain: Cross-build with a given toolchain file
    -h    : this help

   (*) Super user rights required
```

Note that some options require super user privileges. Use `sudo` for example.

To avoid the pollution of the source code tree with build artifacts, it is preferable to make
an out of source configuration and build. The build directory is specified with the `-b` option.
By default, a sub-directory called _build_ is created by the tool at the top level of the source tree.

### <a name="5_2_Build_installation_cleanup"></a>5.2 Build, installation, cleanup

To clean everything, go to the top level of the source tree:
```
$HOME/crtn$ ./crtn_install.sh -c
```
To build the library:
```
$HOME/crtn$ ./crtn_install.sh -B
$HOME/crtn$ ls build
[...]include  lib  man  tests
$HOME/crtn$ ls build/lib
[...]libcrtn.so  libcrtn.so.0  libcrtn.so.0.2.5
```
If mailbox and/or semaphores services are required, add the corresponding options on the command line:
```
$HOME/crtn$ ./crtn_install.sh -B -o mbx -o sem
```

For a complete installation in the default _/usr/local_ subtree (super user rights required):
```
$HOME/crtn$ sudo ./crtn_install.sh -I
$HOME/crtn$ ls -l /usr/local/lib/libcrtn.so*
lrwxrwxrwx 1 root root    12 mars   21 12:04 /usr/local/lib/libcrtn.so -> libcrtn.so.0
lrwxrwxrwx 1 root root    16 mars   21 12:04 /usr/local/lib/libcrtn.so.0 -> libcrtn.so.0.2.5
-r--r--r-- 1 root root 60040 mars   21 12:04 /usr/local/lib/libcrtn.so.0.2.5
$HOME/crtn$ ls -l /usr/local/share/man/man3/crtn*
-r--r--r-- 1 root root 2786 mars   21 12:04 /usr/local/share/man/man3/crtn.3.gz
-r--r--r-- 1 root root   55 mars   21 12:04 /usr/local/share/man/man3/crtn_attr_delete.3.gz
-r--r--r-- 1 root root   52 mars   21 12:04 /usr/local/share/man/man3/crtn_attr_new.3.gz
-r--r--r-- 1 root root   50 mars   21 12:04 /usr/local/share/man/man3/crtn_cancel.3.gz
[...]
$HOME/crtn$ ls -l /usr/local/share/man/man7/crtn*
-r--r--r-- 1 root root 1436 mars   21 12:04 /usr/local/share/man/man7/crtn.7.gz
```
To uninstall the software (super user rights required):
```
$HOME/crtn$ sudo ./crtn_install.sh -U
$HOME/crtn$ ls -l /usr/local/share/man/man7/crtn*
ls: cannot access '/usr/local/share/man/man7/crtn*': No such file or directory
```
To cleanup every build artifacts to go back to original source tree:
```
$HOME/crtn$ ./crtn_install.sh -c
Removing 'build' directory
```

### <a name="5_3_Tests"></a>5.3 Tests

The regression tests are based on [check](https://libcheck.github.io/check/) package. The latter must be installed
prior launching the tests.

To trigger the regression tests for the whole software (i.e. with the optional mailbox and semaphore services):
```
$HOME/crtn$ ./crtn_install.sh -T -o mbx -o sem
[...]
100%: Checks: 32, Failures: 0, Errors: 0
```

### <a name="5_4_Tests_coverage"></a>5.4 Tests coverage

To measure the test coverage, the `gcov/lcov` packages are required.

The test coverage of `crtn` with a display of the result in firefox:
```
$HOME/crtn$ ./crtn_install.sh -C firefox
```
The same with the semaphore/mailbox optional services:
```
$HOME/crtn$ ./crtn_install.sh -C firefox -o mbx -o sem
```
This shows something like this in the browser:

<p align="center"><img src="doc/crtn_coverage.png"></p>

### <a name="5_5_Packaging"></a>5.5 Packaging

To make a tar gzip source package, use the `-A` option of `crtn_install.sh`:
```
$HOME/crtn$ ./crtn_install.sh -A
[...]
Building archive build/crtn_src-0.2.5.tgz...
```
It is also possible to generate Debian (_deb_), Red-Hat Package Manager (_rpm_), Tar GZipped (_tgz_)
and Self Extracting Tar GZipped (_stgz_) binary packages:
```
$HOME/crtn$ ./crtn_install.sh -c -P tgz -P rpm -P deb -P stgz
```
This makes the following binary packages in the _build_ sub-directory:
* _crtn_0.2.5_amd64.deb (deb)_
* _crtn-0.2.5-1.x86_64.rpm (rpm)_
* _crtn-0.2.5-Linux-crtn.tar.gz (tgz)_
* _crtn-0.2.5-Linux-crtn.sh (stgz)_

### <a name="5_6_Cross_compiling"></a>5.6 Cross-compiling

To cross-compile with `cmake`, a toolchain file is required. Some examples are
provided in _cmake/toolchains_ from the top level of the source tree.

For an ARM 32 bits build when **crossbuild-essential-armhf** is installed:
```
$HOME/crtn$ ./crtn_install.sh -X $HOME/crtn/cmake/toolchains/arm-linux-gnueabihf.cmake -o sem -o mbx
-- The C compiler identification is GNU 9.3.0
-- Check for working C compiler: /usr/bin/arm-linux-gnueabihf-gcc
-- Check for working C compiler: /usr/bin/arm-linux-gnueabihf-gcc -- works
[...]
$HOME/crtn$ file build/lib/libcrtn.so.0.2.5
lib/libcrtn.so.0.2.5: ELF 32-bit LSB shared object, ARM, EABI5 version 1 (SYSV), dynamically linked, BuildID[sha1]=9a719ea0ab44ecffe6100f146f08fb6cf6b57e63, with debug_info, not stripped
```

For an ARM 64 bits build when **crossbuild-essential-arm64** is installed:
```
$HOME/crtn$ ./crtn_install.sh -X $HOME/crtn/cmake/toolchains/arch64-linux-gnu.cmake -o sem -o mbx
Removing 'build' directory
-- The C compiler identification is GNU 9.3.0
-- Check for working C compiler: /usr/bin/aarch64-linux-gnu-gcc
-- Check for working C compiler: /usr/bin/aarch64-linux-gnu-gcc -- works
[...]
$HOME/crtn$ file build/lib/libcrtn.so.0.2.5
lib/libcrtn.so.0.2.5: ELF 64-bit LSB shared object, ARM aarch64, version 1 (SYSV), dynamically linked, BuildID[sha1]=6a60c0908cb8f4b94827418dbfa17eebf556b9fa, with debug_info, not stripped
```


## <a name="6_Usage"></a>6 Usage

### <a name="6_1_Online_man"></a>6.1 Online manuals

Once `crtn` is installed, it is possible to access the corresponding online manuals with:
```
$ man 7 crtn       # Overview of crtn and configuration environment variables

$ man 3 crtn       # Manual of crtn API

$ man 3 crtn_mbx   # Manual of crtn mailbox service

$ man 3 crtn_sem   # Manual of crtn semaphore service
```
The latters provide some small example programs in their _EXAMPLES_ section.

### <a name="6_2_API_overw"></a>6.2 Overview of the API

The functions of the API are similar to the [pthread](https://en.wikipedia.org/wiki/POSIX_Threads)'s one.

A coroutine is created with `crtn_spawn()`.  The latter returns a unique coroutine identifier (cid).

The coroutines have several attributes set with the `crtn_set_attr_xxx()` services:
* Two types of coroutines are provided: **stackless** and **stackful** (default).
* Two scheduling types are provided: **stepper** and **standalone** (default). At startup, a **stepper** coroutine is suspended whereas a **standalone** coroutine is always runnable.

A coroutine suspends itself calling `crtn_yield()`. It is resumed when another coroutine calls `crtn_yield()` if it is **standalone** or `crtn_wait()` if it is **stepper**.

A **stepper** coroutine can pass the address of some data to `crtn_yield()`. The coroutine waiting for it, gets those data with the pointer passed to `crtn_wait()`. 

A coroutine terminates when it reaches the end of its entry point, when it calls `crtn_exit()` or when another coroutine calls `crtn_cancel()` to finish it.

A  terminated coroutine stays in a zombie state until another coroutine calls `crtn_join()` to get its termination status and to implicitly free the corresponding internal data structures.  The latter is an integer with a user defined signification.

The state diagram of a coroutine is depicted in the following figure:

<p align="center"><img src="doc/crtn_state_diagram.png"></p>

The scheduling is FIFO oriented. Any coroutine becoming runnable, is put at the beginning of the list. Any running (**standalone**) coroutine yielding the CPU goes at the end of the list. This minimizes CPU starvation.

Additional inter-coroutine communication and synchronization are optionally provided with the `-o` option of the `crtn_install.sh` script or the `HAVE_CRTN_MBX/SEM` cmake defines:
- The mailboxes (`crtn_mbx_new()`, `crtn_mbx_post()`, `crtn_mbx_get()`...) with `-o mbx` or `-DHAVE_CRTN_MBX=ON`;
- The semaphores (`crtn_sem_new()`, `crtn_sem_p()`, `crtn_sem_v()`...) with `-o sem` or `-DHAVE_CRTN_SEM=ON`.

### <a name="6_3_Examples"></a>6.3 Examples

#### <a name="6_3_1_Generator"></a>6.3.1 Generator

Coroutines are well-suited for implementing generators.

In the following example, the main coroutine creates a secondary coroutines with the **stepper** attribute and resumes it every seconds. The secondary coroutine generates the following term of the [fibonacci sequence](https://en.wikipedia.org/wiki/Fibonacci_number) each time it is resumed by the main coroutine. The term is passed through `crtn_wait()`. 

```c
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>

#include <crtn.h>

static int signaled;

static void hdl_sigint(int sig)
{

  printf("Signal %d...\n", sig);
  signaled = 1;

} // hdl_sigint


static int fibonacci(void *param)
{
  unsigned long long prevn_1;
  unsigned long long prevn;
  unsigned long long cur;

  (void)param;

start:

  cur = prevn_1 = 0;
  crtn_yield(&cur);

  cur = prevn = 1;
  crtn_yield(&cur);

  while (1) {
    // Check overflow
    if ((ULLONG_MAX - prevn_1) < prevn) {
      goto start;
    }
    cur = prevn + prevn_1;
    crtn_yield(&cur);
    prevn_1 = prevn;
    prevn = cur;
  }

  return 0;
  
} // fibonacci


int main(void)
{
  crtn_t cid;
  int rc;
  int status;
  unsigned long long *seq;
  crtn_attr_t attr;
  unsigned int i;

  signal(SIGINT, hdl_sigint);

  attr = crtn_attr_new();
  if (!attr) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_attr_new(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_set_attr_type(attr, CRTN_TYPE_STEPPER);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_set_attr_type(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_spawn(&cid, "Fibonacci", fibonacci, 0, attr);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_attr_delete(attr);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_attr_delete(): error '%m' (%d)\n", errno);
    return 1;
  }

  i = 0;
  while(1) {

    rc = crtn_wait(cid, (void **)&seq);
    if (rc != 0) {
      errno = crtn_errno();
      fprintf(stderr, "crtn_wait(%d): error '%m' (%d)\n", cid, errno);
      return 1;
    }
    printf("seq[%u]=%llu\n", i, *seq);
    i ++;
    sleep(1);

    if (signaled) {
      break;
    }
  } // End while

  rc = crtn_cancel(cid);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_cancel(%d): error '%m' (%d)\n", cid, errno);
    return 1;
  }

  rc = crtn_join(cid, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(%d): error '%m' (%d)\n", cid, errno);
    return 1;
  }

  return status;

} // main
```
Build:
```
$ gcc fibonacci.c -o fibonacci -lcrtn
```
Execution:

```
$ ./fibonacci
seq[0]=0
seq[1]=1
seq[2]=1
seq[3]=2
seq[4]=3
seq[5]=5
seq[6]=8
seq[7]=13
seq[8]=21
seq[9]=34
seq[10]=55
seq[11]=89
seq[12]=144
seq[13]=233
^CSignal 2...
```
Many other example programs are available in the _tests_ sub-directory of the source code tree.

#### <a name="6_3_2_Prodcons"></a>6.3.2 Producer/Consumer

Coroutines are well-suited for implementing cooperating tasks in event-driven
applications.

The following is an example of producer/consumer program reimplementing the shell's `wc` program to count the
lines, words and characters from the standard input. The behaviour of the program is depicted in the following
diagram:

<p align="center"><img src="doc/crtn_wc.png"></p>

The main coroutine (the producer) reads **stdin** in non blocking mode (`nb_read()` function) and fills a buffer
with the `fill_buffer()` function. The function relinquishes the CPU when data are written in the buffer. This
resumes the other coroutines.

Each state of the above diagram are implemented with coroutines (the consumers) which merely read the buffer (with `read_buffer()`
function) and relinquish the CPU as soon as the read character is not accepted in the corresponding state. The
latter is put back in the buffer (with `unread_buffer()` function).

The coroutines finish when they encounter **EOF**.

The buffer looks like a pipe between the producer and the consumers. The producer increments a write pointer and
the consumers increment a read pointer. The buffer is empty when both pointers are equal.

Concerning the scheduling, all the coroutines are **standalone**. They don't resume explicitly any other
coroutine. They cooperate implicitly by calling `crtn_yield()` whenever characters are added in the buffer
(for the producer) and the read character is not accepted or the read pointer reaches the write pointer
(for the consumers). For the latters, this works because any character is accepted in only one state. So, if
a consumer coroutine is resumed and encounters an unaccepted character, it suspends itself immediately to
resume any other coroutine. When resumed, the producer suspends itself if the read pointer is not equal to the
write one.

No semaphore is needed as the coroutines run concurrently but not in parallel.
```c
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <sys/select.h>
#include <unistd.h>

#include "crtn.h"

static int w_offset, r_offset;

#define BUFFER_SIZE 128
static char buffer[BUFFER_SIZE];

struct counter_t
{
  size_t nb_chars;
  size_t nb_spaces;
  size_t nb_words;
  size_t nb_lines;
};

struct counter_t cnts;



//----------------------------------------------------------------------------
// Name        : nb_read
// Description : Non blocking read
// Return      : Number of read bytes if OK
//               -1, if error
//               -2, timeout
//               -3, EOF
//----------------------------------------------------------------------------
int nb_read(int fd, char *buf, size_t bufsz, unsigned long to_ms)
{
int            rc;
fd_set         fdset;
int            nfds = fd + 1;
struct timeval to;

  to.tv_sec  = 0;
  to.tv_usec = to_ms * 1000;

  while(1) {
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    rc = select(nfds, &fdset, NULL, NULL, &to);
    switch (rc) {

      // Error
      case -1 : {
        // Interrupted system call ?
        if (EINTR != errno) {
          return -1;
        }
      }
      break;

      // Timeout
      case 0: {
        return -2;        
      }
      break;

      // Incoming data
      default : {
        rc = read(fd, buf, bufsz);

        // Error ?
        if (rc < 0) {
          return -1;
        }

        // EOF ?
        if (0 == rc) {
          return -3;
        }

        // Data
        return rc;
      }
      break;
    } // End switch
  } // End while
} // nb_read


static int read_buffer(void)
{
  if (r_offset == w_offset) {
    crtn_yield(0);
  }

  cnts.nb_chars ++;
  return buffer[r_offset ++];
} // read_buffer

#define unread_buffer(c) do {           \
                  assert(r_offset > 0); \
                  -- r_offset;          \
                  cnts.nb_chars --;     \
                } while(0)

static int fill_buffer(void)
{
  int rc;
  unsigned long to;
  size_t size;

  to = 0;
  while(1) {

    do {

      if (w_offset) {
        // If buffer is empty, reset the pointers
        if (w_offset == r_offset) {
          w_offset = r_offset = 0;
        }
      }

      size = BUFFER_SIZE - w_offset;

      // There is still some space behind the write pointer, tries to fill
      // it with additional input data
      if (size) {
        rc = nb_read(0, &(buffer[w_offset]), size, to);
        to = 0;
      } else {
        // The wrtie pointer is at the end of the buffer and there
        // are still remaining data to read
        crtn_yield(0);
      }

    } while(size == 0);

    switch(rc) {
      case -1: {
        // Error
        buffer[w_offset] = EOF; // Trigger the end of the coroutines  
        w_offset ++;
        crtn_yield(0);
        return -1;
      }
      break;

      case -2: {
        // Timeout
        if (w_offset > r_offset) {
          // There are still data to read in the buffer
          crtn_yield(0);
        } else {

          // Buffer empty, wait for more input data
          to = 250;  // Polling 250 ms
        }
      }
      break;

      case -3: {
        // EOF
        buffer[w_offset] = EOF; // Trigger the end of the coroutines  
        w_offset ++;
        crtn_yield(0);
        return 0;
      }
      break;

      default: {
        // New data
        w_offset += rc;
        crtn_yield(0);
      }
      break;
    } // End switch
  } // End while

} // fill_buffer

static int get_spaces(void *p)
{
  int c;

  (void)p;

  do {

    c = read_buffer();
    while(isspace(c) && (c != '\n') && (c != EOF)) {
      cnts.nb_spaces ++;
      c = read_buffer();
    }
    unread_buffer(c);

    if (c == EOF) {
      break;
    }

    crtn_yield(0);

  } while(1);

  return 0;

} // get_spaces

static int get_word(void *p)
{
  int c;
  size_t count;

  (void)p;

  do {

    count = cnts.nb_chars;
    c = read_buffer();
    while(!isspace(c) && (c != EOF)) {
      c = read_buffer();
    }
    unread_buffer(c);
    if (cnts.nb_chars > count) {
      cnts.nb_words ++;
    }

    if (c == EOF) {
      break;
    }

    crtn_yield(0);

  } while(1);

  return 0;

} // get_word


static int get_lines(void *p)
{
  int c;

  (void)p;

  do {

    c = read_buffer();
    while((c == '\n') && (c != EOF)) {
      cnts.nb_lines ++;
      c = read_buffer();
    }
    unread_buffer(c);

    if (c == EOF) {
      break;
    }

    crtn_yield(0);

  } while(1);

  return 0;

} // get_lines


int main(void)
{
  crtn_t cid_word, cid_spaces, cid_lines;
  int rc;
  int status;
  int exit_code;

  rc = crtn_spawn(&cid_word, "word", get_word, 0, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_spawn(&cid_lines, "lines", get_lines, 0, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  rc = crtn_spawn(&cid_spaces, "space", get_spaces, 0, 0);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_spawn(): error '%m' (%d)\n", errno);
    return 1;
  }

  exit_code = 0;

  rc = fill_buffer();
  if (rc != 0) {
    fprintf(stderr, "Input error '%m' (%d)\n", errno);
    exit_code = 1;
  }

  rc = crtn_join(cid_word, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    exit_code = 1;
  }

  rc = crtn_join(cid_spaces, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    exit_code = 1;
  }

  rc = crtn_join(cid_lines, &status);
  if (rc != 0) {
    errno = crtn_errno();
    fprintf(stderr, "crtn_join(): error '%m' (%d)\n", errno);
    exit_code = 1;
  }

  printf("Lines: %zu / Words: %zu / Spaces: %zu / Characters: %zu\n"
         ,
         cnts.nb_lines, cnts.nb_words, cnts.nb_spaces, cnts.nb_chars
        );

  return exit_code;

} // main
```
Build:
```
$ gcc mywc.c -o mywc -lcrtn
```
Execution:
```
$ > foo
$ cat foo | wc
      0       0       0
$ cat foo | ./mywc
Lines: 0 / Words: 0 / Spaces: 0 / Characters: 0
$ cat /etc/passwd | wc
     50      91    3017
$ cat /etc/passwd | ./mywc 
Lines: 50 / Words: 91 / Spaces: 41 / Characters: 3017
```

### <a name="6_4_Cfg_env_var"></a>6.4 Configuration environment variables

As described in `man 7 crtn`, several environment variables are interpreted at library's initialization time:
- **CRTN_MAX**: Maximum number of coroutines (20 by default);
- **CRTN_MBX_MAX**: Maximum number of mailboxes (64 by default);
- **CRTN_SEM_MAX**: Maximum number of semaphores (64 by default);
- **CRTN_STACK_SIZE**: Size in bytes of the stack of **stackless**/**stackful** coroutines (16384 by default).

## <a name="7_Perf_cons"></a>7 Performance considerations

In several situations, user level scheduled coroutines are more optimal than threads which require a context switch in the Linux kernel for the scheduling. They may also offer better performances than programs written in the caller/callee model. For example, in recursive applications, they don't require to pop all the stack frames to return a result.

But the underlying layer of `crtn` is based on the `get/make/swapcontext()` services. The latters trigger the `rt_sigprocmask()` system call to save/restore the signal mask. This is a drawback for some performance critical applications.

If high performances are required, one may consider reimplementing `get/make/swapcontext()` services without the call to `rt_sigprocmask()`.
 
## <a name="Annexes"></a> Annexes

### <a name="A_1_Notes_rpm"></a>A.1 Notes about RPM package

To get information on a package file:
```
$ rpm -qp --info crtn-0.2.5-1.x86_64.rpm
Name        : crtn
Version     : 0.2.5
Release     : 1
Architecture: x86_64
[...]
License     : GPL/LGPL
Signature   : (none)
Source RPM  : crtn-0.2.5-1.src.rpm
[...]
Relocations : /usr/local 
Vendor      : Rachid Koucha
URL         : https://github.com/Rachid-Koucha/crtn
Summary     : CRTN (CoRouTiNe API for C language)
Description :
CoRouTiNe API for C language
```
To get the pre/post-installation scripts in a package file:
```
$ rpm -qp --scripts rsys-0.2.5-1.x86_64.rpm
preinstall program: /bin/sh
postinstall scriptlet (using /bin/sh):

#!/bin/sh

INSTALL_PREFIX=/usr/local

FILE=${INSTALL_PREFIX}/lib/cmake/FindRsys.cmake
chmod 644  ${FILE}
[...]
```
To list the files for an INSTALLED package:
```
$ rpm -ql crtn
```
To list the files in a package file:
```
$ rpm -ql crtn-0.2.5-1.x86_64.rpm
```
The required package list of an _rpm_ file could be printed with:
```
$ rpm -qp --requires crtn-0.2.5-1.x86_64.rpm
```
### <a name="A_2_Notes_deb"></a>A.2 Notes about DEB package

To get information on a package file:
```
$ dpkg --info crtn_0.2.5_amd64.deb
[...]
 Package: crtn
 Version: 0.2.5
 Section: devel
 Priority: optional
 Architecture: amd64
 Homepage: https://github.com/Rachid-Koucha/crtn
[...]
 Maintainer: Rachid Koucha <rachid dot koucha at gmail dot com>
 Description: CoRouTiNe API for C language
```
To list the files in a package file:
```
$ dpkg -c crtn_0.2.5_amd64.deb
```
To install the content of a package file (super user rights required):
```
$ sudo dpkg -i crtn_0.2.5_amd64.deb
drwxr-xr-x root/root         0 2021-03-12 20:06 ./usr/
drwxr-xr-x root/root         0 2021-03-12 20:06 ./usr/local/
drwxr-xr-x root/root         0 2021-03-12 20:06 ./usr/local/include/
-r--r--r-- root/root      4291 2021-03-12 20:05 ./usr/local/include/crtn.h
drwxr-xr-x root/root         0 2021-03-12 20:06 ./usr/local/lib/
lrwxrwxrwx root/root         0 2021-03-12 20:06 ./usr/local/lib/libcrtn.so
[...]
```
To list the installed packages:
```
$ dpkg -l | grep crtn
ii  crtn  0.2.5    amd64   CoRouTiNe API for C language
```
To uninstall (remove) a package (super user rights required):
```
$ sudo dpkg -r crtn
```
