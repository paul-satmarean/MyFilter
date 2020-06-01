#include "FilterEngine.h"
//#include "FilterEngine.tmh"
#include <fwpmk.h>


//VOID NTAPI
//AcceptClassifyFn(
//    IN const FWPS_INCOMING_VALUES0* inFixedValues,
//    IN const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
//    IN OUT VOID* layerData,
//    IN const FWPS_FILTER0* filter,
//    IN UINT64  flowContext,
//    IN OUT FWPS_CLASSIFY_OUT0* classifyOut
//    )


void AcceptClassifyFn(
    const FWPS_INCOMING_VALUES0* inFixedValues,
    const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    void* layerData,
    const void * classifyContext,
    const FWPS_FILTER3* filter,
    UINT64 flowContext,
    FWPS_CLASSIFY_OUT0* classifyOut
    )
{
    UNREFERENCED_PARAMETER(inFixedValues);
    UNREFERENCED_PARAMETER(inMetaValues);
    UNREFERENCED_PARAMETER(layerData);
    UNREFERENCED_PARAMETER(classifyContext);
    UNREFERENCED_PARAMETER(filter);
    UNREFERENCED_PARAMETER(flowContext);
    UNREFERENCED_PARAMETER(classifyOut);

}


NTSTATUS NTAPI
AcceptNotifyFn(
    IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    IN const GUID* filterKey,
    IN const FWPS_FILTER* filter
    )
{
    UNREFERENCED_PARAMETER(notifyType);
    UNREFERENCED_PARAMETER(filterKey);
    UNREFERENCED_PARAMETER(filter);
    return STATUS_SUCCESS;
}

VOID NTAPI
AcceptFlowDeleteFn(
    IN UINT16  layerId,
    IN UINT32  calloutId,
    IN UINT64  flowContext
    )
{
    UNREFERENCED_PARAMETER(layerId);
    UNREFERENCED_PARAMETER(calloutId);
    UNREFERENCED_PARAMETER(flowContext);
}


VOID NTAPI
ConnectClassifyFn(
    const FWPS_INCOMING_VALUES0* inFixedValues,
    const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    void* layerData,
    const void* classifyContext,
    const FWPS_FILTER3* filter,
    UINT64 flowContext,
    FWPS_CLASSIFY_OUT0* classifyOut
    )
{
    UNREFERENCED_PARAMETER(inFixedValues);
    UNREFERENCED_PARAMETER(inMetaValues);
    UNREFERENCED_PARAMETER(layerData);
    UNREFERENCED_PARAMETER(classifyContext);
    UNREFERENCED_PARAMETER(filter);
    UNREFERENCED_PARAMETER(flowContext);
    UNREFERENCED_PARAMETER(classifyOut);

}

NTSTATUS NTAPI
ConnectNotifyFn(
    IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    IN const GUID* filterKey,
    IN const FWPS_FILTER0* filter
    )
{
    UNREFERENCED_PARAMETER(notifyType);
    UNREFERENCED_PARAMETER(filterKey);
    UNREFERENCED_PARAMETER(filter);
    return STATUS_SUCCESS;
}

VOID NTAPI
ConnectFlowDeleteFn(
    IN UINT16  layerId,
    IN UINT32  calloutId,
    IN UINT64  flowContext
    )
{
    UNREFERENCED_PARAMETER(layerId);
    UNREFERENCED_PARAMETER(calloutId);
    UNREFERENCED_PARAMETER(flowContext);
}



//// Callout registration structure
//const FWPS_CALLOUT0 ConnectCallout =
//{
//    MF_AUTH_CONNECT_GUID,
//    0,      
//    ConnectClassifyFn,
//    ConnectNotifyFn,
//    ConnectFlowDeleteFn
//};
//
//
//// Callout registration structure
//const FWPS_CALLOUT0 AcceptCallout =
//{
//    MF_AUTH_RECV_ACCEPT_GUID,
//    0,      
//    AcceptClassifyFn,
//    AcceptNotifyFn,
//    AcceptFlowDeleteFn
//};

