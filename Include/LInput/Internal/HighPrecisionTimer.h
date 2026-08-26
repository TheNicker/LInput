#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <Windows.h>

    #include <cstdint>
    #include <functional>
    #include <limits>
    #include <mutex>
    #include <system_error>
    #include <utility>

namespace LInput::Internal
{
    class HighPrecisionTimer final
    {
      public:

        using Callback = std::function<void()>;

        explicit HighPrecisionTimer(Callback callback) : fCallback(std::move(callback)) { RegisterWindow(); }

        ~HighPrecisionTimer()
        {
            Enable(false);
            UnregisterWindow();
            if (fTimer != nullptr)
                static_cast<void>(DeleteTimerQueueTimer(nullptr, fTimer, INVALID_HANDLE_VALUE));
        }

        HighPrecisionTimer(const HighPrecisionTimer&)            = delete;
        HighPrecisionTimer& operator=(const HighPrecisionTimer&) = delete;
        HighPrecisionTimer(HighPrecisionTimer&&)                 = delete;
        HighPrecisionTimer& operator=(HighPrecisionTimer&&)      = delete;

        void SetRepeatInterval(uint32_t repeatInterval)
        {
            if (fRepeatInterval != repeatInterval)
            {
                fRepeatInterval = repeatInterval;
                if (fEnabled)
                {
                    Enable(false);
                    Enable(true);
                }
            }
        }

        void SetDueTime(uint32_t dueTime) { fDueTime = dueTime; }
        [[nodiscard]] bool GetEnabled() const { return fEnabled; }

        void Enable(bool enable)
        {
            if (enable == fEnabled)
                return;

            fEnabled = enable;
            if (fEnabled)
            {
                if (fTimer == nullptr)
                {
                    if (CreateTimerQueueTimer(&fTimer, nullptr, TimerQueueCallback, this, fDueTime, fRepeatInterval,
                                              WT_EXECUTEINTIMERTHREAD) == FALSE)
                    {
                        fTimer   = nullptr;
                        fEnabled = false;
                        ThrowLastError("Could not create the LInput timer");
                    }
                }
                else if (ChangeTimerQueueTimer(nullptr, fTimer, fDueTime, fRepeatInterval) == FALSE)
                {
                    fEnabled = false;
                    ThrowLastError("Could not re-enable the LInput timer");
                }
            }
            else if (fTimer != nullptr && ChangeTimerQueueTimer(nullptr, fTimer, Infinite, Infinite) == FALSE)
            {
                ThrowLastError("Could not disable the LInput timer");
            }
        }

      private:

        [[noreturn]] static void ThrowLastError(const char* operation)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), operation);
        }

        static void CALLBACK TimerQueueCallback(void* context, BOOLEAN)
        {
            auto* timer = static_cast<HighPrecisionTimer*>(context);
            SendMessageW(timer->fWindow, TimerMessage, reinterpret_cast<WPARAM>(timer), 0);
        }

        void Execute()
        {
            if (!fEnabled)
                return;

            if (fCallback)
                fCallback();

            if (IsOneShot())
                fEnabled = false;
        }

        static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
        {
            if (message == TimerMessage)
            {
                reinterpret_cast<HighPrecisionTimer*>(wParam)->Execute();
                return 0;
            }

            return DefWindowProcW(window, message, wParam, lParam);
        }

        static void RegisterWindowClass()
        {
            std::call_once(sRegisterWindowClass,
                           []
                           {
                               WNDCLASSW windowClass{};
                               windowClass.lpfnWndProc   = WindowProc;
                               windowClass.hInstance     = GetModuleHandleW(nullptr);
                               windowClass.lpszClassName = WindowClassName;
                               if (RegisterClassW(&windowClass) == 0)
                                   ThrowLastError("Could not register the LInput timer window class");
                           });
        }

        void RegisterWindow()
        {
            RegisterWindowClass();
            fWindow = CreateWindowExW(0, WindowClassName, nullptr, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                      CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, GetModuleHandleW(nullptr),
                                      nullptr);
            if (fWindow == nullptr)
                ThrowLastError("Could not create the LInput timer dispatch window");
        }

        void UnregisterWindow()
        {
            if (fWindow != nullptr)
            {
                DestroyWindow(fWindow);
                fWindow = nullptr;
            }
        }

        [[nodiscard]] bool IsOneShot() const { return fRepeatInterval == Infinite; }

        static constexpr DWORD Infinite            = (std::numeric_limits<DWORD>::max)();
        static constexpr wchar_t WindowClassName[] = L"LInput.HighPrecisionTimerWindow";
        static constexpr UINT TimerMessage         = WM_USER + 1;
        static inline std::once_flag sRegisterWindowClass;

        bool fEnabled = false;
        HANDLE fTimer = nullptr;
        Callback fCallback;
        uint32_t fDueTime        = Infinite;
        uint32_t fRepeatInterval = Infinite;
        HWND fWindow             = nullptr;
    };
}  // namespace LInput::Internal

#else
    #error LInput::Internal::HighPrecisionTimer currently requires Win32
#endif
