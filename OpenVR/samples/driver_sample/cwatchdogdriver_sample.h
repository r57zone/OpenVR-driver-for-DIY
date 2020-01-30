#ifndef CWATCHDOGDRIVER_SAMPLE_H
#define CWATCHDOGDRIVER_SAMPLE_H

#include <openvr_driver.h>

#include <thread>

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

class CWatchdogDriver_Sample : public vr::IVRWatchdogProvider
{
public:
    CWatchdogDriver_Sample()
    {
        m_pWatchdogThread = nullptr;
    }

    virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);
    virtual void Cleanup();

private:
    std::thread *m_pWatchdogThread;
};

void WatchdogThreadFunction();

#endif // CWATCHDOGDRIVER_SAMPLE_H
