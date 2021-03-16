# CoRouTiNe API (CRTN)

[1 Introduction](#Introduction)  
[2 Maintainers](#Maintainers)  
[3 Download ](#Download)  
[4 Administration](#Administration)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.1 crtn_install.sh script](#crtn_install_sh_scritpt)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.2 Build, installation, cleanup](#Build_installation_cleanup)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.3 Tests](#Tests)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.4 Tests coverage measurement](#Tests_coverage_measurement)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.5 Packaging](#Packaging)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.6 Notes about RPM package](#Notes_rpm)  
&nbsp;&nbsp;&nbsp;&nbsp;[4.7 Notes about DEB package](#Notes_deb)  
[5 Documentation](#Documentation)  
&nbsp;&nbsp;&nbsp;&nbsp;[5.1 Online manuals](#Online_man)  
&nbsp;&nbsp;&nbsp;&nbsp;[5.2 Overview of the API](#API_overw)  
&nbsp;&nbsp;&nbsp;&nbsp;[5.3 Example program](#Ex_prog)

## <a name="Introduction"></a>1 Introduction

CoRouTiNe (CRTN) is a shared library providing an API which proposes coroutines in C language programs. This is a set of services to manage concurrent execution flows. A coroutine can be suspended or resumed under the control of the user. The underlying operating system have no idea of their existence.

CRTN is distributed under the GNU LGPL license.

The current document concerns the 0.1.0 version of the CRTN package.


## <a name="Maintainers"></a>2 Maintainers


To report a bug or design enhancement, please contact [Rachid Koucha](mailto:rachid.koucha@gmail.com)


## <a name="Download"></a>3 Download

The source code is available on github. Use the following command to download it:
```
$ git clone https://github.com/Rachid-Koucha/crtn.git
```

## <a name="Administration"></a>4 Administration


### <a name="crtn_install_sh_scritpt"></a>4.1 crtn_install.sh script


This shell script is a swiss army knife to make several things. It implicitly uses `cmake`. Display its help:

```
$ ./crtn_install.sh -h

Usage:

  crtn_install.sh [-b browser] [-c] [-T|-C] [-d install_dir] [-o MBX|SEM]
                  [-B] [-I] [-U] [-A] [-P RPM|DEB|TGZ|STGZ] [-h]

    -b    : Browser's pathname to display the test coverage HTML results
    -c    : Cleanup built objects
    -C    : Launch the test coverage measurement
    -T    : Launch the regression tests
    -d    : Installation directory (default: /usr/local)
    -P (*): Generate DEB/RPM/TGZ/STGZ packages
    -B    : Build the software
    -I (*): Install the software
    -U (*): Uninstall the software
    -A    : Generate an archive of the software (sources)
    -o    : Add MBX|SEM service
    -h    : this help

   (*) Super user rights required
```

Note that some options require super user privileges to run. Use `sudo` for example.


### <a name="Build_installation_cleanup"></a>4.2 Build, installation, cleanup


* To build the library:

`$ ./crtn_install.sh -B`

* If mailbox and/or semaphores are required, add the corresponding option on the command line:

`$ ./crtn_install.sh -B -o mbx -o sem`

* For a complete installation in the default /usr/local subtree (super user rights required):

`$ sudo ./crtn_install.sh -I`

* To uninstall the software (super user rights required):

`$ sudo ./crtn_install.sh -U`

* To cleanup every generated files to go back to original source tree:

`$ ./crtn_install.sh -c`


### <a name="Tests"></a>4.3 Tests

The regression tests are based on CHECK library. The latter must be installed
prior to launch the tests.

To trigger the regression tests for the whole software:

$ ./crtn_install.sh -T
[...]
100%: Checks: 31, Failures: 0, Errors: 0


### <a name="Tests_coverage_measurement"></a>4.4 Tests coverage measurement

The test coverage measurement requires the `gcov/lcov` packages.

To trigger test coverage measurement for CRTN (with a display of the result in firefox):

`$ ./crtn_install.sh -C -b firefox`


### <a name="Packaging"></a>4.5 Packaging

To make a tar gzip source package, use the `-A` option of `crtn_install.sh`:

`$ ./crtn_install.sh -A`

This makes a TGZ file of the complete source tree: crtn_src-0.1.0.tgz


It is also possible to generate Debian (DEB), Red-Hat Package Manager (RPM), Tar GZipped (TGZ) and Self Extracting Tar GZipped (STGZ) binary packages.

To build the packages TGZ, DEB, RPM... (super user rights required):

`$ sudo ./crtn_install.sh -c -P tgz -P rpm -P deb -P stgz`

This makes the following binary packages:
* crtn_0.1.0_amd64.deb (DEB)
* crtn-0.1.0-1.x86_64.rpm (RPM)
* crtn-0.1.0-Linux-crtn.tar.gz (TGZ)
* crtn-0.1.0-Linux-crtn.sh (STGZ)
        

### <a name="Notes_rpm"></a>4.6 Notes about RPM package

Use the following to get information on a package file:
```
$ rpm -qp --info crtn-0.1.0-1.x86_64.rpm
Name        : crtn
Version     : 0.1.0
Release     : 1
Architecture: x86_64
[...]
License     : GPL/LGPL
Signature   : (none)
Source RPM  : crtn-0.1.0-1.src.rpm
[...]
Relocations : /usr/local 
Vendor      : Rachid Koucha
URL         : https://github.com/Rachid-Koucha/crtn
Summary     : CRTN (CoRouTiNe API for C language)
Description :
CoRouTiNe API for C language
```
Use the following to get the pre/post-installation scripts in a package file:
```
$ rpm -qp --scripts rsys-0.1.0-1.x86_64.rpm
preinstall program: /bin/sh
postinstall scriptlet (using /bin/sh):

#!/bin/sh

INSTALL_PREFIX=/usr/local

FILE=${INSTALL_PREFIX}/lib/cmake/FindRsys.cmake
chmod 644  ${FILE}
[...]
```
Use following to list the files for an INSTALLED package:
```
$ rpm -ql crtn
```
Use following to list the files in a package file:
```
$ rpm -ql crtn-0.1.0-1.x86_64.rpm
```
The required package list of an RPM file could be printed with:
```
$ rpm -qp --requires crtn-0.1.0-1.x86_64.rpm
```
### <a name="Notes_deb"></a>4.7 Notes about DEB package

Use the following to get information on a package file:
```
$ dpkg --info crtn_0.1.0_amd64.deb
[...]
 Package: crtn
 Version: 0.1.0
 Section: devel
 Priority: optional
 Architecture: amd64
 Homepage: https://github.com/Rachid-Koucha/crtn
[...]
 Maintainer: Rachid Koucha <rachid dot koucha at gmail dot com>
 Description: CoRouTiNe API for C language
```
Use following to list the files in a package file:
```
$ dpkg -c crtn_0.1.0_amd64.deb
```
Use following to install the content of a package file (super user rights required):
```
$ sudo dpkg -i crtn_0.1.0_amd64.deb
drwxr-xr-x root/root         0 2021-03-12 20:06 ./usr/
drwxr-xr-x root/root         0 2021-03-12 20:06 ./usr/local/
drwxr-xr-x root/root         0 2021-03-12 20:06 ./usr/local/include/
-r--r--r-- root/root      4291 2021-03-12 20:05 ./usr/local/include/crtn.h
drwxr-xr-x root/root         0 2021-03-12 20:06 ./usr/local/lib/
lrwxrwxrwx root/root         0 2021-03-12 20:06 ./usr/local/lib/libcrtn.so
[...]
```
Use the following to list the installed packages:
```
$ dpkg -l | grep crtn
ii  crtn  0.1.0    amd64   CoRouTiNe API for C language
```
Use following to uninstall (remove) a package (super user rights required):
```
$ sudo dpkg -r crtn
```

## <a name="Documentation"></a>5 Documentation

### <a name="Online_man"></a>5.1 Online manuals

Once CRTN is installed, it is possible to access the corresponding online manuals with:
```
$ man 7 crtn       # Overview of CRTN

$ man 3 crtn       # Manual of crtn API

$ man 3 crtn_mbx   # Manual of crtn mailbox service

$ man 3 crtn_sem   # Manual of crtn semaphore service
```
### <a name="API_overw"></a>5.2 Overview of the API

A coroutine is created with `crtn_spawn()`.  The latter returns a unique coroutine identifier (cid).

The coroutines have several attributes set with the `crtn_set_attr_xxx()` service:
* Two types of coroutines are provided: stackless and stackful.
* Two scheduling types are provided: stepper and standalone. A stepper coroutine is by default suspended. A standalone coroutine is by default always runnable.

A coroutine suspends itself calling `crtn_yield()`. It is resumed when another coroutine calls `crtn_yield()` if it is standalone or `crtn_wait()` if it is stepper. 

A coroutine terminates when it reaches the end of its entry point, when it calls `crtn_exit()` or when another coroutine calls `crtn_cancel()` to finish it.

A  terminated coroutine stays in a zombie state until another coroutine calls `crtn_join()` to get its termination status and to implicitly free the corresponding internal data structures.  The latter is an integer with a user defined signification.

Inter-coroutine communication/synchronization are provided: the mailboxes (`crtn_mbx()`) and semaphores (`crtn_sem()`). Those services are optional. They are set at package configuration time with the `-o` option of `crtn_install.sh` script.

### <a name="Ex_prog"></a>5.3 Example program

In the following example, a secondary stepper coroutine generates the following term of the [fibonacci sequence](https://en.wikipedia.org/wiki/Fibonacci_number) each time it is resumed by the main coroutine. The term is passed through
`crtn_wait()`. 

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
    cur = prevn + prevn_1;
    // Check overflow
    if ((ULLONG_MAX - prevn_1) < prevn) {
      goto start;
    }
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

The execution displays:

```
$ fibonacci
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
