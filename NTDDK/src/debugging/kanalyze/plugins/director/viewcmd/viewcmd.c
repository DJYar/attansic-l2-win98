/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    viewcmd.c

Abstract:

    VIEWCMD Plug-In for Kernel Memory Space Analyzer.

Author:


Revision History:

--*/


#include "precomp.h"
#pragma hdrstop


//
// global
//
HKAPLUGIN hPlugIn;
KANALYZE_EXTENSION_APIS KanalyzeExtensionApis;

LIST_ENTRY ItemIdHead;
UINT32 Depth;

//
// type
//
typedef struct _STORED_ITEM_ID {
    KA_ITEM_ID ItemId;
    LIST_ENTRY List;
} STORED_ITEM_ID, *PSTORED_ITEM_ID;

typedef struct _LINK_ITEM_CONTEXT {
    KA_ITEM_ID DataItemId;
    BOOL fLinkTo;
    UINT32 Indent;
    USHORT InputDir;
} LINK_ITEM_CONTEXT, *PLINK_ITEM_CONTEXT;

//
// defines
//
#define MAX_BUFFER_SIZE 1024
#define MAX_CMD_NAME 128
#define LINK_MIXED 0
#define LINK_FORWARD 1
#define LINK_BACK 2

//
// prototypes
//
VOID
CALLBACK
KanalyzeRegisterPlugIns(
    IN KANALYZE_REGISTER_PLUGIN_PROC RegistrationRoutine,
    IN ULONG32 CrashPlatform,
    IN ULONG32 CrashBuildNumber,
    PVOID Reserved1,
    UINT_PTR Reserved2
    );

UINT_PTR
CALLBACK
KanalyzeInterface(
    IN HKAPLUGIN PlugInHandle,
    IN UINT32 OperationCode,
    IN OUT UINT_PTR Param1,
    IN OUT PVOID Param2
    );

UINT32
ViewCmd (
    VOID
    );

VOID
ExecTypeCmd (
    LPWSTR CmdBuffer
    );

VOID
ExecViewCmd (
    LPWSTR CmdBuffer
    );

VOID
ExecFindCmd (
    LPWSTR CmdBuffer
    );

BOOL
IsAddress (
    LPCWSTR Str
    );

UINT32
EnumSpecifiedItem (
    IN PKA_ENUM_STORED_ITEM_SPECIFICATION ItemSpec,
    IN BOOL Verbosity
    );

UINT32
DisplayItem (
    IN KA_ITEM_ID DataItemId,
    IN BOOL Verbosity
    );

UINT32
DisplayType (
    IN HKATYPE TypeHandle,
    BOOL DetailLevel
    );

UINT32
CALLBACK
EnumTypeCallBack (
    IN HKAPLUGIN h,
    IN HKATYPE TypeHandle,
    IN PVOID Context
    );

UINT32
CALLBACK
EnumItemCallBack (
    IN HKAPLUGIN h,
    IN KA_ITEM_ID DataItemId,
    IN OUT PVOID Context
    );

VOID
PrintHelp (
    VOID
    );

VOID
ExecLinkItemCmd (
    LPWSTR CmdBuffer
    );

UINT32
CALLBACK
LinkItemCallback(
    IN HKAPLUGIN h,
    IN CONST KA_LINK_INFO *LinkInfo,
    IN PLINK_ITEM_CONTEXT Context
    );

UINT32
EnumLinkItem (
    KA_ITEM_ID DataItemId,
    USHORT InputDir
    );

UINT32
AddIdToList (
    KA_ITEM_ID DataItemId,
    BOOL *Exist
    );

VOID
FreeItemIdList (
    VOID
    );

VOID
HexDumpDWORD(
    UINT_PTR Start,
    ULONG Num
    );

VOID
HexDumpBYTE(
    UINT_PTR Start,
    ULONG Num
    );

VOID
ExecDumpMemoryCmd(
    ULONG32 Unit,
    LPWSTR CmdBuffer
    );

BOOL
ExecExtensionCmd(
    LPCWSTR PlugInName,
    LPCWSTR Arguments
    );


VOID
CALLBACK
KanalyzeRegisterPlugIns(
    IN KANALYZE_REGISTER_PLUGIN_PROC RegistrationRoutine,
    IN ULONG32 CrashPlatform,
    IN ULONG32 CrashBuildNumber,
    PVOID Reserved1,
    UINT_PTR Reserved2
    )

/*++

Routine Description:
    This routine is called by Kanalyze to register plug-in.
    plug-in DLL must call KANALYZE_REGISTER_PLUGIN_PROC for each plug-in
    to pass interface routine address, and to get KanalyzeExtensionApis and plug-in's handle.

Arguments:
    RegistrationRoutine - supplies the address of a routine that the plug-in DLL must call
                          to register each plug-in it contains

    CrashPlatform       - supplies a value indicating which platform the crash dump is from
                          (i.e., x86 machine, Alpha machine, etc).
                          This is one of the IMAGE_FILE_MACHINE_xxx constants from ntimage.h.

    CrashBuildNumber    - supplies the internal Microsoft build number of the system that crashed.
                          1057 = NT3.51; 1381 = NT4.0.

    Reserved1,Reserved2 - reserved for future expansion

Return Value:
    NONE.

--*/

