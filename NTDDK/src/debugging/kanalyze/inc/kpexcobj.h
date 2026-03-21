/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    kapublic.h

Abstract:

    This includes definitions of methods and private interfaces
    for plug-ins for executive object.

Author:

     Hirokazu Takeuchi ( v-hirokt ) Oct. 29 1998

Revision History:

--*/

/**************************************************************************
    If you want to build your plug-in for NT 3.51,
    enable following definition.
**************************************************************************/
//#define _NT351


///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "EXECUTIVEOBJECT"
//
// Type                                 Method
// -----------------------------------------------------------------------
// ExectiveObject\Process               M_EOPROCESS_GET_BASIC_INFO
//                                      M_EOPROCESS_GET_HANDLE_TABLE_INFO
//                                      M_EOPROCESS_GET_PAGE_INFO 
//                                      M_EOPROCESS_GET_MUTANT
//                                      M_EOPROCESS_GET_LOCKOWNER
//                                      M_EOPROCESS_GET_TIME_INFO
//                                      M_EOPROCESS_GET_POOL_USAGE_INFO
//                                      M_EOPROCESS_GET_WORKINGSET_INFO
//                                      M_EOPROCESS_GET_VM_INFO
//                                      M_EOPROCESS_GET_ACCESS_TOKEN
// ExecutiveObject\Thread               M_EOTHREAD_GET_BASIC_INFO
//                                      M_EOTHREAD_GET_LPC_MSG
//                                      M_EOTHREAD_GET_IMPERSONATION_INFO
//                                      M_EOTHREAD_GET_PERFORMANCE_COUNT_INFO
//                                      M_EOTHREAD_ENUM_IRPS
// ExecutiveObject\EventPair            TBD.
// ExecutiveObject\Profile              TBD.
// ExectuiveObject\Timer                TBD.
// ExecutiveObject\ControlerObject      TBD.
// ExecutiveObject\FileObject           TBD.
// ExecutiveObject\WindowStation        TBD.
// AnomalyDescriptor\...                none
//

#define PLUGIN_EXECUTIVE_OBJECT L"EXECUTIVEOBJECT"

#define EOPROC_METHOD_BASE  0x00011000
#define EOTHRED_METHOD_BASE 0x00012000
#define EOEVTPAIR_METHOD_BASE 0x00013000
#define EOPROF_METHOD_BASE 0x00014000
#define EOTIMER_METHOD_BASE 0x00015000
#define EOCTRLOBJ_METHOD_BASE 0x00016000
#define EOFILE_METHOD_BASE 0x00017000
#define EOWINSTATION_METHOD_BASE 0x00018000


//
// Method codes for ExecutiveObject\Process
//

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PEOBJ_PROCESS_BASIC_INFO BasicInfo
// )
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get process basic information.
//
// MethodCode - supplies integer value to identify method. In this case, 
//              this value must be M_EOPROCESS_GET_BASIC_INFO.
//
// BasicInfo - receives process basic information.
//
//Return Value
//
// This function returns boolean value to indicate this method is success or not. 
// TRUE indicates this method success. FALSE indicates any error is occurred. 
// To get extended error information, use GetLastError().
//
#define M_EOPROCESS_GET_BASIC_INFO EOPROC_METHOD_BASE+1

#define MAX_IMAGE_NAME_LENGTH 32

typedef struct _EOBJ_PROCESS_BASIC_INFO{
  UINT32 ProcessId;
  UINT32 ParentProcessId;
  UINT_PTR PebAddress;
  WCHAR ImageName[ MAX_IMAGE_NAME_LENGTH ];
} EOBJ_PROCESS_BASIC_INFO, *PEOBJ_PROCESS_BASIC_INFO;


// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT UINT_PTR * HandleTableAddress
// )
//
//Routine Description:
//
// This method gets handle table information from specified ExecutiveObject\Process type item. 
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get handle table information.
//
// MethodCode - supplies integer value to identify method. In this case, 
//              this value must be M_EOPROCESS_GET_HANDLE_TABLE_INFO.
//
// HandleTableAddress - receives handle table address.
//
//Return Value
//
// This function returns boolean value to indicate this method is success or not. 
// TRUE indicates this method success. FALSE indicates any error is occurred. 
// To get extended error information, use GetLastError().
//
#define M_EOPROCESS_GET_HANDLE_TABLE_INFO EOPROC_METHOD_BASE+2

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PEOBJ_PROCESS_PAGE_INFO PageInfo
// )
//
//Overview
//
// This method gets page usage information. Page usage information 
// includes VAD root address, Clone root address, etc.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item id to get page information.
//
// MethodCode - supplies integer value to identify method. In this case, 
//              this value must be M_EOPROCESS_GET_PAGE_INFO.
//
// PageInfo - receives page usage information.
//
//Return Value
//
// This function returns boolean value to indicate this method is success or not. 
// TRUE indicates this method success. FALSE indicates any error is occurred. 
// To get extended error information, use GetLastError().
//
#define M_EOPROCESS_GET_PAGE_INFO EOPROC_METHOD_BASE+3

typedef struct _EOBJ_PROCESS_PAGE_INFO{
  UINT_PTR VadRoot;
  UINT_PTR CloneRoot;
  UINT_PTR NumberOfPrivatePages;
  UINT_PTR ModifiedPageCount;
  UINT_PTR NumberOfLockedPages;
} EOBJ_PROCESS_PAGE_INFO, * PEOBJ_PROCESS_PAGE_INFO;

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT UINT_PTR * MutantAddress
// )
//
//Overview
//
// This method gets process mutant object address.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get process mutant.
//
// MethodCode - supplies integer value to identify method. In this case, 
//              this value must be M_EOPROCESS_GET_MUTANT.
//
// MutantAddress - receives address of process mutant object.
//
//Return Value
//
// This function returns boolean value to indicate this method is success or not. 
// TRUE indicates this method success. FALSE indicates any error is occurred. 
// To get extended error information, use GetLastError().
//
#define M_EOPROCESS_GET_MUTANT EOPROC_METHOD_BASE+4

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT UINT_PTR * LockOwnerAddress,
//   OUT UINT_PTR * LockCount
// )
//
//Overview
//
// This method gets address of lock owner thread.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get address of lock owner thread.
//
// MethodCode - supplies integer value to identify method. In this case, 
//              this value must be M_EOPROCESS_GET_LOCKOWNER.
//
// LockOwnerAddress - receives address of the lock owner thread. If lock 
//                    owner thread is not available, this value will be zero.
//
// LockCount - receives how many times lock owner thread is locked this 
//             process object.
//
//Return Value
//
// This function returns boolean value to indicate this method is success or not. 
// TRUE indicates this method success. FALSE indicates any error is occurred. 
// To get extended error information, use GetLastError().
//
#define M_EOPROCESS_GET_LOCKOWNER EOPROC_METHOD_BASE+5

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PEOBJ_PROCESS_TIME_INFO TimeInfo
// )
//
//Overview
//
// This method gets time information about process object. 
// This method gets running time (kernel mode/user mode) and 
// creation time of the process.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get running time information.
//
// MethodCode - supplies integer value to identify method. In this case, 
//              this value must be M_EOPROCESS_GET_TIME_INFO.
//
// TimeInfo - receives time information.
//
//Return Value
//
// This function returns boolean value to indicate this method is 
// success or not. TRUE indicates this method success. FALSE indicates 
// any error is occurred. To get extended error information, use GetLastError().
//
#define M_EOPROCESS_GET_TIME_INFO EOPROC_METHOD_BASE+6

typedef struct _EOBJ_PROCESS_TIME_INFO {
  UINT_PTR KernelTime;
  UINT_PTR UserTime;
  UINT_PTR CreationTime;
} EOBJ_PROCESS_TIME_INFO, * PEOBJ_PROCESS_TIME_INFO;


// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PEOBJ_PROCESS_POOL_USAGE_INFO PoolUsageInfo
// )
//
//Overview
//
// This method gets quota pool usage information.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get quota pool usage information.
//
// MethodCode - supplies integer value to identify method. In this case, 
//              this value must be M_EOPROCESS_GET_POOL_USAGE_INFO.
//
// PoolUsageInfo - receives quota pool usage information.
//
//Return Value
//
// This function returns boolean value to indicate this method is 
// success or not. TRUE indicates this method success. FALSE indicates any 
// error is occurred. To get extended error information, use GetLastError().
//
#define M_EOPROCESS_GET_POOL_USAGE_INFO EOPROC_METHOD_BASE+7

