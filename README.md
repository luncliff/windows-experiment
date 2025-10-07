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

We have to restore packages in [packages.config](./packages.config).
The NuGet artifacts will be located in [packages folder](./packages/).

```ps1
# nuget restore windows-experiment.sln
nuget restore packages.config -SolutionDirectory .
```

### Build

Build the solution file

- [Shared1](./Shared1/Shared1.vcxproj) implements some Models and ViewModels
- [App1](./App1/App1.vcxproj) implements the application
- [App1Package](./App1Package/App1Package.vcxproj) is a Windows Application Packaging project for this repository

```ps1
# Full build of the projects
MSBuild windows-experiment.sln /p:platform="x64" /p:configuration="Debug" /p:VcpkgEnableManifest=true /Verbosity:Detailed
```

### :construction: Test

Need a vstest.console.exe CLI command to run tests on WinUI 3 project.
Our test project is [UnitTestApp1](./UnitTestApp1/UnitTestApp1.vcxproj).

- [Test apps built with the Windows App SDK and WinUI](https://learn.microsoft.com/en-us/windows/apps/winui/winui3/testing/)
  - [Unit tests for Windows UI Library (WinUI) apps in Visual Studio](https://learn.microsoft.com/en-us/windows/apps/winui/winui3/testing/create-winui-unit-test-project)
- [Microsoft.VisualStudio.TestTools.CppUnitTestFramework API reference](https://learn.microsoft.com/en-us/visualstudio/test/microsoft-visualstudio-testtools-cppunittestframework-api-reference?view=vs-2022)
- [VSTest.Console.exe command-line options](https://learn.microsoft.com/en-us/visualstudio/test/vstest-console-options?view=vs-2022#uwp-example) for UWP example

```ps1
# Full build of the projects
MSBuild windows-experiment.sln /t:UnitTestApp1 /p:platform="x64" /p:configuration="Debug" /p:VcpkgEnableManifest=true /Verbosity:Minimal

# After full build, Run tests with /framework option and appxrecipe file
Push-Location x64/Debug/UnitTestApp1
    vstest.console.exe /framework:frameworkuap10 UnitTestApp1.build.appxrecipe
Pop-Location
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
- [Create your first WinUI 3 (Windows App SDK) project](https://learn.microsoft.com/en-us/windows/apps/winui/winui3/create-your-first-winui3-app)

### Additional (Design & Implementation)

#### WinUI 3, [C++/WinRT](https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/)

- [Microsoft.UI.Xaml.Controls Namespace](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.ui.xaml.controls?view=windows-app-sdk-1.7)
- https://github.com/Lewis-Marshall/WinUI3NavigationExample

Sadly, we can't use CommunityToolkit.

```log
Could not install package 'CommunityToolkit.WinUI.Controls.Primitives 8.2.250402'. You are trying to install this package into a project that targets 'native,Version=v0.0', but the package does not contain any assembly references or content files that are compatible with that framework.
```

- [Windows Community Toolkit Documentation](https://learn.microsoft.com/en-us/dotnet/communitytoolkit/windows/)
- https://github.com/CommunityToolkit/Windows

#### Diagnostics & Logging
- LoggingChannel: https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.loggingchannel
- LoggingSession: https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.loggingsession

#### DirectX & DXGI Interop

- [DirectX 12 Programming Guide](https://learn.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide)
- [SwapChainPanel Interop (`ISwapChainPanelNative` in `Microsoft.UI.Xaml` namespace)](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/win32/microsoft.ui.xaml.media.dxinterop/nn-microsoft-ui-xaml-media-dxinterop-iswapchainpanelnative)
- https://github.com/microsoft/DirectX-Graphics-Samples

### Future Works

The followings are for research for future use.
- https://github.com/microsoft/testfx
- https://devblogs.microsoft.com/dotnet/introducing-mstest-34/ (It's for dotnet. Not C++/WinRT)
