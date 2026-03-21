/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    kapublic.h

Abstract:

    This includes definitions of methods and private interfaces
    for plug-ins.

Author:

    MSKK Oem Tools Project    18-Oct-1998

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "VIRTUALMEMORY"
//
// Type                                 Method
// -----------------------------------------------------------------------
// Root\\UserModeArea                   none
// Root\\BootSystemArea                 none
// Root\\SystemMappedViewsArea          none
// Root\\SystemMappedViewsFreeArea      none
// Root\\PageTableArea                  none
// Root\\HyperSpaceArea                 none
// Root\\SystemCacheWorkingSetListArea  none
// Root\\SystemCacheArea                none
// Root\\CrashDumpVaArea                none
// Root\\HalsHeapArea                   none
//
// AnomalyDescriptor\\Root              none
// 
///////////////////////////////////////////////////////////////////////////
#define PLUGIN_VIRTUALMEMORY L"VIRTUALMEMORY"


#ifndef PAGE_SIZE
#if defined(_ALPHA_)
#define PAGE_SIZE   0x2000
#else
#define PAGE_SIZE   0x1000
#endif //_ALPHA_
#endif //PAGE_SIZE

#define VM_PRIVATE_BASE 0x10000000

//BOOL
//(KANALYZE_CALL_PLUGIN_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN LPCWSTR PlugInName,             //L"VIRTUALMEMORY"
//    IN ULONG32 PrivateOperationCode,   //VM_PRIVATE_GET_MEMORY_INFO
//    IN OUT UINT_PTR Param1,            //not used
//    OUT PVM_MEMORY_BASIC_INFO BasicInfo,
//    OUT PUINT_PTR ReturnValue
//    )
//
//Routine Description:
//
//    This routine is a private interface (VM_PRIVATE_GET_MEMORY_BASIC_INFO).
//    If VIRTUALMEMORY plug-in receives this call, it returns basic memory
//    information.
//
//Arguments:
//
//    PlugInHandle - specify your plug-in handle.
//
//    PlugInName - specify L"VIRTUALMEMORY".
//
//    PrivateOperationCode - specify VM_PRIVATE_GET_MEMORY_INFO.
//
//    Param1 - not used, must be zero.
//
//    BasicInfo - specify buffer address for VM_MEMORY_BASIC_INFO structure.
//        VIRTUALMEMORY plug-in will set the member of the structure.
//
//    ReturnValue - Win32 error code. If VIRTUALMEMORY plug-in doesn't support this
//        private operation code, it becomes ERROR_INVALID_FUNCTION.
//        Otherwise it becomes NO_ERROR.
//
//Return Value:
//
//    Boolean value. If VIRTUALMEMORY plug-in doesn't exist, it becomes FALSE.
//    Otherwise TRUE.
//
//
#define VM_PRIVATE_GET_MEMORY_BASIC_INFO    (VM_PRIVATE_BASE + 1)

typedef struct _VM_MEMORY_BASIC_INFO {
    SIZE_T PageSize;
    ULONG NumberOfPhysicalPages;
    ULONG NumberOfPagingFiles;
    ULONG CommitLimitPages;
    ULONG CommitTotalPages;
    ULONG SharedCommitPages;
    ULONG DriverCommitPages;
    ULONG ProcessCommitPages;
    ULONG PagedPoolCommitPages;
    ULONG AvailablePages;
    ULONG ResidentAvailablePages;
    ULONG ExtendedCommitPages;
    ULONG ZeroPages;
    ULONG FreePages;
    ULONG StandbyPages;
    ULONG ModifiedPages;
    ULONG ModifiedNoWritePages;
} VM_MEMORY_BASIC_INFO, *PVM_MEMORY_BASIC_INFO;


///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "PCR"
//
// Type                                 Method
// -----------------------------------------------------------------------
// Processor\ControlRegion             (TDB)
// Processor\ControlBlock              (TDB)
//
// AnomalyDescriptor\Processor\ControlRegion    none
// AnomalyDescriptor\Processor\ControlBlock     none
// 
///////////////////////////////////////////////////////////////////////////
#define PLUGIN_PCR L"PCR"


