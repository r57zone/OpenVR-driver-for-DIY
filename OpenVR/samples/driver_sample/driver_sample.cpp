//============ Copyright (c) Valve Corporation, All rights reserved. ============
//=============== Changed by r57zone (https://github.com/r57zone) ===============

//#include "driverlog.h"

#include "cserverdriver_sample.h"
#include "cwatchdogdriver_sample.h"

#include <string.h>

using namespace vr;

#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec(dllexport)
#define HMD_DLL_IMPORT extern "C" __declspec(dllimport)
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define HMD_DLL_IMPORT extern "C"
#else
#error "Unsupported Platform."
#endif

static CWatchdogDriver_Sample g_watchdogDriverNull;

static CServerDriver_Sample g_serverDriverNull;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
HMD_DLL_EXPORT void *HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
{
    if (0 == strcmp(IServerTrackedDeviceProvider_Version, pInterfaceName)) {
        return &g_serverDriverNull;
    }
    if (0 == strcmp(IVRWatchdogProvider_Version, pInterfaceName)) {
        return &g_watchdogDriverNull;
    }

    if (pReturnCode) {
        *pReturnCode = VRInitError_Init_InterfaceNotFound;
    }

    return NULL;
}

