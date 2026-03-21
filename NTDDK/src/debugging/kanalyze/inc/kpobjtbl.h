/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Public definitions for plug-in "OBJECTTABLE"
//
// Item Type                                         Method
// ---------------------------------------------------------------------------------------------
// ObjectTable\ObjectHeader                          M_OBJECTTABLE_HDR_GET_OBJECT_NAME
//                                                   M_OBJECTTABLE_HDR_GET_OBJECT_TYPE_NAME
//                                                   M_OBJECTTABLE_HDR_GET_QUOTA_INFO
//                                                   M_OBJECTTABLE_HDR_ENUM_OWNER_PROCESSES
//                                                   M_OBJECTTABLE_HDR_GET_FLAGS
//                                                   M_OBJECTTABLE_HDR_GET_PROBE_MODE
//                                                   M_OBJECTTABLE_HDR_GET_CREATOR_PROCESS
//                                                   M_OBJECTTABLE_HDR_GET_POINTER_COUNT
//                                                   M_OBJECTTABLE_HDR_GET_HANDLE_COUNT
//                                                   M_OBJECTTABLE_HDR_GET_OBJECT_BODY
//                                                   M_OBJECTTABLE_HDR_GET_PARENT_DIRECTORY
//
// ObjectTable\ObjectHandleTable                     M_OBJECTTABLE_HTB_GET_OWNER_PROCESS
//                                                   M_OBJECTTABLE_HTB_GET_TABLE_ENTRY_COUNT
//                                                   M_OBJECTTABLE_HTB_ENUM_OBJECTS
//
// ObjectTable\TypeObject                            M_OBJECTTABLE_TYP_GET_TYPE_NAME
//                                                   M_OBJECTTABLE_TYP_GET_STATISTICAL_INFO
//
// ObjectTable\DirectoryObject                       M_OBJECTTABLE_DIR_GET_DIRECTORY_ENTRY_COUNT
//                                                   M_OBJECTTABLE_DIR_ENUM_OBJECTS
//
// ObjectTable\SymbolicLinkObject                    M_OBJECTTABLE_SYM_GET_LINK_TARGET_NAME
//                                                   M_OBJECTTABLE_SYM_GET_LINK_TARGET_OBJECT
//
// AnomalyDescriptor\ObjectTable\ObjectHeader        None
// AnomalyDescriptor\ObjectTable\ObjectHandleTable   None
// AnomalyDescriptor\ObjectTable\TypeObject          None
// AnomalyDescriptor\ObjectTable\DirectoryObject     None
// AnomalyDescriptor\ObjectTable\SymbolicLinkObject  None
//
//
// Private Interface
// ---------------------------------------------------------------------------------------------
// P_OBJECTTABLE_GET_OBJECT_HEADER
// P_OBJECTTABLE_GET_ROOT_DIRECTORY_OBJECT
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#define  PLUGIN_OBJECTTABLE  L"OBJECTTABLE"

#define  MAX_OBJECT_TYPE_NAME_LENGTH  64
#define  MAX_OBJECT_NAME_LENGTH       256


//
// Private Interface definition
//
#define  P_OBJECTTABLE_BASE  0x80000000

#define  P_OBJECTTABLE_GET_OBJECT_HEADER  P_OBJECTTABLE_BASE + 0x0001
// UINT32
// (CALLBACK *KANALYZE_CALL_PLUGIN_ROUTINE)(
//     IN   HKAPLUGIN  hPlugIn,
//     IN   LPCWSTR    PlugInName,
//     IN   UINT32     PrivateOperationCode,
//     IN   UINT_PTR   ObjectBodyAddr           // address of object body to get header address
//     OUT  PUINT_PTR  ObjectHeaderAddr,        // receives the address of object header
//     OUT  PUINT_PTR  ReturnValue
// )

#define  P_OBJECTTABLE_GET_ROOT_DIRECTORY_OBJECT  P_OBJECTTABLE_BASE + 0x0002
// UINT32
// (CALLBACK *KANALYZE_CALL_PLUGIN_ROUTINE)(
//     IN      HKAPLUGIN  hPlugIn,
//     IN      LPCWSTR    PlugInName,
//     IN      UINT32     PrivateOperationCode,
//     OUT     PUINT_PTR  RootDirectoryObjectAddr,       // receives the address of root directory object
//     IN OUT  UINT_PTR   Unused,
//     OUT     PUINT_PTR  ReturnValue
// )


//
// Method Interface definition
//
#define  M_OBJECTTABLE_HDR_BASE 0x80001000

