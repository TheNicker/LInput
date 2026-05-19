# Raw Input Device Identity Fix Plan

## Original Problem

`LInput::RawInput::ProcessWInMessages` can crash during startup while processing `WM_INPUT_DEVICE_CHANGE` with `GIDC_ARRIVAL`.

The original code assumes that every raw-input device handle can produce a usable `RIDI_DEVICENAME`:

```cpp
UINT size = 0;
GetRawInputDeviceInfo(reinterpret_cast<HRAWINPUT>(lparam), RIDI_DEVICENAME, nullptr, &size);
auto buffer = std::make_unique<wchar_t[]>(size);
GetRawInputDeviceInfo(reinterpret_cast<HRAWINPUT>(lparam), RIDI_DEVICENAME, buffer.get(), &size);
std::wstring deviceName(buffer.get());
```

That assumption is not valid in all runtime environments. In Remote Desktop sessions, raw-input mouse and keyboard devices can be reported with valid handles but without usable device interface names.

Observed failing pattern:

```text
type=keyboard sizeResult=0 size=1 sizeLastError=0 nameResult=4294967295 nameLastError=0 name=<failed>
type=mouse    sizeResult=0 size=1 sizeLastError=0 nameResult=4294967295 nameLastError=0 name=<failed>
```

This means the first size query effectively reports only a null terminator, and the second query does not return a real name. `GetLastError()` remains `ERROR_SUCCESS`, so this behaves like an unnamed device rather than a normal Win32 failure with a useful system error.

The same diagnostic binary can return normal names on non-RDP or different physical-device environments:

```text
\\?\HID#VID_...#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}
\\?\ACPI#...#{378de44c-56ef-11d1-bc8c-00a0c91405dd}
```

That points to a runtime input-stack difference, not a simple compiler or SDK header issue.

## Current Design

The existing `RawInput` design has two separate identities:

```text
fDeviceNameToInfo   stable device interface name -> logical device id/type
fDevicehHandleToID  current raw-input handle      -> logical device id
```

This design is sound when `RIDI_DEVICENAME` exists:

1. A device arrives.
2. Windows gives a current `HRAWINPUT` device handle.
3. `RIDI_DEVICENAME` gives a stable device interface name.
4. LInput maps the stable name to a small public `deviceIndex`.
5. LInput maps the current handle to that public `deviceIndex`.
6. Later `WM_INPUT` messages carry only the current handle, so LInput uses `fDevicehHandleToID` to recover the public ID.

The weak point is that the stable name is treated as mandatory. When the name is missing, the current code cannot register the handle correctly and can crash.

There is another hardening issue:

```cpp
return fDevicehHandleToID.find(handle)->second;
```

If an input message arrives for a handle that was not registered, this dereferences `end()` and fails unclearly. It should throw a descriptive `LL_EXCEPTION` instead.

The implementation should go one step further: if a `WM_INPUT` message has a non-null `hDevice` that is not registered yet, lazily register that handle from `RAWINPUTHEADER::dwType`. That keeps input routing working when arrival notification ordering is not ideal. Unknown raw-input types should still throw.

## API Constraints

Microsoft documents `RIDI_DEVICENAME` as returning a device interface name. It is not documented as a guaranteed friendly name or a guaranteed universal stable identity for every raw-input handle.

Important constraints:

- `WM_INPUT_DEVICE_CHANGE` gives `GIDC_ARRIVAL`, `GIDC_REMOVAL`, and the raw-input device handle in `lParam`.
- `RAWINPUTHEADER::hDevice` is a handle to the device generating the raw input data. It is suitable for routing current messages, but it should not be treated as a long-term public stable device identity.
- If `RIDI_DEVICENAME` is missing, raw input does not provide another documented stable per-physical-device identity.
- Multiple unnamed mouse or keyboard handles may exist at the same time.
- Without names or another stable key, those unnamed handles cannot be reliably correlated across removal/arrival cycles.

So there is a tradeoff:

```text
per-handle fallback:
  distinguishes unnamed handles
  but deviceIndex can churn when handles change

per-type fallback:
  keeps deviceIndex stable
  but collapses all unnamed devices of the same type
```

Because `deviceIndex` is user-visible state and stale IDs are worse than collapsing unnamed virtual devices, the best fallback is stable identity by unnamed device type.

## Documentation Links

Relevant Microsoft raw-input documentation:

- `GetRawInputDeviceInfo`: <https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getrawinputdeviceinfoa>
- `WM_INPUT_DEVICE_CHANGE`: <https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-input-device-change>
- `RAWINPUTHEADER`: <https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawinputheader>
- `GetRawInputDeviceList`: <https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getrawinputdevicelist>

Important doc-derived conclusions:

- `RIDI_DEVICENAME` is useful when available, but it is only a device interface-name query.
- `WM_INPUT_DEVICE_CHANGE` gives the current raw-input device handle in `lParam`; it does not provide another durable device identity.
- `RAWINPUTHEADER::hDevice` is suitable for routing the current input message, not for a public stable identity.
- `GetRawInputDeviceList` is not a reliable RDP workaround because Microsoft documents that RDP input devices do not appear in that list.

