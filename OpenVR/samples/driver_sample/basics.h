#ifndef BASICS_H
#define BASICS_H

#include <openvr_driver.h>

#if defined(_WINDOWS)
#include <windows.h>
#else
#include <string.h>
#define _stricmp strcasecmp
int GetAsyncKeyState(int key);

#define VK_NUMPAD3 0x8001
#define VK_NUMPAD1 0x8002
#define VK_NUMPAD4 0x8003
#define VK_NUMPAD6 0x8004
#define VK_NUMPAD8 0x8005
#define VK_NUMPAD2 0x8006
#define VK_NUMPAD9 0x8007
#define VK_UP      0x8008
#define VK_DOWN    0x8009
#define VK_LEFT    0x80010
#define VK_RIGHT   0x80011
#define VK_PRIOR   0x80012
#define VK_NEXT    0x80013
#define VK_END     0x80014
#endif

// keys for use with the settings API
extern const char *const k_pch_Sample_Section;
extern const char *const k_pch_Sample_SerialNumber_String;
extern const char *const k_pch_Sample_ModelNumber_String;
extern const char *const k_pch_Sample_WindowX_Int32;
extern const char *const k_pch_Sample_WindowY_Int32;
extern const char *const k_pch_Sample_WindowWidth_Int32;
extern const char *const k_pch_Sample_WindowHeight_Int32;
extern const char *const k_pch_Sample_RenderWidth_Int32;
extern const char *const k_pch_Sample_RenderHeight_Int32;
extern const char *const k_pch_Sample_SecondsFromVsyncToPhotons_Float;
extern const char *const k_pch_Sample_DisplayFrequency_Float;

extern bool g_bExiting;

inline vr::HmdQuaternion_t HmdQuaternion_Init( double w, double x, double y, double z )
{
    vr::HmdQuaternion_t quat;
    quat.w = w;
    quat.x = x;
    quat.y = y;
    quat.z = z;
    return quat;
}

inline void HmdMatrix_SetIdentity(vr::HmdMatrix34_t *pMatrix)
{
    pMatrix->m[0][0] = 1.f;
    pMatrix->m[0][1] = 0.f;
    pMatrix->m[0][2] = 0.f;
    pMatrix->m[0][3] = 0.f;
    pMatrix->m[1][0] = 0.f;
    pMatrix->m[1][1] = 1.f;
    pMatrix->m[1][2] = 0.f;
    pMatrix->m[1][3] = 0.f;
    pMatrix->m[2][0] = 0.f;
    pMatrix->m[2][1] = 0.f;
    pMatrix->m[2][2] = 1.f;
    pMatrix->m[2][3] = 0.f;
}

#endif // BASICS_H
