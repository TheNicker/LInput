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

#pragma once

#include <vector>
#include <array>
#include <climits>
#include <map>

#include <Windows.h>
#include <LLUtils/Exception.h>
#include <LLUtils/EnumClassBitwise.h>
#include <LLUtils/UniqueIDProvider.h>
#include <LLUtils/Buffer.h>
#include <LInput/Buttons/ButtonType.h>
#include <LInput/Keys/KeyCodeHelper.h>
#include <LInput/Mouse/MouseCode.h>


#include <type_traits>

#include <hidclass.h>
#include <hidusage.h>
#include <hidpi.h>




namespace LInput
{
    class RawInput
    {
    public:

        static constexpr size_t MaxMouseButtons = 8;
        static constexpr size_t MaxHIDButtons = 32;
        

        enum class UsagePage
        {
              Undefined
            , GenericDesktopControls // we use this
            , SimulationControls
            , Vr
            , Sport
            , Game
            , GenericDevice
            , Keyboard
            , LEDs
            , Button
        };
        

        enum class GenericDesktopControlsUsagePage
        {
              Undefined
            , Pointer
            , Mouse
            , Reserved
            , Joystick
            , GamePad
            , Keyboard // we use this
            , Keypad
            , MultiAxisController
            , TabletPCcontrols
        };

        enum class Flags : uint32_t
        {
              HandleCommandKeysForNoLegacy = 0x00000400 //  RIDEV_APPKEYS      If set, the application command keys are handled.RIDEV_APPKEYS can be specified only if RIDEV_NOLEGACY is specified for a keyboard device.
            , DontActivateOtherWindowOnClick = 0x00000200 // RIDEV_CAPTUREMOUSE                 If set, the mouse button click does not activate the other window.
            , EnableDeviceChangeMessage = 0x00002000 //RIDEV_DEVNOTIFY  If set, this enables the caller to receive WM_INPUT_DEVICE_CHANGE notifications for device arrivaland device removal.
           // Windows XP : This flag is not supported until Windows Vista
            , Exclude = 0x00000010 // RIDEV_EXCLUDE  If set, this specifies the top level collections to exclude when reading a complete usage page.This flag only affects a TLC whose usage page is already specified with RIDEV_PAGEONLY.
            , EnableBackgroundIfForegroundNotRegistered = 0x00001000 // RIDEV_EXINPUTSINK   If set, this enables the caller to receive input in the background only if the foreground application does not process it.In other words, if the foreground application is not registered for raw input, then the background application that is registered will receive the input.
           // Windows XP : This flag is not supported until Windows Vista
            , EnableBackground = 0x00000100 // RIDEV_INPUTSINK  If set, this enables the caller to receive the input even when the caller is not in the foreground.Note that hwndTarget must be specified.
            , DisableAppplicationHotKeys = 0x00000200 //  If set, the application - defined keyboard device hotkeys are not handled.However, the system hotkeys; for example, ALT + TAB and CTRL + ALT + DEL, are still handled.By default, all keyboard hotkeys are handled.RIDEV_NOHOTKEYS can be specified even if RIDEV_NOLEGACY is not specified and hwndTarget is NULL.
            , DisableLegacy = 0x00000030 // RIDEV_NOLEGACY   If set, this prevents any devices specified by usUsagePage or usUsage from generating legacy messages.This is only for the mouseand keyboard.See Remarks.
            , PageOnly = 0x00000020 // RIDEV_PAGEONLY  If set, this specifies all devices whose top level collection is from the specified usUsagePage.Note that usUsage must be zero.To exclude a particular top level collection, use RIDEV_EXCLUDE.
            , RemoveDevice = 0x00000001 // RIDEV_REMOVE If set, this removes the top level collection from the inclusion list.This tells the operating system to stop reading from a device which matches the top level collection.
        };
		
		//Add support Enum class support for bitflags
		LLUTILS_DEFINE_ENUM_CLASS_FLAG_OPERATIONS_IN_CLASS(Flags);


        struct RawInputDevice
        {
            UsagePage usagePage;
            GenericDesktopControlsUsagePage usage;
            uint32_t flags;
        };

