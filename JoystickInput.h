/////////////////////////////////////////////////////////////////////
// CRYENGINE Joystick Device Implementation
// Ozan Ozkan - 2017
/////////////////////////////////////////////////////////////////////

#pragma once

#include <CryInput/IInput.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class CJoystick : public IInputDevice
{
public:
	enum EJoystickInput : uint32
	{
		JOY_BUTTON0,
		JOY_BUTTON1,
		JOY_BUTTON2,
		JOY_BUTTON3,
		JOY_BUTTON4,
		JOY_BUTTON5,
		JOY_BUTTON6,
		JOY_BUTTON7,
		JOY_BUTTON8,
		JOY_BUTTON9,
		JOY_BUTTON10,
		JOY_BUTTON11,
		JOY_BUTTON12,
		JOY_BUTTON13,
		JOY_BUTTON14,
		JOY_BUTTON15,
		JOY_BUTTON16,
		JOY_BUTTON17,
		JOY_BUTTON18,
		JOY_BUTTON19,
		JOY_BUTTON20,
		JOY_BUTTON21,
		JOY_BUTTON22,
		JOY_BUTTON23,
		JOY_BUTTON24,
		JOY_BUTTON25,
		JOY_BUTTON26,
		JOY_BUTTON27,
		JOY_BUTTON28,
		JOY_BUTTON29,
		JOY_BUTTON30,
		JOY_BUTTON31,
		JOY_AXIS_X,
		JOY_AXIS_Y,
		JOY_AXIS_Z,
		JOY_SLIDER,

		JOY_OFFSET = KI_EYETRACKER_BASE + KI_MOUSE_BASE
	};

	CJoystick() : m_bEnabled(false) {}
	virtual ~CJoystick() {};

	// Inherited via IInputDevice
	virtual const char * GetDeviceName() const override;
	virtual EInputDeviceType GetDeviceType() const override;
	virtual TInputDeviceId GetDeviceId() const override;
	virtual int GetDeviceIndex() const override;
	virtual bool Init() override;
	virtual void PostInit() override;
	virtual void Update(bool bFocus) override;
	virtual bool SetForceFeedback(IFFParams params) override { return false; }
	virtual bool InputState(const TKeyName & key, EInputState state) override { return false; }
	virtual bool SetExclusiveMode(bool value) override { return false; }
	virtual void ClearKeyState() override {}
	virtual void ClearAnalogKeyState(TInputSymbols & clearedSymbols) override {}
	virtual void SetUniqueId(uint8 const uniqueId) override {}
	virtual const char * GetKeyName(const SInputEvent & event) const override { return nullptr; }
	virtual const char * GetKeyName(const EKeyId keyId) const override { return nullptr; }
	virtual uint32 GetInputCharUnicode(const SInputEvent & event) override { return 0; }
	virtual const char * GetOSKeyName(const SInputEvent & event) override { return nullptr; }
	virtual SInputSymbol * LookupSymbol(EKeyId id) const override { return nullptr; }
	virtual const SInputSymbol * GetSymbolByName(const char * name) const override { return nullptr; }
	virtual bool IsOfDeviceType(EInputDeviceType type) const override { return false; }
	virtual void Enable(bool enable) override;
	virtual bool IsEnabled() const override;
	virtual void OnLanguageChange() override {}
	virtual void SetDeadZone(float fThreshold) override {}
	virtual void RestoreDefaultDeadZone() override {}

	void SetupButtons();
	void PostEvent(SInputSymbol* pSymbol);

private:
	const char* m_DeviceName;
	bool m_bEnabled;
	int* m_buttons;
	SInputSymbol* m_Symbols[60] = { 0 };
};