//============ Copyright (c) Valve Corporation, All rights reserved. ============
//=============== Changed by r57zone (https://github.com/r57zone) ===============

#include <openvr_driver.h>
//#include "driverlog.h"

#include <vector>
#include <thread>
#include <chrono>

#if defined( _WINDOWS )
#include <windows.h>
#endif

using namespace vr;


#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
#define HMD_DLL_IMPORT extern "C" __declspec( dllimport )
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define HMD_DLL_IMPORT extern "C" 
#else
#error "Unsupported Platform."
#endif

inline HmdQuaternion_t HmdQuaternion_Init( double w, double x, double y, double z )
{
	HmdQuaternion_t quat;
	quat.w = w;
	quat.x = x;
	quat.y = y;
	quat.z = z;
	return quat;
}

inline void HmdMatrix_SetIdentity( HmdMatrix34_t *pMatrix )
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


// keys for use with the settings API
static const char * const k_pch_Sample_Section = "driver_null";
static const char * const k_pch_Sample_SerialNumber_String = "serialNumber";
static const char * const k_pch_Sample_ModelNumber_String = "modelNumber";
static const char * const k_pch_Sample_WindowX_Int32 = "windowX";
static const char * const k_pch_Sample_WindowY_Int32 = "windowY";
static const char * const k_pch_Sample_WindowWidth_Int32 = "windowWidth";
static const char * const k_pch_Sample_WindowHeight_Int32 = "windowHeight";
static const char * const k_pch_Sample_RenderWidth_Int32 = "renderWidth";
static const char * const k_pch_Sample_RenderHeight_Int32 = "renderHeight";
static const char * const k_pch_Sample_SecondsFromVsyncToPhotons_Float = "secondsFromVsyncToPhotons";
static const char * const k_pch_Sample_DisplayFrequency_Float = "displayFrequency";

//Head tracking vars
double yaw = 0, pitch = 0, roll = 0;
double pX = 0, pY = 0, pZ = 0;
double t0, t1, t2, t3, t4, t5;

double cyaw = 0, cpitch = 0, croll = 0;
double cpX = 0, cpY = 0, cpZ = 0;
double ct0, ct1, ct2, ct3, ct4, ct5;

double c2pX = 0, c2pY = 0, c2pZ = 0;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

class CWatchdogDriver_Sample : public IVRWatchdogProvider
{
public:
	CWatchdogDriver_Sample()
	{
		m_pWatchdogThread = nullptr;
	}

	virtual EVRInitError Init( vr::IVRDriverContext *pDriverContext ) ;
	virtual void Cleanup() ;

private:
	std::thread *m_pWatchdogThread;
};

CWatchdogDriver_Sample g_watchdogDriverNull;


bool g_bExiting = false;

void WatchdogThreadFunction(  )
{
	/*while ( !g_bExiting )
	{
#if defined( _WINDOWS )
		// on windows send the event when the Y key is pressed.
		if ( (0x01 & GetAsyncKeyState( 'Y' )) != 0 )
		{
			// Y key was pressed. 
			vr::VRWatchdogHost()->WatchdogWakeUp();
		}
		std::this_thread::sleep_for( std::chrono::microseconds( 500 ) );
#else
		// for the other platforms, just send one every five seconds
		std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
		vr::VRWatchdogHost()->WatchdogWakeUp();
#endif
	}*/
}

EVRInitError CWatchdogDriver_Sample::Init( vr::IVRDriverContext *pDriverContext )
{
	VR_INIT_WATCHDOG_DRIVER_CONTEXT( pDriverContext );
	//InitDriverLog( vr::VRDriverLog() );

	// Watchdog mode on Windows starts a thread that listens for the 'Y' key on the keyboard to 
	// be pressed. A real driver should wait for a system button event or something else from the 
	// the hardware that signals that the VR system should start up.
	g_bExiting = false;
	m_pWatchdogThread = new std::thread( WatchdogThreadFunction );
	if ( !m_pWatchdogThread )
	{
		//DriverLog( "Unable to create watchdog thread\n");
		return VRInitError_Driver_Failed;
	}

	return VRInitError_None;
}