        enum class RawInputDeviceType
        {
              Mouse
            , Keyboard
            , GamePad

        };

        struct DeviceInfo
        {
            uint8_t deviceID;
            RawInputDeviceType deviceType;

        };

        struct RawInputEvent
        {
            RawInputDeviceType deviceType;
            int deviceIndex;

        };
        struct RawInputEventKeyBoard : public RawInputEvent
        {
            State state;
            KeyCode scanCode;
        };

        struct RawInputEventMouse : RawInputEvent
        {
            int deltaX;
            int deltaY;
            int16_t wheelDelta;
            std::array<State, MaxMouseButtons> buttonState;
        };

        enum Axes
        {
              X
            , Y
            , Z
            , HatSwitch
            , ZRotate
            , Count
        };
        struct RawInputEventHID : public RawInputEvent
        {
            std::array<State, MaxHIDButtons> buttonState;
            std::array<int8_t, static_cast<size_t>(Axes::Count)> axes;
        };




        using OnInputType = LLUtils::Event< void(const RawInputEvent&)>;

        OnInputType OnInput;

        RawInput(HWND hwnd)  : fIds(1)
        {
            AttachWindow(hwnd);
        }

        template <bool allowThrow = true>
        void DetachCurrentWindow()
        {
            if (fWindowHandle != nullptr)
            {
                Enable(false);
                if (RemoveProp(fWindowHandle, sCurrentInstanceName) == nullptr)
                {
                    if (allowThrow)
                        LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Cannot remove property from window");
                }
                fWindowHandle = nullptr;
                for (const auto& p : fDevicehHandleToID)
                    fIds.Release(p.second);

                fIds.Normalize();

                if (allowThrow)
                if (fIds.GetNextID() != fIds.GetStartID())
                    LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Missing ID's that should have been released");


                fDevicehHandleToID.clear();
                fDeviceNameToInfo.clear();
                fWindowHandle = nullptr;
            }
        }


        void AttachWindow(HWND hwnd)
        {
            DetachCurrentWindow<true>();
            fWindowHandle = hwnd;
            
            if (SetProp(hwnd, sCurrentInstanceName, (HANDLE)this) == 0)
                LL_EXCEPTION_SYSTEM_ERROR("could not set window property");
            Enable(true);
        }

        ~RawInput()
        {
            DetachCurrentWindow<false>();
        }


        void HandleRawInputKeyboard(RAWINPUTHEADER& header, RAWKEYBOARD& rawKeyboard)
        {
            RawInputEventKeyBoard keyEvent{};
            auto evnt = KeyCodeHelper::KeyEventFromRawInput(rawKeyboard);
            keyEvent.state = evnt.state;
			keyEvent.deviceIndex = GetDeviceID(static_cast<HRAWINPUT> (header.hDevice));
            keyEvent.deviceType = RawInputDeviceType::Keyboard;
            keyEvent.scanCode = evnt.keyCode;
            OnInput.Raise(keyEvent);
        }


        void AddDevice(UsagePage page, GenericDesktopControlsUsagePage usage, Flags flags)
        {
            RAWINPUTDEVICE rid{};

            rid.usUsagePage = static_cast<USHORT>(page);
            rid.usUsage = static_cast<USHORT>(usage);
            rid.dwFlags = static_cast<DWORD>(flags | Flags::EnableDeviceChangeMessage);
            rid.hwndTarget = fWindowHandle;


            if (RegisterRawInputDevices(&rid, static_cast<UINT>(1), sizeof(RAWINPUTDEVICE)) == FALSE)
                LL_EXCEPTION_SYSTEM_ERROR("could not register raw input");


        }

