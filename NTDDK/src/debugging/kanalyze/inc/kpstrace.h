/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    kapublic.h

Abstract:

    This includes definitions of methods and private interfaces
    for strace plug-in.

Author:

     Masahiro Furuichi(v-masufu)  04 December 1998

Revision History:

--*/

///////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "STRACE"
//
// Type                             Method
// -----------------------------------------------------------------------
// Stack                            M_STACK_GET_INITIAL_STACK
//                                  M_STACK_GET_STACK_LIMIT
//                                  M_STACK_GET_KERNERL_STACK
//                                  M_STACK_GET_STACK_BASE
//
// Stack\StackFrame                 M_STACKFRAME_GET_CALLED_PROCEDURE_ADDRESS
//                                  M_STACKFRAME_GET_CALLED_PROCEDURE_SYMBOL
//                                  M_STACKFRAME_GET_REURN_ADDRESS
//                                  M_STACKFRAME_GET_ARGUMENT_COUNT
//                                  M_STACKFRAME_GET_ARGUMENT
//                                  M_STACKFRAME_GET_TRUST_LEVEL
//                                  M_STACKFRAME_GET_FPO_DATA
//                                  M_STACKFRAME_GET_INSTRUCTION
//
// Stack\TrapFrame                  M_TRAPFRAME_GET_TRAPFRAME
//
// AnomalyDescriptor\*              none
//

#define PLUGIN_STRACE_OBJECT        L"STRACE"

#define STSTACK_METHOD_BASE         0x00001000
#define STSTACKFRAME_METHOD_BASE    0x00002000
#define STTRAPFRAME_METHOD_BASE     0x00003000


//
// Method codes for Stack
//

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT LPVOID InitialStack
// )
//
//Routine Description:
//
// This method gets initial stack of the specified Stack type item. 
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get initial stack of thread.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACK_GET_INITIAL_STACK.
//
// InitialStack - specifies a pointer to 32-bit value that receives initial stack.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACK_GET_INITIAL_STACK STSTACK_METHOD_BASE+1

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT LPVOID StackLimit
// )
//
//Routine Description:
//
// This method gets stack limit of the specified Stack type item. 
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get stack limit of thread.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACK_GET_STACK_LIMIT.
//
// StackLimit - specifies a pointer to 32-bit value that receives stack limit.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACK_GET_STACK_LIMIT STSTACK_METHOD_BASE+2

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT LPVOID KernelStack
// )
//
//Routine Description:
//
// This method gets kernel stack of the specified Stack type item. 
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get kernel stack of thread.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACK_GET_KERNERL_STACK.
//
// KernelStack - specifies a pointer to 32-bit value that receives kernel stack.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACK_GET_KERNERL_STACK STSTACK_METHOD_BASE+3

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT LPVOID StackBase
// )
//
//Routine Description:
//
// This method gets stack base of the specified Stack type item. 
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get stack base of thread.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACK_GET_STACK_BASE.
//
// StackBase - specifies a pointer to 32-bit value that receives stack base.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACK_GET_STACK_BASE STSTACK_METHOD_BASE+4

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT LPDWORD CalledProcAddress
// )
//
//Routine Description:
//
// This method gets called procedure address of the specified Stack\StackFrame type item. 
// In case that the previous instruction of the return-address, which can be obtained with
// M_STACKFRAME_GET_REURN_ADDRESS, has a operand including some registers, the called
// procedure address cannot be determined, so that this routine sets 0 into CalledProcAddress.
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get called procedure address.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACKFRAME_GET_CALLED_PROCEDURE_ADDRESS.
//
// CalledProcAddress - specifies a pointer to 32-bit value that receives called 
//                     procedure address.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACKFRAME_GET_CALLED_PROCEDURE_ADDRESS   STSTACKFRAME_METHOD_BASE+1

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PST_SYMBOL_CALLED_PROCEDURE CalledProcSymbol
// )
//
//Routine Description:
//
// This method gets called procedure symbol of the specified Stack\StackFrame type item. 
// A called procedure symbol is either symbol which can be obtained from Dbg file or
// assembly Operand, such as [EAX+01234567].
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to called procedure symbol.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACKFRAME_GET_CALLED_PROCEDURE_SYMBOL.
//
// CalledProcSymbol - specifies a pointer to ST_SYMBOL_CALLED_PROCEDURE structer that
//                    receives called procedure symbol.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACKFRAME_GET_CALLED_PROCEDURE_SYMBOL    STSTACKFRAME_METHOD_BASE+2

