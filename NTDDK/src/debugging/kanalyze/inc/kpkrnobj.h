/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    kapublic.h

Abstract:

    This includes definitions of methods and private interfaces
    for plug-ins for kernel object.

Author:

    MSKK OEM Tools Project staff(kktools) Oct. 29 1998

Revision History:

--*/

/*
  NOTE: If you does not have kernel header, you have to
       enable following definition.
*/
//#define NO_KERNEL_HEADER


///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "KERNELAPCDPC"
//
// Type                                 Method
// -----------------------------------------------------------------------
// KernelObject\Apc                     M_KOAPC_GET_APCMODE
//                                      M_KOAPC_GET_KTHREAD
//                                      M_KOAPC_GET_KKERNELROUTINE
//                                      M_KOAPC_GET_KRUNDOWNROUTINE
//                                      M_KOAPC_GET_KNORMALROUTINE
// KernelObject\KAPC_STATE              M_KOAPCSTATE_GET_KPROCESS
// KernelObject\Dpc                     M_KODPC_GET_DEFERREDCONTEXT
//                                      M_KODPC_GET_KDEFERREDROUTINE
//
// AnomalyDescriptor\KernelObject\Apc            none
// AnomalyDescriptor\KernelObject\KAPC_STATE     none
// AnomalyDescriptor\KernelObject\Dpc            none
//

#define KAPC_METHOD_BASE 0x00010000
#define KAPCSTATE_METHOD_BASE 0x00011000
#define KDPC_METHOD_BASE 0x00012000

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT INT * ProcessorMode
//    );
//
// Routine Description:
//
//     This function gets Processor mode from supplied
//     KernelObject\Apc type item.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Apc type item.
//
//     MethodCode - supplies M_KOAPC_GET_APCMODE.
//
//     ProcessorMode - recieves ProcessorMode.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOAPC_GET_APCMODE KAPC_METHOD_BASE+1

#ifdef NO_KERNEL_HEADER

typedef enum {
    KernelMode,
    UserMode,
    MaximumMode
} KAPCDPC_PROCESSOR_MODE ;

#else


#endif

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR KthreadAddr
//    );
//
// Routine Description:
//
//     This function gets address of the KTHREAD object that is
//     Stored in KernelObject\Apc member.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Apc type item.
//
//     MethodCode - supplies M_KOAPC_GET_KTHREAD.
//
//     KthreadAddr - recieves the address of the KTHREAD object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOAPC_GET_KTHREAD KAPC_METHOD_BASE+2

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR KernelRoutineAddr
//    );
//
// Routine Description:
//
//     This function gets Address of the APC kernel routine.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Apc type item.
//
//     MethodCode - supplies M_KOAPC_GET_KKERNELROUTINE.
//
//     KernelRoutineAddr - recieves the address of the APC kernel
//                        routine.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOAPC_GET_KKERNELROUTINE KAPC_METHOD_BASE+3

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR RundownRoutineAddr
//    );
//
// Routine Description:
//
//     This function gets the address of the APC run down
//     routine.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Apc type item.
//
//     MethodCode - supplies M_KOAPC_GET_KRUNDOWNROUTINE.
//
//     RundownRoutineAddr - recieves address of the APC run down
//                         routine.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOAPC_GET_KRUNDOWNROUTINE KAPC_METHOD_BASE+4

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR NormalRoutineAddr
//    );
//
// Routine Description:
//
//     This function gets address of the APC normal routine.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Apc type item.
//
//     MethodCode - supplies M_KOAPC_GET_KNORMALROUTINE.
//
//     NormalRoutineAddr - recieves address of the APC normal
//                        routine.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOAPC_GET_KNORMALROUTINE KAPC_METHOD_BASE+5

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR KProcessAddr
//    );
//
// Routine Description:
//
//     This function gest address of the process obejct that is stored
//     in KernelObject\KAPC_STATE type item.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\KAPC_STATE type item.
//
//     MethodCode - supplies M_KOAPCSTATE_GET_KPROCESS.
//
//     KProcessAddr - recieves address of the process object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOAPCSTATE_GET_KPROCESS KAPCSTATE_METHOD_BASE+1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PVOID * DeferredContext
//    );
//
// Routine Description:
//
//     This function gets DPC Deferred Context.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Dpc type item.
//
//     MethodCode - supplies M_KODPC_GET_DEFERREDCONTEXT
//
//     DefferedConext - recieves address of the Deferred Context.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KODPC_GET_DEFERREDCONTEXT   KDPC_METHOD_BASE+1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR DeferrredRoutineAddr
//    );
//
// Routine Description:
//
//     This function gets Address of the DPC deferred routine.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Dpc type item.
//
//     MethodCode - supplies M_KODPC_GET_KDEFERREDROUTINE.
//
//     DeferredRoutineAddr - recieves address of the DPC deferred
//                           routine.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KODPC_GET_KDEFERREDROUTINE KDPC_METHOD_BASE+2

