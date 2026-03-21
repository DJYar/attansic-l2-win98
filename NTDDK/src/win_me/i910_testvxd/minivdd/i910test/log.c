#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "log.h"

#pragma VxD_LOCKED_CODE_SEG
#pragma VxD_LOCKED_DATA_SEG

#define I910_LOG_PATH_PRIMARY   "C:\\I910TEST.LOG"
#define I910_LOG_PATH_FALLBACK  "C:\\WINDOWS\\I910TEST.LOG"

typedef DWORD HFILE32;

extern DWORD _stdcall IFSMgr_Ring0_OpenCreateFile(
    DWORD pFileName,
    DWORD access,
    DWORD share,
    DWORD attrs,
    DWORD action,
    DWORD reserved,
    DWORD pHandle);

extern DWORD _stdcall IFSMgr_Ring0_WriteFile(
    DWORD hFile,
    DWORD pBuffer,
    DWORD cbToWrite,
    DWORD pBytesWritten,
    DWORD reserved);

extern DWORD _stdcall IFSMgr_Ring0_CloseFile(DWORD hFile);

static HFILE32 g_logFile;
static DWORD g_logCounter;
static DWORD g_logReady;

static BOOL
I910_TryOpenPath(const char *path)
{
    DWORD status;
    DWORD action = 0;

    status = IFSMgr_Ring0_OpenCreateFile((DWORD)path,
                                         0x40000000,
                                         0x00000001,
                                         0,
                                         0x00000012,
                                         0,
                                         (DWORD)&g_logFile);
    if (status != 0)
    {
        g_logFile = 0;
        return FALSE;
    }

    g_logReady = 1;
    g_logCounter = 0;
    UNREFERENCED_PARAMETER(action);
    return TRUE;
}

BOOL
I910_LogInitEarly(void)
{
    if (g_logReady)
    {
        return TRUE;
    }

    if (I910_TryOpenPath(I910_LOG_PATH_PRIMARY) || I910_TryOpenPath(I910_LOG_PATH_FALLBACK))
    {
        I910_Log("log: opened");
        return TRUE;
    }

    return FALSE;
}

void
I910_Log(const char *fmt, ...)
{
    char body[256];
    char line[320];
    va_list ap;
    DWORD len;
    DWORD written = 0;

    if (!g_logReady)
    {
        return;
    }

    va_start(ap, fmt);
    _vsnprintf(body, sizeof(body) - 1, fmt, ap);
    body[sizeof(body) - 1] = '\0';
    va_end(ap);

    g_logCounter++;
    _snprintf(line, sizeof(line) - 1, "[%08lu] %s\r\n", g_logCounter, body);
    line[sizeof(line) - 1] = '\0';

    len = (DWORD)strlen(line);
    IFSMgr_Ring0_WriteFile(g_logFile, (DWORD)line, len, (DWORD)&written, 0);
}
