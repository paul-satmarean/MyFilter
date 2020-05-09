#ifndef __THREAD_FILTER_H__
#define __THREAD_FILTER_H__

#include "MyDriver.h"

NTSTATUS
ThreadFltInitialize();

NTSTATUS
ThreadFltUninitialize();

#endif