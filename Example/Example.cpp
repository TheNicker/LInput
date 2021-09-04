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

#include <iostream>
#include <array>
#include <Windows.h>
#include <LInput/Win32/RawInput/RawInput.h>
#include <LInput/Buttons/ButtonStates.h>
#include <LInput/Buttons/Extensions/ButtonsStdExtension.h>
#include <LInput/Buttons/Extensions/MultiTapExtensions.h>
#include <LInput/Keys/KeyCodeHelper.h>
#include <LInput/Keys/KeyCombination.h>
#include <LInput/Mouse/MouseButton.h>
#include <LInput/Mouse/MouseButtonHelper.h>



//button support for multiple keyboards

template <typename T>
using DeviceGroup = std::map < uint8_t, T>;

using KeyboardButtonType = uint16_t;
constexpr KeyboardButtonType MaxKeyBoardButtons = 58000;
using KeyboardButtonstate = LInput::ButtonsState<KeyboardButtonType, MaxKeyBoardButtons>;
using KeyboardGroup = DeviceGroup<KeyboardButtonstate>;

using MouseButtonstate = LInput::ButtonsState<uint8_t, 8>;
using MouseGroup = DeviceGroup<MouseButtonstate>;
using HIDButtonState = LInput::ButtonsState<uint8_t, 32>;
using HIDGroup = DeviceGroup<HIDButtonState>;



KeyboardGroup keyboardState;
MouseGroup mouseState;
HIDGroup hidState;
uint16_t multiPressRate = 250;
uint16_t repeatRate = 0;


void ParseButtonEvent(const LInput::EventType eventType , int deviceID, std::string buttonName, int pressCounter
	, int repeatCounter, int counter, int actuationTime)
{
	using namespace LInput;
	std::string nameofEvent = eventType == EventType::Pressed ? " Pressed" : " Released";
	std::string msg = std::to_string(counter) + " [Device ID:" + std::to_string(deviceID) + "] " + buttonName + " " + nameofEvent + " press count: " + std::to_string(pressCounter) +
		" repeat count: " + std::to_string(repeatCounter) + " actuation time: " + std::to_string(actuationTime) + "ms" + '\n';
	std::cout << msg;

}

 int c = 0;


void OnKeyBoardEvent(const LInput::ButtonStdExtension<KeyboardButtonType>::ButtonEvent& btnEvent)
{
	using namespace  LInput;


	std::string buttonName = KeyCodeHelper::KeyCodeToString(static_cast<KeyCode>(btnEvent.button));
	ParseButtonEvent(btnEvent.eventType, btnEvent.parent->GetID(), buttonName, btnEvent.counter, btnEvent.repeatCount, c,btnEvent.actuationTime);

	if (static_cast<KeyCode>(btnEvent.button) == KeyCode::Q && btnEvent.counter >= 2)
		PostQuitMessage(0);
}

void OnMouseMultiTap(const LInput::MultitapExtension<uint8_t>::MultiTapEvent& multiTapEvent)
{
	using namespace LInput;
	std::string nameofEvent = "MultiTap";
	std::string buttonName = MouseCodeHelper::MouseCodeToString(static_cast<MouseButton>(multiTapEvent.button));
	std::string msg = std::to_string(c++) + " [Device ID:" + std::to_string(multiTapEvent.parent->GetID()) + "] " + buttonName + " " + nameofEvent + " tap count: " + std::to_string(multiTapEvent.tapCount) +  '\n';
	std::cout << msg;


}



void OnKeyBoardMultiTap(const LInput::MultitapExtension<KeyboardButtonType>::MultiTapEvent& multiTapEvent)
{
	using namespace LInput;
	std::string nameofEvent = "MultiTap";
	std::string buttonName = KeyCodeHelper::KeyCodeToString(static_cast<KeyCode>(multiTapEvent.button));
	std::string msg = std::to_string(c++) + " [Device ID:" + std::to_string(multiTapEvent.parent->GetID()) + "] " + buttonName + " " + nameofEvent + " tap count: " + std::to_string(multiTapEvent.tapCount) + '\n';
	std::cout << msg;


}

void OnMouseEvent(const LInput::ButtonStdExtension<uint8_t>::ButtonEvent& btnEvent)
{
	using namespace  LInput;
	std::string buttonName =  MouseCodeHelper::MouseCodeToString(static_cast<MouseButton>(btnEvent.button));
	ParseButtonEvent(btnEvent.eventType, btnEvent.parent->GetID(), buttonName, btnEvent.counter, btnEvent.repeatCount, c, btnEvent.actuationTime);
}