        void HandleRawInputHID(RAWINPUTHEADER& header, RAWHID& rawHID)
        {

            RawInputEventHID evnt{ };
            evnt.deviceType = RawInputDeviceType::GamePad;
            evnt.deviceIndex = GetDeviceID(static_cast<HRAWINPUT>(header.hDevice));
            std::fill(std::begin(evnt.buttonState), std::end(evnt.buttonState), State::Up);

            USHORT               capsLength;
            
            std::array< USAGE, MaxHIDButtons> usage;
            ULONG                i, usageLength, value;


            //
            // Get the preparsed data block
            //

            UINT                 bufferSize;
            if (GetRawInputDeviceInfo(header.hDevice,
                RIDI_PREPARSEDDATA, NULL, &bufferSize) != 0)
                LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "could not get input device info");

            LLUtils::Buffer preparseData(bufferSize);


            if (GetRawInputDeviceInfo(header.hDevice, RIDI_PREPARSEDDATA, (void*)preparseData.data(), &bufferSize) != bufferSize)
                LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "could not get input device info");



            HIDP_CAPS caps;
            // Button caps
            if (HidP_GetCaps((PHIDP_PREPARSED_DATA)preparseData.data(), &caps) != HIDP_STATUS_SUCCESS)
                LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Unable to retrieve caps");

            //
            // Get the joystick's capabilities
            //

            capsLength = caps.NumberInputButtonCaps;



            std::unique_ptr<HIDP_BUTTON_CAPS[]> pButtonCaps = std::make_unique< HIDP_BUTTON_CAPS[]>(caps.NumberInputButtonCaps);


            if (HidP_GetButtonCaps(HidP_Input, pButtonCaps.get(), &capsLength, (PHIDP_PREPARSED_DATA)preparseData.data()) != HIDP_STATUS_SUCCESS)
                LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Unable to retrieve button caps");

            INT g_NumberOfButtons = pButtonCaps.get()->Range.UsageMax - pButtonCaps.get()->Range.UsageMin + 1;

            // Value caps
            std::unique_ptr<HIDP_VALUE_CAPS[]> pValueCaps = std::make_unique< HIDP_VALUE_CAPS[]>(caps.NumberInputValueCaps);
            capsLength = caps.NumberInputValueCaps;
            if (HidP_GetValueCaps(HidP_Input, pValueCaps.get(), &capsLength, (PHIDP_PREPARSED_DATA)preparseData.data()) != HIDP_STATUS_SUCCESS)
                LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Unable to retrieve value caps");

                //
                // Get the pressed buttons
                //

            usageLength = g_NumberOfButtons;
            if (HidP_GetUsages(
                    HidP_Input, pButtonCaps.get()->UsagePage, 0, usage.data(), &usageLength, (PHIDP_PREPARSED_DATA)preparseData.data(),
                    (PCHAR)rawHID.bRawData, rawHID.dwSizeHid
                ) != HIDP_STATUS_SUCCESS)
                LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Unable to retrieve usage values");


            for (i = 0; i < usageLength; i++)
                evnt.buttonState[usage[i] - pButtonCaps.get()->Range.UsageMin] = State::Down;

            //
            // Get the state of discrete-valued-controls
            //

            for (i = 0; i < caps.NumberInputValueCaps; i++ )
            {
                
                if (HidP_GetUsageValue(
                    HidP_Input, pValueCaps[i].UsagePage, 0, pValueCaps[i].Range.UsageMin, &value, (PHIDP_PREPARSED_DATA)preparseData.data(),
                    (PCHAR)rawHID.bRawData, rawHID.dwSizeHid
                ) != HIDP_STATUS_SUCCESS)
                    LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "Unable to retrieve usage values");