## Stable Device ID Contract

The public `deviceIndex` is expected to remain stable for the lifetime of the `RawInput` instance.

Named devices:

- If a device is unplugged and later replugged with the same `RIDI_DEVICENAME`, it must receive the same `deviceIndex`.
- `GIDC_REMOVAL` must only remove the current `HRAWINPUT` route; it must not erase the name-to-ID entry or release the ID.
- A new `HRAWINPUT` handle on re-arrival is expected and should be routed back to the existing ID.

Unnamed devices:

- If Windows does not provide `RIDI_DEVICENAME`, raw input does not expose a documented stable per-physical-device key.
- The fallback stability contract is therefore per raw-input type: one unnamed mouse ID, one unnamed keyboard ID, and one unnamed HID/gamepad ID if needed.
- Multiple unnamed devices of the same type cannot be distinguished reliably without inventing unstable identity from transient handles.

Application restarts:

- This plan preserves IDs across unplug/replug while the `RawInput` instance is alive.
- Stability across application restarts is out of scope unless the application persists name-to-ID mappings separately.

## Rejected Alternatives

Per-handle fallback:

- Benefit: distinguishes multiple unnamed handles of the same type.
- Problem: public `deviceIndex` churns whenever Windows replaces the handle.
- Result: rejected because stable user-visible IDs are more important than distinguishing unnamed RDP virtual handles.

`GetRawInputDeviceList` fallback:

- Benefit: can enumerate raw-input devices in normal local-device environments.
- Problem: documented as excluding RDP input devices.
- Result: rejected as the main fix for this RDP startup crash.

SetupDi or HID lookup fallback:

- Benefit: can provide richer identity when a device interface path exists.
- Problem: the failing case is specifically missing a usable raw-input device interface name.
- Result: rejected for this minimal fix; it adds complexity without solving unnamed RDP handles reliably.

## Proposed Solution

Keep the existing design for named devices and add a minimal fallback path for unnamed devices.

Identity policy:

```text
named device:
  key = RIDI_DEVICENAME
  stable id is per interface name

unnamed mouse:
  key = unnamed mouse
  stable id is shared by all unnamed mouse handles

unnamed keyboard:
  key = unnamed keyboard
  stable id is shared by all unnamed keyboard handles

unnamed HID/gamepad:
  key = unnamed HID/gamepad
  stable id is shared by all unnamed HID/gamepad handles if needed
```

Routing policy:

```text
current HRAWINPUT handle -> stable deviceIndex
```

Removal policy:

```text
GIDC_REMOVAL:
  erase the transient handle -> id mapping
  keep stable named and unnamed identities
  do not release IDs
```

This preserves the public `deviceIndex` contract and avoids pretending that unnamed RDP handles can be distinguished as stable physical devices.

## Implementation Details

Keep the change small and local to `External/LInput/Include/LInput/Win32/RawInput/RawInput.h`.

Use character-type agnostic code:

```cpp
LLUtils::native_string_type
LLUtils::native_char_type
LLUTILS_TEXT(...)
GetRawInputDeviceInfo(...)
```

Avoid hardcoded `GetRawInputDeviceInfoW` and avoid `std::wstring` in the raw-input identity map.

### Data Members

Change the named-device map to native strings:

```cpp
using MapDeviceNameToInfo = std::map<LLUtils::native_string_type, DeviceInfo>;
```

Add a fallback identity map:

```cpp
using MapUnnamedDeviceTypeToInfo = std::map<RawInputDeviceType, DeviceInfo>;
```

Keep:

```cpp
using MapDeviceHandleToID = std::map<HRAWINPUT, uint8_t>;
```

### Helper Behavior

Add concise private helpers:

```text
GetRawInputDeviceType(DWORD rawType)
  RIM_TYPEMOUSE    -> RawInputDeviceType::Mouse
  RIM_TYPEKEYBOARD -> RawInputDeviceType::Keyboard
  RIM_TYPEHID      -> RawInputDeviceType::GamePad
  otherwise throw LL_EXCEPTION_UNEXPECTED_VALUE

GetRawInputDeviceInfo(HRAWINPUT handle)
  calls RIDI_DEVICEINFO
  checks return value against (UINT)-1
  checks size/result are valid
  throws LL_EXCEPTION_SYSTEM_ERROR on failure

TryGetRawInputDeviceName(HRAWINPUT handle)
  first calls RIDI_DEVICENAME with nullptr to query size
  if query fails with a real GetLastError(), throw LL_EXCEPTION_SYSTEM_ERROR
  if size <= 1, return empty native string
  allocate size + 1 native chars
  call RIDI_DEVICENAME again
  if second call fails with a real GetLastError(), throw LL_EXCEPTION_SYSTEM_ERROR
  if second call fails with GetLastError() == ERROR_SUCCESS, return empty native string
  return the native string

GetOrCreateDeviceID(deviceName, deviceType)
  if deviceName is non-empty, use fDeviceNameToInfo
  otherwise use fUnnamedDeviceTypeToInfo
```