void OnHIDEvent(const LInput::ButtonStdExtension<uint8_t>::ButtonEvent& btnEvent)
{
	using namespace  LInput;
	static int c = 0;
	c++;
	using namespace std::string_literals;
	
	std::string buttonName = "HID Button "s + std::to_string(static_cast<int>( btnEvent.button));
	ParseButtonEvent(btnEvent.eventType, btnEvent.parent->GetID(), buttonName, btnEvent.counter, btnEvent.repeatCount, c, btnEvent.actuationTime);
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

			auto stdExtension = std::make_shared< ButtonStdExtension<KeyboardButtonType>>(evnt.deviceIndex,multiPressRate,repeatRate);
			it->second.AddExtension(std::static_pointer_cast <KeyboardButtonstate::ExtensionType::element_type>(stdExtension));
			stdExtension->OnButtonEvent.Add(std::bind(&OnKeyBoardEvent, std::placeholders::_1));

			//Add multitap extension for click, double click and triple click

			auto multitapextension = std::make_shared<MultitapExtension<KeyboardButtonType>>(evnt.deviceIndex, 200, 4);
			multitapextension->OnButtonEvent.Add(std::bind(OnKeyBoardMultiTap, std::placeholders::_1));
			it->second.AddExtension(std::static_pointer_cast<IButtonStateExtension<KeyboardButtonType>>(multitapextension));
		}
		

		it->second.SetButtonState(static_cast<decltype(keyboardState)::mapped_type::underlying_button_type>(keyEvent.scanCode), keyEvent.state);

		

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
			//Add standard extension
			auto stdExtension = std::make_shared<ButtonStdExtension<uint8_t>>(evnt.deviceIndex, multiPressRate, repeatRate);
			stdExtension->OnButtonEvent.Add(std::bind(&OnMouseEvent, std::placeholders::_1));
			it->second.AddExtension(std::static_pointer_cast<IButtonStateExtension<uint8_t>>(stdExtension));

			//Add multitap extension for click, double click and triple click

			auto multitapextension = std::make_shared<MultitapExtension<uint8_t>>(evnt.deviceIndex, 200, 4);
			multitapextension->OnButtonEvent.Add(std::bind(OnMouseMultiTap, std::placeholders::_1));
			it->second.AddExtension(std::static_pointer_cast<IButtonStateExtension<uint8_t>>(multitapextension));
	
		}

		for (size_t i = 0; i < RawInput::MaxMouseButtons; i++)
			
			it->second.SetButtonState(static_cast<decltype(mouseState)::mapped_type::underlying_button_type>(i), mouseEvent.buttonState[i]);

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

			auto stdExtension = std::make_shared<ButtonStdExtension<uint8_t>>(evnt.deviceIndex, multiPressRate, repeatRate);
			it->second.AddExtension(std::static_pointer_cast<IButtonStateExtension<uint8_t>>(stdExtension));
			stdExtension->OnButtonEvent.Add(std::bind(&OnHIDEvent, std::placeholders::_1));
		}



		for (size_t i = 0; i < RawInput::MaxHIDButtons; i++)
			it->second.SetButtonState(static_cast<decltype(hidState)::mapped_type::underlying_button_type>(i), hidEvent.buttonState[i]);
	}
}


int main()
{

	using namespace LInput;
	
	RawInput rawInput;

	// Add devices
	rawInput.AddDevice(RawInput::UsagePage::GenericDesktopControls, RawInput::GenericDesktopControlsUsagePage::Mouse, RawInput::Flags::EnableBackground);
	rawInput.AddDevice(RawInput::UsagePage::GenericDesktopControls, RawInput::GenericDesktopControlsUsagePage::Keyboard, RawInput::Flags::EnableBackground);
	//rawInput.AddDevice(RawInput::UsagePage::GenericDesktopControls, RawInput::GenericDesktopControlsUsagePage::Joystick, RawInput::Flags::EnableBackground);

	
	//Add input callback 

	rawInput.OnInput.Add(std::bind(&OnRawInput, std::placeholders::_1));
	rawInput.Enable(true);

	std::cout <<  "Press 'Q' three times to quit." << std::endl;


	//Handle application events.
	MessageLoop();

}