{

    //  this Plug-In DLL(viewcmd.dll) provide a single plug-in named "VIEWCMD"
    hPlugIn = (RegistrationRoutine)( KANALYZE_INTERFACE_VERSION,
                                     L"VIEWCMD",
                                     KanalyzeInterface,
                                     &KanalyzeExtensionApis,
                                     (PVOID)NULL,
                                     (UINT_PTR)NULL );

    return;
}



UINT_PTR
CALLBACK
KanalyzeInterface(
    IN HKAPLUGIN PlugInHandle,
    IN UINT32 OperationCode,
    IN OUT UINT_PTR Param1,
    IN OUT PVOID Param2
    )

/*++

Routine Description:
    This routine is called by Kanalyze and is expected to complete the request.

Arguments:
    PlugInHandle  - Spplies an identifier assigned to the plug-in at registration time by Kanalyze
    OperationCode - Supplies a numeric identifier indicating the action to be performed by the plug-in
    Param1,Param2 - OperationCode-specefic arguments

Return Value:
    The return value is dependent on OperationCode.
    If a plug-in does not recognize a particular OperationCode, it MUST return ERROR_INVALID_FUNCTION.
    Win32 error code. NO_ERROR indicates success

--*/

{
    if (PlugInHandle != hPlugIn) {
        return NO_ERROR;
    }

    switch (OperationCode) {
        case KA_INITIALIZE:
            return NO_ERROR; // do nothing

        case KA_REGISTER_TYPES:
            return NO_ERROR; // do nothing

        case KA_START_LOCATE_ITEMS:
            return NO_ERROR; // do nothing

        case KA_PROCESS_ITEM:
            return ERROR_INVALID_FUNCTION; // support no type

        case KA_NOTIFY_ITEM:
            return ERROR_INVALID_FUNCTION; // not demand notification

        case KA_TERMINATE:
            return NO_ERROR; // do nothing

        case KA_FORMAT_DATA_ITEM_DESCRIPTION:
            return ERROR_INVALID_FUNCTION; // support no type

        case KA_PROCESS_RESULT:
            return ViewCmd();

        default: //  if unknown operation code is given, return ERROR_INVALID_FUNCTION
            return ERROR_INVALID_FUNCTION;
    }
}



UINT32
ViewCmd (
    VOID
    )
/*++

Routine Description:
    Read string from command prompt and dispatch appropriate routine.
Arguments:
    NONE
Return Value:
    Win32 Error Code

--*/
{
    WCHAR Buffer[MAX_BUFFER_SIZE];
    int i;
    WCHAR *p,*q;

    WriteText(L"\nKANALYZE>");

    while(1) {
        if (!fgetws(Buffer,MAX_BUFFER_SIZE,stdin)) {
            // pressed Ctrl+C
            if (feof (stdin)) {
                WriteText(L"\nKANALYZE>");
                continue;
            }
        }
        
        //Clear Cancel Flag(in kanalyze.exe)
        KxCheckBreak (hPlugIn, 0);
        
        //
        // Strip off any trailing cr/lf.
        //
        i = lstrlen(Buffer);
        while(i && ((Buffer[i-1] == L'\n') || (Buffer[i-1] == L'\r'))) {
            i--;
        }
        if(i) {
            Buffer[i] = 0;

            //
            // Isolate first token on line and find the start
            // of the command tail.
            //
            p = Buffer;
            while(iswspace(*p)) {
                p++;
            }
            if(*p) {
                q = p+1;
                while(*q && !iswspace(*q)) {
                    q++;
                }

                if(*q) {
                    *q++ = 0;
                    while(iswspace(*q)) {
                        q++;
                    }
                }

                if(!lstrcmpi(p,L"exit")) {
                    return ERROR_CANCELLED; // exit (BUGBUG)
                }

                if (!lstrcmpi(p,L"q")) {
                    return NO_ERROR; // go through next director
                }

                if(!lstrcmpi(p,L"type")) {
                    ExecTypeCmd(q);
                } else if(!lstrcmpi(p,L"view")) {
                    ExecViewCmd(q);
                } else if(!lstrcmpi(p,L"find")) {
                    ExecFindCmd(q);
                } else if(!lstrcmpi(p,L"link")) {
                    ExecLinkItemCmd(q);
                } else if (!lstrcmpi(p,L"dd")) {
                    ExecDumpMemoryCmd(4, q);
                } else if (!lstrcmpi(p,L"db")) {
                    ExecDumpMemoryCmd(1, q);
                } else if (!lstrcmpi(p,L"d")) {
                    ExecDumpMemoryCmd(0, q);
                } else {
                    if (!ExecExtensionCmd(p, q)) {
                        PrintHelp();
                    }
                }
            }
        }

        WriteText(L"\nKANALYZE>");
    }

    return NO_ERROR;
}



VOID
ExecTypeCmd(
    IN LPWSTR CmdBuffer
    )
