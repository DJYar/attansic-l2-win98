/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    kapublic.h

Abstract:

    Module Plug-In private interface header file.

Author:

     Hirokazu Takeuchi ( v-hirokt ) Nov. 12 1998

Revisions History:

--*/

///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "MODULE"
//
// Type                                 Method
// -----------------------------------------------------------------------
// Module\Header                        M_MDHEAD_GET_IMAGE_NAME
//                                      M_MDHEAD_GET_BUILD_NAME
//                                      M_MDHEAD_GET_IMAGE_SIZE
//                                      M_MDHEAD_GET_TIME_DATE_STAMP
//                                      M_MDHEAD_GET_CHECKSUM
//                                      M_MDHEAD_GET_DOS_HEADER
//                                      M_MDHEAD_GET_NT_HEADERS
//                                      M_MDHEAD_ENUM_SECTION_HEADERS
//
// Module\Code                          M_MDSECT_GET_SECTION_NAME
//                                      M_MDSECT_GET_ATTRIBUTE
//                                      M_MDSECT_GET_MODULE_NAME
//                                      M_MDSECT_GET_SECTION_HEADER
//
// Module\Data                          M_MDSECT_GET_SECTION_NAME
//                                      M_MDSECT_GET_ATTRIBUTE
//                                      M_MDSECT_GET_MODULE_NAME
//                                      M_MDSECT_GET_SECTION_HEADER
//
// Loader\LdrDataTable                  - None -
//
// Private Interface Routine            MD_PRIVATE_IS_MP_KERNEL
//                                      MD_PRIVATE_ENUM_MODULE
//                                      MD_PRIVATE_GET_HAL_NAME
//


////////////////////////////////////////
// Private interface code definition
//
#define MODULE_PRIVATE_CODE_BASE KA_PRIVATE+0x00023000

//  BOOL
//  (CALLBACK *KANALYZE_CALL_PLUGIN_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN LPCWSTR PlugInName,
//      IN UINT32 PrivateOperationCode,
//      OUT BOOL * Result
//      IN OUT Unused,
//      OUT PUINT_PTR ReturnValue
//  )
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    PlugInName - supplies target plug-in name.
//
//    PrivateOperationCode - supplies identifier code for private interface 
//                          routine. In this case, this value must be 
//                          MD_PRIVATE_IS_MP_KERNEL.
//
//    Result - receives result of the test. If the NT kernel is multi-processor 
//            version, this function sets this value TRUE. If the NT kernel is 
//            uniprocessor version, this function returns FALSE.
//
//    Unused - unused. This value must be zero.
//
//    ReturnValue - receives error condition of this function. If this function 
//                 success, this value will be NO_ERROR. If this function fails, 
//                 this value will be stores Win32 error code.
//Return Value:
//
//     This function returns boolean value to indicate this function succeeds or 
//    not. TRUE indicates success. To get extended error code, use GetLastError().
//
#define MD_PRIVATE_IS_MP_KERNEL (MODULE_PRIVATE_CODE_BASE + 1)


//  BOOL
//  (CALLBACK *KANALYZE_CALL_PLUGIN_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN LPCWSTR PlugInName,
//      IN UINT32 PrivateOperationCode,
//      IN PKA_MD_ENUM_MODULE CallbackProc,
//      IN OUT PVOID Context,
//      OUT PUINT_PTR ReturnValue
//  )
//
//Arguments
//
//    hPlugIn - supplies plug-in handle.
//
//    PlugInName - supplies target plug-in name.
//
//    PrivateOperationCode - supplies identifier code for private 
//                          interface routine. In this case, this 
//                          value must be MD_PRIVATE_ENUM_MODULE.
//
//    CallbackProc - supplies user defined enumeration callback function. 
//                  Users can design the callback function implementation.
//
//    Context - supplies pointer to context buffer. This value supplied to 
//             CallbackProc. This plug-in does not treat about it. 
//
//    ReturnValue - receives error condition of this function. If this function 
//                 success, this value will be NO_ERROR. If this function 
//                 fails, this value will be stores Win32 error code.
//
//Return Value
//
//    This function returns boolean value to indicate this function succeeds 
//   or not. TRUE indicates success. To get extended error code, use GetLastError().
//
#define MD_PRIVATE_ENUM_MODULE (MODULE_PRIVATE_CODE_BASE + 2)

