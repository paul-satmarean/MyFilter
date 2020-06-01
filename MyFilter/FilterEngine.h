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
#include <initguid.h>

DEFINE_GUID(
    MF_INSPECT_SUBLAYER_GUID,
    0xd637197d,
    0xca9b,
    0x43a9,
    0xb9, 0x25, 0x2d, 0x30, 0xa8, 0xf3, 0xdf, 0xb9);

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