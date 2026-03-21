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
// Public definitions for plug-in "EXPOOL"
//
// Type                                 Method
// -----------------------------------------------------------------------
// PoolBlock\PoolHeader                 M_POOL_HEAD_GET_BASIC_INFO
// PoolBlock\Paged                      M_POOL_BLOCK_GET_BASIC_INFO
// PoolBlock\NonPaged                   M_POOL_BLOCK_GET_BASIC_INFO
// PoolBlock\Paged\FreedBlock           M_POOL_BLOCK_GET_BASIC_INFO
// PoolBlock\NonPaged\FreedBlock        M_POOL_BLOCK_GET_BASIC_INFO
// PoolControl\PoolDescriptor           M_POOL_DESCRIPTOR_GET_BASIC_INFO
//                                      M_POOL_DESCRIPTOR_GET_FREE_BLOCK_INFO
//
// AnomalyDescriptor\PoolBlock\PoolHeader        none
// AnomalyDescriptor\PoolControl\PoolDescriptor  none
//
#define PLUGIN_EXPOOL L"EXPOOL"

#define M_POOL_HEAD_BASE       0x00010000
#define M_POOL_BLOCK_BASE      0x00020000
#define M_POOL_DESCRIPTOR_BASE 0x00030000

typedef enum _KA_POOL_TYPE {
    KaNonPagedPool,
    KaNonPagedPoolMustSucceed,
    KaPagedPool,
    KaFreedPool,
    KaUnknownPool
} KA_POOL_TYPE;

#define KA_POOL_BLOCK_SIZE  32


//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PPOOL_HEAD_BASIC_INFO BasicInfo
//    )
//
// Routine Description:
//
//     This routine returns a basic information of PoolBlock\PoolHeader item.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify PoolBlock\PoolHeader item id.
//
//     MethodCode - specify M_POOL_HEAD_GET_BASIC_INFO.
//
//     BasicInfo - specify buffer address for POOL_HEAD_BASIC_INFO. This plug-in
//        will fill the member of the structure.
//
// Return Value:
//
//     Win32 error code
//
#define M_POOL_HEAD_GET_BASIC_INFO (M_POOL_HEAD_BASE + 1)

typedef struct _POOL_HEAD_BASIC_INFO {
    KA_POOL_TYPE PoolType;
    ULONG32 PoolIndex;
    ULONG32 BlockNum;
    ULONG32 PreviousBlockNum;
    ULONG32 PoolTag;
    BOOL ProtectedTag;
    BOOL Quota;
    UINT_PTR QuotaProcess;
} POOL_HEAD_BASIC_INFO, *PPOOL_HEAD_BASIC_INFO;


//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PPOOL_BLOCK_BASIC_INFO BasicInfo
//    )
//
// Routine Description:
//
//     This routine returns a basic information of PoolBlock\Paged, PoolBlock\NonPaged,
//     PoolBlock\Paged\FreedBlock or PoolBlock\NonPaged\FreedBlock item.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify PoolBlock\Paged, PoolBlock\NonPaged, PoolBlock\Paged\FreedBlock
//         or PoolBlock\NonPaged\FreedBlock item id.
//
//     MethodCode - specify M_POOL_BLOCK_GET_BASIC_INFO.
//
//     BasicInfo - specify buffer address for POOL_BLOCK_BASIC_INFO. This plug-in
//        will fill the member of the structure.
//
// Return Value:
//
//     Win32 error code
//
#define M_POOL_BLOCK_GET_BASIC_INFO (M_POOL_BLOCK_BASE + 1)

typedef struct _POOL_BLOCK_BASIC_INFO {
    KA_ITEM_ID PoolHead;
} POOL_BLOCK_BASIC_INFO, *PPOOL_BLOCK_BASIC_INFO;


