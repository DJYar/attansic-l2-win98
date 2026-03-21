/*++

Copyright (c) 1998-1999 Microsoft Corporation

Module Name:

    kanalyze.h

Abstract:

    Header file for Kernel Memory Space Analyzer.

Author:


Revision History:


--*/

#ifndef _KANALYZE_
#define _KANALYZE_

#ifdef __cplusplus
extern "C" {
#endif


typedef PVOID HKAPLUGIN;
typedef UINT KA_ITEM_ID, *PKA_ITEM_ID;  //phase2
typedef UINT HKATYPE, *PHKATYPE;        //phase2

#define INVALID_TYPE_HANDLE  ((HKATYPE)0)

#define MAX_TYPE_NAME        128        //phase2
#define MAX_TYPE_DESC        128        //phase2
#define MAX_PLUG_IN_NAME     16         //phase2
#define MAX_CONTAINER_TYPES  16         //phase2

//////////////////////////////////////////
// Kanalyze PlugIn Interface Definitions
//////////////////////////////////////////

typedef
UINT_PTR
(CALLBACK *PKANALYZE_INTERFACE_PROC)(
    IN HKAPLUGIN PlugInHandle,
    IN UINT32 OperationCode,
    IN OUT UINT_PTR Param1,
    IN OUT PVOID Param2
    );

//
// interface operations
//
#define KA_INITIALIZE                   0x00
#define KA_REGISTER_TYPES               0x01
#define KA_START_LOCATE_ITEMS           0x02
#define KA_PROCESS_ITEM                 0x03    //phase2
#define KA_NOTIFY_ITEM                  0x04    //phase2
#define KA_PROCESS_RESULT               0x05    //phase2
#define KA_TERMINATE                    0x08
#define KA_FORMAT_DATA_ITEM_DESCRIPTION 0x10    //phase2
#define KA_PRIVATE                      0x8000

//
// KA_REGISTER_TYPES
//
typedef enum {
    SupportAppendContainerTypes = 1,//phase2
    SupportDemandNotification = 2,  //phase2
    SupportSecondary = 5,
    SupportFull = 10
} KanalyzeSupportLevel;

//TypeCharacteristics
#define KA_TC_LOCATE_ALL_ITEMS      1
#define KA_TC_DONT_CHECK_OVERLAP    2
#define KA_TC_ROOT_ITEM             4

typedef
UINT32
(CALLBACK *PKANALYZE_METHOD_PROC)(
    IN HKAPLUGIN PlugInHandle,
    IN KA_ITEM_ID ItemId,
    IN ULONG32 MethodCode,
    IN va_list *Arguments
    );

typedef
UINT32
(CALLBACK *PKANALYZE_REGISTER_TYPES_PROC)(
    IN HKAPLUGIN PlugInHandle,
    IN LPCWSTR TypeName,
    IN LPCWSTR Description,
    IN ULONG32 ContainerTypeCount,
    IN HKATYPE *AllowedContainerTypes,
    IN ULONG32 TypeCharacteristics,
    IN KanalyzeSupportLevel SupportLevel,
    IN PKANALYZE_METHOD_PROC MethodInterface,
    OUT PHKATYPE TypeHandle
    );

//
// KA_LOCATE_ITEMS/ KA_CONFIRM_ITEM / KA_NOTIFY_ITEM
//
typedef struct _KANALYZE_DATA_ITEM {
    HKATYPE TypeHandle;
    UINT_PTR Address;
    SIZE_T Size;
    UINT_PTR TypeSpecificParam;
    PVOID TypeSpecificData;
    SIZE_T TypeSpecificDataSize;    // if 0, erase any existing data
} KANALYZE_DATA_ITEM, *PKANALYZE_DATA_ITEM;

typedef
UINT32
(CALLBACK *PKANALYZE_LOCATE_PROC)(
    IN HKAPLUGIN PlugInHandle,
    IN KA_ITEM_ID SourceDataItemId,  OPTIONAL
    IN PKANALYZE_DATA_ITEM DataItem,
    IN UINT32 Flags,
    OUT PKA_ITEM_ID DataItemId
    );

//
// If this flag is specified and the data item already exists,
// then any existing type-specific data (TypeSpecificParam,
// TypeSpecificData) is left alone and not overwritten.
//
#define KA_FLG_IGNORE_TYPE_SPECIFIC_DATA    0x00000001


//
// KA_TERMINATE
//
typedef enum {
    OutcomeFailure,
    OutcomeSuccessAnomalies,
    OutcomeSuccessNoAnomalies
} KanalyzeFinalOutcome;

//
// KA_FORMAT_DATA_ITEM_DESCRIPTION
//
typedef struct _KANALYZE_DATA_ITEM_STRING_REQUEST {
    LPCWSTR RequestingPlugInName;
    KA_ITEM_ID DataItemId;
    SIZE_T MessageBufferSize;
    LPWSTR MessageBuffer;
} KANALYZE_DATA_ITEM_STRING_REQUEST, *PKANALYZE_DATA_ITEM_STRING_REQUEST;




//////////////////////////////////////////////////
// Kanalyze Helper/Callback Routine Definitions
//////////////////////////////////////////////////

typedef struct _KANALYZE_CRASH_INFO {
    UINT32 SizeOfStruct;
    ULONG64 Flags;
    SIZE_T BugcheckCode;
    SIZE_T BugcheckParameters[4];
    ULONG32 BuildNumber;
    ULONG32 MachineImageType;
    ULONG32 ProcessorCount;
    ULONG32 CurrentProcessor;
} KANALYZE_CRASH_INFO, *PKANALYZE_CRASH_INFO;

typedef
UINT32
(CALLBACK *KANALYZE_GET_CRASH_INFO_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    OUT PKANALYZE_CRASH_INFO CrashInfo
    );

typedef
UINT32
(CALLBACK *KANALYZE_GET_SYMBOL_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN SIZE_T Address,
    OUT LPWSTR SymbolName,
    IN ULONG32 SymbolBufferSize,
    OUT SSIZE_T *Offset
    );

typedef
SIZE_T
(CALLBACK *KANALYZE_SYMBOL_TO_ADDRESS_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN LPCWSTR SymbolName
    );


typedef enum {
    BasicMessage,
    ExpandedMessage,
    DetailedMessage
} KanalyzeMessageDetailLevel;

typedef
BOOL
(CALLBACK *KANALYZE_CHECK_VERBOSITY_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN KanalyzeMessageDetailLevel DetailLevel,
    BOOL Reserved1,  //must be FALSE
    PVOID Reserved2  //must be NULL
    );


#define KAPARAM_DEFAULT_BINARY_PATH ((LPCWSTR)6)
#define KAPARAM_BINARY_SPEC_COUNT   ((LPCWSTR)7)
#define KAPARAM_BINARY_SPEC_NAME    ((LPCWSTR)8)
#define KAPARAM_BINARY_SPEC_PATH    ((LPCWSTR)9)
#define KAPARAM_DEFAULT_SYMBOL_PATH ((LPCWSTR)10)
#define KAPARAM_SYMBOL_SPEC_PATH    ((LPCWSTR)12)

typedef
INT32
(CALLBACK *KANALYZE_GET_PARAMETER_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN LPCWSTR ParameterName,
    IN UINT_PTR AdditionalInfo,
    OUT PVOID Buffer,
    IN UINT32 BufferSize
    );

typedef enum {
    LogFatalErrors,         // logged if logging enabled, printed to stderr
    LogErrors,              // logged if logging enabled, printed to stderr
    LogWarnings,            // logged if logging enabled
    LogInformation,         // logged if logging enabled
    LogDetailedInformation  // logged if logging enabled
} KanalyzeLogLevel;

typedef
UINT32
(CALLBACK *KANALYZE_WRITE_TO_LOG_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN KanalyzeLogLevel LogLevel,
    IN BOOL Reserved,
    IN LPCWSTR FormatString,        OPTIONAL
    IN UINT32 MessageId,            OPTIONAL
    IN va_list *arglist             OPTIONAL
    );

typedef
UINT32
(CALLBACK *KANALYZE_CALL_PLUGIN_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN LPCWSTR PlugInName,
    IN UINT32 PrivateOperationCode,
    IN OUT UINT_PTR Param1,
    IN OUT UINT_PTR Param2,
    OUT PBOOL ReachedPlugIn OPTIONAL
    );

typedef
BOOL
(CALLBACK *KANALYZE_CHECK_BREAK_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN UINT_PTR Reserved
    );

typedef
UINT32
(CALLBACK *KANALYZE_READ_MEMORY_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN PVOID Reserved,
    IN SIZE_T Address,
    OUT PVOID Buffer,
    IN ULONG32 ByteCount,
    OUT PULONG32 BytesRead
    );

typedef
UINT32
(CALLBACK *KANALYZE_READ_CONTROL_SPACE)(
    IN HKAPLUGIN PlugInHandle,
    IN PVOID     Reserved,
    IN USHORT    Processor,
    IN SIZE_T    TargetBaseAddress,
    OUT PVOID    UserInterfaceBuffer,
    IN ULONG32   ByteCount,
    OUT PULONG32 BytesRead
    );

typedef
UINT32
(CALLBACK *KANALYZE_WRITE_MEMORY_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN PVOID Reserved,  //must be NULL
    IN SIZE_T Address,
    IN CONST VOID *Buffer,
    IN ULONG32 ByteCount,
    OUT PULONG32 BytesWritten
    );

typedef
UINT32
(CALLBACK *KANALYZE_GET_CONTEXT_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN UINT_PTR Reserved,  //must be 0
    IN UINT32 Processor,
    OUT PCONTEXT Context
    );

typedef struct _KA_DATA_ITEM_INFO {
    HKATYPE TypeHandle;
    UINT_PTR Address;
    SIZE_T Size;
    UINT_PTR TypeSpecificParam;
    SIZE_T TypeSpecificDataSize;     // if 0, none
    WCHAR ReportedPlugInName[MAX_PLUG_IN_NAME];
} KA_DATA_ITEM_INFO, *PKA_DATA_ITEM_INFO;

typedef
UINT32
(CALLBACK *KANALYZE_QUERY_DATA_ITEM_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN KA_ITEM_ID DataItemId,
    IN UINT_PTR Unused,         //must be 0
    OUT PKA_DATA_ITEM_INFO DataItemInfo
    );

typedef
UINT32
(CALLBACK *PKANALYZE_ENUM_STORED_ITEM_CALLBACK)(
    IN HKAPLUGIN PlugInHandle,
    IN KA_ITEM_ID DataITemId,
    IN OUT PVOID Context
    );

//KA_ENUM_STORED_ITEM_SPECIFICATION Flags member's definition
#define KA_ESI_FLAG_USE_PLUGIN_HANDLE          1
#define KA_ESI_FLAG_USE_TYPE_HANDLE            2
#define KA_ESI_FLAG_ENUM_ALL_OVERLAPPED_ITEMS  4
#define KA_ESI_FLAG_SEARCH_TYPE_SPECIFIC_PARAM 8

typedef struct _KA_ENUM_STORED_ITEM_SPECIFICATION {
    ULONG32 Flags;           //KA_ESI_FLAG_XXX
    LPCWSTR ReportedPlugIn;
    LPCWSTR TypeSpecification;
    UINT_PTR StartAddress;
    UINT_PTR EndAddress;
    UINT_PTR TypeSpecificParam;
} KA_ENUM_STORED_ITEM_SPECIFICATION, *PKA_ENUM_STORED_ITEM_SPECIFICATION;

typedef
UINT32
(CALLBACK *KANALYZE_ENUM_STORED_ITEM_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN PKA_ENUM_STORED_ITEM_SPECIFICATION DataItemSpecification,
    IN UINT_PTR Spare1,                //must be 0
    IN UINT_PTR Spare2,                //must be 0
    IN PKANALYZE_ENUM_STORED_ITEM_CALLBACK Callback,
    IN OUT PVOID Context            //passed to callback routine as is
    );

typedef
HKATYPE
(CALLBACK *KANALYZE_GET_TYPE_HANDLE_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN LPCWSTR TypeName
    );

typedef struct _KA_TYPE_INFO {
    ULONG32 TypeCharacteristics;
    WCHAR TypeName[MAX_TYPE_NAME];
    WCHAR Description[MAX_TYPE_DESC];
    WCHAR SupportPlugInName[MAX_PLUG_IN_NAME];
    ULONG32 ContainerTypeCount;
    HKATYPE AllowedContainerType[MAX_CONTAINER_TYPES];
} KA_TYPE_INFO, *PKA_TYPE_INFO;

typedef
UINT32
(CALLBACK *KANALYZE_QUERY_TYPE_INFO_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN HKATYPE TypeHandle,
    IN UINT_PTR Unused,  //must be 0
    OUT PKA_TYPE_INFO TypeInfo
    );

typedef
UINT32
(CALLBACK *PKANALYZE_ENUM_TYPE_INFO_CALLBACK)(
    IN HKAPLUGIN PlugInHandle,
    IN HKATYPE TypeHandle,
    IN OUT PVOID Context
    );

typedef
UINT32
(CALLBACK *KANALYZE_ENUM_TYPE_INFO_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN LPCWSTR TypeSpecification,
    IN UINT_PTR Spare1,  //must be 0
    IN UINT_PTR Spare2,  //must be 0
    IN PKANALYZE_ENUM_TYPE_INFO_CALLBACK Callback,
    IN OUT PVOID Context
    );

typedef struct _KA_LINK_INFO {
    LPCWSTR ReportedPlugInName;
    KA_ITEM_ID SourceDataItemId;  //phase2
    KA_ITEM_ID TargetDataItemId;  //phase2
} KA_LINK_INFO, *PKA_LINK_INFO;

typedef
UINT32
(CALLBACK *PKANALYZE_ENUM_LINK_INFO_CALLBACK)(
    IN HKAPLUGIN PlugInHandle,
    IN CONST KA_LINK_INFO *LinkInfo,
    IN OUT PVOID Context
    );

typedef
UINT32
(CALLBACK *KANALYZE_ENUM_LINK_INFO_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN KA_ITEM_ID DataItemId,  //if 0, enumerate all links
    IN UINT_PTR Spare1,  //must be 0
    IN UINT_PTR Spare2,  //must be 0
    IN PKANALYZE_ENUM_LINK_INFO_CALLBACK Callback,
    IN OUT PVOID Context
    );

typedef
UINT32
(CALLBACK *KANALYZE_RETRIEVE_TYPE_SPECIFIC_DATA_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN KA_ITEM_ID DataItemId,
    IN SIZE_T TypeSpecificDataBufferSize,
    OUT PVOID TypeSpecificDataBuffer,
    OUT PSIZE_T TypeSpecificDataSize
    );

typedef
UINT32
(CALLBACK *KANALYZE_GET_DATA_ITEM_DESCRIPTION)(
    IN HKAPLUGIN PlugInHandle,
    IN KA_ITEM_ID DataItemId,
    IN SIZE_T MessageBufferSize,
    IN LPWSTR MessageBuffer,
    OUT PSIZE_T CaractersWritten  OPTIONAL
    );


typedef
UINT32
(CALLBACK *KANALYZE_CALL_METHOD_ROUTINE)(
    IN HKAPLUGIN PlugInHandle,
    IN KA_ITEM_ID ItemId,
    IN ULONG32 MethodCode,
    ...                     // arguments depend on MethodCode
    );


//////////////////////////////////////
// Table of Helper Callback Routines
//////////////////////////////////////

typedef struct _KANALYZE_EXTENSION_APIS {
    ULONG32 StructureSize;
    KANALYZE_GET_CRASH_INFO_ROUTINE GetCrashInfoRoutine;
    KANALYZE_GET_SYMBOL_ROUTINE GetSymbolRoutine;
    KANALYZE_SYMBOL_TO_ADDRESS_ROUTINE SymbolToAddressRoutine;
    KANALYZE_GET_PARAMETER_ROUTINE GetParameterRoutine;
    KANALYZE_WRITE_TO_LOG_ROUTINE WriteToLogRoutine;
    KANALYZE_CHECK_VERBOSITY_ROUTINE CheckVerbosityRoutine;
    KANALYZE_CALL_PLUGIN_ROUTINE CallPlugInRoutine;
    KANALYZE_CHECK_BREAK_ROUTINE CheckBreakRoutine;
    KANALYZE_READ_MEMORY_ROUTINE ReadMemoryRoutine;
    KANALYZE_WRITE_MEMORY_ROUTINE WriteMemoryRoutine;
    KANALYZE_GET_CONTEXT_ROUTINE GetContextRoutine;
//    KANALYZE_READ_CONTROL_SPACE ReadControlSpace;
    KANALYZE_GET_TYPE_HANDLE_ROUTINE GetTypeHandleRoutine;             //phase2
    KANALYZE_QUERY_TYPE_INFO_ROUTINE QueryTypeInfoRoutine;             //phase2
    KANALYZE_ENUM_TYPE_INFO_ROUTINE EnumTypeInfoRoutine;               //phase2
    KANALYZE_QUERY_DATA_ITEM_ROUTINE QueryDataItemRoutine;             //phase2
    KANALYZE_ENUM_STORED_ITEM_ROUTINE EnumStoredItemRoutine;           //phase2
    KANALYZE_ENUM_LINK_INFO_ROUTINE EnumLinkInfoRoutine;               //phase2
    KANALYZE_RETRIEVE_TYPE_SPECIFIC_DATA_ROUTINE RetrieveTypeSpecificDataRoutine;//phase2
    KANALYZE_GET_DATA_ITEM_DESCRIPTION GetDataItemDescriptionRoutine;  //phase2
    KANALYZE_CALL_METHOD_ROUTINE CallMethodRoutine;                    //phase2
} KANALYZE_EXTENSION_APIS, *PKANALYZE_EXTENSION_APIS;


////////////////////////////////
// PlugIn Registration Routine
////////////////////////////////

#define KANALYZE_INTERFACE_VERSION   2

typedef
HKAPLUGIN
(CALLBACK *KANALYZE_REGISTER_PLUGIN_PROC)(
    IN UINT Version,
    IN LPCWSTR Identifier,
    IN PKANALYZE_INTERFACE_PROC InterfaceRoutine,
    OUT PKANALYZE_EXTENSION_APIS HelperRoutineTable,
    PVOID Reserved1,
    UINT_PTR Reserved2
    );


////////////////////////////
// Common Export Routine
////////////////////////////

typedef
VOID
(CALLBACK *PKANALYZE_REGISTER_PLUG_INS)(
    IN KANALYZE_REGISTER_PLUGIN_PROC RegistrationRoutine,
    IN ULONG32 CrashPlatform,
    IN ULONG32 CrashBuildNumber,
    PVOID Reserved1,
    UINT_PTR Reserved2
    );

#define REGISTER_PLUG_INS_NAME   "KanalyzeRegisterPlugIns"
#define LREGISTER_PLUG_INS_NAME  L"KanalyzeRegisterPlugIns"


#ifdef KA_EXTAPI

extern KANALYZE_EXTENSION_APIS KanalyzeExtensionApis;

#define KxGetCrashInfo     (KanalyzeExtensionApis.GetCrashInfoRoutine)
#define KxGetSymbol        (KanalyzeExtensionApis.GetSymbolRoutine)
#define KxSymbolToAddress  (KanalyzeExtensionApis.SymbolToAddressRoutine)
#define KxGetParameter     (KanalyzeExtensionApis.GetParameterRoutine)
#define KxWriteToLog       (KanalyzeExtensionApis.WriteToLogRoutine)
#define KxCheckVerbosity   (KanalyzeExtensionApis.CheckVerbosityRoutine)
#define KxCallPlugIn       (KanalyzeExtensionApis.CallPlugInRoutine)
#define KxCheckBreak       (KanalyzeExtensionApis.CheckBreakRoutine)
#define KxReadMemory       (KanalyzeExtensionApis.ReadMemoryRoutine)
#define KxWriteMemory      (KanalyzeExtensionApis.WriteMemoryRoutine)
#define KxGetContext       (KanalyzeExtensionApis.GetContextRoutine)
//#define KxReadControlSpace (KanalyzeExtensionApis.ReadControlSpace)
#define KxGetTypeHandle    (KanalyzeExtensionApis.GetTypeHandleRoutine)
#define KxQueryTypeInfo    (KanalyzeExtensionApis.QueryTypeInfoRoutine)
#define KxEnumTypeInfo     (KanalyzeExtensionApis.EnumTypeInfoRoutine)
#define KxQueryDataItem    (KanalyzeExtensionApis.QueryDataItemRoutine)
#define KxEnumStoredItem   (KanalyzeExtensionApis.EnumStoredItemRoutine)
#define KxEnumLinkInfo     (KanalyzeExtensionApis.EnumLinkInfoRoutine)
#define KxRetrieveTypeSpecificData (KanalyzeExtensionApis.RetrieveTypeSpecificDataRoutine)
#define KxGetDataItemDescription (KanalyzeExtensionApis.GetDataItemDescriptionRoutine)
#define KxCallMethod       (KanalyzeExtensionApis.CallMethodRoutine)

#endif //KA_EXTAPI


//////////////////////////////////////
// Information for Overlap Checking
//////////////////////////////////////

//
// Anomaly type of AnomalyDescriptor\Overlap
// They are set at TypeSpecificParam.
//
#define KA_OVERLAP_ANOMALY_INVALID_CONTAINER    0x01
#define KA_OVERLAP_ANOMALY_INVALID_NESTING      0x02


#ifdef __cplusplus
}
#endif

#endif // _KANALYZE_

