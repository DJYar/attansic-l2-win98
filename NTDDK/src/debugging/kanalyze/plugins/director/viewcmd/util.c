/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    util.c

Abstract:

    Display Items Plug-In for Kernel Memory Space Analyzer.
    This file contains helpful routines for this Plug-In.

Author:


Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


UINT32
WriteMsg(
    IN UINT32 MessageId,
    ...
    )

/*++

Routine Description:

    This routine is used to write messate to console.

Arguments:

    DetailLevel - verbosity level.

    MessageId - the message id defined in message file.

Return Value:

    Length of message string.

--*/

{
    LPWSTR OutputText;
    UINT32 Count;
    va_list Arguments;

    va_start(Arguments,MessageId);

    //
    // Get formatted message from this module.
    // The MessageId must be defined in file msg.mc.
    //
    Count = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                MyModuleHandle,
                MessageId,
                0,
                (LPWSTR)&OutputText,
                0,
                &Arguments
                );

    if (Count) {
        WriteText(OutputText);
        LocalFree(OutputText);
    }

    va_end(Arguments);

    return Count;
}



UINT32
WriteText(
    IN LPCWSTR Format,
    ...
    )
/*++

Routine Description:

    This routine writes message to standard output.
    It doesn't use buffer, and converts return code properly.

Arguments:

    Format - supplies a pointer of format string.

Return Value:

    Length of message string. If an error occured, it returns 0.

--*/
{
    static HANDLE hStdOut = INVALID_HANDLE_VALUE;
    static LONG32 unicodeBufferSize;
    static LONG32 ansiBufferSize;
    static LPWSTR unicodeBuffer;
    static LPSTR ansiBuffer;
    LONG32 unicodeCharCount;
    LONG32 ansiByteCount;
    LONG32 outputByteCount;
    LONG32 written;
    LONG32 i, pp;
    va_list arglist;

    // initialize
    if (hStdOut == INVALID_HANDLE_VALUE) {
        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hStdOut == INVALID_HANDLE_VALUE) {
            return 0;
        }

        unicodeBufferSize = 0x200;  // initial Unicode buffer size
        unicodeBuffer = LocalAlloc(LMEM_FIXED, unicodeBufferSize);
        ansiBufferSize = 0x100;     // initial ANSI buffer size
        ansiBuffer = LocalAlloc(LMEM_FIXED, ansiBufferSize);

        if(!unicodeBuffer || !ansiBuffer) {
            hStdOut = INVALID_HANDLE_VALUE;
            return 0;
        }
    }

    // format string
    va_start(arglist, Format);
    while ((unicodeCharCount = _vsnwprintf(unicodeBuffer,
                                           unicodeBufferSize / sizeof(unicodeBuffer[0]),
                                           Format,
                                           arglist)) < 0) {
        LocalFree(unicodeBuffer);
        unicodeBufferSize *= 2;
        unicodeBuffer = LocalAlloc(LMEM_FIXED, unicodeBufferSize);
        if(!unicodeBuffer) {
            hStdOut = INVALID_HANDLE_VALUE;
            LocalFree(ansiBuffer);
            return 0;
        }
    }
    va_end(arglist);

    // convert to ansi character
    while ((ansiByteCount = WideCharToMultiByte(CP_ACP,
                                                0,
                                                unicodeBuffer,
                                                unicodeCharCount,
                                                ansiBuffer,
                                                ansiBufferSize,
                                                NULL,
                                                NULL)) == 0) {
        LocalFree(ansiBuffer);
        ansiBufferSize *= 2;
        ansiBuffer = LocalAlloc(LMEM_FIXED, ansiBufferSize);
        if(!ansiBuffer) {
            hStdOut = INVALID_HANDLE_VALUE;
            LocalFree(unicodeBuffer);
            return 0;
        }
    }

    // convert return code and write to stdout
    outputByteCount = ansiByteCount;
    pp = 0;
    for (i = 0 ; i < ansiByteCount ; i++) {
        if (ansiBuffer[i] == '\n' && (i == 0 || ansiBuffer[i - 1] != '\r')) {
            if (!WriteFile(hStdOut, &ansiBuffer[pp], i - pp, &written, NULL)
                || !WriteFile(hStdOut, "\r", 1, &written, NULL)) {
                return 0;
            }
            outputByteCount++;
            pp = i;
        }
    }
    if (!WriteFile(hStdOut, &ansiBuffer[pp], ansiByteCount - pp, &written, NULL)) {
        return 0;
    }
    return outputByteCount;
}


UINT32
WriteMsgToLog(
    IN KanalyzeLogLevel LogLevel,
    IN UINT32 MessageId,
    ...
    )
{
    UINT32 Count;
    va_list Arguments;

    va_start(Arguments,MessageId);
    Count = KxWriteToLog(hPlugIn, LogLevel, FALSE, NULL, MessageId, &Arguments);
    va_end(Arguments);

    return Count;
}



UINT32
WriteTextToLog(
    IN KanalyzeLogLevel LogLevel,
    IN LPCWSTR Format,
    ...
    )
{
    UINT32 Count;
    va_list Arguments;

    va_start(Arguments, Format);
    Count = KxWriteToLog(hPlugIn, LogLevel, FALSE, Format, 0, &Arguments);
    va_end(Arguments);

    return Count;
}

