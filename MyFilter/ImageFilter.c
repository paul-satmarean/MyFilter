#include "ImageFilter.h"
#include "CommShared.h"
#include "Communication.h"
#include "Trace.h"
#include <ntstrsafe.h>
#include "ImageFilter.tmh"

#define MSG_BUFFER_SIZE 511


static VOID
ImgFltNotifyRoutine(
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ HANDLE ProcessId,
	_In_ PIMAGE_INFO ImageInfo
	)
{

	UNREFERENCED_PARAMETER(ImageInfo);

	if ((gDrv.MonitoringStarted & notificationImage) == 0) {
		return;
	}

	UNICODE_STRING string;
	UNICODE_STRING format;
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


	if (FullImageName) {
		RtlInitUnicodeString(&format, L"[IMAGE][%I64d] Image %s loaded. ProcessId = %d");
	}
	else {
		RtlInitUnicodeString(&format, L"[IMAGE][%I64d] Image loaded. ProcessId = %d");
	}

	status = RtlStringCchPrintfW(
		string.Buffer,
		MSG_BUFFER_SIZE,
		format.Buffer,
		timestamp.QuadPart,
		FullImageName,
		ProcessId
		);

	if (!NT_SUCCESS(status)) {
		LogError("RtlStringCchPrintfW failed with status 0x%X", status);
		ExFreePoolWithTag(string.Buffer, 'GSM+');
		return;
	}

	status = RtlStringCchLengthW(
		string.Buffer,
		MSG_BUFFER_SIZE,
		&stringSize
		);

	if (!NT_SUCCESS(status)) {
		LogError("RtlStringCchLengthW failed with status 0x%X", status);
		ExFreePoolWithTag(string.Buffer, 'GSM+');
		return;
	}
	stringSize = stringSize * sizeof(WCHAR);

	msgSize += (ULONG)stringSize;

	pMsg = ExAllocatePoolWithTag(PagedPool, msgSize, 'GSM+');

	pMsg->DataLength = (USHORT)stringSize;
	memcpy(pMsg->Data, string.Buffer, stringSize);

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
ImgFltInitialize() {
	return PsSetLoadImageNotifyRoutine(ImgFltNotifyRoutine);
}

NTSTATUS
ImgFltUninitialize() {
	return PsRemoveLoadImageNotifyRoutine(ImgFltNotifyRoutine);
}