//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PPOOL_DESCRIPTOR_BASIC_INFO BasicInfo
//    )
//
// Routine Description:
//
//     This routine returns a basic information of PoolControl\PoolDescriptor item.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify PoolControl\PoolDescriptor item id.
//
//     MethodCode - specify M_POOL_DESCRIPTOR_GET_BASIC_INFO.
//
//     BasicInfo - specify buffer address for POOL_DESCRIPTOR_BASIC_INFO. This plug-in
//        will fill the member of the structure.
//
// Return Value:
//
//     Win32 error code
//
#define M_POOL_DESCRIPTOR_GET_BASIC_INFO (M_POOL_DESCRIPTOR_BASE + 1)

typedef struct _POOL_DESCRIPTOR_BASIC_INFO {
    KA_POOL_TYPE PoolType;
    ULONG PoolIndex;
    ULONG AllocCounts;
    ULONG FreeCounts;
    ULONG TotalAllocPages;
    ULONG TotalAllocBigPages;
    ULONG NumberOfFreeBlockIndex;
} POOL_DESCRIPTOR_BASIC_INFO, *PPOOL_DESCRIPTOR_BASIC_INFO;


//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN ULONG Index,
//    OUT PPOOL_DESCRIPTOR_FREE_BLOCK_INFO FreeBlockInfo
//    )
//
// Routine Description:
//
//     This routine returns a FreeBlock list head item that is associated with
//     specified PoolControl\PoolDescriptor item.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify PoolControl\PoolDescriptor item id.
//
//     MethodCode - specify M_POOL_DESCRIPTOR_GET_FREE_BLOCK_INFO.
//
//     Index - specify FreeBlockIndex you want. Maximum value is 
//         POOL_DESCRIPTOR_BASIC_INFO.NumberOfFreeBlockIndex - 1.
//
//     FreeBlockInfo - specify buffer address for POOL_DESCRIPTOR_FREE_BLOCK_INFO.
//         This plug-in will fill the member of the structure.
//
// Return Value:
//
//     Win32 error code
//
#define M_POOL_DESCRIPTOR_GET_FREE_BLOCK_INFO (M_POOL_DESCRIPTOR_BASE + 2)

typedef struct _POOL_DESCRIPTOR_FREE_BLOCK_INFO {
    KA_ITEM_ID FreeListHead;
} POOL_DESCRIPTOR_FREE_BLOCK_INFO, *PPOOL_DESCRIPTOR_FREE_BLOCK_INFO;


//
// Sub-Anomaly definitions for AnomalyDescriptor\PoolBlock\PoolHeader
//
#define ANOMALY_POOLHEADER_BLOCKSIZE      1
#define ANOMALY_POOLHEADER_PREVSIZE       2
#define ANOMALY_POOLHEADER_POOLINDEX      4

typedef struct _ANOMALY_POOLHEADER_EXTRA {
    ULONG32 Reason;
} ANOMALY_POOLHEADER_EXTRA, *PANOMALY_POOLHEADER_EXTRA;


#ifndef _NT351
///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "LOOKASIDE"
//
// Type                                 Method
// -----------------------------------------------------------------------
// LookasideList\General                M_LOOKASIDE_GENERAL_BASIC_INFO
// LookasideList\SmallPool              M_LOOKASIDE_SMALLPOOL_BASIC_INFO
//
// AnomalyDescriptor\LookasideList\General      none
// AnomalyDescriptor\LookasideList\SmallPool    none
//
#define PLUGIN_LOOKASIDE L"LOOKASIDE"

#define M_LOOKASIDE_GENERAL_BASE       0x00040000
#define M_LOOKASIDE_SMALLPOOL_BASE     0x00050000

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PLOOKASIDE_GENERAL_BASIC_INFO BasicInfo
//    )
//
// Routine Description:
//
//     This routine returns a basic information of LookasideList\General item.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify LookasideList\General item id.
//
//     MethodCode - specify M_LOOKASIDE_GENERAL_BASIC_INFO.
//
//     BasicInfo - specify buffer address for LOOKASIDE_GENERAL_BASIC_INFO. This plug-in
//        will fill the member of the structure.
//
// Return Value:
//
//     Win32 error code
//
#define M_LOOKASIDE_GENERAL_BASIC_INFO (M_LOOKASIDE_GENERAL_BASE + 1)

