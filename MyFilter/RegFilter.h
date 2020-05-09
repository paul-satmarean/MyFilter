#ifndef __REG_FILTER_H__
#define __REG_FILTER_H__

#include "MyDriver.h"

LARGE_INTEGER gRegCookie;

NTSTATUS
RegFltInitialize();

NTSTATUS
RegFltUninitialize();

#endif