///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "KERNELPROCTHRED"
//
// Type                                 Method
// -----------------------------------------------------------------------
// KernelObject\Process                 M_KOPROC_GET_BASIC_INFO
//                                      M_KOPROC_GET_OBJECT_DIR
//                                      M_KOPROC_GET_TIME_INFO
//                                      M_KOPROC_GET_QUANTUM
//                                      M_KOPROC_ENUM_THREADS
//                                      M_KOPROC_IS_VDM_PROCESS KPR
// KernelObject\Thread                  M_KOTHREAD_GET_BASIC_INFO
//                                      M_KOTHREAD_GET_STACK_INFO
//                                      M_KOTHREAD_GET_TIME_INFO
//                                      M_KOTHREAD_GET_WAIT_STATUS
//                                      M_KOTHREAD_GET_PRIORITY_INFO
//                                      M_KOTHREAD_GET_CONTEXT_INFO
//                                      M_KOTHREAD_IS_ALERTABLE
//                                      M_KOTHREAD_IS_KERNEL_STACK_RESIDENT
//                                      M_KOTHREAD_ENUM_WAITBLOCK
// KernelObject\WaitBlock               M_KOWBLOCK_GET_WAIT_OBJECT
//
// AnomalyDescriptor\KernelObject\Process        none
// AnomalyDescriptor\KernelObject\Thread         none
// AnomalyDescriptor\KernelObject\WaitBlock      none
//

#define KPROCESS_METHOD_BASE 0x00001000
#define KTHREAD_METHOD_BASE  0x00002000
#define WAITBLOCK_METHOD_BASE 0x00003000

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKPROC_BASIC_INFO BasicInfo
//    );
//
// Routine Description:
//
//     This function gets basic information of the process
//     object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Process type item.
//
//     MethodCode - supplies M_KOPROC_GET_BASIC_INFO.
//
//     BasicInfo - recieves basic information of the process object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOPROC_GET_BASIC_INFO KPROCESS_METHOD_BASE+1

typedef enum{
    InMemory,
    OutOfMemory,
    InTransition
} KPROC_STATE;

typedef struct _KPROC_BASIC_INFO {
    UINT32 BasePriority;
    KPROC_STATE State;
} KPROC_BASIC_INFO, * PKPROC_BASIC_INFO;

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT UINT_PTR * ObjectDir
//    );
//
// Routine Description:
//
//     This function gets address of the obejct directory.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Process type item.
//
//     MethodCode - supplies M_KOPROC_GET_OBJECT_DIR.
//
//     ObjectDir - recieves address of the object directory.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOPROC_GET_OBJECT_DIR KPROCESS_METHOD_BASE+2

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKPROC_TIME_INFO TimeInfo
//    );
//
// Routine Description:
//
//    This function gets time information of the process object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Process type item.
//
//     MethodCode - supplies M_KOPROC_GET_TIME_INFO.
//
//     TimeInfo - recieves time information of the process object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOPROC_GET_TIME_INFO  KPROCESS_METHOD_BASE+3

