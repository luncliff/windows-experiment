# windows-experiment
Personal WindowsAppSDK, WinUI 3 experiments

![Analysis](https://github.com/luncliff/windows-experiment/actions/workflows/analysis.yml/badge.svg)

- https://github.com/microsoft/WindowsAppSDK
- https://github.com/microsoft/microsoft-ui-xaml

Check current [concerns](./developer-concerns.md) and upcoming [work items](./TODO.md).

- https://github.com/luncliff/windows-experiment/issues  
  The issues and milestones are note organized now. They be overhauled for GitHub Actions workflows soon.

## How To

### Setup

- https://github.com/microsoft/vcpkg

```ps1
vcpkg integrate install
```

We have to restore packages in [packages.config](./App1/packages.config).

```ps1
nuget restore windows-experiment.sln
```

### Build

Build the solution file, or [App1](./App1/App1.vcxproj) VC++ project.

```ps1
# Full build of the projects
MSBuild windows-experiment.sln /p:platform="x64" /p:configuration="Debug" /p:VcpkgEnableManifest=true /Verbosity:Detailed
```

### :construction: Test

Need a CLI command to run MSTest on WinUI 3 project. See [UnitTestApp1](./UnitTestApp1/UnitTestApp1.vcxproj) project.

- https://learn.microsoft.com/en-us/visualstudio/test/microsoft-visualstudio-testtools-cppunittestframework-api-reference?view=vs-2022
- https://github.com/microsoft/testfx (need research to use)
- https://devblogs.microsoft.com/dotnet/introducing-mstest-34/ (It's for dotnet. Not C++/WinRT)

```ps1
# Full build of the projects
MSBuild windows-experiment.sln /t:UnitTestApp1 /p:platform="x64" /p:configuration="Debug" /p:VcpkgEnableManifest=true /Verbosity:Minimal

# Run tests with MSTest

```

### :construction: Deploy

The project creates MSIX bundle using the Visual Studio UI(`publish`).

- https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/single-project-msix

The following command will be tested with codesigning later.
The snippet just show that we will use App1Package project. Not a correct command to create MSIX bundle.

```ps1
# MSBuild windows-experiment.sln /t:App1Package /p:platform="x64" /p:configuration="Debug" /Verbosity:Minimal
```

## References

- https://github.com/PacktPublishing/Modernizing-Your-Windows-Applications-with-the-Windows-Apps-SDK-and-WinUI
- https://github.com/microsoft/WindowsAppSDK-Samples

### Additional (Design & Implementation)

WinUI / Controls
- WinUI Controls (API Overview): https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.ui.xaml.controls?view=windows-app-sdk-1.7
- https://github.com/Lewis-Marshall/WinUI3NavigationExample

Diagnostics & Logging
- LoggingChannel: https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.loggingchannel
- LoggingSession: https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.loggingsession

DirectX & Interop
- DirectX 12 Programming Guide: https://learn.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide
- SwapChainPanel Interop (`ISwapChainPanelNative`): https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/win32/microsoft.ui.xaml.media.dxinterop/nn-microsoft-ui-xaml-media-dxinterop-iswapchainpanelnative
- DirectX Graphics Samples: https://github.com/microsoft/DirectX-Graphics-Samples

C++/WinRT & MVVM
- C++/WinRT Overview: https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/
- Community MVVM Guidance (assorted blog/sample references – to curate)

Future / Extended
- Potential Telemetry Channel (deferred): evaluate after core logging adapter integration.