typedef 
UINT32 
(CALLBACK * PKA_MD_ENUM_MODULE_CALLBACK)(
    IN UINT_PTR BaseAddress,
    IN SIZE_T Size,
    IN LPCWSTR ImageName,
    IN LPCWSTR BuildName,
    IN OUT PVOID Context
);
// Routine Description:
//
//     This is callback function prototype for MD_PRIVATE_ENUM_MODULE. 
//     This function receives a module information. 
//
// Arguments
//
//     BaseAddress - supplies module base address. This value is got from 
//                  loader data table.
//
//     Size - supplies size of module. This value is gotten from loader data 
//           table entry.
//
//     ModuleName - supplies pointer to module name.
//
//     Context - supplies context that was specified in calling this private 
//              interface routine. Users can use this value as they like.
//
// Return Value
//
//     This function returns Win32 error code.
//     NO_ERROR indicates success.
//

//  BOOL
//  (CALLBACK *KANALYZE_CALL_PLUGIN_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN LPCWSTR PlugInName,
//      IN UINT32 PrivateOperationCode,
//      OUT LPWSTR NameBuffer,
//      IN SIZE_T NameBufferSize,
//      OUT PUINT_PTR ReturnValue
//  )
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    PlugInName - supplies target plug-in name.
//
//    PrivateOperationCode - supplies identifier code for private 
//                          interface routine. In this case, this 
//                          value must be MD_PRIVATE_IS_MP_KERNEL.
//
//    NameBuffer - receives build-time HAL.dll file name.
//
//    NamBufferSize - supplies size of NameBuffer in WCHAR.
//
//    ReturnValue - receives error condition of this function. If this 
//                 function success, this value will be NO_ERROR. If 
//                 this function fails, this value will be stores Win32 
//                 error code.
//
//Return Value:
//
//    This function returns boolean value to indicate this function 
//   succeeds or not. TRUE indicates success. To get extended error code, 
//   use GetLastError ().
//
#define MD_PRIVATE_GET_HAL_NAME (MODULE_PRIVATE_CODE_BASE + 3)

#define MODULE_HEADER_METHOD_BASE 0x00021000
#define MODULE_SECTION_METHOD_BASE 0x00022000

/*
  Definitions for Module\Header type items:
*/
//
// Type-specific Data Structure.
//

//
// Data structure for internal data table entry.
//
typedef struct _KA_MD_LOCATE_ITEM_ENTRY {
    LIST_ENTRY ListEntry;
    LPWSTR ModuleName;
    LPWSTR ModuleFullPath;
    UINT_PTR Address;
    SIZE_T Size;
    KA_ITEM_ID LoaderItemId;
    ULONG32 LdrTableFlag;
    BOOL ModulePageable;
    BOOL HeaderPagedOut;
    UINT_PTR ImportTableOffset;
    SIZE_T ImportTableSize;
} KA_MD_LOCATE_ITEM_ENTRY, *PKA_MD_LOCATE_ITEM_ENTRY;

//
// Extended data structure for module header.
//
typedef struct _MD_EXTDATA_MODULE_HEADER{
    PKA_MD_LOCATE_ITEM_ENTRY ModuleInformation;
}MD_EXTDATA_MODULE_HEADER, * PMD_EXTDATA_MODULE_HEADER;

