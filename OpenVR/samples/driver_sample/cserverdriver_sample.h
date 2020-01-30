#ifndef CSERVERDRIVER_SAMPLE_H
#define CSERVERDRIVER_SAMPLE_H

#include <openvr_driver.h>
#include "csampledevicedriver.h"
#include "csamplecontrollerdriver.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CServerDriver_Sample : public vr::IServerTrackedDeviceProvider
{
public:
    virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);
    virtual void Cleanup();
    virtual const char *const *GetInterfaceVersions() { return vr::k_InterfaceVersions; }
    virtual void RunFrame();
    virtual bool ShouldBlockStandbyMode()  { return false; }
    virtual void EnterStandby()  {}
    virtual void LeaveStandby()  {}

private:
    CSampleDeviceDriver *m_pNullHmdLatest = nullptr;
    CSampleControllerDriver *m_pController = nullptr;
    CSampleControllerDriver *m_pController2 = nullptr;
};

#endif // CSERVERDRIVER_SAMPLE_H