typedef struct _KPROC_TIME_INFO {
    ULONG32 KernelTime;
    ULONG32 UserTime;
} KPROC_TIME_INFO, * PKPROC_TIME_INFO;

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT UINT_PTR * Quantum
//    );
//
// Routine Description:
//
//     This function gets default thread quantum of the process obejct.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Process type item.
//
//     MethodCode - supplies MM_KOPROC_GET_QUANTUM.
//
//     Quantum - recieves default thread quantum of the process object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOPROC_GET_QUANTUM    KPROCESS_METHOD_BASE+4

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN PKOPROC_ENUM_THREAD_CALLBACK UserProc,
//    IN OUT PVOID UserContext
//    );
//
// Routine Description:
//
//     This function enumerates thread objects that is associated with
//     the process object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Process type item.
//
//     MethodCode - supplies M_KOPROC_ENUM_THREADS.
//
//     UserProc - supplies pointer to enumerate call back function.
//
//     UserContext - supplies pointer to context for enumerate call
//                   back function.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOPROC_ENUM_THREADS   KPROCESS_METHOD_BASE+5

typedef
UINT32
(CALLBACK * PKOPROC_ENUM_THREAD_CALLBACK)(
    IN UINT_PTR ThreadAddress,
    IN OUT PVOID Context
    );

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT BOOL * Result
//    );
//
// Routine Description:
//
//     This function probes the process objects is in VDM mode or not.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Process type item.
//
//     MethodCode - supplies M_KOPROC_IS_VDM_PROCESS.
//
//     Result - recieves result of the check. TRUE indicates the process
//              object is in VDM mode.
//
// Return Value:
//
//     Win32 error code.
//
// Remarks:
//
//    x86 only.
//
#define M_KOPROC_IS_VDM_PROCESS KPROCESS_METHOD_BASE+6


//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKTHRED_BASIC_INFO BasicInfo
//    );
//
// Routine Description:
//
//     This function gets basic information of the thread object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Thread type item.
//
//     MethodCode - supplies M_KOTHREAD_GET_BASIC_INFO.
//
//     BasicInfo - recieves basic information structure for the thread object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTHREAD_GET_BASIC_INFO KTHREAD_METHOD_BASE+1

#ifdef NO_KERNEL_HEADER

typedef enum {
    Initialized,
    Ready,
    Running,
    Standby,
    Terminated,
    Waiting,
    Transition
}KTHRED_STATUS;

#else // NO_KERNEL_HEADER

typedef KTHREAD_STATE KTHRED_STATUS;

#endif // NO_KERNEL_HEADER

typedef struct _KTHRED_BASIC_INFO{
    KTHRED_STATUS State;
    PVOID Win32Thread;
    PVOID Teb;
} KTHRED_BASIC_INFO, * PKTHRED_BASIC_INFO;

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKTHRED_STACK_INFO StackInfo
//    );
//
// Routine Description:
//
//     This function gets informations about stack in the thread object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Thread type item.
//
//     MethodCode - supplies M_KOTHREAD_GET_STACK_INFO.
//
//     StackInfo - recieves stack information.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTHREAD_GET_STACK_INFO KTHREAD_METHOD_BASE+2

typedef enum {
    Normal,
    StackSwapEnabled,
    LargeStackEnabled,
    LargeStackAndStackSwapEnabled
}KTHRED_STACK_TYPE;

typedef struct _KTHRED_STACK_INFO{
    UINT_PTR StackBase;
    UINT_PTR StackLimit;
    UINT_PTR KernelStack;
    UINT_PTR CallbackStack;
    KTHRED_STACK_TYPE StackType;
}KTHRED_STACK_INFO, * PKTHRED_STACK_INFO;

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKTHRED_TIME_INFO TimeInfo
//    );
//
// Routine Description:
//
//     This function gets time informatins in the trhead object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Thread type item.
//
//     MethodCode - supplies M_KOTHREAD_GET_TIME_INFO.
//
//     TimeInfo - recieves time information.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTHREAD_GET_TIME_INFO KTHREAD_METHOD_BASE+3

