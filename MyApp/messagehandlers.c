//
//   Author(s)    : Radu PORTASE(rportase@bitdefender.com)
//

#include "messagehandlers.h"
#include <ntstatus.h>
#include "globaldata.h"
#include "CommShared.h"
#include <malloc.h>

//
// MsgHandleUnknownMessage
//
_Pre_satisfies_(InputBufferSize >= sizeof(FILTER_MESSAGE_HEADER))
_Pre_satisfies_(OutputBufferSize >= sizeof(FILTER_REPLY_HEADER))
NTSTATUS
MsgHandleUnknownMessage(
    _In_bytecount_(InputBufferSize) PFILTER_MESSAGE_HEADER InputBuffer,
    _In_ DWORD InputBufferSize,
    _Out_writes_bytes_to_opt_(OutputBufferSize, *BytesWritten) PFILTER_REPLY_HEADER OutputBuffer,
    _In_ DWORD OutputBufferSize,
    _Out_ PDWORD BytesWritten
)
{
    UNREFERENCED_PARAMETER(InputBufferSize);
    UNREFERENCED_PARAMETER(OutputBuffer);
    UNREFERENCED_PARAMETER(OutputBufferSize);
    PMY_DRIVER_MESSAGE_HEADER pHeader = (PMY_DRIVER_MESSAGE_HEADER)(InputBuffer + 1);

    wprintf(L"[Error] Unknown message received form driver. Id = %u", pHeader->MessageCode);

    *BytesWritten = 0;
    return STATUS_SUCCESS;
}

//
// MsgHandleProcessCreate
//
_Pre_satisfies_(InputBufferSize >= sizeof(FILTER_MESSAGE_HEADER))
_Pre_satisfies_(OutputBufferSize >= sizeof(FILTER_REPLY_HEADER))
NTSTATUS
MsgHandleProcessCreate(
    _In_bytecount_(InputBufferSize) PFILTER_MESSAGE_HEADER InputBuffer,
    _In_ DWORD InputBufferSize,
    _Out_writes_bytes_to_opt_(OutputBufferSize, *BytesWritten) PFILTER_REPLY_HEADER OutputBuffer,
    _In_ DWORD OutputBufferSize,
    _Out_ PDWORD BytesWritten
)
{
    PMY_DRIVER_PROCESS_NOTIFICATION_FULL_MESSAGE pInput = (PMY_DRIVER_PROCESS_NOTIFICATION_FULL_MESSAGE)InputBuffer;
    PMY_DRIVER_PROCESS_CREATE_FULL_MESSAGE_REPLY  pOutput = (PMY_DRIVER_PROCESS_CREATE_FULL_MESSAGE_REPLY)OutputBuffer;

    *BytesWritten = 0;
    if (InputBufferSize < sizeof(*pInput))
    {
        return STATUS_INVALID_USER_BUFFER;
    }

    if (OutputBufferSize < sizeof(*pOutput))
    {
        return STATUS_INVALID_USER_BUFFER;
    }

    if (sizeof(*pInput) + pInput->Message.ImagePathLength < sizeof(*pInput))
    {
        return STATUS_INTEGER_OVERFLOW;
    }

    if (InputBufferSize < sizeof(*pInput) + pInput->Message.ImagePathLength + pInput->Message.CmdLength)
    {
        return STATUS_INVALID_USER_BUFFER;
    }

    *BytesWritten = sizeof(*pOutput);
    pOutput->Reply.Status = STATUS_SUCCESS;

    if (!pInput->Message.ImagePathLength)
    {
        wprintf(L"[PROC][%I64d] Process Created: id = %d, path = NULL\n", pInput->Message.Timestamp.QuadPart, pInput->Message.ProcessId);
        return STATUS_SUCCESS;
    }

    PWCHAR path = malloc(pInput->Message.ImagePathLength + sizeof(WCHAR));
    if (!path)
    {
        wprintf(L"[PROC] Process Created: id = %d, path = BAD_ALLOC\n", pInput->Message.ProcessId);
        return STATUS_SUCCESS;
    }

    memcpy(path, &pInput->Message.Data[0], pInput->Message.ImagePathLength);
    path[pInput->Message.ImagePathLength >> 1] = L'\0';
   /* wprintf(L"[PROC] Process Created: id = %d, path = %s\n", pInput->Message.ProcessId, path);
    free(path);*/
    
    PWCHAR cmd = malloc(pInput->Message.CmdLength + sizeof(WCHAR));
    if (!cmd)
    {
        wprintf(L"[PROC][%I64d] Process Created: id = %d, path = %s cmd = BAD_ALLOC\n",pInput->Message.Timestamp.QuadPart, pInput->Message.ProcessId, path);
        free(path);
        return STATUS_SUCCESS;
    }

    memcpy(cmd, &pInput->Message.Data[pInput->Message.ImagePathLength], pInput->Message.CmdLength);
    cmd[pInput->Message.CmdLength >> 1] = L'\0';
    wprintf(L"[PROC][%I64d] Process Created: id = %d, path = %s cmd = %s\n", pInput->Message.Timestamp.QuadPart, pInput->Message.ProcessId, path, cmd);
    free(path);
    free(cmd);

    return STATUS_SUCCESS;
}

