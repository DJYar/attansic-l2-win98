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
// Public definitions for plug-in "IO"
//
// Type                                           Method
// -----------------------------------------------------------------------
// Io\Irp                                         M_IRP_GET_OWNER_THREAD
//                                                M_IRP_GET_MDL_ADDRESS
//                                                M_IRP_GET_ASSOCIATED_IRP
//                                                M_IRP_GET_IO_STACK_STATUS
//                                                M_IRP_ENUM_IO_STACK_INFO
// Io\DEVICE_OBJECT                               M_DEVICE_GET_DEVICE_NAME
//                                                M_DEVICE_GET_DRIVER
//                                                M_DEVICE_GET_NEXT_DEVICE
//                                                M_DEVICE_GET_ATTACHED_DEVICE
//                                                M_DEVICE_GET_CURRENT_IRP
//                                                M_DEVICE_GET_REFERENCE_COUNT
//                                                M_DEVICE_GET_DEVICE_TYPE
//                                                M_DEVICE_GET_VPB
//                                                M_DEVICE_GET_DEVICE_EXTENSION
//                                                M_DEVICE_ENUM_QUEUED_IRPS
// Io\DRIVER_OBJECT                               M_DRIVER_GET_DRIVER_NAME
//                                                M_DRIVER_ENUM_DEVICE
// Io\DEVICE_EXTENSION\Class                      M_DEVEXT_CLASS_GET_DEVICE
// Io\DEVICE_EXTENSION\Class2                     M_DEVEXT_CLASS2_GET_DEVICE
// Io\DEVICE_EXTENSION\ScsiPort                   M_DEVEXT_SCSIPORT_GET_DEVICE
//                                                M_DEVEXT_SCSIPORT_GET_HW_EXTENSION
// AnomalyDescriptor\Io\Irp                       none
// AnomalyDescriptor\Io\DEVICE_OBJECT             none
// AnomalyDescriptor\Io\DRIVER_OBJECT             none
// AnomalyDescriptor\Io\DEVICE_EXTENSION\Class    none
// AnomalyDescriptor\Io\DEVICE_EXTENSION\Class2   none
// AnomalyDescriptor\Io\DEVICE_EXTENSION\ScsiPort none
//
///////////////////////////////////////////////////////////////////////////
#define M_IO_BASE 0x00000000

//
// The following are method defines for ItemType "Io\Irp"
//

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR OwnerThread
//    )
// Routine Description:
//
//     This routine returns address of thread which owns specified IRP.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Irp item id.
//
//     MethodCode - specify M_IRP_GET_OWNER_THREAD.
//
//     OwnerThread - specify buffer address for UINT_PTR. This plug-in
//                   will fill the address of thread.
//
// Return Value:
//
//     Win32 error code
//
#define M_IRP_GET_OWNER_THREAD (M_IO_BASE + 1)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PUINT_PTR MdlAddress
//    )
// Routine Description:
//
//     This routine returns address of MDL which owns specified IRP.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Irp item id.
//
//     MethodCode - specify M_IRP_GET_MDL_ADDRESS.
//
//     OwnerThread - specify buffer address for UINT_PTR. This plug-in
//                   will fill the address of MDL.
//
// Return Value:
//
//     Win32 error code
//
#define M_IRP_GET_MDL_ADDRESS (M_IO_BASE + 2)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PIRP_ASSOCIATED_IRP AssociatedIrp;
//    )
// Routine Description:
//
//     This routine returns item id for MasterIrp or address of SystemBuffer or
//     IrpCount.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Irp item id.
//
//     MethodCode - specify M_IRP_GET_ASSOCIATED_IRP.
//
//     AssociatedIrp - specify buffer address for IRP_ASSOCIATED_IRP. This plug-in
//                     will fill the flag and related value.
//
// Return Value:
//
//     Win32 error code
//
#define M_IRP_GET_ASSOCIATED_IRP (M_IO_BASE + 3)
// These flags identifies a member of IRP_MASTER_IRP
#define MASTER_IRP    1
#define SYSTEM_BUFFER 2
#define IRP_COUNT     3