//  BOOL
//  CALL_BACK
//  (* KANALYZE_CALL_METHOD_ROUTINE)( 
//      IN HKAPLUGIN hPlugIn,
//      IN KA_ITEM_ID ItemId,
//      IN ULONG32 MethodCode,
//      IN SIZE_T * BufferSize,
//      OUT LPWSTR Buffer
//   )
//
//Routine Description:
//
//    This method gets module name that is stored in loader data table.
//    Method Function Prototype
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this value must 
//                be M_MDHEAD_GET_IMAGE_NAME.
//
//    BufferSize - supplies size of the buffer, that receives module name.
//
//    Buffer - receives module name.
//
//Return Value:
//
//    This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. 
//    To get extended error information, call GetLastError().
//
#define M_MDHEAD_GET_IMAGE_NAME (MODULE_HEADER_METHOD_BASE + 1)

//  BOOL
//  (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN KA_ITEM_ID ItemId,
//      IN ULONG32 MethodCode,
//      IN SIZE_T * BufferSize,
//      OUT LPWSTR Buffer
//  )
//
//Routine Description:
//
//    This method gets module name.
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this value must 
//                be M_MDHEAD_GET_BUILDTIME_NAME.
//
//    BufferSize - supplies size of the buffer, that receives module name.
//
//    Buffer - receives module name.
//
//Return Value:
//
//    This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. To get extended error 
//    information, call GetLastError().
//
#define M_MDHEAD_GET_BUILD_NAME (MODULE_HEADER_METHOD_BASE + 2)

//  BOOL
//  (CALL_BACK * KANALYZE_CALL_METHOD_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN KA_ITEM_ID ItemId,
//      IN ULONG32 MethodCode,
//      OUT SIZE_T * ModuleSize
//  )
//
//Routine Description:
//
//    This method gets size of module that is stored in LDR_DATA_TABLE_ENTRY. 
//    It contains whole module images: includes discarded section size.
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this value must be 
//                M_MDHEAD_GET_IMAGE_SIZE.
//
//    ModuleSize - receives size of module.
//
//Return Value:
//
//    This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. 
//    To get extended error information, call GetLastError().
//
#define M_MDHEAD_GET_IMAGE_SIZE (MODULE_HEADER_METHOD_BASE + 3)

// BOOL
// (CALL_BACK * KANALYZE_CALL_METHOD_ROUTINE)(
//     IN HKAPLUGIN hPlugIn,
//     IN KA_ITEM_ID ItemId,
//     IN ULONG32 MethodCode,
//     OUT SYSTEM_TIME * FormatedTime,
//     OUT ULONG32 TimeDateStamp
// )
//
//Routine Description:
//
//    This method gets time date stamp.
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this value must be 
//                M_MDHEAD_GET_MODULE_NAME.
//
//    FormatedTime - receives SYSTEMTIME style time date stamp. The time is 
//                  local time.
// 
//    TimeDateStamp - receives ULONG32 style time date stamp.
//
//Return Value:
//
//     This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. To get
//    extended error information, call GetLastError().
//
#define M_MDHEAD_GET_TIME_DATE_STAMP (MODULE_HEADER_METHOD_BASE + 4)

//  BOOL
//  (CALL_BACK * KANALYZE_CALL_METHOD_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN KA_ITEM_ID ItemId,
//      IN ULONG32 MethodCode,
//      OUT ULONG32 * Checksum
//  )
//
//Routine Description:
//
//    This method gets checksum.
//
//Arguments
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this value must be 
//                M_MDHEAD_GET_CHECK_SUM.
//
//    Checksum - receives checksum.
//
//
//Return Value
//
//     This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. 
//    To get extended error information, call GetLastError().
//
#define M_MDHEAD_GET_CHECKSUM (MODULE_HEADER_METHOD_BASE +5)