_Pre_satisfies_(InputBufferSize >= sizeof(FILTER_MESSAGE_HEADER))
_Pre_satisfies_(OutputBufferSize >= sizeof(FILTER_REPLY_HEADER))
NTSTATUS
MsgHandleProcessTerminate(
    _In_bytecount_(InputBufferSize) PFILTER_MESSAGE_HEADER InputBuffer,
    _In_  DWORD InputBufferSize,
    _Out_writes_bytes_to_opt_(OutputBufferSize, *BytesWritten) PFILTER_REPLY_HEADER OutputBuffer,
    _In_  DWORD OutputBufferSize,
    _Out_ PDWORD BytesWritten
)
{
	UNREFERENCED_PARAMETER(OutputBuffer);
	UNREFERENCED_PARAMETER(OutputBufferSize);

    PMY_DRIVER_PROCESS_TERMINATE_MSG_FULL msg = (PMY_DRIVER_PROCESS_TERMINATE_MSG_FULL)InputBuffer;

    if (!InputBuffer || InputBufferSize < sizeof(*msg))
    {
        return STATUS_INVALID_USER_BUFFER;
    }

    wprintf(L"[PROC][%I64d] Pid %u terminates. \n",msg->Msg.Timestamp.QuadPart, msg->Msg.ProcessId);

    *BytesWritten = 0;
	return STATUS_SUCCESS;
}

_Pre_satisfies_(InputBufferSize >= sizeof(FILTER_MESSAGE_HEADER))
_Pre_satisfies_(OutputBufferSize >= sizeof(FILTER_REPLY_HEADER))
NTSTATUS
MsgHandleRegMessage(
    _In_bytecount_(InputBufferSize) PFILTER_MESSAGE_HEADER InputBuffer,
    _In_  DWORD InputBufferSize,
    _Out_writes_bytes_to_opt_(OutputBufferSize, *BytesWritten) PFILTER_REPLY_HEADER OutputBuffer,
    _In_  DWORD OutputBufferSize,
    _Out_ PDWORD BytesWritten
    )
{
    PMY_DRIVER_REG_MESSAGE_FULL pInput = (PMY_DRIVER_REG_MESSAGE_FULL)InputBuffer;
    PMY_DRIVER_PROCESS_CREATE_FULL_MESSAGE_REPLY  pOutput = (PMY_DRIVER_PROCESS_CREATE_FULL_MESSAGE_REPLY)OutputBuffer;

    *BytesWritten = 0;
    if (InputBufferSize < sizeof(*pInput))
    {
        return STATUS_INVALID_USER_BUFFER;
    }

    if (OutputBufferSize < sizeof(*pOutput))
    {
        return STATUS_INVALID_USER_BUFFER;
    }

    if (sizeof(*pInput) + pInput->Message.NameLength < sizeof(*pInput))
    {
        return STATUS_INTEGER_OVERFLOW;
    }

    if (InputBufferSize < sizeof(*pInput) + pInput->Message.NameLength)
    {
        return STATUS_INVALID_USER_BUFFER;
    }
    *BytesWritten = sizeof(*pOutput);
    pOutput->Reply.Status = STATUS_SUCCESS;

    if (!pInput->Message.NameLength)
    {
        wprintf(
            L"[PROC][%I64d] Registry key created by pid = %d, thread = %d , path = NULL\n",
            pInput->Message.Timestamp.QuadPart, 
            pInput->Message.ProcessId,
            pInput->Message.ThreadId
            );
        return STATUS_SUCCESS;
    }

    PWCHAR name = malloc(pInput->Message.NameLength + sizeof(WCHAR));
    if (!name)
    {
        wprintf(
            L"[PROC][%I64d] Registry key created by pid = %d, thread = %d , path = BAD_ALLOC\n",
            pInput->Message.Timestamp.QuadPart,
            pInput->Message.ProcessId,
            pInput->Message.ThreadId
            );
        return STATUS_SUCCESS;
    }

    memcpy(name, &pInput->Message.Name[0], pInput->Message.NameLength);
    name[pInput->Message.NameLength >> 1] = L'\0';
 

    
    wprintf(
        L"[PROC][%I64d] Registry key created by pid = %d, thread = %d , path = %s\n",
        pInput->Message.Timestamp.QuadPart,
        pInput->Message.ProcessId,
        pInput->Message.ThreadId,
        name
        );

    free(name);

    return STATUS_SUCCESS;
}