/*++

Routine Description:
    Type command routine.
Arguments:
    CmdBuffer - argument of Type command
Return Value:
    NONE

--*/
{
    UINT32 rc;
    BOOL DetailLevel;
    int Argc;
    LPWSTR *Argv;
    HKATYPE TypeHandle;

    WriteMsg(MSG_TITLE_TYPE_CMD);

    if (!_wcsicmp (CmdBuffer, L"")) {
        DetailLevel = FALSE;
        rc = KxEnumTypeInfo (hPlugIn,
                             L"*",
                             0,
                             0,
                             (PKANALYZE_ENUM_TYPE_INFO_CALLBACK)EnumTypeCallBack,
                             (PVOID)&DetailLevel);
        if (rc != NO_ERROR && rc != ERROR_CANCELLED) {
            WriteMsgToLog(LogErrors,MSG_CANNOT_ENUM_TYPE, L"*", rc);
        }
        return;
    }
    
    if ((Argv=CommandLineToArgvW(CmdBuffer, &Argc)) == NULL) {
        WriteMsgToLog (LogErrors, MSG_FAIL_PARSE_COMMAND_LINE, CmdBuffer, GetLastError());
        return;
    }
    
    if (!_wcsicmp (*Argv, L"-h")) {
        if (Argc >= 2) {
            if (swscanf(*(Argv+1), L"%x", &TypeHandle) <= 0) {
                goto End_ExecTypeCmd;
            }
            DisplayType (TypeHandle, TRUE);
        }
        goto End_ExecTypeCmd;
    }
    
    if ((*Argv)[wcslen(*Argv)-1] == L'*') {
        DetailLevel = FALSE;
    } else {
        DetailLevel = TRUE;
    }
    rc = KxEnumTypeInfo (hPlugIn,
                         *Argv,
                         0,
                         0,
                         (PKANALYZE_ENUM_TYPE_INFO_CALLBACK)EnumTypeCallBack,
                         (PVOID)&DetailLevel);
    if (rc != NO_ERROR && rc != ERROR_CANCELLED) {
        WriteMsgToLog(LogErrors,MSG_CANNOT_ENUM_TYPE, *Argv, rc);
    }
    goto End_ExecTypeCmd;

End_ExecTypeCmd:
    GlobalFree (Argv);
    return;
}



UINT32
CALLBACK
EnumTypeCallBack (
    IN HKAPLUGIN h,
    IN HKATYPE TypeHandle,
    IN PVOID *DetailLevel
    )
/*++

Routine Description:
    Recieve type handle from ExecTypeCmd() through KxEnumTypeInfo() and pass it to routine which
    prints out information of type.
Arguments:
    PlugInHandle  - Spplies an identifier assigned to the plug-in at registration time by Kanalyze
    TypeHandle - enumerated handle of type.
    DetailLevel - represent what should be printed out.
Return Value:
    Win32 Error Code.

--*/
{
    UINT32 rc = NO_ERROR;

    rc = DisplayType (TypeHandle, *((BOOL *)DetailLevel));
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_MISC_ROUTINE_FAIL, L"DisplayType()", rc);
        }
        return rc;
    }

    if (KxCheckBreak(h, 0)) {
        return ERROR_CANCELLED;
    }

    return NO_ERROR;
}



UINT32
DisplayType (
    IN HKATYPE TypeHandle,
    BOOL DetailLevel
    )
/*++

Routine Description:
    Prints out information of type.
Arguments:
    TypeHandle - handle of type to be processed.
    DetailLevel - represent what should be printed out.
Return Value:
    Win32 Error Code.

--*/
{
    UINT32 rc = NO_ERROR;
    KA_TYPE_INFO TypeInfo, ContainerTypeInfo;
    ULONG32 Count;

    rc = KxQueryTypeInfo(hPlugIn, TypeHandle, 0, &TypeInfo);
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog(LogErrors, MSG_CANNOT_GET_TYPEINFO, TypeHandle, rc);
        }
        return rc;
    }

    WriteMsg(MSG_TYPE_INFO,
             TypeHandle,
             TypeInfo.TypeName,
             wcscmp(TypeInfo.SupportPlugInName,L"") ? TypeInfo.SupportPlugInName : L"-NONE-");

    if (DetailLevel == FALSE) {
        return NO_ERROR;
    } else {
        WriteText(L"\n");
        WriteMsg(MSG_ALLOWED_CONTAINER_TYPES);
        WriteMsg(MSG_TITLE_TYPE_CMD);
        for (Count=0; Count<TypeInfo.ContainerTypeCount; Count++) {
            rc = KxQueryTypeInfo(
                     hPlugIn,
                     TypeInfo.AllowedContainerType[Count],
                     (UINT_PTR)0,
                     &ContainerTypeInfo
                     );
            if (rc != NO_ERROR) {
                if (rc != ERROR_CANCELLED) {
                    WriteMsgToLog (LogErrors, MSG_CANNOT_GET_TYPEINFO, TypeInfo.AllowedContainerType[Count], rc);
                }
                return rc;
            }
            WriteMsg(MSG_TYPE_INFO,
                     TypeInfo.AllowedContainerType[Count],
                     ContainerTypeInfo.TypeName,
                     wcscmp(ContainerTypeInfo.SupportPlugInName,L"") ? ContainerTypeInfo.SupportPlugInName : L"-NONE-");
        }
        WriteText(L"\n");
        WriteMsg(MSG_TITLE_DESC_TYPE);
        WriteText(L"%s\n",TypeInfo.Description);
    }
    
    return NO_ERROR;
}



VOID
ExecViewCmd (
    LPWSTR CmdBuffer
    )
{
    KA_ITEM_ID DataItemId;
    UINT32 rc = NO_ERROR;

    if (!_wcsicmp (CmdBuffer, L"")) {
        return;
    }

    if (swscanf(CmdBuffer, L"%x", &DataItemId) <= 0) {
        return;
    }
            
    WriteMsg(MSG_TITLE_VIEW_ITEM_CMD);
    rc = DisplayItem (DataItemId, TRUE);
    if (rc != NO_ERROR && rc != ERROR_CANCELLED) {
        WriteMsgToLog (LogErrors, MSG_MISC_ROUTINE_FAIL, L"DisplayItem()", rc);
    }

    return;
}