#define  M_OBJECTTABLE_HDR_GET_OBJECT_NAME  M_OBJECTTABLE_HDR_BASE + 0x0001
// UINT32
// (CALLBACK *KANALYZE_CALL_PLUGIN_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     IN   SIZE_T      BufferSizeChars,            // size of name buffer
//     OUT  LPWSTR      NameBuffer,                 // receives the object name of target item
//     OUT  SIZE_T      *CharsWrittenIntoBuffer     // number of characters written into the buffer
// )

#define  M_OBJECTTABLE_HDR_GET_OBJECT_TYPE_NAME  M_OBJECTTABLE_HDR_BASE + 0x0002
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     IN   SIZE_T      BufferSizeChars,            // size of name buffer
//     OUT  LPWSTR      NameBuffer,                 // receives the object type name of target item
//     OUT  SIZE_T      *CharsWrittenIntoBuffer     // number of characters written into the buffer
//     )

#define  M_OBJECTTABLE_HDR_GET_QUOTA_INFO        M_OBJECTTABLE_HDR_BASE + 0x0005
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN                    PlugInHandle,
//     IN   KA_ITEM_ID                   ItemId,
//     IN   ULONG32                      MethodCode,
//     OUT  POBJECTTABLE_HDR_QUOTA_INFO  QuotaInfo      // pointer to following structure
//     )
//
    typedef struct _OBJECTTABLE_HDR_QUOTA_INFO {
        ULONG     PagedPoolCharge;
        ULONG     NonPagedPoolCharge;
        ULONG     SecurityDescriptorCharge;
        UINT_PTR  ExclusiveProcessAddr;
    } OBJECTTABLE_HDR_QUOTA_INFO, *POBJECTTABLE_HDR_QUOTA_INFO;

#define  M_OBJECTTABLE_HDR_ENUM_OWNER_PROCESSES  M_OBJECTTABLE_HDR_BASE + 0x0006
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN  HKAPLUGIN                                   PlugInHandle,
//     IN  KA_ITEM_ID                                  ItemId,
//     IN  ULONG32                                     MethodCode,
//     IN  POBJECTTABLE_HDR_ENUM_OWNER_PROCESSES_PROC  Callback,        // pointer to callback routine
//         PVOID                                       Context          // pointer to the parameter which is handled to callback routine
//     )
//
    typedef
    UINT32                                                // if non Zero - to terminate callback
    (CALLBACK *POBJECTTABLE_HDR_ENUM_OWNER_PROCESSES_PROC)(
        IN  UINT_PTR  OwnerEProcessAddr,                // supplies the address of owner process(EPROCESS object)
                                                        // (if NULL - indicates end of enumeration)
            PVOID     Context
        );

#define  M_OBJECTTABLE_HDR_GET_FLAGS             M_OBJECTTABLE_HDR_BASE + 0x0007
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     OUT  UCHAR       *Flags              // receives the value of 'Flags' field in object header
//     )

#define  M_OBJECTTABLE_HDR_GET_PROBE_MODE        M_OBJECTTABLE_HDR_BASE + 0x0008
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     OUT  CCHAR       *Mode         // receives the value of 'ProbeMode' field in object header 
//     )

#define  M_OBJECTTABLE_HDR_GET_CREATOR_PROCESS   M_OBJECTTABLE_HDR_BASE + 0x0009
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     OUT  PHANDLE     ProcessHandle       // receives the value of 'CreatorUniqueProcess' field in object header
//     )

#define  M_OBJECTTABLE_HDR_GET_POINTER_COUNT     M_OBJECTTABLE_HDR_BASE + 0x000A
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     OUT  PLONG       PointerCount           // receives the value of 'PointerCount' field in object header
//     )

#define  M_OBJECTTABLE_HDR_GET_HANDLE_COUNT      M_OBJECTTABLE_HDR_BASE + 0x000B
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     OUT  PLONG       HandleCount            // receives the value of 'HandleCount' field in object header
//     )

#define  M_OBJECTTABLE_HDR_GET_OBJECT_BODY  M_OBJECTTABLE_HDR_BASE + 0x0004
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     OUT  PUINT_PTR   ObjectBodyAddr         // receives the object body address of target item
//     )

#define  M_OBJECTTABLE_HDR_GET_PARENT_DIRECTORY  M_OBJECTTABLE_HDR_BASE + 0x0003
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN    PlugInHandle,
//     IN   KA_ITEM_ID   ItemId,
//     IN   ULONG32      MethodCode,
//     OUT  PUINT_PTR    ParentDirectoryAddr   // receives the address of parent directory object
//     )


#define  M_OBJECTTABLE_HTB_BASE 0x80002000

#define  M_OBJECTTABLE_HTB_GET_OWNER_PROCESS  M_OBJECTTABLE_HTB_BASE + 0x0001
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     OUT  PUINT_PTR   OwnerEProcessAddr           // receives the address of owner process(EPROCESS object)
//     )