typedef struct _ASSOCIATED_IRP {
    USHORT Flags;
    union {
        KA_ITEM_ID MasterIrp;  // If Flags is MASTER_IRP
        UINT_PTR SystemBuffer; // If Flags is SYSTEMBUFFER
        LONG IrpCount;         // If Flags is IRP_COUNT
    } Value;
} ASSOCIATED_IRP, *PASSOCIATED_IRP;

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PIRP_IO_STACK_STATUS IoStackStatus
//    )
// Routine Description:
//
//     This routine returns status of IO_STACK_LOCATION for specified IRP.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Irp item id.
//
//     MethodCode - specify M_IRP_GET_IO_STACK_STATUS.
//
//     AssociatedIrp - specify buffer address for IRP_IO_STACK_STATUS. TThis plug-in
//                     will fill the member of the structure.
//
// Return Value:
//
//     Win32 error code
//
#define M_IRP_GET_IO_STACK_STATUS (M_IO_BASE + 4)

typedef struct _IRP_IO_STACK_STATUS {
    CHAR ActiveStacks;
    CHAR CurrentStack;
} IRP_IO_STACK_STATUS, *PIRP_IO_STACK_STATUS;

#ifndef _NT351
//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN PIRP_ENUM_IO_STACK_INFO Callback
//    IN OUT PVOID MethodContext
//    )
// Routine Description:
//
//     This routine pass information of IO_STACK_LOCATION for specified IRP
//     to given callback function.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Irp item id.
//
//     MethodCode - specify M_IRP_ENUM_IO_STACK_INFO.
//
//     Callback - pointer to callback function which receives IRP_STACK_INFO.
//
//     MethodContext - you can specify any type of data in order to use it in
//                     callback function.
//
// Return Value:
//
//     Win32 error code
//
#define M_IRP_ENUM_IO_STACK_INFO (M_IO_BASE + 5)

typedef struct _IRP_STACK_INFO {
    CHAR StackNum;
    UCHAR MajorFunction;
    UCHAR MinorFunction;
    UCHAR Flags;
    UCHAR Control;
    KA_ITEM_ID DeviceObject;
    KA_ITEM_ID FileObject;
    UINT_PTR CompletionRoutine;
    UINT_PTR Context;
    struct {
        UINT_PTR Argument1;
        UINT_PTR Argument2;
        UINT_PTR Argument3;
        UINT_PTR Argument4;
    } Param;
} IRP_STACK_INFO, *PIRP_STACK_INFO;

typedef
UINT32
(CALLBACK * PIRP_ENUM_IO_STACK_INFO)(
    IN PIRP_STACK_INFO StackInfo,
    IN OUT PVOID MethodContext
    );
#endif