typedef struct _EOBJ_PROCESS_POOL_USAGE_INFO{
  UINT_PTR PagedPool;
  UINT_PTR NonpagedPool;
}EOBJ_PROCESS_POOL_USAGE_INFO, * PEOBJ_PROCESS_POOL_USAGE_INFO;

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PEOBJ_PROCESS_WORKINGSET_INFO WorkingSetInfo
// )
//
//Overview
//
// This method gets working set usage information.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get working set information.
//
// MethodCode - supplies integer value to identify method. In this case,
//              this value must be M_EOPROCESS_GET_WORKINGSET_INFO.
//
// WorkingSetInfo - receives working set information.
//
//Return Value
//
// This function returns boolean value to indicate this method is success or not. 
// TRUE indicates this method success. FALSE indicates any error is occurred. 
// To get extended error information, use GetLastError().
//
#define M_EOPROCESS_GET_WORKINGSET_INFO EOPROC_METHOD_BASE+8

typedef struct _EOBJ_PROCESS_WORKINGSET_INFO {
 SIZE_T CurrentSize;
 SIZE_T MinimumSize;
 SIZE_T MaximumSize;
 SIZE_T PeakWorkingSetSize;
} EOBJ_PROCESS_WORKINGSET_INFO, * PEOBJ_PROCESS_WORKINGSET_INFO;

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PEOBJ_PROCESS_VM_INFO VmInfo
// )
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get virtual memory usage information.
//
// MethodCode - supplies integer value to identify method. In this case, 
//              this value must be M_EOPROCESS_GET_VM_INFO.
//
// VmInfo - receives virtual memory usage information.
//
//Return Value
//
// This function returns boolean value to indicate this method is success or not. 
// TRUE indicates this method success. FALSE indicates any error is occurred. 
// To get extended error information, use GetLastError().
//
#define M_EOPROCESS_GET_VM_INFO EOPROC_METHOD_BASE+9

typedef struct _EOBJ_PROCESS_VM_INFO {
 SIZE_T VirtualSize;
 SIZE_T PeakVirtualSize;
 SIZE_T PageFaultCount;
 SIZE_T CommitCharge;
} EOBJ_PROCESS_VM_INFO, * PEOBJ_PROCESS_VM_INFO;

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT UINT_PTR * AccessTokenAddress
// )
//
//Overview
//
// This method gets address of access token that is owned by process.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get address of access token.
//
// MethodCode - supplies integer value to identify method. In this case,
//              this value must be M_EOPROCESS_GET_ACCESS_TOKEN.
//
// AccessTokenAddress - receives address of access token.
//
//Return Value
//
// This function returns boolean value to indicate this method is 
// success or not. TRUE indicates this method success. FALSE indicates
// any error is occurred. To get extended error information, use GetLastError().
//
#define M_EOPROCESS_GET_ACCESS_TOKEN EOPROC_METHOD_BASE+10


//
// Method codes for ExecutiveObject\Thread
//

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PEOBJ_THREAD_BASIC_INFO BasicInfo
// )
//
//Overview
//
// This method gets executive thread's basic information.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get .
//
// MethodCode - supplies integer value to identify method. 
//              In this case, this value must be M_EOTHREAD_GET_BASIC_INFO.
//
// BasicInfo - receives executive thread's basic information.
//
//Return Value
//
// This function returns boolean value to indicate this method is 
// success or not. TRUE indicates this method success. FALSE indicates any 
// error is occurred. To get extended error information, use GetLastError().
//
#define M_EOTHREAD_GET_BASIC_INFO EOTHRED_METHOD_BASE+1

typedef struct _EOBJ_THREAD_BASIC_INFO {
 UINT32 OwnerProcessId;
 UINT32 ThreadId;
 UINT_PTR StartAddress;
 UINT_PTR Win32StartAddress;
} EOBJ_THREAD_BASIC_INFO, * PEOBJ_THREAD_BASIC_INFO;

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT BOOL * LpcMsgAvailable,
//   OUT PEOBJ_THREAD_LPC_MSG_INFO LpcMsgInfo
// )
//
//Overview
//
// This method gets LPC message Information.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get .
//
// MethodCode - supplies integer value to identify method. 
//              In this case, this value must be M_EOTHREAD_GET_LPC_MSG.
//
// LpcMsgAvailable - receives boolean value to indicate the 
//                   thread has a pointer to LPC message or not.
//                   TRUE indicates the thread has a LPC message. 
//                   FALSE indicates the thread does not have a 
//                   LPC message.
//
// LpcMsgInfo - receives LPC message information. If the thread 
//              does not have a LPC message, this value will be 
//              filled by zero.
//
//Return Value
//
// This function returns boolean value to indicate this method is 
// success or not. TRUE indicates this method success. FALSE indicates 
// any error is occurred. To get extended error information, use GetLastError().
//
#define M_EOTHREAD_GET_LPC_MSG EOTHRED_METHOD_BASE+2

