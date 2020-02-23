#include "csamplecontrollerdriver.h"
#include "basics.h"

#include <math.h>

using namespace vr;

static double cyaw = 0, cpitch = 0, croll = 0;
static double cpX = 0, cpY = 0, cpZ = 0;
static double ct0, ct1, ct2, ct3, ct4, ct5;

static double c2pX = 0, c2pY = 0, c2pZ = 0;

CSampleControllerDriver::CSampleControllerDriver()
{
    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
    m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;
}

void CSampleControllerDriver::SetControllerIndex(int32_t CtrlIndex)
{
    ControllerIndex = CtrlIndex;
}

CSampleControllerDriver::~CSampleControllerDriver()
{
}

vr::EVRInitError CSampleControllerDriver::Activate(vr::TrackedDeviceIndex_t unObjectId)
{
    m_unObjectId = unObjectId;
    m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "vive_controller");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_LegacyInputProfile_String, "vive_controller");

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "ViveMV");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "HTC");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_TrackingSystemName_String, "VR Controller");
    vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_DeviceClass_Int32, TrackedDeviceClass_Controller);

    switch (ControllerIndex) {
    case 1:
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, "CTRL1Serial");
        break;
    case 2:
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, "CTRL2Serial");
        break;
    }

    uint64_t supportedButtons = 0xFFFFFFFFFFFFFFFFULL;
    vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, vr::Prop_SupportedButtons_Uint64, supportedButtons);

    // return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
    //vr::VRProperties()->SetUint64Property( m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2 );

    // avoid "not fullscreen" warnings from vrmonitor
    //vr::VRProperties()->SetBoolProperty( m_ulPropertyContainer, Prop_IsOnDesktop_Bool, false );

    // our sample device isn't actually tracked, so set this property to avoid having the icon blink in the status window
    //vr::VRProperties()->SetBoolProperty( m_ulPropertyContainer, Prop_NeverTracked_Bool, false );

    // even though we won't ever track we want to pretend to be the right hand so binding will work as expected

    switch (ControllerIndex) {
    case 1:
        vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_LeftHand);
        break;
    case 2:
        vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_RightHand);
        break;
    }

    // this file tells the UI what to show the user for binding this controller as well as what default bindings should
    // be for legacy or other apps
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_InputProfilePath_String, "{null}/input/mycontroller_profile.json");

    //  Buttons handles
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/application_menu/click", &HButtons[0]);
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/grip/click", &HButtons[1]);
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/system/click", &HButtons[2]);
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trackpad/click", &HButtons[3]);

    // Analog handles
    vr::VRDriverInput()->CreateScalarComponent(
                m_ulPropertyContainer, "/input/trackpad/x", &HAnalog[0],
            vr::EVRScalarType::VRScalarType_Absolute, vr::EVRScalarUnits::VRScalarUnits_NormalizedTwoSided
            );
    vr::VRDriverInput()->CreateScalarComponent(
                m_ulPropertyContainer, "/input/trackpad/y", &HAnalog[1],
            vr::EVRScalarType::VRScalarType_Absolute, vr::EVRScalarUnits::VRScalarUnits_NormalizedTwoSided
            );
    vr::VRDriverInput()->CreateScalarComponent(
                m_ulPropertyContainer, "/input/trigger/value", &HAnalog[2],
            vr::EVRScalarType::VRScalarType_Absolute, vr::EVRScalarUnits::VRScalarUnits_NormalizedOneSided
            );

    vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);

    // create our haptic component
    vr::VRDriverInput()->CreateHapticComponent(m_ulPropertyContainer, "/output/haptic", &m_compHaptic);

    return VRInitError_None;
}

void CSampleControllerDriver::Deactivate()
{
    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
}

void CSampleControllerDriver::EnterStandby()
{
}

void *CSampleControllerDriver::GetComponent(const char *pchComponentNameAndVersion)
{
    // override this to add a component to a driver
    return NULL;
}

void CSampleControllerDriver::PowerOff()
{
}

void CSampleControllerDriver::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize)
{
    if (unResponseBufferSize >= 1) {
        pchResponseBuffer[0] = 0;
    }
}