//  BOOL
//  (CALL_BACK * KANALYZE_CALL_METHOD_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN KA_ITEM_ID ItemId,
//      IN ULONG32 MethodCode,
//      OUT PIMAGE_DOS_HEADER DosHeaderBuffer,
//      IN SIZE_T BufferSize,
//      OUT SIZE_T * BytesRead
//  )
//
//Routine Description:
//
//    This method gets DOS header.
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this value must be 
//              M_MDHEAD_GET_MODULE_NAME.
//
//    DosHeaderBuffer - receives DOS header.
//
//    BufferSize - supplies size of buffer to receive DOS header.
//
//    BytesRead - receives actual size of DOS header in bytes.
//
//Return Value:
//
//     This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. 
//    To get extended error information, call GetLastError().
//    If buffer size is smaller than DOS header, this method sets error 
//    status as ERROR_MORE_DATA.
//
#define M_MDHEAD_GET_DOS_HEADER (MODULE_HEADER_METHOD_BASE + 6)

//  BOOL
//  (CALL_BACK * KANALYZE_CALL_METHOD_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN KA_ITEM_ID ItemId,
//      IN ULONG32 MethodCode,
//      OUT PIMAGE_NT_HEADERS NtHeadersBUffer,
//      IN SIZE_T BufferSize,
//      OUT SIZE_T * BytesRead
//  )
//
//RoutineDecscription:
//
//    This method gets module name.
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this 
//                value must be M_MDHEAD_GET_MODULE_NAME.
//
//    NtHeadersBuffer - receives NT headers.
//
//    BufferSize - supplies size of NTHeadersBuffer.
//
//    BytesRead - receives actual size of NT headers in byte.
//
//Return Value:
//
//    This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. 
//    To get extended error information, call GetLastError().
//    If the buffer is smaller than NT headers, this method sets error 
//    status as ERROR_MODE_DATA.
//
#define M_MDHEAD_GET_NT_HEADERS (MODULE_HEADER_METHOD_BASE + 7)

//  BOOL
//  (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//      IN HKAPLUGIN hPLugIn,
//      IN KA_ITEM_ID ItemId,
//      IN ULONG32 MethodCode,
//      IN PMD_ENUM_SECTION_HEADER_CALLBACK CallbackProc,
//      IN OUT PVOID Context
//  )
//
//Routine Description:
//
//    This method enumerates section headers in Module\Header type item.
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID.
//
//    MethodCode - supplies integer value to identify method. In this case, 
//                this value must be M_MDHEAD_ENUM_SECTION_HEADER.
//
//    CallbackProc - supplies pointer to user defined call back function.
//
//    Context - supplies pointer to user defined context structure.
//
//Return Value:
//
//    This method returns boolean value to indicate the method is success. 
//   TRUE indicates success. FALSE indicates any error is occurred. To get 
//   extended error information, call GetLastError().
//
//Remarks:
//
//    If supplied call back function returns error, this method stops enumeration immediately.
//
#define M_MDHEAD_ENUM_SECTION_HEADERS (MODULE_HEADER_METHOD_BASE + 8)

//
// Enumerate section header call back function.
//
typedef
UINT32
(CALLBACK * PMD_ENUM_SECTION_HEADER_CALLBACK)(
   IN PIMAGE_SECTION_HEADER SectionHeader,
   IN OUT PVOID Context
);
//Routine Description:
//
//     MD_ENUM_SECTION_HEADER_CALLBACK is call back function for enumerate section method. 
//    This function receives section header object. Users can design this function for their 
//    purpose.
//
//Arguments:
//
//    SectionHeader - supplies pointer to section header object. The section header object 
//                   is loaded from dump image.
//
//    Context - supplies pointer to user defined context structure.
//
//Return Value:
//
//    This function returns Win32 error code.
//    NO_ERROR indicates success.
//


/*
  Definitions for Module\Code & Module\Data type items:
*/

//
// Extended data structure for section.
//
typedef struct _MD_EXTDATA_SECTION{
    KA_ITEM_ID ModuleHeaderItemId;
    UINT_PTR SectionHeaderAddress;
}MD_EXTDATA_SECTION, * PMD_EXTDATA_SECTION;