typedef struct _KTHRED_TIME_INFO {
    ULONG32 KernelTime;
    ULONG32 UserTime;
} KTHRED_TIME_INFO, * PKTHRED_TIME_INFO;

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKTHRED_WAIT_STATUS WaitStatus
//    );
//
// Routine Description:
//
//     This function gets informations about wait state of the thread object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Thread type item.
//
//     MethodCode - supplies M_KOTHREAD_GET_WAIT_STATUS.
//
//     WaitStatus - recieves information structure of the wait state.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTHREAD_GET_WAIT_STATUS KTHREAD_METHOD_BASE+4
#ifdef NO_KERNEL_HEADER

typedef enum {
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    MaximumWaitReason
}KTHRED_WAIT_REASON;

#else  // NO_KERNEL_HEADER

typedef KWAIT_REASON KTHRED_WAIT_REASON;

#endif  //NO_KERNEL_HEADER

#define KTHRED_KERNEL_MODE 0x01
#define KTHRED_USER_MODE 0x00
#define KTHRED_ALERTABLE_WAIT 0x10
#define KTHRED_NONALERTABLE_WAIT 0x00

typedef enum {
  NonAlertableUserMode = ( KTHRED_USER_MODE | KTHRED_NONALERTABLE_WAIT ),
  NonAlertableKernelMode = ( KTHRED_KERNEL_MODE | KTHRED_NONALERTABLE_WAIT ),
  AlertableUserMode = ( KTHRED_USER_MODE | KTHRED_ALERTABLE_WAIT ),
  AlertableKernelMode = ( KTHRED_KERNEL_MODE | KTHRED_ALERTABLE_WAIT )
}KTHRED_WAIT_MODE;

typedef struct _KTHRED_WAIT_STATUS{
    UINT_PTR WaitTime;
    UINT_PTR SuspendCount;
    UINT_PTR FreezeCount;
    KTHRED_WAIT_REASON WaitReason;
    KTHRED_WAIT_MODE WaitMode;
}KTHRED_WAIT_STATUS, * PKTHRED_WAIT_STATUS;

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKTHRED_PRIORITY_INFO PriorityInfo
//    );
//
// Routine Description:
//
//    This function gets informations about thread priority.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Thread type item.
//
//     MethodCode - supplies M_KOTHREAD_GET_PRIORITY_INFO.
//
//     PriorityInfo - recieves priority information.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTHREAD_GET_PRIORITY_INFO KTHREAD_METHOD_BASE+5

typedef struct _KTHRED_PRIORITY_INFO{
    UCHAR BasePriority;
    UCHAR DecrementCount;
    UCHAR PriorityDecrement;
    UCHAR Quantum;
} KTHRED_PRIORITY_INFO, * PKTHRED_PRIORITY_INFO;

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKTHRED_CONTEXT_INFO ContextInfo
//    );
//
// Routine Description:
//
//     This function gets processor context information about the thread object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Thread type item.
//
//     MethodCode - supplies M_KOTHREAD_GET_CONTEXT_INFO.
//
//     ContextInfo - recieves context information.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTHREAD_GET_CONTEXT_INFO KTHREAD_METHOD_BASE+6