DriverPose_t CSampleControllerDriver::GetPose()
{
    DriverPose_t pose = { 0 };
    //pose.poseIsValid = false;
    pose.poseIsValid = true;
    //pose.result = TrackingResult_Calibrating_OutOfRange;
    pose.result = TrackingResult_Running_OK;
    pose.deviceIsConnected = true;

    pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
    pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

    if (ControllerIndex == 1) {
        if ((GetAsyncKeyState(70) & 0x8000) != 0) {
            cyaw += 0.1;                                       //F
        }
        if ((GetAsyncKeyState(72) & 0x8000) != 0) {
            cyaw += -0.1;                                       //H
        }
        if ((GetAsyncKeyState(84) & 0x8000) != 0) {
            croll += 0.1;                                       //T
        }
        if ((GetAsyncKeyState(71) & 0x8000) != 0) {
            croll += -0.1;                                       //G
        }
        if ((GetAsyncKeyState(66) & 0x8000) != 0) { //B
            cpitch = 0;
            croll = 0;
        }

        //Change position controller1
        if ((GetAsyncKeyState(87) & 0x8000) != 0) {
            cpZ += -0.01;                                       //W
        }
        if ((GetAsyncKeyState(83) & 0x8000) != 0) {
            cpZ += 0.01;                                       //S
        }
        if ((GetAsyncKeyState(65) & 0x8000) != 0) {
            cpX += -0.01;                                       //A
        }
        if ((GetAsyncKeyState(68) & 0x8000) != 0) {
            cpX += 0.01;                                       //D
        }
        if ((GetAsyncKeyState(81) & 0x8000) != 0) {
            cpY += 0.01;                                       //Q
        }
        if ((GetAsyncKeyState(69) & 0x8000) != 0) {
            cpY += -0.01;                                       //E
        }
        if ((GetAsyncKeyState(82) & 0x8000) != 0) {
            cpX = 0;
            cpY = 0;
            cpZ = 0;
        }                                                                        //R

        pose.vecPosition[0] = cpX;
        pose.vecPosition[1] = cpY;
        pose.vecPosition[2] = cpZ;
    } else {
        //Controller2

        if ((GetAsyncKeyState(73) & 0x8000) != 0) {
            c2pZ += -0.01;                                       //I
        }
        if ((GetAsyncKeyState(75) & 0x8000) != 0) {
            c2pZ += 0.01;                                       //K
        }
        if ((GetAsyncKeyState(74) & 0x8000) != 0) {
            c2pX += -0.01;                                       //J
        }
        if ((GetAsyncKeyState(76) & 0x8000) != 0) {
            c2pX += 0.01;                                       //L
        }
        if ((GetAsyncKeyState(85) & 0x8000) != 0) {
            c2pY += 0.01;                                       //U
        }
        if ((GetAsyncKeyState(79) & 0x8000) != 0) {
            c2pY += -0.01;                                       //O
        }
        if ((GetAsyncKeyState(80) & 0x8000) != 0) {
            c2pX = 0;
            c2pY = 0;
            c2pZ = 0;
        }                                                                           //P

        pose.vecPosition[0] = c2pX;
        pose.vecPosition[1] = c2pY;
        pose.vecPosition[2] = c2pZ;
    }

    //Convert yaw, pitch, roll to quaternion
    ct0 = cos(cyaw * 0.5);
    ct1 = sin(cyaw * 0.5);
    ct2 = cos(croll * 0.5);
    ct3 = sin(croll * 0.5);
    ct4 = cos(cpitch * 0.5);
    ct5 = sin(cpitch * 0.5);

    //Set controller rotation
    pose.qRotation.w = ct0 * ct2 * ct4 + ct1 * ct3 * ct5;
    pose.qRotation.x = ct0 * ct3 * ct4 - ct1 * ct2 * ct5;
    pose.qRotation.y = ct0 * ct2 * ct5 + ct1 * ct3 * ct4;
    pose.qRotation.z = ct1 * ct2 * ct4 - ct0 * ct3 * ct5;

    return pose;
}

void CSampleControllerDriver::RunFrame()
{
#if defined(_WINDOWS)
    // Your driver would read whatever hardware state is associated with its input components and pass that
    // in to UpdateBooleanComponent. This could happen in RunFrame or on a thread of your own that's reading USB
    // state. There's no need to update input state unless it changes, but it doesn't do any harm to do so.

    if (ControllerIndex == 1) {
        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[0], (0x8000 & GetAsyncKeyState('Z')) != 0, 0); //Application Menu
        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[1], (0x8000 & GetAsyncKeyState('C')) != 0, 0); //Grip
        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[2], (0x8000 & GetAsyncKeyState('V')) != 0, 0); //System
        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[3], (0x8000 & GetAsyncKeyState('1')) != 0, 0); //Trackpad

        vr::VRDriverInput()->UpdateScalarComponent(HAnalog[0], 0.0, 0); //Trackpad x
        vr::VRDriverInput()->UpdateScalarComponent(HAnalog[1], 0.0, 0); //Trackpad y

        if ((GetAsyncKeyState('2') & 0x8000) != 0) {
            vr::VRDriverInput()->UpdateScalarComponent(HAnalog[0], 1.0, 0);
        }

        if ((GetAsyncKeyState('3') & 0x8000) != 0) {
            vr::VRDriverInput()->UpdateScalarComponent(HAnalog[1], 1.0, 0);
        }

        if ((GetAsyncKeyState('X') & 0x8000) != 0) { //Trigger
            vr::VRDriverInput()->UpdateScalarComponent(HAnalog[2], 1.0, 0);
        } else {
            vr::VRDriverInput()->UpdateScalarComponent(HAnalog[2], 0.0, 0);
        }
    } else {
        //Controller2
        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[0], (0x8000 & GetAsyncKeyState(190)) != 0, 0); //Application Menu
        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[1], (0x8000 & GetAsyncKeyState(191)) != 0, 0); //Grip
        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[2], (0x8000 & GetAsyncKeyState('N')) != 0, 0); //System
        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[3], (0x8000 & GetAsyncKeyState('2')) != 0, 0); //Trackpad

        vr::VRDriverInput()->UpdateScalarComponent(HAnalog[0], 0.0, 0); //Trackpad x
        vr::VRDriverInput()->UpdateScalarComponent(HAnalog[1], 0.0, 0); //Trackpad y

        if ((GetAsyncKeyState('4') & 0x8000) != 0) { //Trigger
            vr::VRDriverInput()->UpdateScalarComponent(HAnalog[2], 1.0, 0);
        } else {
            vr::VRDriverInput()->UpdateScalarComponent(HAnalog[2], 0.0, 0);
        }
    }

    if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) {
        vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(DriverPose_t));
    }

#endif
}

void CSampleControllerDriver::ProcessEvent(const vr::VREvent_t &vrEvent)
{
    switch (vrEvent.eventType) {
    case vr::VREvent_Input_HapticVibration:
        if (vrEvent.data.hapticVibration.componentHandle == m_compHaptic) {
            // This is where you would send a signal to your hardware to trigger actual haptic feedback
            //DriverLog( "BUZZ!\n" );
        }
        break;
    }
}

std::string CSampleControllerDriver::GetSerialNumber() const
{
    switch (ControllerIndex) {
    case 1:
        return "CTRL1Serial";
        break;
    case 2:
        return "CTRL2Serial";
        break;
    }
}