#define  M_OBJECTTABLE_HTB_GET_TABLE_ENTRY_COUNT  M_OBJECTTABLE_HTB_BASE + 0x0002
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     OUT  PULONG32    TableEntryCount             // receives the count of table entry
//     )

#define  M_OBJECTTABLE_HTB_ENUM_OBJECTS  M_OBJECTTABLE_HTB_BASE + 0x0003
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN                           PlugInHandle,
//     IN   KA_ITEM_ID                          ItemId,
//     IN   ULONG32                             MethodCode,
//     IN   POBJECTTABLE_HTB_ENUM_OBJECTS_PROC  Callback,       // pointer to callback routine
//          PVOID                               Context         // pointer to the parameter which is handled to callback routine
//     )
//
    typedef
    UINT32                                            // if non Zero - to terminate callback
    (CALLBACK *POBJECTTABLE_HTB_ENUM_OBJECTS_PROC)(
        IN  UINT_PTR  Handle,                       // supplies the handle value of target object in the handle table.
                                                    //  (if NULL - indicates end of enumeration)
        IN  UINT_PTR  ObjectBodyAddr,               // supplies the address of contained object.
                                                    //  (if NULL - indicates end of enumeration)
            PVOID     Context
        );


#define  M_OBJECTTABLE_TYP_BASE 0x80003000

#define  M_OBJECTTABLE_TYP_GET_TYPE_NAME  M_OBJECTTABLE_TYP_BASE + 0x0001
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     IN   SIZE_T      BufferSizeChars,
//     OUT  LPWSTR      NameBuffer,                 // receives the type name of target item
//     OUT  SIZE_T      *CharsWrittenIntoBuffer
//     )

#define  M_OBJECTTABLE_TYP_GET_STATISTICAL_INFO  M_OBJECTTABLE_TYP_BASE + 0x0002
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN                          PlugInHandle,
//     IN   KA_ITEM_ID                         ItemId,
//     IN   ULONG32                            MethodCode,
//     OUT  POBJECTTABLE_TYP_STATISTICAL_INFO  Statistics     // supplies the pointer to following structure
//     )
//
    typedef struct _OBJECTTABLE_TYP_STATISTICAL_INFO {
        ULONG TotalNumberOfObjects;
        ULONG TotalNumberOfHandles;
        ULONG HighWaterNumberOfObjects;
        ULONG HighWaterNumberOfHandles;
    } OBJECTTABLE_TYP_STATISTICAL_INFO, *POBJECTTABLE_TYP_STATISTICAL_INFO;


#define  M_OBJECTTABLE_DIR_BASE 0x80004000

#define  M_OBJECTTABLE_DIR_GET_DIRECTORY_ENTRY_COUNT  M_OBJECTTABLE_DIR_BASE + 0x0001
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     OUT  PULONG32    Count             // receives the directory entry count of target item
//     )

#define  M_OBJECTTABLE_DIR_ENUM_OBJECTS  M_OBJECTTABLE_DIR_BASE + 0x0002
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN                           PlugInHandle,
//     IN   KA_ITEM_ID                          ItemId,
//     IN   ULONG32                             MethodCode,
//     IN   POBJECTTABLE_DIR_ENUM_OBJECTS_PROC  Callback,           // supplies the pointer to callback routine
//          PVOID                               Context             // pointer to the parameter which is handled to callback routine
//     )
//
    typedef
    UINT32                                           // if non Zero - to terminate callback
    (CALLBACK *POBJECTTABLE_DIR_ENUM_OBJECTS_PROC)(
        IN  UINT_PTR  ObjectBodyAddr,               // supplies the address of contained object
                                                    // (if NULL - indicates end of enumeration)
            PVOID     Context
        );


#define  M_OBJECTTABLE_SYM_BASE 0x80005000

#define  M_OBJECTTABLE_SYM_GET_LINK_TARGET_NAME  M_OBJECTTABLE_SYM_BASE + 0x0001
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     IN   SIZE_T      BufferSizeChars,            // size of name buffer
//     OUT  LPWSTR      NameBuffer,                 // receives the name of link target object
//     OUT  SIZE_T      *CharsWrittenIntoBuffer     // number of characters written into the buffer
//     )

#define  M_OBJECTTABLE_SYM_GET_LINK_TARGET_OBJECT  M_OBJECTTABLE_SYM_BASE + 0x0002
// UINT32
// (KANALYZE_CALL_METHOD_ROUTINE)(
//     IN   HKAPLUGIN   PlugInHandle,
//     IN   KA_ITEM_ID  ItemId,
//     IN   ULONG32     MethodCode,
//     OUT  PUINT_PTR   LinkTargetObjectAddr        // receives the address of link target object
//     )
//
// This method is not applicable to Windows NT 3.51.
