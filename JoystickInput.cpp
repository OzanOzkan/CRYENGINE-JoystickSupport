/////////////////////////////////////////////////////////////////////
// CRYENGINE Joystick Device Implementation
// Ozan Ozkan - 2017
/////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "JoystickInput.h"

LINK_SYSTEM_LIBRARY("dinput8.lib")
LINK_SYSTEM_LIBRARY("dxguid.lib")

LPDIRECTINPUT8 m_pDI;
LPDIRECTINPUTDEVICE8 m_currentJoystick;
DIDEVCAPS m_devCapabilities;
const DIDEVICEINSTANCE* m_instance;

// Initialize each axis.
BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context)
{
	HWND hDlg = (HWND)context;

	DIPROPRANGE propRange;
	propRange.diph.dwSize = sizeof(DIPROPRANGE);
	propRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	propRange.diph.dwHow = DIPH_BYID;
	propRange.diph.dwObj = instance->dwType;
	propRange.lMin = -1000;
	propRange.lMax = +1000;

	if (FAILED(m_currentJoystick->SetProperty(DIPROP_RANGE, &propRange.diph)))
	{
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
	m_instance = instance;

	HRESULT hr;

	// Create the device
	LPDIRECTINPUTDEVICE8 joystick;
	hr = m_pDI->CreateDevice(instance->guidInstance, &joystick, NULL);
	m_currentJoystick = joystick;

	if (FAILED(hr))
	{
		return DIENUM_CONTINUE;
	}

	if (FAILED(hr = joystick->SetDataFormat(&c_dfDIJoystick2)))
	{
		return DIENUM_CONTINUE;
	}

	// Joystick capabilities
	m_devCapabilities.dwSize = sizeof(DIDEVCAPS);

	if (FAILED(hr = joystick->GetCapabilities(&m_devCapabilities)))
	{
		return DIENUM_CONTINUE;
	}

	if (FAILED(hr = joystick->EnumObjects(EnumAxesCallback, NULL, DIDFT_AXIS)))
	{
		return DIENUM_CONTINUE;
	}

	if (FAILED(hr = joystick->SetCooperativeLevel(NULL, DISCL_EXCLUSIVE | DISCL_FOREGROUND)))
	{
		return DIENUM_CONTINUE;
	}

	return DIENUM_STOP;
}

HRESULT PollJoystick(DIJOYSTATE2* js, LPDIRECTINPUTDEVICE8 joystick)
{
	HRESULT hr;

	if (joystick == NULL)
	{
		return S_OK;
	}

	hr = joystick->Poll();

	if (FAILED(hr))
	{

		hr = joystick->Acquire();

		while (hr == DIERR_INPUTLOST)
		{
			hr = joystick->Acquire();
		}

		if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED))
		{
			return E_FAIL;
		}

		if (hr == DIERR_OTHERAPPHASPRIO)
		{
			return S_OK;
		}
	}

	if (FAILED(hr = joystick->GetDeviceState(sizeof(DIJOYSTATE2), js)))
	{
		return hr;
	}

	return S_OK;
}

const char * CJoystick::GetDeviceName() const
{
	return m_DeviceName;
}

EInputDeviceType CJoystick::GetDeviceType() const
{
	return EInputDeviceType::eIDT_Joystick;
}

TInputDeviceId CJoystick::GetDeviceId() const
{
	return 7000;
}

int CJoystick::GetDeviceIndex() const
{
	return 0;
}

bool CJoystick::Init()
{
	CryLogAlways("Initializing Joystick");

	HRESULT hr = DirectInput8Create(CryGetCurrentModule(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, 0);
	if (FAILED(hr))
	{
		CryLogAlways("Cannot initialize Joystick");
		return false;
	}

	m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY);

	m_DeviceName = m_instance->tszProductName;
	m_buttons = new int[m_devCapabilities.dwButtons];

	m_bEnabled = true;

	SetupButtons();

	return true;
}