                switch (pValueCaps[i].Range.UsageMin)
                {
                    //case RIV_BUTTON
                case HID_USAGE_GENERIC_X:	// X-axis
                    evnt.axes[static_cast<uint8_t>(Axes::X)] = static_cast<int8_t>(static_cast<std::make_signed_t<decltype(value)>>(value) - 128);
                    break;

                case HID_USAGE_GENERIC_Y:	// Y-axis
                    evnt.axes[static_cast<uint8_t>(Axes::Y)] = static_cast<int8_t>(static_cast<std::make_signed_t<decltype(value)>>(value) - 128);
                    break;

                case HID_USAGE_GENERIC_Z: // Z-axis
                    evnt.axes[static_cast<uint8_t>(Axes::Z)] = static_cast<int8_t>(static_cast<std::make_signed_t<decltype(value)>>(value) - 128);
                    break;
                

                case HID_USAGE_GENERIC_RZ: // Rotate-Z
                    evnt.axes[static_cast<uint8_t>(Axes::ZRotate)] = static_cast<int8_t>(static_cast<std::make_signed_t<decltype(value)>>(value) - 128);
                    break;

                case HID_USAGE_GENERIC_HATSWITCH:	
                    evnt.axes[static_cast<uint8_t>(Axes::HatSwitch)]  = static_cast<int8_t>(value);
                    break;
                }
            }

            OnInput.Raise(evnt);
        }


        void HandleRawInputMouse(RAWINPUTHEADER& header, RAWMOUSE& mouse)
        {
            RawInputEventMouse evnt{};
            evnt.deltaX = mouse.lLastX;
            evnt.deltaY = mouse.lLastY;
			evnt.deviceIndex = GetDeviceID(static_cast<HRAWINPUT>(header.hDevice));
            evnt.deviceType = RawInputDeviceType::Mouse;
            if (mouse.ulButtons & RI_MOUSE_WHEEL)
                evnt.wheelDelta = static_cast<int16_t>(mouse.usButtonData) / WHEEL_DELTA;

            for (size_t i = 0; i < MaxMouseButtons; i++)
            {

                State& state = evnt.buttonState[i];
         
                if (mouse.ulButtons & (1ul << (i * 2)))
                    state = State::Down;


                
                if (mouse.ulButtons & (2ul << (i * 2)))
                    state = State::Up;
            }
            OnInput.Raise(evnt);
        }

		uint8_t GetDeviceID(HRAWINPUT handle)
		{
			return fDevicehHandleToID.find(handle)->second;
		}

        void ProcessRawInputMessage(RAWINPUT* rawInput)
        {

            if (rawInput->header.hDevice != nullptr) // Fix trackpad issues in laptops
            {
                switch (rawInput->header.dwType)
                {
                case RIM_TYPEMOUSE:
                    HandleRawInputMouse(rawInput->header, rawInput->data.mouse);
                    break;
                case RIM_TYPEKEYBOARD:
                    HandleRawInputKeyboard(rawInput->header, rawInput->data.keyboard);
                    break;
                case RIM_TYPEHID:
                    HandleRawInputHID(rawInput->header, rawInput->data.hid);
                    break;
                default:
                    LL_EXCEPTION_UNEXPECTED_VALUE;

                }
            }
        }


        void ProcessWInMessages([[maybe_unused]] HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            switch (msg)
            {

            case  WM_INPUT:
            {
                UINT dwSize;
                if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER)) != 0)
                    LL_EXCEPTION_SYSTEM_ERROR("can not get raw input data");


                LLUtils::Buffer lpb(dwSize);


                if (GetRawInputData((HRAWINPUT)lparam,
                    RID_INPUT,
                    lpb.data(),
                    &dwSize,
                    sizeof(RAWINPUTHEADER)) != dwSize)
                {
                    LL_EXCEPTION_SYSTEM_ERROR("can not get raw input data");
                }

                ProcessRawInputMessage(reinterpret_cast<RAWINPUT*>(lpb.data()));
                break;
            }

            case WM_INPUT_DEVICE_CHANGE:
            {
                const HRAWINPUT rawInputHandle = reinterpret_cast<HRAWINPUT>(lparam);
                if (wparam == GIDC_ARRIVAL)
                {

                    UINT size = 0;
                    GetRawInputDeviceInfo(reinterpret_cast<HRAWINPUT>(lparam), RIDI_DEVICENAME, nullptr, &size);
                    auto buffer = std::make_unique<wchar_t[]>(size);
                    GetRawInputDeviceInfo(reinterpret_cast<HRAWINPUT>(lparam), RIDI_DEVICENAME, buffer.get(), &size);
                    std::wstring deviceName(buffer.get());

                    RID_DEVICE_INFO info;
                    info.cbSize = sizeof(info);
                    size = sizeof(info);
                    GetRawInputDeviceInfo(reinterpret_cast<HRAWINPUT>(lparam), RIDI_DEVICEINFO, &info, &size);

                    auto it = fDeviceNameToInfo.find(deviceName);
                    uint8_t id = 0;
                    if (it == std::end(fDeviceNameToInfo))
                    {
                        id = fIds.Acquire();
                        fDeviceNameToInfo.emplace_hint(it, deviceName, DeviceInfo{ id , static_cast<RawInputDeviceType>(info.dwType)});
                    }
                    else
                    {
                        id = it->second.deviceID;
                    }


                    //Remove old handle if exists.

                    for (const auto p : fDevicehHandleToID)
                    {
                        if (p.second == id)
                        {
                            fDevicehHandleToID.erase(p.first);
                            break;
                        }
                    }

                    //Add new handle
                    fDevicehHandleToID.emplace(rawInputHandle, id);
                }
                else // (wparam == GIDC_REMOVAL)
                {

                }
            }

            break;

            }
        }

        static  LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            RawInput* _this = reinterpret_cast<RawInput*>(GetProp(hwnd, sCurrentInstanceName));
            _this->ProcessWInMessages(hwnd, msg, wparam, lparam);

            return _this->fLastWndProc(hwnd, msg, wparam, lparam);
        }