typedef struct _KTHRED_CONTEXT_INFO{
    UCHAR NextProcessor;
    ULONG ContextSwitches;
}KTHRED_CONTEXT_INFO, * PKTHRED_CONTEXT_INFO;

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT BOOL * Alertable
//    );
//
// Routine Description:
//
//     This function probes supplied thread obejct is alertable or not.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Thread type item.
//
//     MethodCode - supplies M_KOTHREAD_IS_ALERTABLE.
//
//     Alertable - recieves result of the probe. TRUE indicates the thread
//                 object is alertable.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTHREAD_IS_ALERTABLE KTHREAD_METHOD_BASE+7

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT BOOL * KernelStackResident
//    );
//
// Routine Description:
//
//     This function probes the thread kernel stack is resident or not.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Thread type item.
//
//     MethodCode - supplies M_KOTHREAD_IS_KERNEL_STACK_RESIDENT.
//
//     KernelStackResident - recieves result of the probe. TRUE indicates
//                          kernel stack is regident.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTHREAD_IS_KERNEL_STACK_RESIDENT KTHREAD_METHOD_BASE+8

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN PKOTHRED_ENUM_WAITBLOCK_CALLBACK UserProc,
//    IN OUT PVOID UserContext
//    );
//
// Routine Description:
//
//     This function enumerates wait block objects that associated with
//     the thread object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Thread type item.
//
//     MethodCode - supplies M_KOTHREAD_ENUM_WAITBLOCK.
//
//     UserProc - supplies user defined enumerate wait block call back function.
//
//     UserContext - supplies context structure for enumerate wait block call
//                   back function.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTHREAD_ENUM_WAITBLOCK KTHREAD_METHOD_BASE+9

typedef struct _KTHRED_WAIT_BLOCK {
    UINT_PTR Thread;
    UINT_PTR Object;
    USHORT WaitKey;
    USHORT WaitType;
} KTHRED_WAIT_BLOCK, * PKTHRED_WAIT_BLOCK;

typedef
UINT32
(CALLBACK * PKOTHRED_ENUM_WAITBLOCK_CALLBACK)(
    IN UINT_PTR WaitBlockItemAddress,
    IN PKTHRED_WAIT_BLOCK WaitBlock,
    IN OUT PVOID Context
    );

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT UINT_PTR * WaitObjectAddress
//    );
//
// Routine Description:
//
//     This function gets address of the object that is waited by
//     the wait block object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\WaitBlock type item.
//
//     MethodCode - supplies M_KOWBLOCK_GET_WAIT_OBJECT.
//
//     WaitObjectAddress - recieves address of the object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOWBLOCK_GET_WAIT_OBJECT WAITBLOCK_METHOD_BASE+1


///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "KERNELMISCOBJ"
//
// Type                                 Method
// -----------------------------------------------------------------------
// KernelObject\DeviceQueue             M_KODEVQ_GET_CURRENT_TOP
//                                      M_KODEVQ_GET_CURRENT_TOP_BY_KEY
//                                      M_KODEVQ_IS_BUSY
//                                      M_KODEVQ_ENUM_ENTRIES
// KernelObject\DeviceQueueEntry        M_KODEVQENTRY_GET_KEY
// KernelObject\Interrupt               M_KOINTERRUPT_GET_VECTOR
//                                      M_KOINTERRUPT_GET_SERVICE_ROUTINE
//                                      M_KOINTERRUPT_GET_IRQL
//                                      M_KOINTERRUPT_GET_INTERRUPT_MODE
// KernelObject\Profile                 none
// KernelObject\ActiveProfileSource     none
// KernelObject\Queue                   M_KOQUEUE_GET_CURRENT_TOP
//                                      M_KOQUEUE_GET_SIGNAL_STATE
//
//
// AnomalyDescriptor\KernelObject\DeviceQueue            none
// AnomalyDescriptor\KernelObject\DeviceQueueEntry       none
// AnomalyDescriptor\KernelObject\Interrupt              none
// AnomalyDescriptor\KernelObject\Profile                none
// AnomalyDescriptor\KernelObject\ActiveProfileSource    none
// AnomalyDescriptor\KernelObject\Queue                  none
//

