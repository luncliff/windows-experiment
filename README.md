# windows-experiment
Personal WindowsAppSDK, WinUI 3 experiments

![Analysis](https://github.com/luncliff/windows-experiment/actions/workflows/analysis.yml/badge.svg)

- https://github.com/microsoft/WindowsAppSDK
- https://github.com/microsoft/microsoft-ui-xaml

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
MSBuild windows-experiment.sln /t:App1 /p:platform="x64" /p:configuration="Debug" /p:VcpkgEnableManifest=true /Verbosity:Detailed
```

### Test

Need a CLI command to run MSTest on WinUI 3 project. See [UnitTestApp1](./UnitTestApp1/UnitTestApp1.vcxproj) project.

- https://github.com/microsoft/testfx
- https://devblogs.microsoft.com/dotnet/introducing-mstest-34/

...

### Deploy

```ps1
MSBuild windows-experiment.sln /t:App1Package /p:platform="x64" /p:configuration="Debug" /Verbosity:Minimal
```

## References

- https://github.com/PacktPublishing/Modernizing-Your-Windows-Applications-with-the-Windows-Apps-SDK-and-WinUI
- https://github.com/Lewis-Marshall/WinUI3NavigationExample
