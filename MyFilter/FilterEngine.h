#pragma warning (disable : 4201)
#ifndef __FILTER_ENGINE_H__
#define __FILTER_ENGINE_H__
#define NDIS_SUPPORT_NDIS6 1
#define INITUGID

#include <ndis.h>
#include <wdm.h>
#include <fwpsk.h>
#include <guiddef.h>
#include <fwpmk.h>

VOID NTAPI
ClassifyFn(
    IN const FWPS_INCOMING_VALUES0* inFixedValues,
    IN const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    IN OUT VOID* layerData,
    IN const FWPS_FILTER0* filter,
    IN UINT64  flowContext,
    IN OUT FWPS_CLASSIFY_OUT0* classifyOut
    );

NTSTATUS NTAPI
NotifyFn(
    IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    IN const GUID* filterKey,
    IN const FWPS_FILTER0* filter
    );

VOID NTAPI
FlowDeleteFn(
    IN UINT16  layerId,
    IN UINT32  calloutId,
    IN UINT64  flowContext
    );


DEFINE_GUID(
    MF_AUTH_RECV_ACCEPT_GUID,
    0xceee60f0,
    0x2614,
    0x4937,
    0x91, 0xe5, 0x7, 0x61, 0x24, 0x49, 0xbe, 0x43
    );

DEFINE_GUID(
    MF_AUTH_CONNECT_GUID,
    0x6c6f4c63,
    0xe81a,
    0x4e4b,
    0xaf, 0x38, 0x93, 0x83, 0xa5, 0x32, 0x93, 0x60
    );



#endif