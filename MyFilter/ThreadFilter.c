#include "ThreadFilter.h"
#include "Communication.h"
#include "CommShared.h"
#include "Trace.h"
#include "ThreadFilter.tmh"


void
ThreadFltSendMessageThreadCreate(
	_In_ HANDLE ProcessId,
	_In_ HANDLE ThreadId
	)
{
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(ThreadId);

	LARGE_INTEGER timestamp;
	KeQuerySystemTimePrecise(&timestamp);
}

void
ThreadFltSendMessageThreadClose(
	_In_ HANDLE ProcessId,
	_In_ HANDLE ThreadId
	)
{
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(ThreadId);

	LARGE_INTEGER timestamp;
	KeQuerySystemTimePrecise(&timestamp);
}


static VOID
ThreadFltNotifyRoutine(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
	)
{
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(ThreadId);
	UNREFERENCED_PARAMETER(Create);

	if ((gDrv.MonitoringStarted & notificationThread) == 0) {
		return;
	}

	if (Create)
	{

 	}
	else
	{

	}

	

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