#define KQUEUE_METHOD_BASE 0x00004000
#define KDEVQ_METHOD_BASE 0x00005000
#define KDEVQENT_METHOD_BASE 0x00006000
#define KPROF_METHOD_BASE 0x00007000
#define KAPS_METHOD_BASE 0x00008000
#define KINTR_METHOD_BASE 0x00009000

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT UINT_PTR * TopAddress
//    );
//
// Routine Description:
//
//     This function gets address of the top entry in the device queue.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\DeviceQueue type item.
//
//     MethodCode - supplies M_KODEVQ_GET_CURRENT_TOP.
//
//     TopAddress - recieves address of the top entry in the device queue.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KODEVQ_GET_CURRENT_TOP KDEVQ_METHOD_BASE+1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN UINT32 Key,
//    OUT BOOL * Found,
//    OUT UINT_PTR * TopAddress
//    );
//
// Routine Description:
//
//     This function searches address of the top entry in the device queue by supplied key.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\DeviceQueue type item.
//
//     MethodCode - supplies MM_KODEVQ_GET_CURRENT_TOP_BY_KEY.
//
//     Key - supplies key value to search.
//
//     Found - recieves boolean value to indicate the device queue entry that
//             has the supplied key is found or not.
//
//     TopAddress - recieves address of the top entry in the device queue.
//                  If the entry is not found, this value is invalid.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KODEVQ_GET_CURRENT_TOP_BY_KEY KDEVQ_METHOD_BASE+2

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT BOOL * Busy
//    );
//
// Routine Description:
//
//     This function probes the device queue is busy or not.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\DeviceQueue type item.
//
//     MethodCode - supplies M_KODEVQ_IS_BUSY.
//
//     Busy - recieves result of the probe. TRUE indicates the device queue is
//            busy.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KODEVQ_IS_BUSY KDEVQ_METHOD_BASE+3

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN PKMISC_ENUM_DEBQENTRY_CALLBACK UserProc,
//    IN OUT PVOID UserContext
//    );
//
// Routine Description:
//
//     This function enumerates device queue entry that assciates with the
//     device queue object.  
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\DeviceQueue type item.
//
//     MethodCode - supplies M_KODEVQ_ENUM_ENTRIES.
//
//     UserProc - supplies pointer to user defined enumerate call back function. 
//
//     UserContext - supplies pointer to context for user defined enumerate call
//                  back function.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KODEVQ_ENUM_ENTRIES KDEVQ_METHOD_BASE+4

typedef 
UINT32
(CALLBACK * PKMISC_ENUM_DEVQENTRY_CALLBACK)(
    IN UINT_PTR DevQEntryAddress,
    IN OUT PVOID Context
    );

// Method codes for KernelObject\DeviceQueueEntry
//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT UINT32 * Key
//    );
//
// Routine Description:
//
//     This function gets key value from the device queue entry.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\DeviceQueueEntry type item.
//
//     MethodCode - supplies M_KODEVQENTRY_GET_KEY.
//
//     Key - recieves key value.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KODEVQENTRY_GET_KEY KDEVQENT_METHOD_BASE+1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKMISC_INTERRUPT_VECTOR_INFO VectorInfo
//    );
//
// Routine Description:
//
//     This function gets informations about the interrupt object's vector
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Interrupt type item.
//
//     MethodCode - supplies M_KOINTERRUPT_GET_VECTOR.
//
//     VectorInfo - recieves vector information.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOINTERRUPT_GET_VECTOR KINTR_METHOD_BASE+1

typedef struct _KMISC_INTERRUPT_VECTOR_INFO{
    ULONG32 Vector;
    BOOL Shared;
    BOOL Connected;
} KMISC_INTERRUPT_VECTOR_INFO, * PKMISC_INTERRUPT_VECTOR_INFO;

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKMISC_SERVICE_ROUTINE_INFO ServiceRoutineInfo
//    );
//
// Routine Description:
//
//     This function gets information about the interrupt service routine.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Interrupt type item.
//
//     MethodCode - supplies M_KOINTERRUPT_GET_SERVICE_ROUTINE.
//
//     ServiceRoutine - recieves information about the interrupt serviceroutine.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOINTERRUPT_GET_SERVICE_ROUTINE KINTR_METHOD_BASE+2

