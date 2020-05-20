/*
Copyright (c) 2020 Lior Lahav

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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


//button support for multiple keyboards

template <typename T>
using DeviceGroup = std::map < uint8_t, T>;

using KeyboardButtonstate = LInput::ButtonsState<uint16_t, 58000>;
using KeyboardGroup = DeviceGroup<KeyboardButtonstate>;
using MouseButtonstate = LInput::ButtonsState<uint8_t, 8>;
using MouseGroup = DeviceGroup<MouseButtonstate>;
using HIDButtonState = LInput::ButtonsState<uint8_t, 32>;
using HIDGroup = DeviceGroup<HIDButtonState>;



KeyboardGroup keyboardState;
MouseGroup mouseState;
HIDGroup hidState;

void OnKeyBoardEvent(const LInput::ButtonStdExtension<uint16_t>::ButtonEvent& btnEvent)
{
	using namespace  LInput;
	static int c = 0;
	c++;

	std::string buttonName = KeyCodeHelper::KeyCodeToString(static_cast<KeyCode>(btnEvent.button));
	std::string nameofEvent = btnEvent.eventType == ButtonStdExtension<uint16_t>::EventType::Pressed ? " Pressed" : " Released";
	std::string msg = std::to_string(c) + " [Device ID:" + std::to_string(btnEvent.parent->GetID()) + "] " + buttonName + " " + nameofEvent + " press count: " + std::to_string(btnEvent.counter) + 
		" repeat count: " + std::to_string(btnEvent.repeatCount) + '\n';

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
	std::string msg = std::to_string(c) + " [Device ID:" + std::to_string(btnEvent.parent->GetID()) + "] " + buttonName + " " + nameofEvent + " " + std::to_string(btnEvent.counter) + "\n";

	

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

	std::string msg = std::to_string(c) + " [Device ID:" + std::to_string(btnEvent.parent->GetID()) + "] " + buttonName + " " + nameofEvent + " " + std::to_string(btnEvent.counter) + "\n";

	std::cout << msg.c_str();
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
		
		// Add button states for multiple keyboards. 
		auto it = keyboardState.find(evnt.deviceIndex);
		//if keyboard ID not found add new buttonstates entry.
		if (it == std::end(keyboardState))
		{
			it = keyboardState.emplace(evnt.deviceIndex, decltype(keyboardState)::mapped_type()).first;
			
			std::shared_ptr<ButtonStdExtension<uint16_t>> stdExtension = std::make_shared<ButtonStdExtension<uint16_t>>(evnt.deviceIndex,250,10);
			it->second.AddExtension(std::static_pointer_cast<IButtonable<uint16_t>>(stdExtension));
			stdExtension->OnButtonEvent.Add(std::bind(&OnKeyBoardEvent, std::placeholders::_1));
		}
		

		it->second.SetButtonState(static_cast<decltype(keyboardState)::mapped_type::ButtonType>(keyEvent.scanCode), keyEvent.state);

		

	}
	else if (evnt.deviceType == RawInput::RawInputDeviceType::Mouse)
	{
		const auto& mouseEvent = static_cast<const RawInput::RawInputEventMouse&>(evnt);

		// Add button states for multiple mouses. 
		auto it = mouseState.find(evnt.deviceIndex);
		//if mouse ID not found add new buttonstates entry.
		if (it == std::end(mouseState))
		{
			it = mouseState.emplace(evnt.deviceIndex, decltype(mouseState)::mapped_type()).first;

			auto stdExtension = std::make_shared<ButtonStdExtension<uint8_t>>(evnt.deviceIndex,250,10);
			it->second.AddExtension(std::static_pointer_cast<IButtonable<uint8_t>>(stdExtension));
			stdExtension->OnButtonEvent.Add(std::bind(&OnMouseEvent, std::placeholders::_1));
		}

		for (size_t i = 0; i < RawInput::MaxMouseButtons; i++)
			
			it->second.SetButtonState(static_cast<decltype(mouseState)::mapped_type::ButtonType>(i), mouseEvent.buttonState[i]);

		if (mouseEvent.wheelDelta != 0)
		{
			std::cout << std::endl << " Wheel delta " << mouseEvent.wheelDelta;
		}


	}

	else if (evnt.deviceType == RawInput::RawInputDeviceType::GamePad)
	{
		const auto& hidEvent = static_cast<const RawInput::RawInputEventHID&>(evnt);

		// Add button states for multiple mouses. 
		auto it = hidState.find(evnt.deviceIndex);
		//if mouse ID not found add new buttonstates entry.
		if (it == std::end(hidState))
		{
			it = hidState.emplace(evnt.deviceIndex, decltype(hidState)::mapped_type()).first;

			auto stdExtension = std::make_shared<ButtonStdExtension<uint8_t>>(evnt.deviceIndex,250 ,10);
			it->second.AddExtension(std::static_pointer_cast<IButtonable<uint8_t>>(stdExtension));
			stdExtension->OnButtonEvent.Add(std::bind(&OnHIDEvent, std::placeholders::_1));
		}



		for (size_t i = 0; i < RawInput::MaxHIDButtons; i++)
			it->second.SetButtonState(static_cast<decltype(hidState)::mapped_type::ButtonType>(i), hidEvent.buttonState[i]);

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
	
	LInput::Win32::Window window;
	
	RawInput rawInput(window.GetHandle());

	// Add devices
	rawInput.AddDevice(RawInput::UsagePage::GenericDesktopControls, RawInput::GenericDesktopControlsUsagePage::Mouse, RawInput::Flags::EnableBackground);
	rawInput.AddDevice(RawInput::UsagePage::GenericDesktopControls, RawInput::GenericDesktopControlsUsagePage::Keyboard, RawInput::Flags::EnableBackground);
	rawInput.AddDevice(RawInput::UsagePage::GenericDesktopControls, RawInput::GenericDesktopControlsUsagePage::Joystick, RawInput::Flags::EnableBackground);

	
	//Add input callback 

	rawInput.OnInput.Add(std::bind(&OnRawInput, std::placeholders::_1));
	rawInput.Enable(true);

	std::cout <<  "Press 'Q' three times to quit." << std::endl;


	//Handle application events.
	MessageLoop();

}
