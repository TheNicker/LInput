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
#include <algorithm>
#include <string>
#include "KeyCode.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace LInput
{

    class KeyCodeHelper
    {
    public:

        static std::string KeyCodeToString(KeyCode keycode)
        {
			
            auto foundItem = std::find_if(KeyCodeString.begin(), KeyCodeString.end(),
                [&keycode](decltype(KeyCodeString)::value_type const& item)
            {
                return keycode == item.first;
            });
			
			return foundItem !=  KeyCodeString.end() ?  foundItem->second : "Key not found";
        }

        static KeyCode KeyNameToKeyCode(const std::string& keyName)
        {
            auto foundItem = std::find_if(KeyCodeString.begin(), KeyCodeString.end(),
                [&keyName](decltype(KeyCodeString)::value_type const& item)
            {
                return keyName == item.second;
            });

            return foundItem != KeyCodeString.end() ? foundItem->first :KeyCode::UNASSIGNED;
        }

        static std::vector<std::vector<size_t>> ComputeCombinations(std::vector<size_t> groupSizes)
        {
            using namespace std;

            vector<vector<size_t>> result;
            const size_t totalGroups = groupSizes.size();
            vector<size_t> accumulativeSize = std::vector<size_t>(totalGroups);
            size_t totalElements = 1;
            for (size_t i = 0; i < groupSizes.size(); i++)
            {
                totalElements *= groupSizes[i];
                accumulativeSize[i] = totalElements;
            }

            for (size_t e = 0; e < totalElements; e++)
            {
                vector<size_t> indices = std::vector<size_t>(totalGroups);

                for (size_t i = 0; i < indices.size(); i++)
                {
                    size_t accumulativeFactor = i == 0 ? 1 : accumulativeSize[i - 1];
                    indices[i] = (e / accumulativeFactor) % groupSizes[i];
                }
                result.push_back(indices);

            }
            return result;
        }

        struct KeyEventParams
        {
            unsigned short repeatCount : 16;  // 0 - 15	The repeat count for the current message.The value is the number of times the keystroke is autorepeated as a result of the user holding down the key.If the keystroke is held long enough, multiple messages are sent.However, the repeat count is not cumulative.
            unsigned char scanCode : 8;    //16 - 23	The scan code.The value depends on the OEM.
            bool isExtented : 1;    //24	Indicates whether the key is an extended key, such as the right - hand ALT and CTRL keys that appear on an enhanced 101 - or 102 - key keyboard.The value is 1 if it is an extended key; otherwise, it is 0.
            unsigned char reserved : 4;    //25 - 28	Reserved; do not use.
            unsigned char contextCode : 1;    //29	The context code.The value is always 0 for a WM_KEYDOWN message.
            unsigned char previousKeyState : 1;     //30	The previous key state.The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
            unsigned char transitionstate : 1;    //31	The transition state.The value is always 0 for a WM_KEYDOWN message.
        };

      
        static KeyCode KeyCodeFromVK(uint32_t key, uint32_t params)
        {
            KeyEventParams* keydown = reinterpret_cast<KeyEventParams*>(&params);
            uint16_t scanCode = ((keydown->isExtented == true ? 0xe0 : 0)  << 8) |  MapVirtualKey(key, MAPVK_VK_TO_VSC_EX);
            return   static_cast<KeyCode>(scanCode);
        }

		static KeyCode KeyCodeFromRawInput(const RAWKEYBOARD& keyboard)
		{
			return static_cast<KeyCode>
				(
			  static_cast<uint16_t>(0)
			| (   (keyboard.Flags  & RI_KEY_E0) != 0  ? static_cast<uint16_t>(0xE000) : 0 )
			| ( (keyboard.Flags  & RI_KEY_E1) != 0    ? static_cast<uint16_t>(0xE100) : 0)
			| static_cast<uint16_t>(keyboard.MakeCode)
					)
				;
				
		}

		static KeyEvent KeyEventFromRawInput(const RAWKEYBOARD& keyboard)
		{
			KeyCode keyCode  = KeyCodeFromRawInput(keyboard);
			State state = ( (keyboard.Flags & RI_KEY_BREAK) != 0) ? State::Up : State::Down;

			return {keyCode, state};
		}


    };
}