typedef struct _KMISC_SERVICE_ROUTINE_INFO {
    UINT_PTR RoutineAddress;
    UINT_PTR ContextAddress;
    UINT_PTR DispatchAddress;
} KMISC_SERVICE_ROUTINE_INFO, * PKMISC_SERVICE_ROUTINE_INFO;

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT UINT32 * Irql,
//    OUT UINT32 * SynchronizeIrql
//    );
//
// Routine Description:
//
//     This function gets IRQL and synchronize IRQL from the interrupt
//     object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Interrupt type item.
//
//     MethodCode - supplies  M_KOINTERRUPT_GET_IRQL.
//
//     Irql - recieves IRQL.
//
//     SynchronizeIrql - recieves synchronize IRQL.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOINTERRUPT_GET_IRQL KINTR_METHOD_BASE+3

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT KMISC_INTERRUPT_MODE * InterruptMode
//    );
//
// Routine Description:
//
//     This function gets interrupt mode from the interrupt object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Interrupt type item.
//
//     MethodCode - supplies _KOINTERRUPT_GET_INTERRUPT_MODE.
//
//     InterruptMode - recieves interrupt mode.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOINTERRUPT_GET_INTERRUPT_MODE KINTR_METHOD_BASE+4

#ifdef NO_KERNEL_HEADER

typedef enum {
    LevelSensitive,
    Latched
}KMISC_INTERRUPT_MODE;

#else

typedef KINTERRUPT_MODE KMISC_INTERRUPT_MODE;

#endif

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT UINT_PTR * TopAddr
//    );
//
// Routine Description:
//
//     This function gets address of the top entry of the queue object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Queue type item.
//
//     MethodCode - supplies M_KOQUEUE_GET_CURRENT_TOP.
//
//     TopAddr - recieves address of the top entry of the queue object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOQUEUE_GET_CURRENT_TOP KQUEUE_METHOD_BASE+1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT UINT32 * SignalState
//    );
//
// Routine Description:
//
//     This function gets signal state of the queue object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Queue type item.
//
//     MethodCode - supplies M_KOQUEUE_GET_SIGNAL_STATE.
//
//     SignalState - recieves signal state.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOQUEUE_GET_SIGNAL_STATE KQUEUE_METHOD_BASE+2

///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "KERNELSYNC"
//
// Type                                 Method
// -----------------------------------------------------------------------
// KernelObject\Event                   M_KOEVENT_GET_SIGNALSTATE
// KernelObject\EventPair               M_KOEVENTPAIR_GET_EVENT_LOW
//                                      M_KOEVENTPAIR_GET_EVENT_HIGH
// KernelObject\Mutant                  M_KOMUTANT_GET_SIGNALSTATE
//                                      M_KOMUTANT_GET_OWNER_THREAD
// KernelObject\Semaphore               M_KOSEMAPHORE_GET_SIGNALSTATE
//                                      M_KOSEMAPHORE_GET_LIMIT
//
// AnomalyDescriptor\KernelObject\Event        none
// AnomalyDescriptor\KernelObject\EventPair    none
// AnomalyDescriptor\KernelObject\Mutant       none
// AnomalyDescriptor\KernelObject\Semaphore    none
//

#define KEVENT_METHOD_BASE 0x00013000
#define KMUTANT_METHOD_BASE 0x00014000
#define KSEMAPHORE_METHOD_BASE 0x00015000
#define KEVENTPAIR_METHOD_BASE 0x00016000