_Pre_satisfies_(InputBufferSize >= sizeof(FILTER_MESSAGE_HEADER))
_Pre_satisfies_(OutputBufferSize >= sizeof(FILTER_REPLY_HEADER))
NTSTATUS
MsgHandleGeneric(
    _In_bytecount_(InputBufferSize) PFILTER_MESSAGE_HEADER InputBuffer,
    _In_  DWORD InputBufferSize,
    _Out_writes_bytes_to_opt_(OutputBufferSize, *BytesWritten) PFILTER_REPLY_HEADER OutputBuffer,
    _In_  DWORD OutputBufferSize,
    _Out_ PDWORD BytesWritten
    )
{
    PMY_DRIVER_GENERIC_MESSAGE_FULL pInput = (PMY_DRIVER_GENERIC_MESSAGE_FULL)InputBuffer;
    PMY_DRIVER_PROCESS_CREATE_FULL_MESSAGE_REPLY  pOutput = (PMY_DRIVER_PROCESS_CREATE_FULL_MESSAGE_REPLY)OutputBuffer;

    *BytesWritten = 0;
    if (InputBufferSize < sizeof(*pInput))
    {
        return STATUS_INVALID_USER_BUFFER;
    }

    if (OutputBufferSize < sizeof(*pOutput))
    {
        return STATUS_INVALID_USER_BUFFER;
    }

    if (sizeof(*pInput) + pInput->Message.DataLength < sizeof(*pInput))
    {
        return STATUS_INTEGER_OVERFLOW;
    }

    if (InputBufferSize < sizeof(*pInput) + pInput->Message.DataLength)
    {
        return STATUS_INVALID_USER_BUFFER;
    }

    *BytesWritten = sizeof(*pOutput);
    pOutput->Reply.Status = STATUS_SUCCESS;
    
    PWCHAR string = malloc(pInput->Message.DataLength + sizeof(WCHAR));
    if (!string)
    {
        wprintf(L"[PROC] Thread Created. BAD_ALLOC\n");
        return STATUS_SUCCESS;
    }

    memcpy(string, &pInput->Message.Data[0], pInput->Message.DataLength);
    string[pInput->Message.DataLength >> 1] = L'\0';

    wprintf(L"%s \n", string);

    free(string);

    return STATUS_SUCCESS;
}
//
// MsgDispatchNewMessage
//
_Pre_satisfies_(InputBufferSize >= sizeof(FILTER_MESSAGE_HEADER))
_Pre_satisfies_(OutputBufferSize >= sizeof(FILTER_REPLY_HEADER))
NTSTATUS
MsgDispatchNewMessage(
    _In_bytecount_(InputBufferSize) PFILTER_MESSAGE_HEADER InputBuffer,
    _In_ DWORD InputBufferSize,
    _Out_writes_bytes_to_opt_(OutputBufferSize, *BytesWritten) PFILTER_REPLY_HEADER OutputBuffer,
    _In_ DWORD OutputBufferSize,
    _Out_ PDWORD BytesWritten
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    if (InputBufferSize < sizeof(FILTER_MESSAGE_HEADER) + sizeof(MY_DRIVER_COMMAND_HEADER))
    {
        wprintf(L"[Error] Message size is too small to dispatch. Size = %d\n", InputBufferSize);
        return STATUS_BUFFER_TOO_SMALL;
    }

    PMY_DRIVER_MESSAGE_HEADER pHeader = (PMY_DRIVER_MESSAGE_HEADER)(InputBuffer + 1);

    switch (pHeader->MessageCode)
    {
    case msgProcessCreate:
        status = MsgHandleProcessCreate(InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, BytesWritten);
        break;
    case msgProcessTerminate:
        status = MsgHandleProcessTerminate(InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, BytesWritten);
        break;
    case msgRegistryMessage:
        status = MsgHandleRegMessage(InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, BytesWritten);

    case msgGenericMessage:
        status = MsgHandleGeneric(InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, BytesWritten);
        break;
    default:
        status =  MsgHandleUnknownMessage(InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, BytesWritten);
        break;
    }
    
    return status;
}
