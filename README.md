# CoRouTiNe service(CRTN)
   
### 1 Introduction
### 2 Maintainers
### 3 Build suite
### 4 Download
### 5 cmake
#### 5.1 crtn_install.sh script
#### 5.2 Build, installation, cleanup
#### 5.3 Tests
#### 5.4 Tests coverage measurement
#### 5.5 Packaging
#### 5.6 Notes about RPM package
#### 5.7 Notes about DEB package
#### 5.8 Cross-compilation
### 6 Documentation
#### 6.1 On line manuals

## 1 Introduction

CoRouTiNe (CRTN) is an API which proposes coroutines in C language programs.

CRTN is distributed under the GNU LGPL license.

The current document concerns the 0.1.0 version of the CRTN package.


## 2 Maintainers


To report a bug or design enhancement, please contact [Rachid Koucha](mailto:rachid.koucha@gmail.com)


## 3 Build suite

CRTN is built with CMAKE.


## 4 Download

The source code is available on github. Use the following command to download it:
```
$ git clone https://github.com/Rachid-Koucha/crtn.git
```

## 5 cmake


### 5.1 crtn_install.sh script


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


### 5.2 Build, installation, cleanup


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


### 5.3 Tests

The regression tests are based on CHECK library. The latter must be installed
prior to launch the tests.


To trigger the regression tests for the whole software:

$ ./crtn_install.sh -T
[...]
100%: Checks: 31, Failures: 0, Errors: 0


### 5.4 Tests coverage measurement

The test coverage measurement requires the `gcov/lcov` packages.

To trigger test coverage measurement for CRTN (with a display of the result in firefox):

`$ ./crtn_install.sh -C -b firefox`


### 5.5 Packaging

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
        

### 5.6 Notes about RPM package

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
### 5.7 Notes about DEB package

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

## 6 DOCUMENTATION

### 6.1 On line manuals

Once CRTN is installed, it is possible to access the corresponding on line manuals with:
```
$ man 7 crtn       # Overview of CRTN

$ man 3 crtn       # Manual of crtn API

$ man 3 crtn_mbx   # Manual of crtn mailbox service

$ man 3 crtn_sem   # Manual of crtn semaphore service
```