//  BOOL
//  (CALL_BACK * KANALYZE_CALL_METHOD_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN KA_ITEM_ID ItemId,
//      IN SIZE_T SectionNameBufferSize,
//      OUT LPWSTR SectionNameBuffer 
//  )
//
//Routine Description:
//
//    This method gets section name.
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this value must be 
//                M_MDSECT_GET_SECTION_NAME.
//
//    SectionNameBufferSize - supplies size of section name buffer.
//
//    SectionNameBuffer - receives section name.
//
//Return Value
//
//    This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. 
//    To get extended error information, call GetLastError().
//
#define M_MDSECT_GET_SECTION_NAME (MODULE_SECTION_METHOD_BASE + 1)

//  BOOL
//  (CALL_BACK * KANALYZE_CALL_METHOD_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN KA_ITEM_ID ItemId,
//      IN ULONG32 MethodCode,
//      OUT PMD_SECT_ATTRIB Attribute
//  )
//
//Routine Description:
//
//    This method gets section attribute.
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this value 
//                must be M_MDSECT_GET_ATTRIBUTE.
//
//    Attribute - receives structure that describes section's attribute.
//
//Return Value:
//
//    This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. 
//    To get extended error information, call GetLastError().
//
//Remarks
//
//    In actual case, section attribute that was probed by this method is 
//    not trustable. Because We found the section attribute is not pagable, 
//    but some pages in the module are paged-out. Therefore, we must check
//    SectionPointer in LDR_DATA_TABLE_ENTRY. This value equals 0xFFFFFFFF.
//    It indicates the section is pagable.
//
#define M_MDSECT_GET_ATTRIBUTE (MODULE_SECTION_METHOD_BASE + 2)

//
// Structure to indicate Module attribute 
//
typedef enum {
    InitializedData,
    UninitializedData,
    Code
} MD_SECT_TYPE;

typedef struct _MD_SECT_ATTRIB{
    BOOL Writable;
    BOOL Readable;
    BOOL Executable;
    BOOL Pageable;
    MD_SECT_TYPE SectionType;
} MD_SECT_ATTRIB, * PMD_SECT_ATTRIB;


//  BOOL
//  (CALL_BACK * KANALYZE_CALL_METHOD_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN KA_ITEM_ID ItemId,
//      IN ULONG32 MethodCode,
//      IN SIZE_T * BufferSize,
//      OUT LPWSTR Buffer
//  )
//Routine Description:
//
//    This method gets module name.
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this value must be
//                M_MDSECT_GET_MODULE_NAME.
//
//    BufferSize - supplies size of the buffer, that receives module name.
//
//    Buffer - receives module name.
//
//Return Value:
//
//    This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. 
//    To get extended error information, call GetLastError().
//
#define M_MDSECT_GET_MODULE_NAME (MODULE_SECTION_METHOD_BASE + 3)

//  BOOL
//  (CALL_BACK * KANALYZE_CALL_METHOD_ROUTINE)(
//      IN HKAPLUGIN hPlugIn,
//      IN KA_ITEM_ID ItemId,
//      IN ULONG32 MethodCode,
//      OUT UINT_PTR * SectionHeaderAddress,
//  )
//
//Routine Description:
//
//    This method gets section name.
//
//Arguments:
//
//    hPlugIn - supplies plug-in handle.
//
//    ItemId - supplies data item ID to get the module name.
//
//    MethodCode - supplies method code. In this case, this value must be 
//                M_MDSECT_GET_SECTION_HEADER.
//
//    SectionHeaderAddress - receives address of section header.
//
//Return Value:
//
//    This function returns boolean value.
//    TRUE indicates this function success.
//    FALSE indicates that any errors are occurred in this method. 
//    To get extended error information, call GetLastError().
//
#define M_MDSECT_GET_SECTION_HEADER (MODULE_SECTION_METHOD_BASE + 4)
