#include <windows.h>
#include <basetsd.h>

#include <stdio.h>
#include <stdlib.h>

#define KA_EXTAPI
#include "kanalyze.h"
#include "kppool.h"


KANALYZE_EXTENSION_APIS KanalyzeExtensionApis;

typedef struct _PERTAGINFO {
    ULONG Tag;
    ULONG Usage[2];
} PERTAGINFO, *PPERTAGINFO;

#define USAGE_PAGED     0
#define USAGE_NONPAGED  1

PPERTAGINFO PerTagInfo;
UINT TagCount;
UINT TagArraySize;

ULONG ErrorBlockCount[2];
ULONG QuotaBlockInfo[2];


UINT_PTR
CALLBACK
PoolTagListInterfaceRoutine(
    IN HKAPLUGIN PlugInHandle,
    IN UINT32 OperationCode,
    IN OUT UINT_PTR Param1,
    IN OUT PVOID Param2
    );

VOID
PoolTagListDoWork(
    IN HKAPLUGIN PlugInHandle
    );

int
__cdecl
PoolTagListSortCB(
    CONST VOID *p1,
    CONST VOID *p2
    );

BOOL
PoolTagListGetInfoSortedByTag(
    IN HKAPLUGIN PlugInHandle,
    IN HKATYPE TypeHandle,
    IN UINT ArrayIndex,
    IN BOOL StartOver
    );

UINT32
PoolTagListEnumItemCB(
    IN HKAPLUGIN PlugInHandle,
    IN KA_ITEM_ID DataItemId,
    PVOID Context
    );




VOID
CALLBACK
KanalyzeRegisterPlugIns(
    IN KANALYZE_REGISTER_PLUGIN_PROC RegistrationRoutine,
    IN ULONG CrashPlatform,
    IN ULONG CrashBuildNumber,
    PVOID Reserved1,
    UINT_PTR Reserved2
    )
{
    HKAPLUGIN hPlugIn;

    hPlugIn = RegistrationRoutine(
                    KANALYZE_INTERFACE_VERSION,
                    L"PoolTagList",
                    PoolTagListInterfaceRoutine,
                    &KanalyzeExtensionApis,
                    0,
                    0
                    );

    return;
}


UINT_PTR
CALLBACK
PoolTagListInterfaceRoutine(
    IN HKAPLUGIN PlugInHandle,
    IN UINT32 OperationCode,
    IN OUT UINT_PTR Param1,
    IN OUT PVOID Param2
    )
{
    UINT_PTR rc;

    switch(OperationCode) {

    case KA_INITIALIZE:
    case KA_REGISTER_TYPES:
    case KA_START_LOCATE_ITEMS:
    case KA_PROCESS_ITEM:
        rc = NO_ERROR;
        break;

    case KA_PROCESS_RESULT:
        PoolTagListDoWork(PlugInHandle);
        rc = NO_ERROR;
        break;

    default:
        rc = ERROR_INVALID_FUNCTION;
        break;
    }

    return(rc);
}


VOID
PoolTagListDoWork(
    IN HKAPLUGIN PlugInHandle
    )
{
    HKATYPE TypeHandle;
    UINT u;
    static BOOL FirstTime = TRUE;

    if(FirstTime) {

        TypeHandle = KxGetTypeHandle(PlugInHandle,L"PoolBlock\\NonPaged");
        if(!TypeHandle) {
            printf("Unable to find PoolBlock\\NonPaged type!\n");
            return;
        }

        if(!PoolTagListGetInfoSortedByTag(PlugInHandle,TypeHandle,USAGE_NONPAGED,TRUE)) {
            return;
        }

        TypeHandle = KxGetTypeHandle(PlugInHandle,L"PoolBlock\\Paged");
        if(!TypeHandle) {
            printf("Unable to find PoolBlock\\Paged type!\n");
            return;
        }

        if(!PoolTagListGetInfoSortedByTag(PlugInHandle,TypeHandle,USAGE_PAGED,FALSE)) {
            return;
        }

        //
        // Sort by tag
        //
        qsort(PerTagInfo,TagCount,sizeof(PERTAGINFO),PoolTagListSortCB);

        FirstTime = FALSE;
    }

    printf("Tag    paged     nonpaged  total\n");
    printf("-----  --------  --------  --------\n");

    printf(
        "QUOTA  %8x  %8x  %8x\n",
        QuotaBlockInfo[USAGE_PAGED],
        QuotaBlockInfo[USAGE_NONPAGED],
        QuotaBlockInfo[USAGE_PAGED] + QuotaBlockInfo[USAGE_NONPAGED]
        );

    for(u=0; u<TagCount; u++) {

        printf(
            "%c%c%c%c   %8x  %8x  %8x\n",
            ((PUCHAR)&PerTagInfo[u].Tag)[0],
            ((PUCHAR)&PerTagInfo[u].Tag)[1],
            ((PUCHAR)&PerTagInfo[u].Tag)[2],
            ((PUCHAR)&PerTagInfo[u].Tag)[3],
            PerTagInfo[u].Usage[USAGE_PAGED],
            PerTagInfo[u].Usage[USAGE_NONPAGED],
            PerTagInfo[u].Usage[USAGE_PAGED] + PerTagInfo[u].Usage[USAGE_NONPAGED]
            );
    }

    printf(
        "\n(Error blocks: %u paged; %u non-paged)\n",
        ErrorBlockCount[USAGE_PAGED],
        ErrorBlockCount[USAGE_NONPAGED]
        );
}


