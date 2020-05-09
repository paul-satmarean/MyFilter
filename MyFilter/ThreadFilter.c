#include "ThreadFilter.h"
#include "Communication.h"
#include "CommShared.h"
#include "Trace.h"
#include <ntstrsafe.h>
#include "ThreadFilter.tmh"

#define MSG_BUFFER_SIZE 511

//void
//ThreadFltSendMessageThreadCreate(
//	_In_ HANDLE ProcessId,
//	_In_ HANDLE ThreadId
//	)
//{
//	UNICODE_STRING string;
//	LARGE_INTEGER timestamp;
//	size_t stringSize = 0;
//	PMY_DRIVER_GENERIC_MESSAGE pMsg = NULL;
//	ULONG msgSize = sizeof(MY_DRIVER_GENERIC_MESSAGE);
//	MY_DRIVER_PROCESS_CREATE_MESSAGE_REPLY reply = { 0 };
//	ULONG replySize = sizeof(reply);
//	NTSTATUS status = STATUS_SUCCESS;
//
//	KeQuerySystemTimePrecise(&timestamp);
//
//	string.Buffer = ExAllocatePoolWithTag(PagedPool, MSG_BUFFER_SIZE, 'GSM+');
//	if (!string.Buffer) {
//		return;
//	}
//	string.MaximumLength = MSG_BUFFER_SIZE;
//	memset(string.Buffer, 0, sizeof(UCHAR) * MSG_BUFFER_SIZE);
//
//	status = RtlStringCchPrintfW(
//		string.Buffer,
//		MSG_BUFFER_SIZE,
//		L"[THREAD][%I64d] Thread %d started. ProcessId = %d \n",
//		timestamp.QuadPart,
//		ThreadId,
//		ProcessId
//		);
//
//	if (!NT_SUCCESS(status)) {
//		LogError("RtlStringCchPrintfW failed with status 0x%X", status);
//		return;
//	}
//
//	status = RtlStringCchLengthW(
//		string.Buffer,
//		MSG_BUFFER_SIZE,
//		&stringSize
//		);
//	
//	if (!NT_SUCCESS(status)) {
//		LogError("RtlStringCchLengthW failed with status 0x%X", status);
//		return;
//	}
//	string.Length = (USHORT)stringSize;
//
//	msgSize += string.Length;
//
//	pMsg = ExAllocatePoolWithTag(PagedPool, msgSize, 'GSM+');
//	
//	pMsg->DataLength = string.Length;;
//	memcpy(pMsg->Data, string.Buffer, string.Length);
//
//	pMsg->Header.MessageCode = msgGenericMessage;
//	status = CommSendMessage(
//		pMsg,
//		msgSize,
//		&reply,
//		&replySize
//		);
//	if (!NT_SUCCESS(status)) {
//		LogError("CommSendMessage failed with status = 0x%X", status);
//	}
//
//	ExFreePoolWithTag(pMsg, 'GSM+');
//	ExFreePoolWithTag(string.Buffer, 'GSM+');
//}
//
//void
//ThreadFltSendMessageThreadClose(
//	_In_ HANDLE ProcessId,
//	_In_ HANDLE ThreadId
//	)
//{
//	UNICODE_STRING string;
//	LARGE_INTEGER timestamp;
//	size_t stringSize = 0;
//	PMY_DRIVER_GENERIC_MESSAGE pMsg = NULL;
//	ULONG msgSize = sizeof(MY_DRIVER_GENERIC_MESSAGE);
//	MY_DRIVER_PROCESS_CREATE_MESSAGE_REPLY reply = { 0 };
//	ULONG replySize = sizeof(reply);
//	NTSTATUS status = STATUS_SUCCESS;
//
//	KeQuerySystemTimePrecise(&timestamp);
//
//	string.Buffer = ExAllocatePoolWithTag(PagedPool, MSG_BUFFER_SIZE, 'GSM+');
//	if (!string.Buffer) {
//		return;
//	}
//	string.MaximumLength = MSG_BUFFER_SIZE;
//	memset(string.Buffer, 0, sizeof(UCHAR) * MSG_BUFFER_SIZE);
//
//	status = RtlStringCchPrintfW(
//		string.Buffer,
//		MSG_BUFFER_SIZE,
//		L"[THREAD][%I64d] Thread %d started. ProcessId = %d \n",
//		timestamp.QuadPart,
//		ThreadId,
//		ProcessId
//		);
//
//	if (!NT_SUCCESS(status)) {
//		LogError("RtlStringCchPrintfW failed with status 0x%X", status);
//		return;
//	}
//
//	status = RtlStringCchLengthW(
//		string.Buffer,
//		MSG_BUFFER_SIZE,
//		&stringSize
//		);
//
//	if (!NT_SUCCESS(status)) {
//		LogError("RtlStringCchLengthW failed with status 0x%X", status);
//		return;
//	}
//	string.Length = (USHORT)stringSize;
//
//	msgSize += string.Length;
//
//	pMsg = ExAllocatePoolWithTag(PagedPool, msgSize, 'GSM+');
//
//	pMsg->DataLength = string.Length;;
//	memcpy(pMsg->Data, string.Buffer, string.Length);
//
//	pMsg->Header.MessageCode = msgGenericMessage;
//	status = CommSendMessage(
//		pMsg,
//		msgSize,
//		&reply,
//		&replySize
//		);
//	if (!NT_SUCCESS(status)) {
//		LogError("CommSendMessage failed with status = 0x%X", status);
//	}
//
//	ExFreePoolWithTag(pMsg, 'GSM+');
//	ExFreePoolWithTag(string.Buffer, 'GSM+');
//}


