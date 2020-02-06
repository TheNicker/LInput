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
#include <string>
#include <vector>
#include "KeyCode.h"
#include <LLUtils/StringUtility.h>
#include <LLUtils/Exception.h>

#pragma pack(push, 1)
namespace LInput
{
    class KeyCombination;
    using ListKeyCombinations = std::vector<KeyCombination> ;
    class KeyCombination
    {
    public:
        struct Hash
        {
            size_t operator()(const KeyCombination& key) const
            {
                static_assert(sizeof(KeyCombination) <= sizeof(size_t)
                    , "For the benefit of fast hashing the size of key combination must be less or equal to the size of size_t");
                return static_cast<size_t>(key.keyValue);
            }
        };

        bool operator ==( const KeyCombination& rhs) const
        {
            return keyValue == rhs.keyValue;
        }

        static ListKeyCombinations  FromString(const std::string& string)
        {
            using namespace LLUtils;
            std::string upper = StringUtility::ToUpper(string);

            KeyCombination combination;
            using KeyCodeList = std::vector<KeyCode>;
            using KeyCodeListList = std::vector<KeyCodeList>;

            KeyCodeListList duplicateCombinations2Darray;

            ListKeyCombinations bindings;


            ListAString keyCombination = StringUtility::split(upper, '+');

            for (const std::string& key : keyCombination)
            {
                if (key == "CONTROL")
                    duplicateCombinations2Darray.push_back({ KeyCode::LCONTROL, KeyCode::RCONTROL });
                else if (key == "ALT")
                    duplicateCombinations2Darray.push_back({ KeyCode::LALT, KeyCode::RALT });
                else if (key == "SHIFT")
                    duplicateCombinations2Darray.push_back({ KeyCode::LSHIFT, KeyCode::RSHIFT });
                else if (key == "WINKEY")
                    duplicateCombinations2Darray.push_back({ KeyCode::LWIN, KeyCode::RWIN });
                else if (key == "ENTER")
                    duplicateCombinations2Darray.push_back({ KeyCode::ENTERMAIN, KeyCode::KEYPADENTER });
                else
                {
                    KeyCode keyCode = KeyCodeHelper::KeyNameToKeyCode(key);
                    if (keyCode == KeyCode::UNASSIGNED)
                        LL_EXCEPTION(LLUtils::Exception::ErrorCode::BadParameters, std::string("The key name '") + key + "' could not be found");
                    combination.AssignKey(keyCode);
                }

            }

            if (duplicateCombinations2Darray.empty() == false)
            {
                using namespace std;
                vector<size_t> sizes;

                for (const KeyCodeList& vec : duplicateCombinations2Darray)
                    sizes.push_back(vec.size());

                vector<vector<size_t>> indices = KeyCodeHelper::ComputeCombinations(sizes);

                for (const vector<size_t>& currentIndices : indices)
                {
                    KeyCombination extraCombination = combination;
                    for (size_t i = 0; i < currentIndices.size(); i++)
                        extraCombination.AssignKey((duplicateCombinations2Darray[i])[currentIndices[i]]);

                    bindings.push_back(extraCombination);
                }
            }
            else
            {
                bindings.push_back(combination);
            }

            return bindings;
        }
        std::string ToString();
#ifdef _WIN32
        static KeyCombination FromVirtualKey(uint32_t key, uint32_t params)
        {
            KeyCombination combination;
            combination.leftAlt = (GetKeyState(VK_LMENU) & static_cast<USHORT>(0x8000)) != 0;
            combination.rightAlt = (GetKeyState(VK_RMENU) & static_cast<USHORT>(0x8000)) != 0;
            combination.leftCtrl = (GetKeyState(VK_LCONTROL) & static_cast<USHORT>(0x8000)) != 0;
            combination.rightCtrl = (GetKeyState(VK_RCONTROL) & static_cast<USHORT>(0x8000)) != 0;
            combination.leftShift = (GetKeyState(VK_LSHIFT) & static_cast<USHORT>(0x8000)) != 0;
            combination.rightShift = (GetKeyState(VK_RSHIFT) & static_cast<USHORT>(0x8000)) != 0;
            combination.leftWinKey = (GetKeyState(VK_LWIN) & static_cast<USHORT>(0x8000)) != 0;
            combination.rightWinKey = (GetKeyState(VK_RWIN) & static_cast<USHORT>(0x8000)) != 0;
            combination.keycode = KeyCodeHelper::KeyCodeFromVK(key, params);
            return combination;
        }
#endif
     
      
    private:
        void AssignKey(KeyCode key)
        {
            switch (key)
            {
            case KeyCode::LALT:
                leftAlt = 1;
                break;
            case KeyCode::RALT:
                rightAlt = 1;
                break;
            case KeyCode::RCONTROL:
                rightCtrl = 1;
                break;
            case KeyCode::LCONTROL:
                leftCtrl = 1;
                break;
            case KeyCode::RSHIFT:
                rightShift = 1;
                break;
            case KeyCode::LSHIFT:
                leftShift = 1;
                break;
            case KeyCode::RWIN:
                rightWinKey = 1;
                break;
            case KeyCode::LWIN:
                leftWinKey = 1;
                break;
            default:
                //Not a modifer - assign key.
                keycode = key;
                break;
            }
        }

#pragma region memeber fields

        union
        {
            uint32_t keyValue = 0;
            struct
            {
                KeyCode keycode;
                unsigned char leftCtrl : 1;
                unsigned char rightCtrl : 1;
                unsigned char leftAlt : 1;
                unsigned char rightAlt : 1;
                unsigned char leftShift : 1;
                unsigned char rightShift : 1;
                unsigned char leftWinKey : 1;
                unsigned char rightWinKey : 1;
            };
        };

#pragma endregion //memeber fields
    };
}

#pragma pack(pop)