void CWatchdogDriver_Sample::Cleanup()
{
	g_bExiting = true;
	if ( m_pWatchdogThread )
	{
		m_pWatchdogThread->join();
		delete m_pWatchdogThread;
		m_pWatchdogThread = nullptr;
	}

	//CleanupDriverLog();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CSampleDeviceDriver : public vr::ITrackedDeviceServerDriver, public vr::IVRDisplayComponent
{
public:
	CSampleDeviceDriver(  )
	{
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
		m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;

		//DriverLog( "Using settings values\n" );
		m_flIPD = vr::VRSettings()->GetFloat( k_pch_SteamVR_Section, k_pch_SteamVR_IPD_Float );

		char buf[1024];
		vr::VRSettings()->GetString( k_pch_Sample_Section, k_pch_Sample_SerialNumber_String, buf, sizeof( buf ) );
		m_sSerialNumber = buf;

		vr::VRSettings()->GetString( k_pch_Sample_Section, k_pch_Sample_ModelNumber_String, buf, sizeof( buf ) );
		m_sModelNumber = buf;

		m_nWindowX = vr::VRSettings()->GetInt32( k_pch_Sample_Section, k_pch_Sample_WindowX_Int32 );
		m_nWindowY = vr::VRSettings()->GetInt32( k_pch_Sample_Section, k_pch_Sample_WindowY_Int32 );
		m_nWindowWidth = vr::VRSettings()->GetInt32( k_pch_Sample_Section, k_pch_Sample_WindowWidth_Int32 );
		m_nWindowHeight = vr::VRSettings()->GetInt32( k_pch_Sample_Section, k_pch_Sample_WindowHeight_Int32 );
		m_nRenderWidth = vr::VRSettings()->GetInt32( k_pch_Sample_Section, k_pch_Sample_RenderWidth_Int32 );
		m_nRenderHeight = vr::VRSettings()->GetInt32( k_pch_Sample_Section, k_pch_Sample_RenderHeight_Int32 );
		m_flSecondsFromVsyncToPhotons = vr::VRSettings()->GetFloat( k_pch_Sample_Section, k_pch_Sample_SecondsFromVsyncToPhotons_Float );
		m_flDisplayFrequency = vr::VRSettings()->GetFloat( k_pch_Sample_Section, k_pch_Sample_DisplayFrequency_Float );

		/*DriverLog( "driver_null: Serial Number: %s\n", m_sSerialNumber.c_str() );
		DriverLog( "driver_null: Model Number: %s\n", m_sModelNumber.c_str() );
		DriverLog( "driver_null: Window: %d %d %d %d\n", m_nWindowX, m_nWindowY, m_nWindowWidth, m_nWindowHeight );
		DriverLog( "driver_null: Render Target: %d %d\n", m_nRenderWidth, m_nRenderHeight );
		DriverLog( "driver_null: Seconds from Vsync to Photons: %f\n", m_flSecondsFromVsyncToPhotons );
		DriverLog( "driver_null: Display Frequency: %f\n", m_flDisplayFrequency );
		DriverLog( "driver_null: IPD: %f\n", m_flIPD );*/
	}

	virtual ~CSampleDeviceDriver()
	{
	}


	virtual EVRInitError Activate( vr::TrackedDeviceIndex_t unObjectId ) 
	{
		m_unObjectId = unObjectId;
		m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer( m_unObjectId );


		vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, Prop_ModelNumber_String, m_sModelNumber.c_str() );
		vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str() );
		vr::VRProperties()->SetFloatProperty( m_ulPropertyContainer, Prop_UserIpdMeters_Float, m_flIPD );
		vr::VRProperties()->SetFloatProperty( m_ulPropertyContainer, Prop_UserHeadToEyeDepthMeters_Float, 0.f );
		vr::VRProperties()->SetFloatProperty( m_ulPropertyContainer, Prop_DisplayFrequency_Float, m_flDisplayFrequency );
		vr::VRProperties()->SetFloatProperty( m_ulPropertyContainer, Prop_SecondsFromVsyncToPhotons_Float, m_flSecondsFromVsyncToPhotons );

		// return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
		vr::VRProperties()->SetUint64Property( m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2 );

		// avoid "not fullscreen" warnings from vrmonitor
		vr::VRProperties()->SetBoolProperty( m_ulPropertyContainer, Prop_IsOnDesktop_Bool, false );

		//Debug mode activate Windowed Mode (borderless fullscreen), locked to 30 FPS, for testing
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DisplayDebugMode_Bool, true);

		// Icons can be configured in code or automatically configured by an external file "drivername\resources\driver.vrresources".
		// Icon properties NOT configured in code (post Activate) are then auto-configured by the optional presence of a driver's "drivername\resources\driver.vrresources".
		// In this manner a driver can configure their icons in a flexible data driven fashion by using an external file.
		//
		// The structure of the driver.vrresources file allows a driver to specialize their icons based on their HW.
		// Keys matching the value in "Prop_ModelNumber_String" are considered first, since the driver may have model specific icons.
		// An absence of a matching "Prop_ModelNumber_String" then considers the ETrackedDeviceClass ("HMD", "Controller", "GenericTracker", "TrackingReference")
		// since the driver may have specialized icons based on those device class names.
		//
		// An absence of either then falls back to the "system.vrresources" where generic device class icons are then supplied.
		//
		// Please refer to "bin\drivers\sample\resources\driver.vrresources" which contains this sample configuration.
		//
		// "Alias" is a reserved key and specifies chaining to another json block.
		//
		// In this sample configuration file (overly complex FOR EXAMPLE PURPOSES ONLY)....
		//
		// "Model-v2.0" chains through the alias to "Model-v1.0" which chains through the alias to "Model-v Defaults".
		//
		// Keys NOT found in "Model-v2.0" would then chase through the "Alias" to be resolved in "Model-v1.0" and either resolve their or continue through the alias.
		// Thus "Prop_NamedIconPathDeviceAlertLow_String" in each model's block represent a specialization specific for that "model".
		// Keys in "Model-v Defaults" are an example of mapping to the same states, and here all map to "Prop_NamedIconPathDeviceOff_String".
		//
		bool bSetupIconUsingExternalResourceFile = true;
		if ( !bSetupIconUsingExternalResourceFile )
		{
			// Setup properties directly in code.
			// Path values are of the form {drivername}\icons\some_icon_filename.png
			vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{null}/icons/headset_sample_status_off.png" );
			vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, "{null}/icons/headset_sample_status_searching.gif" );
			vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{null}/icons/headset_sample_status_searching_alert.gif" );
			vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{null}/icons/headset_sample_status_ready.png" );
			vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{null}/icons/headset_sample_status_ready_alert.png" );
			vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, "{null}/icons/headset_sample_status_error.png" );
			vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{null}/icons/headset_sample_status_standby.png" );
			vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, "{null}/icons/headset_sample_status_ready_low.png" );
		}

		return VRInitError_None;
	}

	virtual void Deactivate() 
	{
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
	}

	virtual void EnterStandby()
	{
	}

	void *GetComponent( const char *pchComponentNameAndVersion )
	{
		if ( !_stricmp( pchComponentNameAndVersion, vr::IVRDisplayComponent_Version ) )
		{
			return (vr::IVRDisplayComponent*)this;
		}

		// override this to add a component to a driver
		return NULL;
	}

	virtual void PowerOff() 
	{
	}

	/** debug request from a client */
	virtual void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize ) 
	{
		if( unResponseBufferSize >= 1 )
			pchResponseBuffer[0] = 0;
	}

	virtual void GetWindowBounds( int32_t *pnX, int32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight ) 
	{
		*pnX = m_nWindowX;
		*pnY = m_nWindowY;
		*pnWidth = m_nWindowWidth;
		*pnHeight = m_nWindowHeight;
	}

	virtual bool IsDisplayOnDesktop() 
	{
		return true;
	}

	virtual bool IsDisplayRealDisplay() 
	{
		return false;
	}

	virtual void GetRecommendedRenderTargetSize( uint32_t *pnWidth, uint32_t *pnHeight ) 
	{
		*pnWidth = m_nRenderWidth;
		*pnHeight = m_nRenderHeight;
	}

	virtual void GetEyeOutputViewport( EVREye eEye, uint32_t *pnX, uint32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight ) 
	{
		*pnY = 0;
		*pnWidth = m_nWindowWidth / 2;
		*pnHeight = m_nWindowHeight;
	
		if ( eEye == Eye_Left )
		{
			*pnX = 0;
		}
		else
		{
			*pnX = m_nWindowWidth / 2;
		}
	}

	virtual void GetProjectionRaw( EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom ) 
	{
		*pfLeft = -1.0;
		*pfRight = 1.0;
		*pfTop = -1.0;
		*pfBottom = 1.0;	
	}

	virtual DistortionCoordinates_t ComputeDistortion( EVREye eEye, float fU, float fV ) 
	{
		DistortionCoordinates_t coordinates;
		coordinates.rfBlue[0] = fU;
		coordinates.rfBlue[1] = fV;
		coordinates.rfGreen[0] = fU;
		coordinates.rfGreen[1] = fV;
		coordinates.rfRed[0] = fU;
		coordinates.rfRed[1] = fV;
		return coordinates;
	}

	virtual DriverPose_t GetPose() 
	{
		DriverPose_t pose = { 0 };
		pose.poseIsValid = true;
		pose.result = TrackingResult_Running_OK;
		pose.deviceIsConnected = true;
		
		pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
		pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

		//Simple change yaw, pitch, roll with numpad keys
		if ((GetAsyncKeyState(VK_NUMPAD3) & 0x8000) != 0) yaw += 0.01;
		if ((GetAsyncKeyState(VK_NUMPAD1) & 0x8000) != 0) yaw += -0.01;

		if ((GetAsyncKeyState(VK_NUMPAD4) & 0x8000) != 0) pitch += 0.01;
		if ((GetAsyncKeyState(VK_NUMPAD6) & 0x8000) != 0) pitch += -0.01;

		if ((GetAsyncKeyState(VK_NUMPAD8) & 0x8000) != 0) roll += 0.01;
		if ((GetAsyncKeyState(VK_NUMPAD2) & 0x8000) != 0) roll += -0.01;

		if ((GetAsyncKeyState(VK_NUMPAD9) & 0x8000) != 0)
		{
			yaw = 0;
			pitch = 0;
			roll = 0;
		}

		if ((GetAsyncKeyState(VK_UP) & 0x8000) != 0) pZ += -0.01;
		if ((GetAsyncKeyState(VK_DOWN) & 0x8000) != 0) pZ += 0.01;

		if ((GetAsyncKeyState(VK_LEFT) & 0x8000) != 0) pX += -0.01;
		if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0) pX += 0.01;

		if ((GetAsyncKeyState(VK_PRIOR) & 0x8000) != 0) pY += 0.01;
		if ((GetAsyncKeyState(VK_NEXT) & 0x8000) != 0) pY += -0.01;

		if ((GetAsyncKeyState(VK_END) & 0x8000) != 0) { pX = 0; pY = 0; pZ = 0; }

		pose.vecPosition[0] = pX;
		pose.vecPosition[1] = pY;
		pose.vecPosition[2] = pZ;

		//Convert yaw, pitch, roll to quaternion
		t0 = cos(yaw * 0.5);
		t1 = sin(yaw * 0.5);
		t2 = cos(roll * 0.5);
		t3 = sin(roll * 0.5);
		t4 = cos(pitch * 0.5);
		t5 = sin(pitch * 0.5);

		//Set head tracking rotation
		pose.qRotation.w = t0 * t2 * t4 + t1 * t3 * t5;
		pose.qRotation.x = t0 * t3 * t4 - t1 * t2 * t5;
		pose.qRotation.y = t0 * t2 * t5 + t1 * t3 * t4;
		pose.qRotation.z = t1 * t2 * t4 - t0 * t3 * t5;

		return pose;
	}
	

	void RunFrame()
	{
		// In a real driver, this should happen from some pose tracking thread.
		// The RunFrame interval is unspecified and can be very irregular if some other
		// driver blocks it for some periodic task.
		if ( m_unObjectId != vr::k_unTrackedDeviceIndexInvalid )
		{
			vr::VRServerDriverHost()->TrackedDevicePoseUpdated( m_unObjectId, GetPose(), sizeof( DriverPose_t ) );
		}
	}

	std::string GetSerialNumber() const { return m_sSerialNumber; }