// {D637197D-CA9B-43A9-B925-2D30A8F3DFB9}



UINT32 gAcceptCalloutId;
UINT32 gConnectCalloutId;


NTSTATUS
RegisterFilterCallouts(
    PDEVICE_OBJECT DeviceObject
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE engineHandle = NULL;
    UNREFERENCED_PARAMETER(DeviceObject);
    FWPM_SUBLAYER InspectSubLayer;
    FWPM_SESSION session = { 0 };
    FWPS_CALLOUT sAcceptCallout;
    FWPM_CALLOUT mAcceptCallout;
    FWPS_CALLOUT sConnectCallout;
    FWPM_CALLOUT mConnectCallout;


    session.flags = FWPM_SESSION_FLAG_DYNAMIC;
    
    status = FwpmEngineOpen(
        NULL,
        RPC_C_AUTHN_WINNT,
        NULL,
        &session,
        &engineHandle
        );

    if (!NT_SUCCESS(status)) {
        KdPrint(("FwpmEngineOpen failed with %d", status));
        return status;
    }

    status = FwpmTransactionBegin(engineHandle, 0); 
    if (!NT_SUCCESS(status))
    {
        KdPrint(("FwpmTransactionBegin failed with %d", status));
        FwpmEngineClose(engineHandle);
        return status;
    }

    RtlZeroMemory(&InspectSubLayer, sizeof(FWPM_SUBLAYER));  

    InspectSubLayer.subLayerKey = MF_INSPECT_SUBLAYER_GUID;   
    InspectSubLayer.displayData.name = L"Transport Inspect Sub-Layer"; 
    InspectSubLayer.displayData.description = L"Sub-Layer for use by Transport Inspect callouts";
    InspectSubLayer.flags = 0;
    InspectSubLayer.weight = 0;

    status = FwpmSubLayerAdd(
        engineHandle,
        &InspectSubLayer, 
        NULL);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("FwpmSubLayerAdd failed with %d", status));
        FwpmTransactionAbort(engineHandle);
        FwpmEngineClose(engineHandle);
        return status;
    }

    // avem sublayer, adaugam callouts
    RtlZeroMemory(&sAcceptCallout, sizeof(FWPS_CALLOUT));
    RtlZeroMemory(&mAcceptCallout, sizeof(FWPM_CALLOUT));
    RtlZeroMemory(&sConnectCallout, sizeof(FWPS_CALLOUT));
    RtlZeroMemory(&mConnectCallout, sizeof(FWPM_CALLOUT));

    sAcceptCallout.calloutKey = MF_AUTH_RECV_ACCEPT_GUID;
    sAcceptCallout.classifyFn = AcceptClassifyFn;
    //sAcceptCallout.flowDeleteFn = AcceptFlowDeleteFn;
    //sAcceptCallout.notifyFn = AcceptNotifyFn;

    status = FwpsCalloutRegister(
        DeviceObject, 
        &sAcceptCallout,
        &gAcceptCalloutId);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("FwpsCalloutRegister failed with %d", status));
        FwpmTransactionAbort(engineHandle);
        FwpmEngineClose(engineHandle);
        return status;
    }


    mAcceptCallout.calloutKey = MF_AUTH_RECV_ACCEPT_GUID;
    mAcceptCallout.displayData.name = L"Transport Inspect ALE Classify Callout";
    mAcceptCallout.displayData.description = L"Intercepts inbound or outbound connect attempts";
    mAcceptCallout.applicableLayer = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;

    status = FwpmCalloutAdd(engineHandle,
        &mAcceptCallout,
        NULL,
        NULL);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("FwpmCalloutAdd failed with %d", status));
        FwpsCalloutUnregisterById(gAcceptCalloutId);
        FwpmTransactionAbort(engineHandle);
        FwpmEngineClose(engineHandle);
        return status;
    }

    // am adaugat primul callout, pume si filtru



    return status;
}