void CJoystick::SetupButtons()
{
	for (uint32 i = JOY_BUTTON0; i <= JOY_BUTTON31; ++i)
	{
		std::string buttonStr = "joy_button_" + std::to_string(i);

		CryLogAlways("CJoystick::SetupButtons: ButtonStr: %s, buttonName: %s", buttonStr.c_str(), buttonName.c_str());

		m_Symbols[i] = new SInputSymbol(eIDT_Joystick, (EKeyId)(JOY_OFFSET + i), string(pButtonStr).c_str(), SInputSymbol::Button);
		m_Symbols[i]->state = eIS_Released;
	}
				
	m_Symbols[JOY_AXIS_X] = new SInputSymbol(eIDT_Joystick, (EKeyId)(JOY_OFFSET + JOY_AXIS_X), "joy_axis_x", SInputSymbol::Axis);
	m_Symbols[JOY_AXIS_Y] = new SInputSymbol(eIDT_Joystick, (EKeyId)(JOY_OFFSET + JOY_AXIS_Y), "joy_axis_y", SInputSymbol::Axis);
	m_Symbols[JOY_AXIS_Z] = new SInputSymbol(eIDT_Joystick, (EKeyId)(JOY_OFFSET + JOY_AXIS_Z), "joy_axis_z", SInputSymbol::Axis);

	m_Symbols[JOY_SLIDER] = new SInputSymbol(eIDT_Joystick, (EKeyId)(JOY_OFFSET + JOY_SLIDER), "joy_slider", SInputSymbol::Toggle);
}

// Post input events.
void CJoystick::PostEvent(SInputSymbol * pSymbol)
{
	SInputEvent event;
	pSymbol->deviceType = GetDeviceType();
	pSymbol->AssignTo(event);
	gEnv->pInput->PostInputEvent(event);
}

void CJoystick::PostInit()
{
}

void CJoystick::Update(bool bFocus)
{
	DIJOYSTATE2 currentJoystickState;
	PollJoystick(&currentJoystickState, m_currentJoystick);
	
	// AXIS X Y Z
	SInputSymbol* pAxisSymbol;
	static float xPos, yPos, zPos;
	if (xPos != static_cast<float>(currentJoystickState.lX))
	{
		xPos = static_cast<float>(currentJoystickState.lX);
		pAxisSymbol = m_Symbols[JOY_AXIS_X];
		pAxisSymbol->state = eIS_Changed;
		pAxisSymbol->value = xPos;
		PostEvent(pAxisSymbol);
	}
	if (yPos != static_cast<float>(currentJoystickState.lY))
	{
		yPos = static_cast<float>(currentJoystickState.lY);
		pAxisSymbol = m_Symbols[JOY_AXIS_Y];
		pAxisSymbol->state = eIS_Changed;
		pAxisSymbol->value = yPos;
		PostEvent(pAxisSymbol);
	}
	if (zPos != static_cast<float>(currentJoystickState.lRz))
	{
		zPos = static_cast<float>(currentJoystickState.lRz);
		pAxisSymbol = m_Symbols[JOY_AXIS_Z];
		pAxisSymbol->state = eIS_Changed;
		pAxisSymbol->value = zPos;
		PostEvent(pAxisSymbol);
	}
	// ~AXIS X Y Z

	// SLIDER
	SInputSymbol* pSliderSymbol;
	static float sliderValue;
	if (sliderValue != static_cast<float>(currentJoystickState.rglSlider[0]))
	{
		sliderValue = static_cast<float>(currentJoystickState.rglSlider[0]);
		pSliderSymbol = m_Symbols[JOY_SLIDER];
		pSliderSymbol->state = eIS_Changed;
		pSliderSymbol->value = sliderValue;
		PostEvent(pSliderSymbol);
	}
	// ~SLIDER

	// BUTTONS
	for (int i = 0; i < m_devCapabilities.dwButtons; ++i)
	{
		SInputSymbol* pSymbol = m_Symbols[i];

		if (static_cast<int>(currentJoystickState.rgbButtons[i]) > 0 && pSymbol->state == eIS_Released)
		{
			pSymbol->value = 1.f;
			pSymbol->state = eIS_Pressed;
			PostEvent(pSymbol);

		}
		else if (static_cast<int>(currentJoystickState.rgbButtons[i]) <= 0 && pSymbol->state == eIS_Down)
		{
			pSymbol->value = 0.f;
			pSymbol->state = eIS_Released;
			PostEvent(pSymbol);
		}
	}
	// ~BUTTONS
}

void CJoystick::Enable(bool enable)
{
	m_bEnabled = true;
}

bool CJoystick::IsEnabled() const
{
	return m_bEnabled;
}