typedef struct _EOBJ_THREAD_LPC_MSG_INFO {
 BOOL ReceivedLpcMsgIdValid;
 UINT_PTR ReceivedLpcMsgId;
 UINT_PTR ReplyLpcMsg;
 UINT_PTR ReplyLpcMsgId;
} EOBJ_THREAD_LPC_MSG_INFO, * PEOBJ_THREAD_LPC_MSG_INFO;


#if ! defined( _NT351 )
// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT UINT_PTR * ImpersonationInfoAddr
// )
//
//Overview
//
// This method gets address of the thread's impersonation information.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get address of the thread's 
//          impersonation information.
//
// MethodCode - supplies integer value to identify method. In this case, 
//              this value must be M_EOTHREAD_GET_IMPERSONATION_INFO.
//
// ImpersonationInfoAddr - receives address of the impersonation information.
//
//Return Value
//
// This function returns boolean value to indicate this method is success or not. 
// TRUE indicates this method success. FALSE indicates any error is occurred. 
// To get extended error information, use GetLastError().
//
#define M_EOTHREAD_GET_IMPERSONATION_INFO EOTHRED_METHOD_BASE+3
#endif // ! defined( _NT3.51 )


// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PEOBJ_THREAD_PERFORMANCE_COUNT_INFO PerformanceCountInfo
// )
//
//Overview
//
// This method gets thread's performance count information.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to get performance count information.
//
// MethodCode - supplies integer value to identify method. In this case, 
//              this value must be M_EOTHREAD_GET_PERFORMANCE_COUNT_INFO.
//
// PerformanceCountInfo - receives performance count information.
//
//Return Value
//
// This function returns boolean value to indicate this method is success or not. 
// TRUE indicates this method success. FALSE indicates any error is occurred.
// To get extended error information, use GetLastError().
//
#define M_EOTHREAD_GET_PERFORMANCE_COUNT_INFO EOTHRED_METHOD_BASE+4

typedef struct _EOBJ_THREAD_PERFORMANCE_COUNT_INFO {
  UINT_PTR PerformanceCountHigh;
  UINT_PTR PerformanceCountLow;
} EOBJ_THREAD_PERFORMANCE_COUNT_INFO, * PEOBJ_THREAD_PERFORMANCE_COUNT_INFO;

// BOOL
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   IN PEOBJ_THREAD_ENUM_IRPS_CALLBACK CallbackProc,
//   IN PVOID Context
// )
//
//Overview
//
// This method enumerates associated IRPs with thread object.
//
//Arguments
//
// hPlugIn - supplies plug-in handle.
//
// ItemId - supplies data item ID to enumerate associated IRPs.
//
// MethodCode - supplies integer value to identify method. 
//              In this case, this value must be M_EOTHREAD_ENUM_IRP.
//
// CallbackProc - supplies user defined call back function to process IRPs.
//
// Context - supplies user defined context structure to supply context 
//           information to call back procedure.
//
//Return Value
//
// This function returns boolean value to indicate this method is success or not. 
// TRUE indicates this method success. FALSE indicates any error is occurred. 
// To get extended error information, use GetLastError().
//
//Remarks
//
// If supplied call back function fails, this method stops enumeration immediately.
//
#define M_EOTHREAD_ENUM_IRPS EOTHRED_METHOD_BASE+5

typedef
UINT32
(CALLBACK * PEOBJ_THREAD_ENUM_IRPS_CALLBACK)(
  IN UINT_PTR IrpAddress,
  IN OUT PVOID Context
);


//
// Method codes for ExecutiveObject\EventPair
//

//
// Method codes for ExecutiveObject\Profile
//

//
// Method codes for ExecutiveObejct\Timer
//

//
// Method codes for ExecutiveObject\ControllerObject
//

//
// Method codes for ExecutiveObject\FileObject
//

//
// Method codes for ExecutiveObject\WindowStation
//

