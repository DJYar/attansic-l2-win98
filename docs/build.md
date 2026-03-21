# Build Instructions (Windows 2000 DDK style)

## 1) Open a DDK build environment from this repository

This repository already contains the DDK tree at `NTDDK/`.
From a Windows command prompt, run `setenv.bat` from that local tree:

```bat
cd \path\to\repo
NTDDK\bin\setenv.bat NTDDK checked W2K
```

(Use `free` instead of `checked` for a release-style build.)

## 2) Build the driver

Change to the driver directory and run:

```bat
cd \path\to\repo\driver\src
build
```

You can also use a clean build:

```bat
build -cZ
```

## 3) Output location

The resulting driver binary is generated under the build output tree in the
`driver` directory, typically:

- `objchk\i386\l2ndis.sys` for checked builds
- `objfre\i386\l2ndis.sys` for free builds