VOID
ExecFindCmd (
    LPWSTR CmdBuffer
    )
/*++

Routine Description:
    Find command routine.
Arguments:
    CmdBuffer - argument of Type command
Return Value:
    NONE

--*/
{
    KA_ENUM_STORED_ITEM_SPECIFICATION ItemSpec;
    int Argc;
    LPWSTR *Argv;
    WCHAR Buff[MAX_PATH];
    BOOL Verbosity = FALSE;
    UINT32 rc = NO_ERROR;

    ItemSpec.Flags = 0;
    ItemSpec.ReportedPlugIn = L"";
    ItemSpec.TypeSpecification = L"*";
    ItemSpec.StartAddress = 0;
    ItemSpec.EndAddress = 0xFFFFFFFF;
    ItemSpec.TypeSpecificParam = 0;

    
    if (!_wcsicmp (CmdBuffer, L"")) {
        WriteMsg(MSG_TITLE_VIEW_ITEM_CMD);
        EnumSpecifiedItem (&ItemSpec, Verbosity);
        return;
    }
    
    if ((Argv=CommandLineToArgvW(CmdBuffer, &Argc)) == NULL) {
        WriteMsgToLog (LogErrors, MSG_FAIL_PARSE_COMMAND_LINE, CmdBuffer, GetLastError());
        return;
    }
    
    if (IsAddress (*Argv)) {
        if (swscanf(*Argv, L"%x", &ItemSpec.StartAddress) <= 0) {
            goto End_ExecFindCmd;
        }
        ItemSpec.EndAddress = ItemSpec.StartAddress;
        if (Argc >= 2) {
            if (!_wcsicmp (*(Argv+1), L"-o")) {
                ItemSpec.Flags |= KA_ESI_FLAG_ENUM_ALL_OVERLAPPED_ITEMS;
                if (Argc >= 3 && !_wcsicmp (*(Argv+2), L"-v")) {
                    Verbosity = TRUE;
                }
            } else if (!_wcsicmp (*(Argv+1), L"-v")) {
                Verbosity = TRUE;
            } else {
                if (swscanf(*(Argv+1), L"%x", &ItemSpec.EndAddress) <= 0) {
                    goto End_ExecFindCmd;
                }
                if (Argc >= 3) {
                    if (!_wcsicmp (*(Argv+2), L"-o")) {
                        ItemSpec.Flags |= KA_ESI_FLAG_ENUM_ALL_OVERLAPPED_ITEMS;
                        if (Argc >= 4 && !_wcsicmp (*(Argv+3), L"-v")) {
                                Verbosity = TRUE;
                        }
                    } else if (!_wcsicmp (*(Argv+2), L"-v")) {
                        Verbosity = TRUE;
                    } 
                }
            }
        }
    // TypeName is given.
    } else {
        if (swscanf(*Argv, L"%s", &Buff) <= 0) {
            goto End_ExecFindCmd;
        }
        ItemSpec.TypeSpecification = Buff;
        if (Argc >= 2) {
            if (!_wcsicmp (*(Argv+1), L"-v")) {
                Verbosity = TRUE;
            } else {
                if (swscanf(*(Argv+1), L"%x", &ItemSpec.StartAddress) <= 0) {
                    goto End_ExecFindCmd;
                }
                ItemSpec.EndAddress = ItemSpec.StartAddress;
                if (Argc >=3) {
                    if (!_wcsicmp (*(Argv+2), L"-o")) {
                        ItemSpec.Flags |= KA_ESI_FLAG_ENUM_ALL_OVERLAPPED_ITEMS;
                        if (Argc >= 4 && !_wcsicmp (*(Argv+3), L"-v")) {
                            Verbosity = TRUE;
                        }
                    } else if (!_wcsicmp (*(Argv+2), L"-v")) {
                        Verbosity = TRUE;
                    } else {
                        if (swscanf(*(Argv+2), L"%x", &ItemSpec.EndAddress) <= 0) {
                            goto End_ExecFindCmd;
                        }
                        if (Argc >= 4) {
                            if (!_wcsicmp (*(Argv+3), L"-o")) {
                                ItemSpec.Flags |= KA_ESI_FLAG_ENUM_ALL_OVERLAPPED_ITEMS;
                                if (Argc >= 5 && !_wcsicmp (*(Argv+4), L"-v")) {
                                    Verbosity = TRUE;
                                }
                            } else if (!_wcsicmp (*(Argv+3), L"-v")) {
                                Verbosity = TRUE;
                            }
                        }
                    }
                }
            }
        }
    }

    WriteMsg(MSG_TITLE_VIEW_ITEM_CMD);
    rc = EnumSpecifiedItem (&ItemSpec, Verbosity);
    if (rc != NO_ERROR && rc != ERROR_CANCELLED) {
        WriteMsgToLog (LogErrors, MSG_MISC_ROUTINE_FAIL, L"EnumSpecifiedItem()", rc);
    }

End_ExecFindCmd:
    GlobalFree (Argv);
    return;
}


BOOL
IsAddress (
    LPCWSTR Str
    )
