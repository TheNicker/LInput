#pragma once

#include <windows.h>
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
            const wchar_t CLASS_NAME[] = L"Sample Window Class";

            WNDCLASS wc = { };

            wc.lpfnWndProc = DefWindowProc;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.lpszClassName = CLASS_NAME;

            RegisterClass(&wc);

            // Create the window.

            fHandle = CreateWindowEx(
                0,                              // Optional window styles.
                CLASS_NAME,                     // Window class
                L"Learn to Program Windows",    // Window text
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
        //ShowWindow(hwnd, SW_SHOW);
    };

      /*  static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            if (msg == WM_CREATE)
            {
                CREATESTRUCT* s = (CREATESTRUCT*)lparam;
                if (SetProp(hwnd, L"windowClass", s->lpCreateParams) == 0)
                    std::exception("Unable to set window property");
                reinterpret_cast<Window*>(s->lpCreateParams)->handle = hwnd;

            }


            Window* target = reinterpret_cast<Window*>(GetProp(hwnd, (L"windowClass")));
            if (target != nullptr)

                return target->WndProc(hwnd, msg, wparam, lparam);
            else return DefWindowProc(hwnd, msg, wparam, lparam);

        }
       
        LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }*/

}

