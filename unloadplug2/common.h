// Copyright (c) 2009 Alex Budovski.

#ifndef INC_COMMON_H
#define INC_COMMON_H

// Only print in debug builds.
#ifdef _DEBUG
#define DPrintf DebugPrint
#else
#define DPrintf
#endif

// Class IDs.
#define UNLOAD_PLUG2_CLASS_ID Class_ID(0x65a5e7f, 0x666629e0)

#ifdef _WIN64
#define LIB_DESCRIPTION _T("UnloadPlugin (x64)")
#else
#define LIB_DESCRIPTION _T("UnloadPlugin (x86)")
#endif

#endif