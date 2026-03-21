/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    dispdll.c

Abstract:

    This file contains the dll Win32 enry point and associated routines.

Author:


Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//
// Module handle, saved away at process attach time.
//
HANDLE MyModuleHandle;


BOOL
WINAPI
DllMain(
    IN HANDLE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved
    )

/*++

Routine Description:

    This routine is this dll's Win32 dll entry point, called indirectly
    because we're using the CRT DLL with _DllMainCRTStartup as the dll
    entry point.

Arguments:

    Standard win32 dll entry arguments.

Return Value:

    Always TRUE.

--*/

{

    BOOL b;

    UNREFERENCED_PARAMETER(Reserved);

    b = TRUE;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:

        MyModuleHandle = DllHandle;
        break;

    case DLL_THREAD_ATTACH:

        break;

    case DLL_PROCESS_DETACH:

        break;

    case DLL_THREAD_DETACH:

        break;
    }

    return (b);
}
