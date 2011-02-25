// Copyright (c) 2009 Alex Budovski.

#include <max.h>

#include "common.h"

#define DLLEXPORT extern "C" __declspec(dllexport)

ClassDesc *GetUnloadPlug2Desc();
HINSTANCE hInstance;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved) {
  switch(fdwReason) {
    case DLL_PROCESS_ATTACH:
      hInstance = hinstDLL;
      DisableThreadLibraryCalls(hinstDLL);
      break;
  }
  return TRUE;
}

DLLEXPORT int LibInitialize() {
  return TRUE;
}

DLLEXPORT int LibShutdown() {
  return TRUE;
}

DLLEXPORT const TCHAR *LibDescription() {
  return LIB_DESCRIPTION;
}

DLLEXPORT int LibNumberClasses() {
  return 1;
}

DLLEXPORT ClassDesc *LibClassDesc(int /*i*/) {
  return GetUnloadPlug2Desc();
}

DLLEXPORT ULONG LibVersion() {
  return VERSION_3DSMAX;
}
