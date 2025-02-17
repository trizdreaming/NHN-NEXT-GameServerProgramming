﻿#include "stdafx.h"
#include "ThreadLocal.h"
#include "Log.h"

__declspec(thread) int LThreadType = -1;
__declspec(thread) int LIoThreadId = -1;

__declspec( thread ) ThreadCallHistory* LThreadCallHistory = nullptr;
__declspec( thread ) ThreadCallElapsedRecord* LThreadCallElapsedRecord = nullptr;
__declspec( thread ) void* LRecentThisPointer = nullptr;

ThreadCallHistory* GThreadCallHistory[MAX_IO_THREAD] = { 0, };
ThreadCallElapsedRecord* GThreadCallElapsedRecord[MAX_IO_THREAD] = { 0, };
