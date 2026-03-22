#include "l2ndis.h"

NDIS_HANDLE g_NdisWrapperHandle = NULL;

NDIS_STATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    NDIS_STATUS status;
    NDIS_MINIPORT_CHARACTERISTICS miniChars;

    NdisMInitializeWrapper(&g_NdisWrapperHandle, DriverObject, RegistryPath, NULL);
    if (g_NdisWrapperHandle == NULL) {
        return NDIS_STATUS_FAILURE;
    }

    NdisZeroMemory(&miniChars, sizeof(miniChars));
    miniChars.MajorNdisVersion = 5;
    miniChars.MinorNdisVersion = 0;
    miniChars.CheckForHangHandler = L2MiniportCheckForHang;
    miniChars.InitializeHandler = L2MiniportInitialize;
    miniChars.HaltHandler = L2MiniportHalt;
    miniChars.QueryInformationHandler = L2MiniportQueryInformation;
    miniChars.ResetHandler = L2MiniportReset;
    miniChars.SendHandler = L2MiniportSend;
    miniChars.SetInformationHandler = L2MiniportSetInformation;

    status = NdisMRegisterMiniport(g_NdisWrapperHandle, &miniChars, sizeof(miniChars));
    if (status != NDIS_STATUS_SUCCESS) {
        NdisTerminateWrapper(g_NdisWrapperHandle, NULL);
        g_NdisWrapperHandle = NULL;
    }

    return status;
}
