/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    util.h

Abstract:

Author:

Revision History:

--*/

//
// routines in util.c
//
UINT32
WriteMsg(
    IN UINT32 MessageId,
    ...
    );

UINT32
WriteText(
    IN LPCWSTR Format,
    ...
    );

UINT32
WriteMsgToLog(
    IN KanalyzeLogLevel LogLevel,
    IN UINT32 MessageId,
    ...
    );

UINT32
WriteTextToLog(
    IN KanalyzeLogLevel LogLevel,
    IN LPCWSTR Format,
    ...
    );