private:
	vr::TrackedDeviceIndex_t m_unObjectId;
	vr::PropertyContainerHandle_t m_ulPropertyContainer;

	std::string m_sSerialNumber;
	std::string m_sModelNumber;

	int32_t m_nWindowX;
	int32_t m_nWindowY;
	int32_t m_nWindowWidth;
	int32_t m_nWindowHeight;
	int32_t m_nRenderWidth;
	int32_t m_nRenderHeight;
	float m_flSecondsFromVsyncToPhotons;
	float m_flDisplayFrequency;
	float m_flIPD;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

class CSampleControllerDriver : public vr::ITrackedDeviceServerDriver
{
	int32_t ControllerIndex;
public:
	CSampleControllerDriver()
	{
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
		m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;
	}

	virtual void SetControllerIndex(int32_t CtrlIndex)
	{
		ControllerIndex = CtrlIndex;
	}

	virtual ~CSampleControllerDriver()
	{
	}

	virtual EVRInitError Activate( vr::TrackedDeviceIndex_t unObjectId )
	{
		m_unObjectId = unObjectId;
		m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer( m_unObjectId );

		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "vive_controller");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_LegacyInputProfile_String, "vive_controller");

		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "ViveMV");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "HTC");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");

		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_TrackingSystemName_String, "VR Controller");
		vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_DeviceClass_Int32, TrackedDeviceClass_Controller);

		switch (ControllerIndex)
		{
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

		switch (ControllerIndex)
		{
		case 1:
			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_LeftHand);
			break;
		case 2:
			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_RightHand);
			break;
		}
			

		// this file tells the UI what to show the user for binding this controller as well as what default bindings should
		// be for legacy or other apps
		vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, Prop_InputProfilePath_String, "{null}/input/mycontroller_profile.json" );

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
		vr::VRDriverInput()->CreateHapticComponent( m_ulPropertyContainer, "/output/haptic", &m_compHaptic);

		return VRInitError_None;
	}

	virtual void Deactivate()
	{
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
	}


	virtual void EnterStandby()
	{
	}

	void *GetComponent( const char *pchComponentNameAndVersion )
	{
		// override this to add a component to a driver
		return NULL;
	}

	virtual void PowerOff()
	{
	}

	/** debug request from a client */
	virtual void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize )
	{
		if ( unResponseBufferSize >= 1 )
			pchResponseBuffer[0] = 0;
	}

	virtual DriverPose_t GetPose()
	{
		DriverPose_t pose = { 0 };
		//pose.poseIsValid = false;
		pose.poseIsValid = true;
		//pose.result = TrackingResult_Calibrating_OutOfRange;
		pose.result = TrackingResult_Running_OK;
		pose.deviceIsConnected = true;

		pose.qWorldFromDriverRotation = HmdQuaternion_Init( 1, 0, 0, 0 );
		pose.qDriverFromHeadRotation = HmdQuaternion_Init( 1, 0, 0, 0 );

		if (ControllerIndex == 1) {

			if ((GetAsyncKeyState(70) & 0x8000) != 0) cyaw += 0.1; //F
			if ((GetAsyncKeyState(72) & 0x8000) != 0) cyaw += -0.1; //H

			if ((GetAsyncKeyState(84) & 0x8000) != 0) croll += 0.1; //T
			if ((GetAsyncKeyState(71) & 0x8000) != 0) croll += -0.1; //G

			if ((GetAsyncKeyState(66) & 0x8000) != 0) //B
			{
				cpitch = 0;
				croll = 0;
			}

			//Change position controller1
			if ((GetAsyncKeyState(87) & 0x8000) != 0) cpZ += -0.01; //W
			if ((GetAsyncKeyState(83) & 0x8000) != 0) cpZ += 0.01; //S

			if ((GetAsyncKeyState(65) & 0x8000) != 0) cpX += -0.01; //A
			if ((GetAsyncKeyState(68) & 0x8000) != 0) cpX += 0.01; //D

			if ((GetAsyncKeyState(81) & 0x8000) != 0) cpY += 0.01; //Q
			if ((GetAsyncKeyState(69) & 0x8000) != 0) cpY += -0.01; //E

			if ((GetAsyncKeyState(82) & 0x8000) != 0) { cpX = 0; cpY = 0; cpZ = 0; } //R

			pose.vecPosition[0] = cpX;
			pose.vecPosition[1] = cpY;
			pose.vecPosition[2] = cpZ;

		} else { 
			//Controller2

			if ((GetAsyncKeyState(73) & 0x8000) != 0) c2pZ += -0.01; //I
			if ((GetAsyncKeyState(75) & 0x8000) != 0) c2pZ += 0.01; //K

			if ((GetAsyncKeyState(74) & 0x8000) != 0) c2pX += -0.01; //J
			if ((GetAsyncKeyState(76) & 0x8000) != 0) c2pX += 0.01; //L

			if ((GetAsyncKeyState(85) & 0x8000) != 0) c2pY += 0.01; //U
			if ((GetAsyncKeyState(79) & 0x8000) != 0) c2pY += -0.01; //O

			if ((GetAsyncKeyState(80) & 0x8000) != 0) { c2pX = 0; c2pY = 0; c2pZ = 0; } //P

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

		//Set head tracking rotation
		pose.qRotation.w = ct0 * ct2 * ct4 + ct1 * ct3 * ct5;
		pose.qRotation.x = ct0 * ct3 * ct4 - ct1 * ct2 * ct5;
		pose.qRotation.y = ct0 * ct2 * ct5 + ct1 * ct3 * ct4;
		pose.qRotation.z = ct1 * ct2 * ct4 - ct0 * ct3 * ct5;

		return pose;
	}

	void RunFrame()
	{
#if defined( _WINDOWS )
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

			if ((GetAsyncKeyState('2') & 0x8000) != 0)
				vr::VRDriverInput()->UpdateScalarComponent(HAnalog[0], 1.0, 0);

			if ((GetAsyncKeyState('3') & 0x8000) != 0)
				vr::VRDriverInput()->UpdateScalarComponent(HAnalog[1], 1.0, 0);

			if ((GetAsyncKeyState('X') & 0x8000) != 0) //Trigger
			{
				vr::VRDriverInput()->UpdateScalarComponent(HAnalog[2], 1.0, 0);
			}
			else {
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

			if ((GetAsyncKeyState('4') & 0x8000) != 0) //Trigger
			{
				vr::VRDriverInput()->UpdateScalarComponent(HAnalog[2], 1.0, 0);
			}
			else {
				vr::VRDriverInput()->UpdateScalarComponent(HAnalog[2], 0.0, 0);
			}
		}

		if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid)
		{
			vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(DriverPose_t));
		}

