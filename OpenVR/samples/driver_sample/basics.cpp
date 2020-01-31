#include "basics.h"

// keys for use with the settings API
const char *const k_pch_Sample_Section = "driver_null";
const char *const k_pch_Sample_SerialNumber_String = "serialNumber";
const char *const k_pch_Sample_ModelNumber_String = "modelNumber";
const char *const k_pch_Sample_WindowX_Int32 = "windowX";
const char *const k_pch_Sample_WindowY_Int32 = "windowY";
const char *const k_pch_Sample_WindowWidth_Int32 = "windowWidth";
const char *const k_pch_Sample_WindowHeight_Int32 = "windowHeight";
const char *const k_pch_Sample_RenderWidth_Int32 = "renderWidth";
const char *const k_pch_Sample_RenderHeight_Int32 = "renderHeight";
const char *const k_pch_Sample_SecondsFromVsyncToPhotons_Float = "secondsFromVsyncToPhotons";
const char *const k_pch_Sample_DisplayFrequency_Float = "displayFrequency";

bool g_bExiting = false;

#if !defined(_WINDOWS)

int GetAsyncKeyState(int key)
{
    // TODO implement
    return 0;
}

#endif