/*++

Routine Description:
    Check given string represent address or not.
Arguments:
    CmdBuffer - part of argument for Find command
Return Value:
    If Str represent address return TRUE. If not FALSE.

--*/
{
    UINT32 i=0, j=0;
    
    if (*(Str)==L'0' && (*(Str+1)==L'x' || *(Str+1)==L'X')) {
        j = 2;
    }
    
    for (i=j; i < wcslen(Str); i++) {
        if (!iswxdigit (Str[i])) {
            return FALSE;
         }
    }
    return TRUE;
}



UINT32
EnumSpecifiedItem (
    IN PKA_ENUM_STORED_ITEM_SPECIFICATION ItemSpec,
    IN BOOL Verbosity
    )
/*++

Routine Description:
    Enumerate item. Condition is given from argument of find command.
Arguments:
    ItemSpec - condition to search item.
    Verbosity - indicate what should be printed out for specified item.
Return Value:
    Win32 Error Code..

--*/
{
    UINT32 rc = NO_ERROR;
    
    rc = KxEnumStoredItem (hPlugIn,
                           ItemSpec,
                           0,
                           0,
                           (PKANALYZE_ENUM_STORED_ITEM_CALLBACK)EnumItemCallBack,
                           &Verbosity);
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_CANNOT_ENUM_ITEM, ItemSpec->TypeSpecification, rc);
        }
        return rc;
    }
    
    return NO_ERROR;
}




UINT32
CALLBACK
EnumItemCallBack (
    IN HKAPLUGIN h,
    IN KA_ITEM_ID DataItemId,
    IN OUT PVOID Context
    )
/*++

Routine Description:
    Receive a enumerated item in codition specified at Find command.
Arguments:
    PlugInHandle - Spplies an identifier assigned to the plug-in at registration time by Kanalyze
    DataItemId - Enumerated item.
    Context - DetailLevel that indicate what should be printed out.
Return Value:

--*/
{

    UINT32 rc = NO_ERROR;
    
    rc = DisplayItem (DataItemId, *((BOOL *)Context));
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_MISC_ROUTINE_FAIL, L"DisplayItem()", rc);
        }
        return rc;
    }

    if (KxCheckBreak (h,0)) {
        return ERROR_CANCELLED;
    }

    return NO_ERROR;
}



UINT32
DisplayItem (
    IN KA_ITEM_ID DataItemId,
    IN BOOL Verbosity
    )
/*++

Routine Description:
    Print out information of spedified item.
Arguments:
    DataItemId - item id to be printed out.
    Verbosity - indicate what should be printed out.
Return Value:

--*/
{
    UINT32 rc = NO_ERROR;
    KA_DATA_ITEM_INFO DataItemInfo;
    KA_TYPE_INFO TypeInfo;
    SIZE_T CharactersWritten = 0;
    LPWSTR Description;

    rc = KxQueryDataItem(hPlugIn, DataItemId, 0, &DataItemInfo);
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_CANNOT_GET_ITEMINFO, DataItemId, rc);
        }
        return rc;
    }

    rc = KxQueryTypeInfo (hPlugIn, DataItemInfo.TypeHandle, 0, &TypeInfo);
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_CANNOT_GET_TYPEINFO, DataItemInfo.TypeHandle, rc);
        }
        return rc;
    }

    WriteMsg(MSG_ITEM_INFO,
             DataItemInfo.Address,
             DataItemInfo.Size,
             DataItemId,
             TypeInfo.TypeName,
             DataItemInfo.ReportedPlugInName);
    
    if (Verbosity) {
        WriteMsg(MSG_TITLE_DESC_ITEM_CMD);
        
        // First invoking KxGetDataItemDescription to get the size of required buffer.
        rc = KxGetDataItemDescription(
                 hPlugIn,
                 DataItemId,
                 0,                     //Size of buffer
                 NULL,                  //Pointer to buffer are not specified.
                 &CharactersWritten     //We need only CharactersWritten now.
                 );
        if (rc != NO_ERROR) {
            if (rc == ERROR_CANCELLED) {
                return rc;
            } else if (rc != ERROR_MORE_DATA) {
                // Even if error returned(except ERROR_CANCELLED), go to next item. (#RAID356)
                WriteMsgToLog (LogErrors, MSG_CANNOT_GET_DESCRIPTION, DataItemId, rc);
                return NO_ERROR;
            }
        }

        if (!(Description = LocalAlloc (LPTR,CharactersWritten * sizeof(WCHAR)))) {
            rc = GetLastError();
            WriteMsgToLog (LogErrors, MSG_CANNOT_ALLOC_MEMORY, L"DisplayItem()", CharactersWritten * sizeof(WCHAR), rc);
            return rc;
        }

        // Second invoking KxGetDataItemDescription to get the description.
        rc = KxGetDataItemDescription(
                 hPlugIn,
                 DataItemId,
                 CharactersWritten,                   //Actually description
                 Description,                         //associated with DataItemId is copied
                 &CharactersWritten                   //to buffer(Description).
                 );
        if (rc != NO_ERROR) {
            if (rc == ERROR_CANCELLED) {
                LocalFree (Description);
                return rc;
            } else {
                // Even if error returned(except ERROR_CANCELLED), go to next item. (#RAID356)
                WriteMsgToLog (LogErrors, MSG_CANNOT_GET_DESCRIPTION, DataItemId, rc);
                LocalFree (Description);
                return NO_ERROR;
            }
        } else {
            WriteText(L"%s\n",Description);
            WriteMsg(MSG_LINE);
            LocalFree(Description);
        }
    }

    return NO_ERROR;
}