///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "MMPOOL"
//
// Type                                 Method
// -----------------------------------------------------------------------
// Root\\NonPagedPoolMustSucceedArea    none
// Root\\NonPagedPoolInitialArea        none
// Root\\NonPagedPoolExpansionArea      none
// Root\\PagedPoolArea                  none
// MmPool\PagedPool                     none
// MmPool\PagedPool\FreePages           none
// MmPool\NonPagedPool                  none
// MmPool\NonPagedPool\FreePages        none
// MmPool\NonPagedPool\MappedToKseg0    none
//
// AnomalyDescriptor\MmPool             none
// 
///////////////////////////////////////////////////////////////////////////
#define PLUGIN_MMPOOL L"MMPOOL"

//
// Root\* type stores UsageMap array (UCHAR) of the pool area as
// type-specific data of the item.
//
#define KA_PAGE_IN_USE        0x01
#define KA_PAGE_PAGED_OUT     0x02   // Paged Pool Only
#define KA_PAGE_MAP_KSEG0     0x02   // Non-paged Pool Only
#define KA_PAGE_START_PAGE    0x10
#define KA_PAGE_END_PAGE      0x20

//
// for AnomalyDescriptor\MmPool
//
typedef struct _MMPOOL_ANOMALY_PARAMS {
    UINT_PTR Param[4];
} MMPOOL_ANOMALY_PARAMS, *PMMPOOL_ANOMALY_PARAMS;

#define ANOMALY_MMPOOL_NONPAGED_MUSTSUCCEED_LIST_INVALID   1
#define ANOMALY_MMPOOL_NONPAGED_FREELIST_INVALID           2
#define ANOMALY_MMPOOL_FIRSTFREESYSTEMPTE_INVALID          3
#define ANOMALY_MMPOOL_CHECKCONTIGUOUSPAGE_FAILURE         4

///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "MMSYS"
//
// Type                                 Method
// -----------------------------------------------------------------------
// Root\NonPagedSystemArea              none
//
// AnomalyDescriptor\MmSys              none
// 
///////////////////////////////////////////////////////////////////////////
#define PLUGIN_MMSYS L"MMSYS"





///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "LIST"
//
// Type                                 Method
// -----------------------------------------------------------------------
// ListEntry\ListHead\*                 M_LIST_HEAD_GET_BASIC_INFO
//                                      M_LIST_HEAD_GET_LINKED_ENTRIES
// ListEntry\ListEntry\*                M_LIST_ENTRY_GET_BASIC_INFO
//
// AnomalyDescriptor\ListEntry\ListHead      none
// AnomalyDescriptor\ListEntry\ListEntry     none
//
#define PLUGIN_LIST L"LIST"

#define M_LIST_HEAD_BASE    0x00080000
#define M_LIST_ENTRY_BASE   0x00090000

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PLIST_HEAD_BASIC_INFO BasicInfo
//    )
//
//Routine Description:
//
//    This routine returns a basic information of ListEntry\ListHead item.
//
//Arguments:
//
//    PlugInHandle - specify your plug-in handle.
//
//    ItemId - specify ListEntry\ListHead item id.
//
//    MethodCode - specify M_LIST_HEAD_GET_BASIC_INFO.
//
//    BasicInfo - specify buffer address for LIST_HEAD_BASIC_INFO. This plug-in
//        will fill the member of the structure.
//
//Return Value:
//
//    Win32 error code;
//
//    NO_ERROR - success.
//    ERROR_INVALID_PARAMETER - specified item's type name is not correct.
//    Others - specified item id is not correct.
//
#define M_LIST_HEAD_GET_BASIC_INFO (M_LIST_HEAD_BASE + 1)