//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT LONG * SignalState
//    );
//
// Routine Description:
//
//     This function gets signal state of the event object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Event type item.
//
//     MethodCode - supplies M_KOEVENT_GET_SIGNALSTATE.
//
//     SignalState - recieves signal state.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOEVENT_GET_SIGNALSTATE   KEVENT_METHOD_BASE+1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR EventLowAddr
//    );
//
// Routine Description:
//
//     This function gets address of the lower event object in event pair object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\EventPair type item.
//
//     MethodCode - supplies M_KOEVENTPAIR_GET_EVENT_LOW.
//
//     EventLowAddr - recieves address of the lower event.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOEVENTPAIR_GET_EVENT_LOW  KEVENTPAIR_METHOD_BASE+1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR EventHighAddr
//    );
//
// Routine Description:
//
//     This function gets address of the higher event object
//     in the event pair object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\EventPair type item.
//
//     MethodCode - supplies M_KOEVENTPAIR_GET_EVENT_HIGH.
//
//     EventHighAddr - recieves address of the higher event object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOEVENTPAIR_GET_EVENT_HIGH  KEVENTPAIR_METHOD_BASE+2

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT LONG * SignalState
//    );
//
// Routine Description:
//
//     This function gets signal state of the mutant object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Mutant type item.
//
//     MethodCode - supplies M_KOMUTANT_GET_SIGNALSTATE.
//
//     SignalState - recieves signal state.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOMUTANT_GET_SIGNALSTATE  KMUTANT_METHOD_BASE+1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR OwnerThreadAddr
//    );
//
// Routine Description:
//
//      This function gets address of the thread object that is owned by
//      supplied mutant object. 
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Mutant type item.
//
//     MethodCode - supplies MM_KOMUTANT_GET_OWNER_THREAD.
//
//     OwnerThreadAddr - recieves address of the thread object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOMUTANT_GET_OWNER_THREAD  KMUTANT_METHOD_BASE+2

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT LONG * SignalState
//    );
//
// Routine Description:
//
//     This function gets signal state of the semaphore object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Semaphore type item.
//
//     MethodCode - supplies M_KOSEMAPHORE_GET_SIGNALSTATE.
//
//     SignalState - recieves signal state.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOSEMAPHORE_GET_SIGNALSTATE KSEMAPHORE_METHOD_BASE+1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT LONG * Limit
//    );
//
// Routine Description:
//
//     This function gets limit of the semaphore object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Semaphore type item.
//
//     MethodCode - supplies M_KOSEMAPHORE_GET_LIMIT.
//
//     Limit - recieves limit of the semaphore object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOSEMAPHORE_GET_LIMIT KSEMAPHORE_METHOD_BASE+2

///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "KERNELTIMER"
//
// Type                                 Method
// -----------------------------------------------------------------------
// KernelObject\Timer                   M_KOTIMER_GET_DUETIME
//                                      M_KOTIMER_GET_DPC
//                                      M_KOTIMER_GET_SIGNALSTATE
//                                      M_KOTIMER_GET_PERIOD
//
// AnomalyDescriptor\KernelObject\Timer        none
//

#define KTIMER_METHOD_BASE 0x00017000

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT ULARGE_INTEGER * DueTime
//    );
//
// Routine Description:
//
//     This function gets due time of the timer object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Timer type item.
//
//     MethodCode - supplies M_KOTIMER_GET_DUETIME.
//
//     DueTime - recieves due time.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTIMER_GET_DUETIME  KTIMER_METHOD_BASE+1

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR DpcAddr
//    )
//
// Routine Description:
//
//     This function gets address of the DPC object in the timer object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Timer type item.
//
//     MethodCode - supplies M_KOTIMER_GET_DPC.
//
//     DpcAddr - recieves address of the DPC object.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTIMER_GET_DPC  KTIMER_METHOD_BASE+2

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT LONG * SignalState
//    )
//
// Routine Description:
//
//     This function gets signal state of the timer object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Timer type item.
//
//     MethodCode - supplies M_KOTIMER_GET_SIGNALSTATE.
//
//     SignalState - recieves signal state.
//
// Return Value:
//
//     Win32 error code.
//
#define M_KOTIMER_GET_SIGNALSTATE  KTIMER_METHOD_BASE+3

//UINT32
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT LONG * Period
//    )
//
// Routine Description:
//
//     This function gets period value from the timer object.
//
// Arguments:
//
//     PlugInHandle - supplies plug-in handle.
//
//     ItemId - supplies item ID for KernelObject\Timer type item.
//
//     MethodCode - supplies M_KOTIMER_GET_PERIOD.
//
//     Period - recieves period value.
//
// Return Value:
//
//     Win32 error code.
//
// Remarks:
//     NT4.0 or later.
//
#define M_KOTIMER_GET_PERIOD  KTIMER_METHOD_BASE+4
