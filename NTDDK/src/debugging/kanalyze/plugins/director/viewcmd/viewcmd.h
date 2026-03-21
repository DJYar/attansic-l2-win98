/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    viewcmd.h

Abstract:

    private header file for View Command Plug-In for Kernel Memory Space Analyzer.

Author:


Revision History:

--*/

#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <basetsd.h>
#include <shellapi.h>
#define KA_EXTAPI
#include <kanalyze.h>

#include "util.h"
#include "msg.h"
#include "res.h"

//
//  Doubly-linked list manipulation routines.  Implemented as macros
//  but logically these are procedures.
//  *From ntddk.h
//  VOID
//  InitializeListHead(
//      PLIST_ENTRY ListHead
//      );
//

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))
//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

//
// externs in vcmddll.c
//
extern HANDLE MyModuleHandle;

//
// externs in viewcmd.c
//
extern HKAPLUGIN hPlugIn;
extern KANALYZE_EXTENSION_APIS KanalyzeExtensionApis;