//
// The following are method defines for ItemType "Io\DEVICE_OBJECT"
//

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN SIZE_T BufferSizeChars,
//    OUT LPCWSTR DeviceName,
//    OUT SIZE_T *CharsWrittenToBuffer
//    )
// Routine Description:
//
//     This routine return name of device for specified device object.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Object item id.
//
//     MethodCode - specify M_DEVICE_GET_DEVICE_NAME.
//
//     BufferSizeChars - number of chars including terminating NULL that DeviceName can holds.
//
//     DeviceName - pointer to buffer which will hold name of device.
//
//     CharsWrittenToBuffer - pointer to holds chars of device name. If BufferSizeChars from caller is not
//                            enough to hold name, this value is number of chars for description including
//                            teminating NULL. If not, this value is number of chars for description not
//                            including terminating NULL.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVICE_GET_DEVICE_NAME (M_IO_BASE + 6)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKA_ITEM_ID DriverObject
//    )
// Routine Description:
//
//     This routine return item id for driver object of specified device object.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Object item id.
//
//     MethodCode - specify M_DEVICE_GET_DRIVER.
//
//     DriverObject - pointer to buffer which will hold item id of driver object.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVICE_GET_DRIVER (M_IO_BASE + 7)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKA_ITEM_ID NextDeviceObject
//    )
// Routine Description:
//
//     This routine return item id for linked device object with specified device object.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Object item id.
//
//     MethodCode - specify M_DEVICE_GET_NEXT_DEVICE.
//
//     NextDeviceObject - pointer to buffer which will hold item id of device object.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVICE_GET_NEXT_DEVICE (M_IO_BASE + 11)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKA_ITEM_ID AttachedDeviceObject
//    )
// Routine Description:
//
//     This routine return item id for attached device object with specified device object.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Object item id.
//
//     MethodCode - specify M_DEVICE_GET_ATTACHED_DEVICE.
//
//     NextDeviceObject - pointer to buffer which will hold item id of device object.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVICE_GET_ATTACHED_DEVICE (M_IO_BASE + 12)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKA_ITEM_ID CurrentIrp
//    )
// Routine Description:
//
//     This routine return item id for IRP which is processed currently with specified device object.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Object item id.
//
//     MethodCode - specify M_DEVICE_GET_CURRENT_IRP.
//
//     NextDeviceObject - pointer to buffer which will hold item id of IRP.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVICE_GET_CURRENT_IRP (M_IO_BASE + 8)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PLONG ReferenceCount
//    )
// Routine Description:
//
//     This routine return reference count of specified deivce object.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Object item id.
//
//     MethodCode - specify M_DEVICE_GET_REFERENCE_COUNT.
//
//     ReferenceCount - pointer to buffer which will hold reference count.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVICE_GET_REFERENCE_COUNT (M_IO_BASE + 9)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PULONG DeviceType
//    )
// Routine Description:
//
//     This routine return type of device for specified deivce object.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Object item id.
//
//     MethodCode - specify M_DEVICE_GET_DEVICE_TYPE.
//
//     ReferenceCount - pointer to buffer which will hold type of device.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVICE_GET_DEVICE_TYPE (M_IO_BASE + 10)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT UINT_PTR Vpb
//    )
// Routine Description:
//
//     This routine return address of volume parameter block for specified deivce object.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Object item id.
//
//     MethodCode - specify M_DEVICE_GET_VPB.
//
//     ReferenceCount - pointer to buffer which will hold address of volume parameter block.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVICE_GET_VPB (M_IO_BASE + 13)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT PKA_ITEM_ID DeviceExtension
//    )
// Routine Description:
//
//     This routine return item id of device extension for specified deivce object.
//     You can specify item id of device object for Io\Deivce_Extension\...
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Object item id.
//
//     MethodCode - specify M_DEVICE_GET_DEVICE_EXTENSION.
//
//     ReferenceCount - pointer to buffer which will hold item id of device extension.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVICE_GET_DEVICE_EXTENSION (M_IO_BASE + 14)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN PDEVICE_ENUM_QUEUED_IRPS Callback
//    IN OUT PVOID MethodContext
//    )
// Routine Description:
//
//     This routine pass item id of IRP queued in device queue of specififed device object
//     to given callback function.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Object item id.
//
//     MethodCode - specify M_DEVICE_ENUM_QUEUED_IRPS.
//
//     Callback - pointer to callback function which receives item id of IRP.
//
//     MethodContext - you can specify any type of data in order to use it in
//                     callback function.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVICE_ENUM_QUEUED_IRPS (M_IO_BASE + 15)

typedef
UINT32
(CALLBACK * PDEVICE_ENUM_QUEUED_IRPS)(
    IN KA_ITEM_ID Irp,
    IN OUT PVOID MethodContext
    );



