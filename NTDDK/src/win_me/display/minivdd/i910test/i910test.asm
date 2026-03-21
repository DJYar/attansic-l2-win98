.386p
.xlist
include VMM.INC
include MINIVDD.INC
include VWIN32.INC
.list

Declare_Virtual_Device I910TEST, \
    1, \
    0, \
    I910_Control, \
    Undefined_Device_ID, \
    VDD_Init_Order, \
    , \
    , \
    ,

extern _I910_InitializeDriver@4:PROC
extern _I910_DeviceIoControl@16:PROC

VxD_DATA_SEG
public C WindowsVMHandle
WindowsVMHandle dd ?

public C DisplayInfoStructure
DisplayInfoStructure db size DISPLAYINFO dup(0)
VxD_DATA_ENDS

VxD_ICODE_SEG
public MiniVDD_Sys_Critical_Init
BeginProc MiniVDD_Sys_Critical_Init
    clc
    ret
EndProc MiniVDD_Sys_Critical_Init

public MiniVDD_Dynamic_Init
BeginProc MiniVDD_Dynamic_Init
    mov WindowsVMHandle, ebx

    lea eax, DisplayInfoStructure
    mov ecx, DISPLAYINFO_SIZE
    VxDCall VDD_Get_DISPLAYINFO

    mov eax, DWORD PTR DisplayInfoStructure.diDevNodeHandle
    push eax
    call _I910_InitializeDriver@4

    clc
    ret
EndProc MiniVDD_Dynamic_Init
VxD_ICODE_ENDS

VxD_LOCKED_CODE_SEG
Begin_Control_Dispatch I910
    Control_Dispatch Sys_Critical_Init, MiniVDD_Sys_Critical_Init
    Control_Dispatch Device_Init, MiniVDD_Dynamic_Init
    Control_Dispatch Sys_Dynamic_Device_Init, MiniVDD_Dynamic_Init
    Control_Dispatch W32_DeviceIoControl, _I910_DeviceIoControl@16, sCall, <ecx, ebx, edx, esi>
End_Control_Dispatch I910
VxD_LOCKED_CODE_ENDS

END
