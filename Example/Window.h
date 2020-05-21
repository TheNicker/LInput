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


#include <windows.h>
#include <LLUtils/StringDefs.h>
#include <LLUtils/Buffer.h>
#include <functional>
namespace LInput::Win32
{
    class Window
    {
    public:
     
        Window()
        {
            // Register the window class.
        	
            const LLUtils::native_char_type CLASS_NAME[] = LLUTILS_TEXT("Sample Window Class");

            WNDCLASS wc = { };

            wc.lpfnWndProc = DefWindowProc;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.lpszClassName = CLASS_NAME;

            RegisterClass(&wc);

            // Create the window.

            fHandle = CreateWindowEx(
                0,                              // Optional window styles.
                CLASS_NAME,                     // Window class
                LLUTILS_TEXT("Learn to Program Windows"),    // Window text
                WS_OVERLAPPEDWINDOW,            // Window style

                // Size and position
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                NULL,       // Parent window    
                NULL,       // Menu
                GetModuleHandle(nullptr),  // Instance handle
                this        // Additional application data
            );
        }

        HWND GetHandle() const
        {
            return fHandle;
        }
    private:
        HWND fHandle = nullptr;
        
    };
}