typedef struct _LIST_HEAD_BASIC_INFO {
    UINT_PTR Flink;
    UINT_PTR Blink;
    ULONG Hint;
    ULONG NumberOfFlink;
    ULONG NumberOfBlink;
    UINT_PTR Bad1;    //Flink 
    UINT_PTR Bad2;    //Flink last
    UINT_PTR Bad3;    //Blink last
    UINT_PTR Bad4;    //Blink 
} LIST_HEAD_BASIC_INFO, *PLIST_HEAD_BASIC_INFO;

#define LIST_HEAD_HINT_GOOD_LINKS 0
#define LIST_HEAD_HINT_CORRUPT_BAD2_BLINK 1
#define LIST_HEAD_HINT_CORRUPT_BAD1_FLINK 2
#define LIST_HEAD_HINT_CORRUPT_BAD2_FLINK_AND_BLINK 3
#define LIST_HEAD_HINT_CORRUPT_MORE 4


//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN BOOL fFlink,
//    IN ULONG32 NumberOfEntries,
//    OUT PKA_ITEM_ID EntryIdArray
//    )
//
//Routine Description:
//
//    This routine returns an array of ids these are associated with the list head.
//
//Arguments:
//
//    fFlink - if TRUE, forward link entries are returned. if FALSE means backword
//        link.
//
//    NumberOfEntries - specifies number of ids you request. You need enough buffer
//        size for EntryIdArray. The maximum number of link entries are specified
//        in LIST_HEAD_BASIC_INFO.NumberOfFlink member.
//
//    EntryIdArray - supplies an array of KA_ITEM_ID. This function fills this array
//        with ids.
//
//Return Value:
//
//    Win32 error code;
//
//    NO_ERROR - success.
//    ERROR_INVALID_PARAMETER - specified item's type name is not correct, or N.
//    Others - specified item id is not correct.
//
#define M_LIST_HEAD_GET_LINKED_ENTRIES  (M_LIST_HEAD_BASE + 2)


//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PLIST_ENTRY_BASIC_INFO BasicInfo
//    )
//
// Routine Description:
//
//    This routine returns a basic information of the specified ListEntry\ListEntry item.
//
// Arguments:
//
//    PlugInHandle - specify your plug-in handle.
//
//    ItemId - specify ListEntry\ListEntry item id.
//
//    MethodCode - specify M_LIST_ENTRY_GET_BASIC_INFO.
//
//    BasicInfo - specify buffer address for LIST_ENTRY_BASIC_INFO. This plug-in
//        will fill the member of the structure.
//
// Return Value:
//
//    Win32 error code;
//
//    NO_ERROR - success.
//    ERROR_INVALID_PARAMETER - specified item's type name is not correct.
//    ERROR_NO_ACCESS - specified item is not accessible (paged out or invalid address).
//    Others - specified item id is not correct.
//
#define M_LIST_ENTRY_GET_BASIC_INFO (M_LIST_ENTRY_BASE + 1)

typedef struct _LIST_ENTRY_BASIC_INFO {
    UINT_PTR Flink;
    UINT_PTR Blink;
    KA_ITEM_ID HeadId;
} LIST_ENTRY_BASIC_INFO, *PLIST_ENTRY_BASIC_INFO;


//
// Sub-Anomaly definitions for AnomalyDescriptor\ListEntry\ListEntry
//
#define ANOMALY_LISTENTRY_FLINK           1
#define ANOMALY_LISTENTRY_BLINK           2
#define ANOMALY_LISTENTRY_FLINK_AND_BLINK 3
#define ANOMALY_LISTENTRY_SUSPECTED       4



#ifndef _NT351
///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "SLIST"
//
// Type                                 Method
// -----------------------------------------------------------------------
// SList\SListHead\*                    M_SLIST_HEAD_GET_BASIC_INFO
//                                      M_SLIST_HEAD_GET_LINKED_ENTRIES
// SList\SListEntry\*                   M_SLIST_ENTRY_GET_BASIC_INFO
//
// AnomalyDescriptor\SList\SListHead             none
// AnomalyDescriptor\SList\SListEntry            none
//
#define PLUGIN_SLIST L"SLIST"