static VOID
ThreadFltNotifyRoutine(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
	)
{

	if ((gDrv.MonitoringStarted & notificationThread) == 0) {
		return;
	}

	UNICODE_STRING string;
	LARGE_INTEGER timestamp;
	size_t stringSize = 0;
	PMY_DRIVER_GENERIC_MESSAGE pMsg = NULL;
	ULONG msgSize = sizeof(MY_DRIVER_GENERIC_MESSAGE);
	MY_DRIVER_PROCESS_CREATE_MESSAGE_REPLY reply = { 0 };
	ULONG replySize = sizeof(reply);
	NTSTATUS status = STATUS_SUCCESS;

	KeQuerySystemTimePrecise(&timestamp);

	string.Buffer = ExAllocatePoolWithTag(PagedPool, MSG_BUFFER_SIZE, 'GSM+');
	if (!string.Buffer) {
		return;
	}
	string.MaximumLength = MSG_BUFFER_SIZE;
	memset(string.Buffer, 0, sizeof(UCHAR) * MSG_BUFFER_SIZE);

	if (Create) {
		status = RtlStringCchPrintfW(
			string.Buffer,
			MSG_BUFFER_SIZE,
			L"[THREAD][%I64d] Thread %d started. ProcessId = %d \n",
			timestamp.QuadPart,
			ThreadId,
			ProcessId
			);
	}
	else {
		status = RtlStringCchPrintfW(
			string.Buffer,
			MSG_BUFFER_SIZE,
			L"[THREAD][%I64d] Thread %d stopped. ProcessId = %d \n",
			timestamp.QuadPart,
			ThreadId,
			ProcessId
			);
	}
	

	if (!NT_SUCCESS(status)) {
		LogError("RtlStringCchPrintfW failed with status 0x%X", status);
		return;
	}

	status = RtlStringCchLengthW(
		string.Buffer,
		MSG_BUFFER_SIZE,
		&stringSize
		);

	if (!NT_SUCCESS(status)) {
		LogError("RtlStringCchLengthW failed with status 0x%X", status);
		return;
	}
	string.Length = (USHORT)stringSize;

	msgSize += string.Length;

	pMsg = ExAllocatePoolWithTag(PagedPool, msgSize, 'GSM+');

	pMsg->DataLength = string.Length;;
	memcpy(pMsg->Data, string.Buffer, string.Length);

	pMsg->Header.MessageCode = msgGenericMessage;
	status = CommSendMessage(
		pMsg,
		msgSize,
		&reply,
		&replySize
		);
	if (!NT_SUCCESS(status)) {
		LogError("CommSendMessage failed with status = 0x%X", status);
	}

	ExFreePoolWithTag(pMsg, 'GSM+');
	ExFreePoolWithTag(string.Buffer, 'GSM+');
	

}

NTSTATUS
ThreadFltInitialize()
{
	return PsSetCreateThreadNotifyRoutine(ThreadFltNotifyRoutine);
}

NTSTATUS
ThreadFltUninitialize()
{
	return PsRemoveCreateThreadNotifyRoutine(ThreadFltNotifyRoutine);
}