For future maintainers, keep the source comments short and focused:

- The name-query helper should explain that missing `RIDI_DEVICENAME` is expected in some valid environments.
- The identity maps should document named identity, unnamed fallback identity, and transient handle routing separately.
- The arrival/input registration path should document why named devices replace old handles while unnamed devices keep multiple live handles.
- The input path should document lazy registration for handles whose arrival notification was missed or not delivered first.

### Arrival Handling

For `WM_INPUT_DEVICE_CHANGE / GIDC_ARRIVAL`:

```text
handle = reinterpret_cast<HRAWINPUT>(lparam)
info = GetRawInputDeviceInfo(handle)
deviceType = GetRawInputDeviceType(info.dwType)
deviceName = TryGetRawInputDeviceName(handle)
id = GetOrCreateDeviceID(deviceName, deviceType)

if deviceName is non-empty:
  remove any old handle mapped to the same id

fDevicehHandleToID.insert_or_assign(handle, id)
```

The old-handle removal should stay limited to named devices. For unnamed fallback devices, there can be many live handles sharing the same fallback ID, and removing all previous handles of that ID would break routing for still-live handles.

### Removal Handling

For `WM_INPUT_DEVICE_CHANGE / GIDC_REMOVAL`:

```text
fDevicehHandleToID.erase(handle)
```

Do not erase `fDeviceNameToInfo`.

Do not erase `fUnnamedDeviceTypeToInfo`.

Do not release the ID back to `fIds`.

### Input Handling Hardening

Change `GetDeviceID(HRAWINPUT handle)` so it checks the lookup:

```text
if handle is not found:
  lazily register the handle using RAWINPUTHEADER::dwType
```

This avoids crashing through an invalid iterator and handles message-ordering cases where input arrives before a matching device-change notification.

### Window Startup Hardening

Keep this minimal:

- Check `CreateWindowEx` result.
- Check `RegisterClass`, but allow `ERROR_CLASS_ALREADY_EXISTS`.
- Keep existing `SetProp` check.
- In `WndProc`, if `GetProp` returns `nullptr`, return `DefWindowProc`.

This protects startup and teardown edges without changing the input design.

## Throwing Policy

Throw with `LL_EXCEPTION_SYSTEM_ERROR` when a required Win32 API call fails and `GetLastError()` is meaningful:

- `RIDI_DEVICEINFO` failure.
- `RIDI_DEVICENAME` size query failure with non-zero `GetLastError()`.
- `RIDI_DEVICENAME` data query failure with non-zero `GetLastError()`.
- `RegisterClass` failure other than `ERROR_CLASS_ALREADY_EXISTS`.
- `CreateWindowEx` failure.
- `SetProp` failure.

Do not throw when `RIDI_DEVICENAME` is simply unavailable:

```text
size <= 1
empty returned string
(UINT)-1 with GetLastError() == ERROR_SUCCESS
```

Treat those as unnamed raw-input devices and use the stable fallback identity.

Throw for internal state violations that cannot be recovered:

- `WM_INPUT` arrives with an unsupported raw-input type.
- Required raw-input metadata cannot be queried while lazily registering an unknown handle.

## Testing Plan

Build OIViewer:

```powershell
cmake --build D:\dev\oiv\build\Clang-22.1\Debug --target OIViewer
```

Build the LInput example:

```powershell
cmake --build D:\dev\oiv\External\LInput\build\debug --target LInputExample
```

Run in an RDP session:

```powershell
D:\dev\oiv\External\LInput\build\debug\Example\LInputExample.exe
D:\dev\oiv\build\Clang-22.1\Debug\bin\OIViewer.exe
```

Expected RDP result:

- Missing mouse/keyboard device names do not crash.
- Input events are produced.
- All unnamed mouse handles share one stable mouse `deviceIndex`.
- All unnamed keyboard handles share one stable keyboard `deviceIndex`.
- Replugged unnamed mouse/keyboard devices reuse their type fallback `deviceIndex`.
- Handle mappings are removed on `GIDC_REMOVAL`.

Run on a local/non-RDP machine:

- Named `\\?\HID#...` and `\\?\ACPI#...` devices still receive distinct stable IDs.
- Unplug/replug of a named device reuses the ID associated with its interface name, even when Windows provides a new `HRAWINPUT` handle.

If the app still throws, inspect:

```text
C:\Users\liolah01\AppData\Roaming\OIV\0.18.0.846\oiv.log
```

The expected remaining failures should be descriptive `LL_EXCEPTION` messages rather than invalid iterator crashes or generic startup failures.

## Acceptance Criteria

- OIViewer no longer crashes at startup under RDP because `RIDI_DEVICENAME` is missing.
- Missing device interface names are handled as expected runtime behavior.
- Required API failures still throw.
- Public `deviceIndex` remains stable for unnamed RDP mouse/keyboard input.
- Named physical devices keep the same public `deviceIndex` across unplug/replug when `RIDI_DEVICENAME` is unchanged.
- Multiple unnamed devices of the same type are explicitly collapsed to one stable fallback ID.
- The code remains local, minimal, and non-intrusive.
