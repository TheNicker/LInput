
#define UNICODE
#define _UNICODE


#include <iostream>
#include <array>
#include <Windows.h>
#include "Window.h"
#include <LInput/Win32/RawInput/RawInput.h>
#include <LInput/Buttons/IButtonable.h>
#include <LInput/Buttons/ButtonStates.h>
#include <LInput/Buttons/ButtonsStdExtension.h>
#include <LInput/Keys/KeyCodeHelper.h>
#include <LInput/Keys/KeyCombination.h>
#include <LInput/Mouse/MouseCode.h>
#include <LInput/Mouse/MouseCodeHelper.h>



LInput::ButtonsState<uint16_t, 58000> keyboardState;
LInput::ButtonsState<uint8_t,8> mouseState;
LInput::ButtonsState<uint8_t,32> hidState;
//KeyDoubleTap doubleTap;

void OnKeyBoardEvent(const LInput::ButtonStdExtension<uint16_t>::ButtonEvent& btnEvent)
{
	using namespace  LInput;
	static int c = 0;
	c++;

	std::string buttonName = KeyCodeHelper::KeyCodeToString(static_cast<KeyCode>(btnEvent.button));

	std::string nameofEvent = btnEvent.eventType == ButtonStdExtension<uint16_t>::EventType::Pressed ? " Pressed" : " Released";

	std::string msg = std::to_string(c) + ": " + buttonName + " " + nameofEvent + " " + std::to_string(btnEvent.counter) + "\n";

	std::cout << msg.c_str();
	//OutputDebugStringA(msg.c_str());
	if (static_cast<KeyCode>(btnEvent.button) == KeyCode::Q && btnEvent.counter >= 2)
		PostQuitMessage(0);
}

void OnMouseEvent(const LInput::ButtonStdExtension<uint8_t>::ButtonEvent& btnEvent)
{
	using namespace  LInput;
	static int c = 0;
	c++;

	std::string buttonName =  MouseCodeHelper::MouseCodeToString(static_cast<MouseButton>(btnEvent.button));

	std::string nameofEvent = btnEvent.eventType == ButtonStdExtension<uint8_t>::EventType::Pressed ? " Pressed" : " Released";

	std::string msg = std::to_string(c) + ": " + buttonName + " " + nameofEvent + " " + std::to_string(btnEvent.counter) + "\n";

	

	std::cout << msg.c_str();
	//OutputDebugStringA(msg.c_str());
}

void OnHIDEvent(const LInput::ButtonStdExtension<uint8_t>::ButtonEvent& btnEvent)
{
	using namespace  LInput;
	static int c = 0;
	c++;
	using namespace std::string_literals;
	
	std::string buttonName = "HID Button "s + std::to_string(static_cast<int>( btnEvent.button));

	std::string nameofEvent = btnEvent.eventType == ButtonStdExtension<uint8_t>::EventType::Pressed ? " Pressed" : " Released";

	std::string msg = std::to_string(c) + ": " + buttonName + " " + nameofEvent + " " + std::to_string(btnEvent.counter) + "\n";

	std::cout << msg.c_str();
}



void TestKeys()
{
	using namespace LInput;
	

}

static void MessageLoop()
{
	MSG msg;
	BOOL bRet;
	while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// handle the error and possibly exit
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}


void OnRawInput(const LInput::RawInput::RawInputEvent& evnt)
{
	using namespace LInput;
	if (evnt.deviceType == RawInput::RawInputDeviceType::Keyboard)
	{
		const auto& keyEvent = static_cast<const RawInput::RawInputEventKeyBoard&>(evnt);
		keyboardState.SetButtonState(static_cast<decltype(keyboardState)::ButtonType>(keyEvent.scanCode), keyEvent.state);
	}
	else if (evnt.deviceType == RawInput::RawInputDeviceType::Mouse)
	{
		const auto& mouseEvent = static_cast<const RawInput::RawInputEventMouse&>(evnt);

		for (size_t i = 0; i < RawInput::MaxMouseButtons; i++)
			mouseState.SetButtonState(static_cast<decltype(mouseState)::ButtonType>(i), mouseEvent.buttonState[i]);

		if (mouseEvent.wheelDelta != 0)
		{
			std::cout << std::endl << " Wheel delta " << mouseEvent.wheelDelta;
		}


	}

	else if (evnt.deviceType == RawInput::RawInputDeviceType::GamePad)
	{
		const auto& hidEvent = static_cast<const RawInput::RawInputEventHID&>(evnt);

		for (size_t i = 0; i < RawInput::MaxHIDButtons; i++)
			hidState.SetButtonState(static_cast<decltype(hidState)::ButtonType>(i), hidEvent.buttonState[i]);

	/*	std::cout << std::endl << "X: " << (int)hidEvent.axes[RawInput::Axes::X] << " Y: " << (int)hidEvent.axes[RawInput::Axes::Y]
			<< " Z: " << (int)hidEvent.axes[RawInput::Axes::Z] << " RZ: " << (int)hidEvent.axes[RawInput::Axes::ZRotate] << " Hat: " << (int)hidEvent.axes[RawInput::Axes::HatSwitch];
		*/	 
	
		//
		//std::cout << std::endl << "X: " << (int)hidEvent.buttonState[0];

	
	}
}


int main()
{

	using namespace LInput;
	
	Win32::Window window;
	RawInput rawInput(window.GetHandle());

	// Add devices
	rawInput.AddDevice(RawInput::UsagePage::GenericDesktopControls, RawInput::GenericDesktopControlsUsagePage::Mouse,    RawInput::Flags::EnableBackground);
	rawInput.AddDevice(RawInput::UsagePage::GenericDesktopControls, RawInput::GenericDesktopControlsUsagePage::Keyboard, RawInput::Flags::EnableBackground);
	rawInput.AddDevice(RawInput::UsagePage::GenericDesktopControls, RawInput::GenericDesktopControlsUsagePage::Joystick, RawInput::Flags::EnableBackground);
	
	//Add input callback 

	rawInput.OnInput.Add(std::bind(&OnRawInput, std::placeholders::_1));

	rawInput.Enable(true);
	//Add button management

	std::shared_ptr<ButtonStdExtension<uint16_t>> stdExtension = std::make_shared<ButtonStdExtension<uint16_t>>();
	keyboardState.AddExtension(std::static_pointer_cast<IButtonable<uint16_t>>(stdExtension));
	stdExtension->OnButtonEvent.Add(std::bind(&OnKeyBoardEvent, std::placeholders::_1));

	std::shared_ptr<ButtonStdExtension<uint8_t>> stdExtensionMouse = std::make_shared<ButtonStdExtension<uint8_t>>();
	mouseState.AddExtension(std::static_pointer_cast<IButtonable<uint8_t>>(stdExtensionMouse));
	stdExtensionMouse->OnButtonEvent.Add(std::bind(&OnMouseEvent, std::placeholders::_1));

	std::shared_ptr<ButtonStdExtension<uint8_t>> stdExtensionHID = std::make_shared<ButtonStdExtension<uint8_t>>();
	hidState.AddExtension(std::static_pointer_cast<IButtonable<uint8_t>>(stdExtensionHID));
	stdExtensionHID->OnButtonEvent.Add(std::bind(&OnHIDEvent, std::placeholders::_1));


	//Handle application events.
	MessageLoop();

}