VOID
PrintHelp(
    VOID
    )
/*++

Routine Description:
    Print out help.
Arguments:
    NONE
Return Value:
    NONE

--*/
{
    WriteMsg(MSG_HELP);
    return;
}



VOID
HexDumpDWORD(
    UINT_PTR Start,
    ULONG Num
    )
/*++

Routine Description:
    Dump value on dumpfile.
Arguments:
    Start - start address
    Num   - range
Return Value:
    NONE

--*/
{
    UINT32 rc;
    WCHAR hexstring[80];
    WCHAR bytestring[20];
    WCHAR ch;
    ULONG Data;
    UINT_PTR Address;
    SIZE_T i, j, BytesRead;
    WCHAR s[10];

    i = 0;
    Address = Start;
    wsprintf(s, L"%08x ", Address);
    lstrcpy(hexstring, s);
    lstrcpy(bytestring, L"");

    while (Num) {
        if (KxCheckBreak(hPlugIn, 0)) return;

        rc = KxReadMemory(hPlugIn, NULL, Address, &Data, sizeof(ULONG), &BytesRead);
        if (rc == NO_ERROR) {
            wsprintf(s, L" %08x", Data);
            lstrcat(hexstring, s);

            for (j = 0; j < 4; j++) {
                ch = (WCHAR)((Data >> (j * 8)) & 0xff);
                if (ch < 0x20 || ch > 0x7e) {
                    ch = L'.';
                }
                wsprintf(s, L"%c", ch);
                lstrcat(bytestring, s);
            }

        } else {
            lstrcat(hexstring, L" ????????");
            lstrcat(bytestring, L"????");
        }

        i++;
        Address += sizeof(ULONG);
        Num--;

        if (i == 4) {
            WriteText(hexstring);
            WriteText(L"   ");
            WriteText(bytestring);
            WriteText(L"\n");

            wsprintf(s, L"%08x ", Address);
            lstrcpy(hexstring, s);
            lstrcpy(bytestring, L"");
            i = 0;
        }
    }

    if (i != 0) {
        WriteText(hexstring);
        for (j = 0; j < (4 - i); j++) {
            WriteText(L"         ");
        }
        WriteText(L"   ");
        WriteText(bytestring);
        WriteText(L"\n");
    }

    return;
}


VOID
HexDumpBYTE(
    UINT_PTR Start,
    ULONG Num
    )
/*++

Routine Description:
    Dump value on dumpfile.
Arguments:
    Start - start address
    Num   - range
Return Value:
    NONE

--*/
{
    UINT32 rc;
    WCHAR hexstring[80];
    WCHAR bytestring[20];
    WCHAR ch;
    BYTE Data;
    UINT_PTR Address;
    SIZE_T i, j, BytesRead;
    WCHAR s[10];

    i = 0;
    j = 0;
    Address = Start;
    wsprintf(s, L"%08x ", Address);
    lstrcpy(hexstring, s);
    lstrcpy(bytestring, L"");

    while (Num) {
        if (KxCheckBreak(hPlugIn, 0)) return;

        rc = KxReadMemory(hPlugIn, NULL, Address, &Data, sizeof(BYTE), &BytesRead);
        if (rc == NO_ERROR) {
            wsprintf(s, L" %02x", Data);
            lstrcat(hexstring, s);

            ch = (WCHAR)((Data >> (j * 8)) & 0xff);
            if (ch < 0x20 || ch > 0x7e) {
                ch = L'.';
            }
            wsprintf(s, L"%c", ch);
            lstrcat(bytestring, s);

        } else {
            lstrcat(hexstring, L" ??");
            lstrcat(bytestring, L"?");
        }

        i++;
        Address += sizeof(BYTE);
        Num--;

        if (i == 8) {
            lstrcat(hexstring, L" -");
        }

        if (i == 16) {
            WriteText(hexstring);
            WriteText(L"   ");
            WriteText(bytestring);
            WriteText(L"\n");

            wsprintf(s, L"%08x ", Address);
            lstrcpy(hexstring, s);
            lstrcpy(bytestring, L"");
            i = 0;
        }

    }

    if (i != 0) {
        WriteText(hexstring);
        for (j = 0; j < (16 - i); j++) {
            WriteText(L"   ");
        }
        WriteText(L"   ");
        WriteText(bytestring);
        WriteText(L"\n");
    }

    return;
}



VOID
ExecDumpMemoryCmd(
    ULONG32 Unit,
    LPWSTR CmdBuffer
    )
/*++

Routine Description:
    Dump value on dumpfile.
Arguments:
    Unit - indicate that dump with DWORD or BYTE
    CmdBuffer - argument of D{B|D} command
Return Value:
    NONE

--*/
{
    UINT_PTR Address;
static UINT_PTR PrevAddr;
static ULONG32 PrevUnit;
    ULONG Num;

    Address = PrevAddr; Num = 32;

    swscanf(CmdBuffer, L"%x %x", &Address, &Num);
    if (Unit == 0) Unit = PrevUnit;
    if (Unit == 1) {
        HexDumpBYTE(Address, Num);
        PrevAddr = Address + Num * sizeof(BYTE);
        PrevUnit = 1;
    } else {
        HexDumpDWORD(Address, Num);
        PrevAddr = Address + Num * sizeof(ULONG);
        PrevUnit = 4;
    }
}



VOID
ExecLinkItemCmd (
    LPWSTR CmdBuffer
    )