#endif
	}

	void ProcessEvent( const vr::VREvent_t & vrEvent )
	{
		switch ( vrEvent.eventType )
		{
		case vr::VREvent_Input_HapticVibration:
		{
			if ( vrEvent.data.hapticVibration.componentHandle == m_compHaptic )
			{

				// This is where you would send a signal to your hardware to trigger actual haptic feedback
				//DriverLog( "BUZZ!\n" );
			}
		}
		break;
		}
	}

	std::string GetSerialNumber() const { 
		
		switch (ControllerIndex)
		{
		case 1:
			return "CTRL1Serial";
			break;
		case 2:
			return "CTRL2Serial";
			break;
		}
	}

private:
	vr::TrackedDeviceIndex_t m_unObjectId;
	vr::PropertyContainerHandle_t m_ulPropertyContainer;

	//vr::VRInputComponentHandle_t m_compA;
	//vr::VRInputComponentHandle_t m_compB;
	//vr::VRInputComponentHandle_t m_compC;
	vr::VRInputComponentHandle_t m_compHaptic;

	vr::VRInputComponentHandle_t HButtons[4], HAnalog[3];
	//std::string m_sSerialNumber;
	//std::string m_sModelNumber;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CServerDriver_Sample: public IServerTrackedDeviceProvider
{
public:
	virtual EVRInitError Init( vr::IVRDriverContext *pDriverContext ) ;
	virtual void Cleanup() ;
	virtual const char * const *GetInterfaceVersions() { return vr::k_InterfaceVersions; }
	virtual void RunFrame() ;
	virtual bool ShouldBlockStandbyMode()  { return false; }
	virtual void EnterStandby()  {}
	virtual void LeaveStandby()  {}

private:
	CSampleDeviceDriver *m_pNullHmdLatest = nullptr;
	CSampleControllerDriver *m_pController = nullptr;
	CSampleControllerDriver *m_pController2 = nullptr;
};