typedef struct _ST_SYMBOL_CALLED_PROCEDURE {
    WCHAR       FormatedSymbol[MAX_PATH+11];
} ST_SYMBOL_CALLED_PROCEDURE, *PST_SYMBOL_CALLED_PROCEDURE;

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT LPDWORD ReturnAddress
// )
//
//Routine Description:
//
// This method gets called return-address of the specified Stack\StackFrame type item. 
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get return-address.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACKFRAME_GET_REURN_ADDRESS.
//
// ReturnAddress - specifies a pointer to 32-bit value that receives return-address.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACKFRAME_GET_REURN_ADDRESS  STSTACKFRAME_METHOD_BASE+3

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT LPDWORD ArgumentCount
// )
//
//Routine Description:
//
// This method gets argument count of the specified Stack\StackFrame type item. 
// If argument count is unknown, this routine sets 0 into ArgumentCount.
// The reason why argument count is unknown is that
//      1) the called procedure address is unknown.
//      2) fpo frame type of the called procedure address is not fpo type.
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get argument count.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACKFRAME_GET_ARGUMENT_COUNT.
//
// ArgumentCount - specifies a pointer to 32-bit value that receives argument count.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACKFRAME_GET_ARGUMENT_COUNT STSTACKFRAME_METHOD_BASE+4

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   IN DWORD ArgumentIndex,
//   OUT LPVOID ArgumentValue
// )
//
//Routine Description:
//
// This method gets ArgumentIndex-th argument of the specified Stack\StackFrame type item. 
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get argument.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACKFRAME_GET_ARGUMENT.
//
// ArgumentIndex - specifies 0-based index of argument to get
//
// ArgumentValue - specifies a pointer to 32-bit value that receives Argument.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACKFRAME_GET_ARGUMENT   STSTACKFRAME_METHOD_BASE+5

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT Trustworthiness* TrustLevel
// )
//
//Routine Description:
//
// This method gets trustworthiness level of call-caller relationship for the specified
// Stack\StackFrame type item. 
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get trustworthiness level.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACKFRAME_GET_TRUST_LEVEL.
//
// TrustLevel - specifies a pointer to Trustworthiness that receives trustworthiness level.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACKFRAME_GET_TRUST_LEVEL    STSTACKFRAME_METHOD_BASE+6

#ifndef _STACKLIB_

typedef enum {
    TrustLow,
    TrustNormal,
    TrustHigh
} Trustworthiness;

#endif //_STACKLIB_

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PFPO_DATA* Fpo_data
// )
//
//Routine Description:
//
// This method gets fpo_data for the called procedure of the specified Stack\StackFrame type item. 
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get fpo_data.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACKFRAME_GET_FPO_DATA.
//
// Fpo_data - specifies a pointer to PFPO_DATA structer that receives fpo_data.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACKFRAME_GET_FPO_DATA    STSTACKFRAME_METHOD_BASE+7

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PST_INSTRUCTION Instruction
// )
//
//Routine Description:
//
// This method gets the previous instruction of the return-address for the specified
// Stack\StackFrame type item. 
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get instruction.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_STACKFRAME_GET_INSTRUCTION.
//
// Instruction - specifies a pointer to ST_INSTRUCTION structer that receives instruction.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_STACKFRAME_GET_INSTRUCTION    STSTACKFRAME_METHOD_BASE+8

#ifndef _STACKLIB_

//
// INSTRUCTION is used to represent a instruction symbolically.
//
// ex.)
//      instruction     : FF15A8211A80
//      assembly code   : CALL dword ptr [801A21A8]
//      Mnemonic        : CALL
//      Operand         : dword ptr [801A21A8]
//
typedef struct _ST_INSTRUCTION {
    WCHAR   Mnemonic[8];    // String that represents symbolically a opecode in assembly language
    WCHAR   Operand[31];    // String that represents symbolically opecodes in assembly language
} ST_INSTRUCTION, *PST_INSTRUCTION;

#endif // _STACKLIB_

// UINT32
// (CALLBACK * KANALYZE_CALL_METHOD_ROUTINE)(
//   IN HKAPLUGIN hPlugIn,
//   IN KA_ITEM_ID ItemId,
//   IN ULONG32 MethodCode,
//   OUT PST_TRAP_FRAME TrapFrame
// )
//
//Routine Description:
//
// This method gets trapframe for the specified Stack\StackFrame type item. 
// If specified item has no trapframe, this routine return FALSE.
//
//Arguments
//
// hPlugIn - specifies plug-in handle.
//
// ItemId - specifies data item ID to get a trapframe.
//
// MethodCode - specifies integer value to identify method. In this case, 
//              this value must be M_TRAPFRAME_GET_TRAPFRAME.
//
// TrapFrame - specifies a pointer to ST_TRAP_FRAME structer that receives a trapframe.
//
//Return Value
//
// If the function succeeds, the return value is NO_ERROR.
// If the function fails, the return value is WIN32 Error Code.
//
#define M_TRAPFRAME_GET_TRAPFRAME   STTRAPFRAME_METHOD_BASE+1

typedef struct _ST_TRAP_FRAME {

//
// Control registers
//
    ULONG Eip;
    ULONG Cs;
    ULONG EFlags;
    ULONG Esp;
    ULONG Ss;

//
//  Non-volatile registers
//
    ULONG Edi;
    ULONG Esi;
    ULONG Ebx;
    ULONG Ebp;

//
// Segment registers
//
    ULONG Gs;
    ULONG Es;
    ULONG Ds;
    ULONG Fs;

//
//  Volatile registers
//
    ULONG Edx;
    ULONG Ecx;
    ULONG Eax;

//
//  Debug registers.
//
    ULONG Dr0;
    ULONG Dr1;
    ULONG Dr2;
    ULONG Dr3;
    ULONG Dr6;
    ULONG Dr7;

} ST_TRAP_FRAME, *PST_TRAP_FRAME;