/*++

Routine Description:
    Link command routine.
Arguments:
    CmdBuffer - argument of Type command
Return Value:
    NONE

--*/
{
    KA_ITEM_ID ItemId;
    PSTORED_ITEM_ID StoredItemId;
    KA_DATA_ITEM_INFO ItemInfo;
    KA_TYPE_INFO TypeInfo;
    int Argc;
    LPWSTR *Argv;
    WCHAR StrOption[MAX_PATH];
    USHORT InputDir = LINK_BACK;
    UINT32 rc = NO_ERROR;

    ZeroMemory (&ItemInfo, sizeof(KA_DATA_ITEM_INFO));
    ZeroMemory (&TypeInfo, sizeof(KA_TYPE_INFO));

    Depth = 0;
    if (!_wcsicmp (CmdBuffer, L"")) {
        return;
    }
    
    if ((Argv=CommandLineToArgvW(CmdBuffer, &Argc)) == NULL) {
        WriteMsgToLog (LogErrors, MSG_FAIL_PARSE_COMMAND_LINE, CmdBuffer, GetLastError());
        return;
    }
    
    if (swscanf(*Argv, L"%x", &ItemId) <= 0) {
        return;
    }
    
    if (Argc >= 2) {
        if (!_wcsicmp (*(Argv+1), L"-f")) {
            InputDir = LINK_FORWARD;
            Depth = 1;
        } else if (!_wcsicmp (*(Argv+1), L"-b")) {
            InputDir = LINK_BACK;
            Depth = 1;
        } else if (!_wcsicmp (*(Argv+1), L"-m")) {
            InputDir = LINK_MIXED;
            Depth = 1;
        } else {
            if (swscanf(*(Argv+1), L"%x", &Depth) <= 0) {
                return;
            }
        }
        if (Argc >= 3) {
            if (!_wcsicmp (*(Argv+2), L"-f")) {
                InputDir = LINK_FORWARD;
            } else if (!_wcsicmp (*(Argv+2), L"-b")) {
                InputDir = LINK_BACK;
            } else if (!_wcsicmp (*(Argv+2), L"-m")) {
                InputDir = LINK_MIXED;
            }
        }
    }
    
    if (Depth == 0) {
        Depth = 1;
    }
    
    rc = KxQueryDataItem (hPlugIn, ItemId, 0, &ItemInfo);
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_CANNOT_GET_ITEMINFO, ItemId, rc);
        }
        return;
    }

    rc = KxQueryTypeInfo (hPlugIn, ItemInfo.TypeHandle, 0, &TypeInfo);
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_CANNOT_GET_TYPEINFO, ItemInfo.TypeHandle, rc);
        }
        return;
    }

    InitializeListHead (&ItemIdHead);
    if (!(StoredItemId = LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT, sizeof(STORED_ITEM_ID)))) {
        //Error Status is set in LocalAlloc()
        WriteMsgToLog(LogErrors,MSG_CANNOT_ALLOC_MEMORY, L"ExecLinkItemCmd()", sizeof(STORED_ITEM_ID), GetLastError());
        return ;
    }

    StoredItemId->ItemId = ItemId;
    InsertTailList (&ItemIdHead, &StoredItemId->List);
    
    WriteMsg (MSG_ITEM_START_LINK, TypeInfo.TypeName, ItemInfo.Address, ItemId, ItemInfo.ReportedPlugInName);
    
    EnumLinkItem (ItemId, InputDir);
    FreeItemIdList();
    
    return;
}



UINT32
EnumLinkItem (
    KA_ITEM_ID DataItemId,
    USHORT InputDir
    )
/*++

Routine Description:
    Enumerate link info associated with specified data item.
Arguments:
    DataItemId - link info associated with DataItemId will be enumerated
    InputDir - direction to be enumerated link info.
Return Value:
    Win32 Error Code

--*/
{
    UINT32 rc;
    LINK_ITEM_CONTEXT Context;
    static UINT32 Indent;

    if (!DataItemId) {
        Indent = 0;
        return ERROR_INVALID_PARAMETER;
    }
    Indent++;
    
    Context.DataItemId = DataItemId;
    Context.fLinkTo = TRUE;
    Context.Indent = Indent;
    Context.InputDir = InputDir;
    rc = KxEnumLinkInfo(
             hPlugIn,
             DataItemId,
             0,
             0,
             (PKANALYZE_ENUM_LINK_INFO_CALLBACK)LinkItemCallback,
             (PVOID)&Context
             );
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_CANNOT_ENUM_LINK, DataItemId, rc);
        }
        Indent = 0;
        return rc;
    }

    Context.DataItemId = DataItemId;
    Context.fLinkTo = FALSE;
    Context.Indent = Indent;
    Context.InputDir = InputDir;
    rc = KxEnumLinkInfo(
             hPlugIn,
             DataItemId,
             0,
             0,
             (PKANALYZE_ENUM_LINK_INFO_CALLBACK)LinkItemCallback,
             (PVOID)&Context
             );
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_CANNOT_ENUM_LINK, DataItemId, rc);
        }
        Indent = 0;
        return rc;
    }

    Indent--;
    return NO_ERROR;
}



UINT32
CALLBACK
LinkItemCallback(
    IN HKAPLUGIN h,
    IN CONST KA_LINK_INFO *LinkInfo,
    IN PLINK_ITEM_CONTEXT Context
    )
