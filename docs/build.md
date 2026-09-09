# Build instructions (Windows 2000 DDK style)

## Prerequisites

Install or unpack these separately; their files are not distributed in this
repository:

- Windows 2000 DDK, exposed as an `NTDDK` tree
- Microsoft Visual C++ 6.0 toolchain

The source was validated with the DDK `build.exe` flow targeting W2K.  Both
checked and free configurations produce a driver loadable by Windows 98 SE.

## Standard DDK environment

Open a command prompt, initialize the DDK environment, then build from the
driver source directory:

```bat
C:\NTDDK\bin\setenv.bat C:\NTDDK checked W2K
cd C:\path\to\attansic-l2-win98\driver\src
build -cZ
```

Use `free` instead of `checked` for a release-style build. Output is normally
written to:

- `driver\src\objchk\i386\l2ndis.sys` for checked builds
- `driver\src\objfre\i386\l2ndis.sys` for free builds

## Reproducible helper script

`tools\build-win10.cmd` compensates for stale paths commonly embedded in an
archived DDK environment, runs a clean build, and verifies that the requested
binary was actually produced.

Set the external toolchain locations before invoking it when they differ from
the defaults:

```bat
set L2_DDK_ROOT=C:\NTDDK
set L2_VC6_ROOT=C:\Tools\VC6_Toolchain\VC98
set L2_VC6_COMMON=C:\Tools\VC6_Toolchain\Common\MSDev98
tools\build-win10.cmd checked
tools\build-win10.cmd free
```

The defaults are the paths shown above. The script selects `libchk`/`objchk`
or `libfre`/`objfre` consistently, removes any previous target binary, and
fails if a stale artifact could otherwise disguise a build error.
