

Use the [README.md](../README.md) for build/test instructions.

## Coding Convention

Global functions will use snake_case. See [pch.h](../App1/pch.h) and [pch.cpp](../App1/pch.cpp).

We use PascalCase for class names. However we mix PascalCase and snake_case for member functions.
For example,

- [SettingsPage](../App1/SettingsPage.xaml.h) class overrides XAML Control virtual functions like [Page.OnNavigatedFrom](https://learn.microsoft.com/en-us/uwp/api/windows.ui.xaml.controls.page.onnavigatedfrom) and [Page.OnNavigatedTo](https://learn.microsoft.com/en-us/uwp/api/windows.ui.xaml.controls.page.onnavigatedto). For those existing functions in XAML, it will use PascalCase.
- [MainWindow.xaml](../App1/MainWindow.xaml) defines custom event handlers in XAML file. To distinguish the handlers between XAML Control member functions, we will use snake_case. See [MainWindow.xaml.h](../App1/MainWindow.xaml.h)

## Windows Development References

Always prefer search existing code and web posts, before creating new one.

The followings are mostly examples, references to help developers.

### C++/WinRT & WinUI 3

For Windows platform.

- https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/
  - https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/get-started
  - https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/faq
- https://github.com/microsoft/WindowsAppSDK-Samples
- https://github.com/microsoft/Windows-universal-samples

### DirectX, DXGI(DirectX Graphics Infrastructure)

Start search from the [DirectX Landing Page](https://devblogs.microsoft.com/directx/landing-page/)

- [DirectX graphics and gaming](https://learn.microsoft.com/en-us/windows/win32/directx)
- [Direct3D 12 programming guide](https://learn.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide)
  - [Direct3D 12 reference](https://learn.microsoft.com/en-us/windows/win32/direct3d12/direct3d-12-reference)
- [DXGI Reference](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/d3d10-graphics-reference-dxgi)
  - [Programming Guide for DXGI](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/dx-graphics-dxgi-overviews)
- https://github.com/microsoft/DirectX-Graphics-Samples
- https://github.com/microsoft/DirectX-SDK-Samples (legacy samples)
- https://github.com/microsoft/DirectML
  - https://github.com/microsoft/DirectML/tree/master/Samples/DirectMLSuperResolution
- https://github.com/walbourn/directx-vs-templates (For DeviceResources class and its usage)
  - [Working with D3D12 and vcpkg](https://github.com/walbourn/directx-vs-templates/tree/main/d3d12game_vcpkg)
  - [Workign with C++/WinRT](https://github.com/walbourn/directx-vs-templates/tree/main/d3d12game_uwp_cppwinrt_dr)
- [DirectX Agility SDK](https://devblogs.microsoft.com/directx/directx12agility/)

### Windows Media Foundation SDK

- https://github.com/microsoft/media-foundation
  - https://github.com/microsoft/media-foundation/tree/master/samples/XamlSwapchainSample

### Helpful Information, Blog Posts

The following blogs require more complicated search, but will be helpful for developer to gather more details and understand API concepts.

- [The Old New Thing - Practical development throughout the evolution of Windows](https://devblogs.microsoft.com/oldnewthing/)
- [Games for Windows and the DirectX SDK blog](https://walbourn.github.io/tags/)
- [DirectX Developer Blog](https://devblogs.microsoft.com/directx/)

### MSVC

When more details are required for compiler options, see the following manuals.
Expect MSVC 170 or later. (Add `?view=msvc-170` after the following URLs)

- [Compiler options listed by category](https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category#header-unitsmodules)
- [Compiler options listed alphabetically](https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically)


### vcpkg package manager

The project is using vcpkg manifest mode.
Current dependency and configuration can be found in the [vcpkg.json manifest](../vcpkg.json) file and [vcpkg configuration](../vcpkg-configuration.json) file.

- https://learn.microsoft.com/en-us/vcpkg/
- https://github.com/microsoft/vcpkg
- https://github.com/microsoft/vcpkg-tool
- https://github.com/luncliff/vcpkg-registry

When `vcpkg search` command is not available in the current developer environment, we have to search packages from the web. In the case, we will use the vcpkg.io.

For example, if we are searching `directx-agility` and `directx-headers`, the following URLs will give more information.

- https://vcpkg.io/en/package/directx12-agility
- https://vcpkg.io/en/package/directx-headers