//
// The following are method defines for ItemType "Io\DRIVER_OBJECT"
//

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN SIZE_T BufferSizeChars,
//    OUT LPCWSTR DriverName
//    OUT SIZE_T *CharsWrittenToBuffer
//    )
// Routine Description:
//
//     This routine return name of driver for specified driver object.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Driver_Object item id.
//
//     MethodCode - specify M_DRIVER_GET_DRIVER_NAME.
//
//     BufferSizeChars - number of chars including terminating NULL that DriverName can holds.
//
//     DriverName - pointer to buffer which will hold name of device.
//
//     CharsWrittenToBuffer - pointer to holds chars of device name. If BufferSizeChars from caller is not
//                            enough to hold name, this value is number of chars for description including
//                            teminating NULL. If not, this value is number of chars for description not
//                            including terminating NULL.
//
// Return Value:
//
//     Win32 error code
//
#define M_DRIVER_GET_DRIVER_NAME (M_IO_BASE + 16)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    IN PDRIVER_ENUM_DEVICE Callback
//    IN OUT PVOID MethodContext
//    )
// Routine Description:
//
//     This routine pass item id of IRP queued in device queue of specififed device object
//     to given callback function.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Driver_Object item id.
//
//     MethodCode - specify M_DRIVER_ENUM_DEVICE.
//
//     Callback - pointer to callback function which receives item id of drier object.
//
//     MethodContext - you can specify any type of data in order to use it in
//                     callback function.
//
// Return Value:
//
//     Win32 error code
//
#define M_DRIVER_ENUM_DEVICE (M_IO_BASE + 17)

typedef
UINT32
(CALLBACK * PDRIVER_ENUM_DEVICE)(
    IN KA_ITEM_ID DeviceObject,
    IN OUT PVOID MethodContext
    );



//
// The following are method defines for ItemType "Io\DEVICE_EXTENSION\Class"
//

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT KA_ITEM_ID DeviceObject
//    )
// Routine Description:
//
//     This routine return item id for device object which is related with specified device extension.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Extension\Class item id.
//
//     MethodCode - specify M_DEVEXT_CLASS_GET_DEVICE.
//
//     NextDeviceObject - pointer to buffer which will hold item id of device object.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVEXT_CLASS_GET_DEVICE (M_IO_BASE + 18)



//
// The following are method defines for ItemType "Io\DEVICE_EXTENSION\Class2"
//

#ifndef _NT351
//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT KA_ITEM_ID DeviceObject
//    )
// Routine Description:
//
//     This routine return item id for device object which is related with specified device extension.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Extension\Class2 item id.
//
//     MethodCode - specify M_DEVEXT_CLASS2_GET_DEVICE.
//
//     NextDeviceObject - pointer to buffer which will hold item id of device object.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVEXT_CLASS2_GET_DEVICE (M_IO_BASE + 19)
#endif



//
// The following are method defines for ItemType "Io\DEVICE_EXTENSION\ScsiPort"
//

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT KA_ITEM_ID DeviceObject
//    )
// Routine Description:
//
//     This routine return item id for device object which is related with specified device extension.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Extension\ScsiPort item id.
//
//     MethodCode - specify M_DEVEXT_SCSIPORT_GET_DEVICE.
//
//     NextDeviceObject - pointer to buffer which will hold item id of device object.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVEXT_SCSIPORT_GET_DEVICE (M_IO_BASE + 20)

//BOOL
//(KANALYZE_CALL_METHOD_ROUTINE)(
//    IN HKAPLUGIN PlugInHandle,
//    IN KA_ITEM_ID ItemId,
//    IN ULONG32 MethodCode,
//    OUT KA_ITEM_ID HwDeviceExtension
//    )
// Routine Description:
//
//     This routine return item id for HwDeviceExtension which is related with specified device extension.
//
// Arguments:
//
//     PlugInHandle - specify your plug-in handle.
//
//     ItemId - specify Io\Device_Extension\ScsiPort item id.
//
//     MethodCode - specify M_DEVEXT_SCSIPORT_GET_HW_EXTENSION.
//
//     NextDeviceObject - pointer to buffer which will hold item id of HwDeviceExtension.
//
// Return Value:
//
//     Win32 error code
//
#define M_DEVEXT_SCSIPORT_GET_HW_EXTENSION (M_IO_BASE + 21)

