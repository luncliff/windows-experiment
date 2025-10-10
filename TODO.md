# TODO (Forward-Looking)

Scope: Items here are intentionally deferred until after Shared1 static library integration and validation.

## 1. Shared2 (WinRT / DLL Component)

Objectives:
- Provide projected (WinRT) surfaces for stable ViewModel & service APIs.
- Encapsulate legacy COM interop behind modern async-friendly wrappers.
- Consolidate DirectX device management and interop helpers.

## 2. DirectX / Graphics Consolidation
- Inventory current `DX` namespace utilities (responsibilities, dependencies).
- Implement abstraction boundary (factory or service pattern) that shields UI from device recreation details.
- Add test harness for device lost simulation (skip gracefully if unsupported environment).

## 3. Logging Enhancements
- Evaluate log volume per channel (heuristic counters in debug builds).
- Add log file size monitoring + rotation policy prototype.
- Introduce optional Telemetry channel (privacy & policy review prerequisite).
- Structured logging: adopt `LoggingFields` for settings persistence and device creation events.

## 4. Performance & Diagnostics
- Add lightweight timing helper (RAII scope logger) integrated with adapter.
- Conditional compilation flags for high-frequency DX diagnostics.
- Explore integration with ETW consumer tooling (trace session guidelines doc).
- Potential Telemetry Channel (deferred): evaluate after core logging adapter integration.

## 5. Reliability / Resilience
- Introduce retry/backoff for settings persistence failures.
- Add watchdog for graphics device resets (log escalation to Warning/Error).
- Unit test harness for simulated failure injection (mock adapter? optional test-only hooks).

## 6. Documentation Roadmap
- Create `graphics-design.md` once DirectX migration starts.
- Create `logging-rotation-design.md` for size governance strategy.
- Expand README with a dedicated "Architecture Overview" after Shared2 surfaces stabilize.

## 8. References
- WinUI Controls: https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.ui.xaml.controls?view=windows-app-sdk-1.7
- SwapChainPanel Interop (`ISwapChainPanelNative`): https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/win32/microsoft.ui.xaml.media.dxinterop/nn-microsoft-ui-xaml-media-dxinterop-iswapchainpanelnative
- LoggingChannel: https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.loggingchannel
- LoggingSession: https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.loggingsession
- C++/WinRT: https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/
- DirectX 12 Programming Guide: https://learn.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide
- Direct3D/Swap Chain Samples: https://github.com/microsoft/DirectX-Graphics-Samples
- MVVM Patterns (C++/WinRT community): (assorted blog/sample links – curate later)

(End of forward-looking TODO list)
