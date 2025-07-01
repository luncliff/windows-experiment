# windows-experiment
Personal WindowsAppSDK, WinUI 3 experiments

- https://github.com/microsoft/WindowsAppSDK
- https://github.com/microsoft/microsoft-ui-xaml

## How To

### Setup

- https://github.com/microsoft/vcpkg

```
vcpkg integrate install
```

### Build

We have to restore packages in [packages.config](./App1/packages.config).

```
nuget restore windows-experiment.sln
MSBuild windows-experiment.sln /p:platform="x64" /p:configuration="Debug" /p:VcpkgEnableManifest=true
```

### Test

...