typedef struct _LOOKASIDE_GENERAL_BASIC_INFO {
    KA_POOL_TYPE PoolType;
    ULONG PoolTag;
    ULONG BlockSize;
    ULONG CurrentDepth;
    ULONG MaximumDepth;
    ULONG TotalAllocates;
    ULONG AllocateMisses;
    ULONG TotalFrees;
    ULONG FreeMisses;
    UINT_PTR AllocateFunction;
    UINT_PTR FreeFunction;
    KA_ITEM_ID SListHead;
} LOOKASIDE_GENERAL_BASIC_INFO, *PLOOKASIDE_GENERAL_BASIC_INFO;

#endif //_NT351



//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PLOOKASIDE_SMALLPOOL_BASIC_INFO BasicInfo
//    )
//
// Routine Description:
//
//     This routine returns a basic information of LookasideList\SmallPool item.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify LookasideList\SmallPool item id.
//
//     MethodCode - specify M_LOOKASIDE_SMALLPOOL_BASIC_INFO.
//
//     BasicInfo - specify buffer address for LOOKASIDE_SMALLPOOL_BASIC_INFO. This plug-in
//        will fill the member of the structure.
//
// Return Value:
//
//     Win32 error code
//
#define M_LOOKASIDE_SMALLPOOL_BASIC_INFO (M_LOOKASIDE_SMALLPOOL_BASE + 1)

typedef struct _LOOKASIDE_SMALLPOOL_BASIC_INFO {
    KA_POOL_TYPE PoolType;
    ULONG BlockSize;
    ULONG CurrentDepth;
    ULONG MaximumDepth;
    ULONG TotalAllocates;
    ULONG AllocateMisses;
    ULONG TotalFrees;
    ULONG FreeMisses;
    KA_ITEM_ID SListHead;
} LOOKASIDE_SMALLPOOL_BASIC_INFO, *PLOOKASIDE_SMALLPOOL_BASIC_INFO;


///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "POOLEXT"
//
// Type                                 Method
// -----------------------------------------------------------------------
// PoolExt\ExtraHeader                  M_POOLEXT_EH_GET_ALLOCATOR_INFO
// PoolExt\PoolLog                      none
//
// AnomalyDescriptor\PoolExt\ExtraHeader     none
//
#define PLUGIN_POOLEXT L"POOLEXT"

#define M_POOLEXT_EH_BASE   0x000a0000

//
// Sub-Anomaly definitions for AnomalyDescriptor\PoolExt\ExtraHeader
//
#define ANOMALY_POOLEXT_EH_CHECKSUM_MISMATCH    1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PPOOLEXT_ALLOCATOR_INFO AllocatorInfo
//    )
//
// Routine Description:
//
//     This routine returns a basic information of PoolExt\ExtraHeader item.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify PoolExt\ExtraHeader item id.
//
//     MethodCode - specify M_POOLEXT_EH_GET_ALLOCATOR_INFO.
//
//     BasicInfo - specify buffer address for POOLEXT_ALLOCATOR_INFO. This plug-in
//        will fill the member of the structure.
//
// Return Value:
//
//     Win32 error code
//
#define M_POOLEXT_EH_GET_ALLOCATOR_INFO (M_POOLEXT_EH_BASE + 1)

typedef struct _POOLEXT_ALLOCATOR_INFO {
    WCHAR AllocatorName[MAX_PATH];
    UINT_PTR CallerReturnAddress;
} POOLEXT_ALLOCATOR_INFO, *PPOOLEXT_ALLOCATOR_INFO;