int
__cdecl
PoolTagListSortCB(
    CONST VOID *p1,
    CONST VOID *p2
    )
{
    CONST PERTAGINFO *t1,*t2;
    UCHAR s1[5],s2[5];

    t1 = p1;
    t2 = p2;

    CopyMemory(s1,&t1->Tag,4);
    CopyMemory(s2,&t2->Tag,4);

    s1[4] = 0;
    s2[4] = 0;

    return(_stricmp(s1,s2));
}


BOOL
PoolTagListGetInfoSortedByTag(
    IN HKAPLUGIN PlugInHandle,
    IN HKATYPE TypeHandle,
    IN UINT ArrayIndex,
    IN BOOL StartOver
    )
{
    KA_ENUM_STORED_ITEM_SPECIFICATION ItemSpec;
    UINT32 u;

    if(StartOver) {
        if(PerTagInfo) {
            LocalFree(PerTagInfo);
            PerTagInfo = NULL;
        }
        TagCount = 0;
        TagArraySize = 0;

        ZeroMemory(ErrorBlockCount,sizeof(ErrorBlockCount));
        ZeroMemory(QuotaBlockInfo,sizeof(QuotaBlockInfo));

        PerTagInfo = LocalAlloc(LPTR,50*sizeof(PERTAGINFO));
        if(!PerTagInfo) {
            return(FALSE);
        }

        TagCount = 0;
        TagArraySize = 50;
    }

    ZeroMemory(&ItemSpec,sizeof(KA_ENUM_STORED_ITEM_SPECIFICATION));
    ItemSpec.Flags = KA_ESI_FLAG_USE_TYPE_HANDLE;
    ItemSpec.TypeSpecification = (LPCWSTR)TypeHandle;
    ItemSpec.EndAddress = (UINT_PTR)(-1);

    u = KxEnumStoredItem(
            PlugInHandle,
            &ItemSpec,
            0,0,
            PoolTagListEnumItemCB,
            (PVOID)ArrayIndex
            );

    return(u == NO_ERROR);
}


UINT32
PoolTagListEnumItemCB(
    IN HKAPLUGIN PlugInHandle,
    IN KA_ITEM_ID DataItemId,
    PVOID Context
    )
{
    UINT32 u;
    KA_DATA_ITEM_INFO ItemInfo;
    POOL_BLOCK_BASIC_INFO BlockInfo;
    POOL_HEAD_BASIC_INFO HeaderInfo;
    PVOID p;

    if(KxCheckBreak(PlugInHandle,0)) {
        return(ERROR_CANCELLED);
    }

    //
    // Get info about the item (used for size).
    //
    if(KxQueryDataItem(PlugInHandle,DataItemId,0,&ItemInfo) != NO_ERROR) {
        ErrorBlockCount[(UINT_PTR)Context]++;
        return(NO_ERROR);
    }

    //
    // Find the header for this block.
    //
    u = KxCallMethod(
            PlugInHandle,
            DataItemId,
            M_POOL_BLOCK_GET_BASIC_INFO,
            &BlockInfo
            );

    if(u != NO_ERROR) {
        ErrorBlockCount[(UINT_PTR)Context]++;
        return(NO_ERROR);
    }

    //
    // The block may not actually have a header.
    //
    if(!BlockInfo.PoolHead) {
        return(NO_ERROR);
    }

    //
    // Now get info from the header.
    //
    u = KxCallMethod(
            PlugInHandle,
            BlockInfo.PoolHead,
            M_POOL_HEAD_GET_BASIC_INFO,
            &HeaderInfo
            );

    if(u != NO_ERROR) {
        ErrorBlockCount[(UINT_PTR)Context]++;
        return(NO_ERROR);
    }

    //
    // If the block is quota tracked, there's no tag.
    //
    if(HeaderInfo.Quota) {
        QuotaBlockInfo[(UINT_PTR)Context] += ItemInfo.Size;
        return(NO_ERROR);
    }

    //
    // OK, it's a normal block. Track its tag.
    // We use a dumb simple algorithm here.
    //
    HeaderInfo.PoolTag &= 0x7f7f7f7f;

    for(u=0; u<TagCount; u++) {
        if(PerTagInfo[u].Tag == HeaderInfo.PoolTag) {
            PerTagInfo[u].Usage[(UINT_PTR)Context] += ItemInfo.Size;
            return(NO_ERROR);
        }
    }

    if(TagCount < TagArraySize) {
        PerTagInfo[TagCount].Tag = HeaderInfo.PoolTag;
        PerTagInfo[TagCount].Usage[(UINT_PTR)Context] = ItemInfo.Size;
        TagCount++;
        return(NO_ERROR);
    }

    //
    // Need to grow the array.
    //
    p = LocalReAlloc(
            PerTagInfo,
            (TagArraySize+20)*sizeof(PERTAGINFO),
            LMEM_MOVEABLE|LMEM_ZEROINIT
            );

    if(!p) {
        return(ERROR_OUTOFMEMORY);
    }

    PerTagInfo = p;
    TagArraySize += 20;

    PerTagInfo[TagCount].Tag = HeaderInfo.PoolTag;
    PerTagInfo[TagCount].Usage[(UINT_PTR)Context] = ItemInfo.Size;
    TagCount++;

    return(NO_ERROR);
}