#if 0 
       static LRESULT HookProcedure(int code, WPARAM wParam, LPARAM lParam)
        {

            if (code < 0)
                return CallNextHookEx(fHook, code, wParam, lParam);

            CWPRETSTRUCT* sa = reinterpret_cast<PCWPRETSTRUCT>(lParam);
            CWPRETSTRUCT& s = *sa;
            
            if (s.message == WM_SIZE)
            {
                int k = 0;
            }
            RawInput* _this = reinterpret_cast<RawInput*>(GetProp(s.hwnd, sCurrentInstanceName));
            if ( _this != nullptr)
                _this->ProcessWInMessages(s.hwnd, s.message, s.wParam ,s.lParam);

             return CallNextHookEx(fHook, code,wParam, lParam);
       }
#endif


        

        

        void Enable(bool enable)
        {
#if 0 
            if (fWindowHandle != nullptr) 
            {
                if (enable == true && fHook == nullptr)
                    fHook =  SetWindowsHookEx(WH_GETMESSAGE, reinterpret_cast<HOOKPROC>(&HookProcedure), nullptr, GetCurrentThreadId());
                else if (enable == false && fHook != nullptr)
                {
                    UnhookWindowsHookEx(fHook);
                    fHook = nullptr;
                }
            }
#else 
            //Change WindowProc directly.
            
            if (fWindowHandle != nullptr) // Enable or disable only if window is attached
            {
                if (enable == true && fLastWndProc == nullptr)
                {
                    fLastWndProc = (WNDPROC)SetWindowLongPtr(fWindowHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WndProc));

                    if (fLastWndProc == nullptr)
                        LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "could not get set custom window procedure");
                }
                else if (enable == false && fLastWndProc != nullptr)
                {
                    if (SetWindowLongPtr(fWindowHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(fLastWndProc)) != reinterpret_cast<LONG_PTR>(&WndProc))
                        LL_EXCEPTION(LLUtils::Exception::ErrorCode::InvalidState, "could not restore original window procedure");

                    fLastWndProc = nullptr;
                }
            }
#endif
        }

    private:
        static constexpr wchar_t sCurrentInstanceName[] = L"__LINPUT_CURRENT_INSTANCE__";
		using MapDeviceHandleToID = std::map<HRAWINPUT, uint8_t> ;
		using MapDeviceNameToInfo = std::map<std::wstring, DeviceInfo>;
		
        MapDeviceHandleToID fDevicehHandleToID;
        MapDeviceNameToInfo fDeviceNameToInfo;
		LLUtils::UniqueIdProvider<uint8_t> fIds;
        HWND fWindowHandle = nullptr;
#if 0
        static inline HHOOK fHook = nullptr;
#endif
        WNDPROC fLastWndProc = nullptr;
    };
    
}

