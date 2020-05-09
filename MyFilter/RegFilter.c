#include "RegFilter.h"
#include "Communication.h"
#include "CommShared.h"
#include "Trace.h"
#include <ntstrsafe.h>
#include "RegFilter.tmh"

#define MSG_BUFFER_SIZE 511

void RegFltCreateKey(
	_In_opt_ PVOID Argument2
	)
{
	
	NTSTATUS status = STATUS_SUCCESS;
	PUNICODE_STRING name;
	PREG_SET_VALUE_KEY_INFORMATION preInfo;
	PREG_POST_OPERATION_INFORMATION postInfo;
	UNICODE_STRING path;
	LARGE_INTEGER timestamp;
	UNICODE_STRING string;
	size_t stringSize = 0;
	HANDLE ProcessId;
	HANDLE ThreadId;
	PMY_DRIVER_GENERIC_MESSAGE pMsg = NULL;
	ULONG msgSize = sizeof(MY_DRIVER_GENERIC_MESSAGE);
	MY_DRIVER_PROCESS_CREATE_MESSAGE_REPLY reply = { 0 };
	ULONG replySize = sizeof(reply);

	KeQuerySystemTimePrecise(&timestamp);
	ProcessId = PsGetCurrentProcessId();
	ThreadId = PsGetCurrentThreadId();

	postInfo = (PREG_POST_OPERATION_INFORMATION)Argument2;
	if (!NT_SUCCESS(postInfo->Status)) {
		return;
	}

	status = CmCallbackGetKeyObjectIDEx(
		&gRegCookie,
		postInfo->Object,
		NULL,
		&name,
		0
		);

	if (!NT_SUCCESS(status)) {
		LogError("CmCallbackGetKeyObjectIDEx failed with status = 0x%X", status);
		return;
	}

	preInfo = (PREG_SET_VALUE_KEY_INFORMATION)postInfo->PreInformation;
	NT_ASSERT(preInfo);
	


	ProcessId = PsGetCurrentProcessId();
	ThreadId = PsGetCurrentThreadId();


	path.Buffer = ExAllocatePoolWithTag(PagedPool, MSG_BUFFER_SIZE, 'GSM+');
	if (!path.Buffer) {
		return;
	}
	memset(path.Buffer, 0, MSG_BUFFER_SIZE);
	memcpy_s(path.Buffer, (MSG_BUFFER_SIZE - sizeof(WCHAR)), name->Buffer, name->Length);
	path.MaximumLength = MSG_BUFFER_SIZE;
	path.Length = name->Length + sizeof(WCHAR);

	string.Buffer = ExAllocatePoolWithTag(PagedPool, MSG_BUFFER_SIZE, 'GSM+');
	if (!string.Buffer) {
		ExFreePoolWithTag(path.Buffer, 'GSM+');
		return;
	}
	string.MaximumLength = MSG_BUFFER_SIZE;
	memset(string.Buffer, 0, MSG_BUFFER_SIZE);
	//RtlZeroBytes(string.Buffer, string.MaximumLength);


	status = RtlStringCchPrintfW(
		string.Buffer,
		MSG_BUFFER_SIZE,
		L"[Reg][%I64d] Registry Create Key ProcessId = %d, ThreadId = %d, Path = %s",
		timestamp.QuadPart,
		ProcessId,
		ThreadId,
		path.Buffer
		);

	//we dont need the path anymore regardless
	ExFreePoolWithTag(path.Buffer, 'GSM+');

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
	if (!pMsg) {
		ExFreePoolWithTag(string.Buffer, 'GSM+');
		return;
	}
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
RegFltCallback(
	_In_	 PVOID CallbackContext,
	_In_opt_ PVOID Argument1,
	_In_opt_ PVOID Argument2
	)
{
	UNREFERENCED_PARAMETER(CallbackContext);

	if ((gDrv.MonitoringStarted & notificationRegistry) == 0) {
		return STATUS_SUCCESS;
	}
	switch ((REG_NOTIFY_CLASS)((ULONG_PTR)Argument1)) {

		case RegNtPostCreateKey:
			break;

		case RegNtPostSetValueKey:
			RegFltCreateKey(Argument2);
			break;
			
		case RegNtPostDeleteKey:
			break;

		case RegNtPostDeleteValueKey:
			break;

		case RegNtPostLoadKey:
			break;

		case RegNtPostRenameKey:
			break;

		default:
			LogInfo("Unimplemented registry operation type.");
			return STATUS_SUCCESS;
	}

	return STATUS_SUCCESS;
}

NTSTATUS
RegFltInitialize(
	_In_ PDRIVER_OBJECT DriverObject
	)
{
	UNICODE_STRING altitude = RTL_CONSTANT_STRING(L"7657.124");

	//init cookie
	memset(&gRegCookie, 0, sizeof(LARGE_INTEGER));

	return CmRegisterCallbackEx(
		RegFltCallback,
		&altitude,
		DriverObject,
		NULL,
		&gRegCookie,
		NULL
		);
}

NTSTATUS
RegFltUninitialize()
{
	return CmUnRegisterCallback(gRegCookie);
}