CServerDriver_Sample g_serverDriverNull;


EVRInitError CServerDriver_Sample::Init( vr::IVRDriverContext *pDriverContext )
{
	VR_INIT_SERVER_DRIVER_CONTEXT( pDriverContext );
	//InitDriverLog( vr::VRDriverLog() );

	m_pNullHmdLatest = new CSampleDeviceDriver();
	vr::VRServerDriverHost()->TrackedDeviceAdded( m_pNullHmdLatest->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, m_pNullHmdLatest );

	m_pController = new CSampleControllerDriver();
	m_pController->SetControllerIndex(1);
	vr::VRServerDriverHost()->TrackedDeviceAdded(m_pController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_pController );
	

	m_pController2 = new CSampleControllerDriver();
	m_pController2->SetControllerIndex(2);
	vr::VRServerDriverHost()->TrackedDeviceAdded(m_pController2->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_pController2);

	return VRInitError_None;
}

void CServerDriver_Sample::Cleanup() 
{
	//CleanupDriverLog();
	delete m_pNullHmdLatest;
	m_pNullHmdLatest = NULL;
	delete m_pController;
	m_pController = NULL;
	delete m_pController2;
	m_pController2 = NULL;
}


void CServerDriver_Sample::RunFrame()
{
	if ( m_pNullHmdLatest )
	{
		m_pNullHmdLatest->RunFrame();
	}
	if ( m_pController )
	{
		m_pController->RunFrame();
	}
	if (m_pController2)
	{
		m_pController2->RunFrame();
	}

	/*vr::VREvent_t vrEvent;
	while ( vr::VRServerDriverHost()->PollNextEvent( &vrEvent, sizeof( vrEvent ) ) )
	{
		if ( m_pController )
		{
			m_pController->ProcessEvent( vrEvent );
		}
	}*/
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
HMD_DLL_EXPORT void *HmdDriverFactory( const char *pInterfaceName, int *pReturnCode )
{
	if( 0 == strcmp( IServerTrackedDeviceProvider_Version, pInterfaceName ) )
	{
		return &g_serverDriverNull;
	}
	if( 0 == strcmp( IVRWatchdogProvider_Version, pInterfaceName ) )
	{
		return &g_watchdogDriverNull;
	}

	if( pReturnCode )
		*pReturnCode = VRInitError_Init_InterfaceNotFound;

	return NULL;
}