/*++

Routine Description:
    KxEnumLinkInfo() helper routine invoke this routine to pass DataItemId.
    This routine prints out item with arrow representing direction of link.
Arguments:
    LinkInfo - Enumerated link info.
    Context - hold level of indent, direction of link to be searched.
Return Value:
    Win32 error code.

--*/
{
    UINT32 rc;
    KA_ITEM_ID id;
    KA_DATA_ITEM_INFO ItemInfo;
    KA_TYPE_INFO TypeInfo;
    UINT32 i;
    BOOL Exist;

    if (KxCheckBreak(h, 0)) {
        return ERROR_CANCELLED;
    }

    if (Context->fLinkTo) {
        if (Context->InputDir == LINK_BACK) {
            return NO_ERROR;
        }
        if (LinkInfo->SourceDataItemId == Context->DataItemId) {
            id = LinkInfo->TargetDataItemId;
        } else {
            return NO_ERROR;
        }
    } else {
        if (Context->InputDir == LINK_FORWARD) {
            return NO_ERROR;
        }
        if (LinkInfo->TargetDataItemId == Context->DataItemId) {
            id = LinkInfo->SourceDataItemId;
        } else {
            return NO_ERROR;
        }
    }

    rc = KxQueryDataItem(h, id, 0, &ItemInfo);
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_CANNOT_GET_ITEMINFO, id, rc);
        }
        return rc;
    }
    rc = KxQueryTypeInfo(hPlugIn, ItemInfo.TypeHandle, 0, &TypeInfo);
    if (rc != NO_ERROR) {
        if (rc != ERROR_CANCELLED) {
            WriteMsgToLog (LogErrors, MSG_CANNOT_GET_TYPEINFO, ItemInfo.TypeHandle, rc);
        }
        return rc;
    }
    for (i=0; i<Context->Indent; i++) { WriteText(L"  "); }
    WriteText (L"%s",
               (LinkInfo->SourceDataItemId == Context->DataItemId) ? L"-->" : L"<--");
    WriteMsg (MSG_LINK_ITEM, TypeInfo.TypeName, ItemInfo.Address, id, LinkInfo->ReportedPlugInName);

    rc = AddIdToList(id, &Exist);
    if (rc != NO_ERROR) {
        return rc;
    } else {
        if (Exist) {
            return NO_ERROR;
        }
    }
    
    if (Context->Indent < Depth) {
        return EnumLinkItem(id, Context->InputDir);
    }
    
    return NO_ERROR;
}



UINT32
AddIdToList (
    KA_ITEM_ID DataItemId,
    BOOL *Exist
    )
/*++

Routine Description:
    Store item id which is enumerated if its id not exist in list.
Arguments:
    DataItemId - item id which was enumerated.
    Exist - item identified DataItemId exists in list or not.
Return Value:
    Win32 error code.

--*/
{
    UINT32 rc;
    PSTORED_ITEM_ID StoredItemId, CurrentStoredItemId;
    PLIST_ENTRY CurrentList, PreviousList;

    *Exist = FALSE;
    CurrentList = ItemIdHead.Flink;
    PreviousList = &ItemIdHead;

    while (CurrentList != &ItemIdHead) {
        CurrentStoredItemId = CONTAINING_RECORD (CurrentList, STORED_ITEM_ID, List);
        if (CurrentStoredItemId->ItemId == DataItemId) {
            *Exist = TRUE;
            return NO_ERROR;
        }
        PreviousList = CurrentList;
        CurrentList = CurrentList->Flink;
    }

    if (!(StoredItemId = LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT, sizeof(STORED_ITEM_ID)))) {
        //Error Status is set in LocalAlloc()
        rc = GetLastError();
        WriteMsgToLog(LogErrors,MSG_CANNOT_ALLOC_MEMORY, L"AddIdToList()", sizeof(STORED_ITEM_ID), rc);
        return rc;
    }
    
    StoredItemId->ItemId = DataItemId;
    InsertTailList (&ItemIdHead, &StoredItemId->List);
    *Exist = FALSE;
    
    return NO_ERROR;
}



VOID
FreeItemIdList (
    VOID
    )
/*++

Routine Description:
    Free list which store data item.
Arguments:
    NONE
Return Value:
    NONE

--*/
{
    PSTORED_ITEM_ID CurrentStoredItemId;
    PLIST_ENTRY CurrentList, PreviousList;

    CurrentList = ItemIdHead.Flink;
    PreviousList = &ItemIdHead;

    while (CurrentList != &ItemIdHead) {
        CurrentStoredItemId = CONTAINING_RECORD (CurrentList, STORED_ITEM_ID, List);
        PreviousList = CurrentList;
        CurrentList = CurrentList->Flink;

        LocalFree (CurrentStoredItemId);
    }
}



BOOL
ExecExtensionCmd(
    LPCWSTR PlugInName,
    LPCWSTR Arguments
    )
/*++

Routine Description:
    Invoke director plug-in
Arguments:
    PlugInName - Invoked plug-in name
    Arguments - argument passed to invoked plug-in
Return Value:
    If successful to invoke plug-in return TRUE. If not FALSE.

--*/
{
    UINT32 rc;
    BOOL bReachedPlugIn;

    rc = KxCallPlugIn(
             hPlugIn,
             PlugInName,
             KA_PROCESS_RESULT,
             0,
             (UINT_PTR)Arguments,
             &bReachedPlugIn
             );

    if (!bReachedPlugIn) {
        return FALSE;
    }

    return TRUE;
}
