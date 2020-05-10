#include "RegFilter.h"
#include "Communication.h"
#include "CommShared.h"
#include "Trace.h"
#include <ntstrsafe.h>
#include "RegFilter.tmh"

#define MSG_BUFFER_SIZE 1024

void RegHandleOperation(
	_In_opt_ PVOID Argument2,
	_In_ MY_DRIVER_REG_OP_CODE OperationCode
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	PUNICODE_STRING name;
	PREG_SET_VALUE_KEY_INFORMATION preInfo;
	PREG_POST_OPERATION_INFORMATION postInfo;
	LARGE_INTEGER timestamp;
	PMY_DRIVER_REG_MESSAGE pMsg = NULL;
	ULONG msgSize = sizeof(MY_DRIVER_REG_MESSAGE);
	MY_DRIVER_PROCESS_CREATE_MESSAGE_REPLY reply = { 0 };
	ULONG replySize = sizeof(reply);

	KeQuerySystemTimePrecise(&timestamp);

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

	msgSize += name->Length;

	pMsg = ExAllocatePoolWithTag(PagedPool, msgSize, 'GSM+');
	if (!pMsg) {
		return;
	}

	pMsg->ProcessId = HandleToUlong(PsGetCurrentProcessId());
	pMsg->ThreadId = HandleToUlong(PsGetCurrentThreadId());

	pMsg->NameLength = name->Length;
	RtlCopyMemory(pMsg->Name, name->Buffer, name->Length);

	pMsg->Timestamp = timestamp;
	pMsg->Operation = regCreateKey;
	pMsg->Header.MessageCode = OperationCode;

	status = CommSendMessage(
		pMsg,
		msgSize,
		&reply,
		&replySize
		);
	if (!NT_SUCCESS(status))
	{
		LogError("CommSendMessage failed with status = 0x%X", status);
	}

	ExFreePoolWithTag(pMsg, 'GSM+');
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
			RegHandleOperation(Argument2, regCreateKey);
			break;

		case RegNtPostSetValueKey:
			RegHandleOperation(Argument2, regSetValueKey);
			break;
			
		case RegNtPostDeleteKey:
			RegHandleOperation(Argument2, RegNtPostDeleteKey);
			break;

		case RegNtPostDeleteValueKey:
			RegHandleOperation(Argument2, regDeleteValueKey);
			break;

		case RegNtPostLoadKey:
			RegHandleOperation(Argument2, regLoadKey);
			break;

		case RegNtPostRenameKey:
			RegHandleOperation(Argument2, regRenameKey);
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