#define M_SLIST_HEAD_BASE    0x00060000
#define M_SLIST_ENTRY_BASE   0x00070000

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PSLIST_HEAD_BASIC_INFO BasicInfo
//    )
//
// Routine Description:
//
//    This routine returns a basic information of the specified SList\SListHead item.
//
// Arguments:
//
//    PlugInHandle - specify your plug-in handle.
//
//    ItemId - specify SList\SListHead item id.
//
//    MethodCode - specify M_SLIST_HEAD_GET_BASIC_INFO.
//
//    BasicInfo - specify buffer address for SLIST_HEAD_BASIC_INFO. This plug-in
//        will fill the member of the structure.
//
// Return Value:
//
//    Win32 error code;
//
//    NO_ERROR - success.
//    ERROR_INVALID_PARAMETER - specified item's type name is not correct.
//    ERROR_NO_ACCESS - specified item is not accessible (paged out or invalid address).
//    Others - specified item id is not correct.
//
//
#define M_SLIST_HEAD_GET_BASIC_INFO (M_SLIST_HEAD_BASE + 1)

typedef struct _SLIST_HEAD_BASIC_INFO {
    UINT_PTR Next;
    ULONG Hint;
    ULONG NumberOfFlink;
    UINT_PTR Bad1;    //Flink 
    UINT_PTR Bad2;    //Flink last
} SLIST_HEAD_BASIC_INFO, *PSLIST_HEAD_BASIC_INFO;

#define SLIST_HEAD_HINT_GOOD_LINKS 0
#define SLIST_HEAD_HINT_CORRUPT_LINKS 1


//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN ULONG32 NumberOfEntries,
//    OUT PKA_ITEM_ID EntryIdArray
//    )
//
// Routine Description:
//
//    This routine returns an array of ids these are associated with the slist head.
//
//Arguments:
//
//    NumberOfEntries - specifies number of ids you request. You need enough buffer
//        size for EntryIdArray. The maximum number of link entries are specified
//        in SLIST_HEAD_BASIC_INFO.NumberOfFlink member.
//
//    EntryIdArray - supplies an array of KA_ITEM_ID. This function fills this array
//        with ids.
//
//Return Value:
//
//    Win32 error code;
//
//    NO_ERROR - success.
//    ERROR_INVALID_PARAMETER - specified item's type name is not correct, or N.
//    Others - specified item id is not correct.
//
//
#define M_SLIST_HEAD_GET_LINKED_ENTRIES  (M_SLIST_HEAD_BASE + 2)


//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PSLIST_ENTRY_BASIC_INFO BasicInfo
//    )
//
// Routine Description:
//
//    This routine returns a basic information of the specified SList\SListHead item.
//
// Arguments:
//
//    PlugInHandle - specify your plug-in handle.
//
//    ItemId - specify SList\SListEntry item id.
//
//    MethodCode - specify M_SLIST_ENTRY_GET_BASIC_INFO.
//
//    BasicInfo - specify buffer address for SLIST_ENTRY_BASIC_INFO. This plug-in
//        will fill the member of the structure.
//
// Return Value:
//
//    Win32 error code;
//
//    NO_ERROR - success.
//    ERROR_INVALID_PARAMETER - specified item's type name is not correct.
//    ERROR_NO_ACCESS - specified item is not accessible (paged out or invalid address).
//    Others - specified item id is not correct.
//
//
#define M_SLIST_ENTRY_GET_BASIC_INFO (M_SLIST_ENTRY_BASE + 1)

typedef struct _SLIST_ENTRY_BASIC_INFO {
    UINT_PTR Next;
    UINT_PTR Previous;
    KA_ITEM_ID HeadId;
} SLIST_ENTRY_BASIC_INFO, *PSLIST_ENTRY_BASIC_INFO;


//
// Sub-Anomaly definitions for AnomalyDescriptor\SList\SListEntry
//
#define ANOMALY_SLISTENTRY_BROKEN_ENTRY    1
#define ANOMALY_SLISTENTRY_TOO_LONG_ENTRY  2
#define ANOMALY_SLISTENTRY_ILLEGAL_DEPTH   3

#endif //_NT351

