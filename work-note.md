# Project Work Notes

## Session Log
- 2025-10-05: Initialized design exploration. Goal: introduce `Shared1` component to host Model & ViewModel for reuse by `App1` (UI) and `UnitTestApp1` (tests). No code added yet per instructions.

## 1. Requirements (Updated)
### Explicit (Original + New)
1. Proof-of-concept Windows Store style desktop application using C++20, C++/WinRT, WinUI 3; target Windows 11 SDK 10.0.26100 (x64 & ARM64).
2. Support local run, debug, and unit testing.
3. Introduce new shared component project `Shared1` (static library) for Model & ViewModel core logic.
4. Future `Shared2` (DLL / possible WinRT component) to encapsulate legacy COM & provide projected/binding surfaces.
5. `App1` remains UI (XAML) layer; `App1Package` for packaging; `UnitTestApp1` for tests.
6. No implementation code additions during design phase; documentation only.
7. Must integrate STL-heavy, Win32 API, DirectX 12 & DXGI modules (in Shared1 & later Shared2).
8. Logging to standardize on `Windows.Foundation.Diagnostics` (`LoggingSession` + per-module `LoggingChannel`). External telemetry integration can layer on top later (no custom sink abstraction now).
9. Shared1 may include `Microsoft.UI.Xaml` headers where needed to implement `INotifyPropertyChanged` patterns for UI-friendly objects.

### Implicit / Derived (Revised)
1. Single root namespace: `winrt::App1` (and `winrt::App1::implementation`); Shared1 adopts same to reduce friction. Tests keep `winrt::UnitTestApp1`.
2. Because Shared1 is static (no .winmd), any types requiring XAML binding will initially remain in App1 or use thin wrapper classes until Shared2 exists.
3. Layering: Core data + logic in Shared1 (plain C++ with C++/WinRT helpers) → optional wrappers in App1 → future projected types in Shared2.
4. Shared1 gets its own minimal PCH to reduce compile times; avoid heavy UI includes in widely-used headers.
5. Strict acyclic dependency graph: Shared1 has no dependencies on sibling projects.
6. Tests should exercise logic without XAML thread where possible; isolate dispatcher needs behind abstractions.
7. Logging lifecycle: application owns a single `FileLoggingSession` instance and distributes (shares) module-specific `LoggingChannel` objects (one per logical concern) to Shared* modules.
8. DirectX utilities can begin in Shared1 but may migrate to Shared2 once ABI or COM boundary needed.

### Out-of-Scope (This Phase)
- Creating `.idl` for Shared1 (deferred until surface stabilizes or Shared2 creation).
- Full DI container implementation (interfaces only for now).
- UI automation / E2E testing.

## 2. Reference Findings (Augmented)
Key references & impact:
- WinUI 3 testing docs → validate library extraction approach.
- Windows.Foundation.Diagnostics samples → shape logging abstraction (`LoggingChannel`).
- Community MVVM patterns for C++/WinRT → inform minimal PropertyChanged helper design.
- DirectX DeviceResources samples → candidate centralization (likely Shared2 later).

Implications:
- Start static (.lib) to iterate fast; introduce projection only when stable.
- Logging wrapper ensures pluggability (system channel vs external).
- Keep ViewModel core independent from binding projection for easier future ABI stabilization.

## 3. Architectural Options for `Shared1` (Revised)
### Option A: Static Library (.lib) (Chosen now)
Pros: rapid iteration, unrestricted C++20/Win32/DirectX usage, simpler refactors.
Cons: no direct XAML binding metadata; wrappers required.

### Option B: Windows Runtime Component (Deferred / Shared2)
Pros: direct binding, cross-language reuse, stable ABI.
Cons: early lock-in, more boilerplate, slower iteration.

### Migration Path
1. Implement logic & plain C++ ViewModel-like classes in Shared1.
2. Identify stable surfaces.
3. Introduce Shared2 as WinRT component with adapters.
4. Gradually expose needed types via IDL.

Decision: Proceed with Option A (static library) for PoC; plan Shared2 for projection needs.

## 4. Project Responsibility Split (Updated)
- Shared1 (static): domain models, core ViewModel logic (plain C++ + PropertyChanged helper), may call shared `LoggingChannel` instances passed from App1, optional DirectX utilities, no IDL.
- App1: XAML UI, binding wrappers/adapters until Shared2; app bootstrap (create `FileLoggingSession`, create/register channels, supply them to Shared1 and future Shared2).
- UnitTestApp1: tests for Shared1 logic; can create ephemeral in-memory (or file) `FileLoggingSession` + `LoggingChannel` for verification; limited UI-thread tests.
- Shared2 (future DLL / WinRT component): projected ABI surfaces, legacy COM wrappers, device/resource management, may register additional channels within the same session.
- App1Package: packaging assets unchanged.

## 5. Namespace & Folder Plan within `Shared1` (Static Library, Flattened)
Flat layout (no `include/` vs `src/` split). Each file documents its role with a header comment.
```
Shared1/
  Shared1.vcxproj
  pch.h
  pch.cpp
  PropertyChangedBase.h            (@file: helper for INotifyPropertyChanged pattern)
  SettingsState.h / SettingsState.cpp (@file: core settings logic; persistence)
  DxDeviceHelpers.h / DxDeviceHelpers.cpp (@file: optional DirectX device utilities - deferred if not immediate)
  LoggingChannels.h                (@file: declarations of channel names/ids constants)
  (future) CounterCore.h / CounterCore.cpp
  (future) Other domain/model files...
```
Guidelines:
- Keep headers self-contained; forward declare where possible.
- Limit DirectX includes to `.cpp` unless unavoidable.
- Avoid cyclic includes by relying on forward declarations in headers.

## 6. Dependency Boundaries (Updated)
- Shared1: C++20 STL, selective WinRT headers (`winrt/Windows.Foundation.h`, `winrt/Microsoft.UI.Xaml.Data.h`, `winrt/Windows.Foundation.Diagnostics.h`), DirectX 12/DXGI as needed, no custom logging abstraction—uses `LoggingChannel` references provided by App1 (dependency passed, not globally fetched).
- App1: depends on Shared1; owns `FileLoggingSession` and channel creation.
- UnitTestApp1: depends on Shared1; creates its own channels/sessions for verification (or supplies null channel to test no-logging paths).
- Shared2 (future): depends on Shared1; may register additional channels into existing session.

## 7. Build Integration Notes
- Add Shared1 (static lib) early in solution; reference from App1 + UnitTestApp1.
- Match existing configurations (Debug/Release x64/ARM64).
- Reuse root vcpkg manifest; no new manifest file.
- PCH to accelerate heavy WinRT/DirectX includes.
- Provide CMake-like optionality later (future consideration) — out of scope now.

## 8. Testing Strategy Outline (Adjusted)
- Use `[TestMethod]` for core & ViewModel logic (avoid UI thread).
- Reserve `[UITestMethod]` for future projected/binding wrappers only.
- PropertyChanged tests (in `UnitTestApp1`):
  - Verify event raised on value change.
  - Ensure no event on idempotent set.
  - (Later) Add thread-affinity guideline test once dispatcher policy defined.
- Logging tests: create disposable `FileLoggingSession` + `LoggingChannel`, invoke Shared1 logic, then query session channel status (future—once logic logs messages). For now only ensure passing channel pointer compiles & is optional (can be null).
- DirectX tests (future): attempt device creation; mark inconclusive/skip if unsupported environment.
- E2E/UI automation deferred until Shared2 introduces stable projection.

## 9. Risks & Mitigations (Revised)
| Risk | Impact | Mitigation |
|------|--------|------------|
| Transition from static to projected component requires refactor | Medium | Keep logic decoupled; design adapter layer early |
| Early need for binding surfaces | Medium | Temporary wrappers in App1; accelerate Shared2 if wrappers proliferate |
| Logging channel proliferation (too many channels) | Low | Define naming convention & cap initial set (Core, ViewModel, DX, Persistence) |
| Logging session file growth | Medium | Implement rotation/size monitoring later; document current PoC limitation |
| DirectX device duplication | Medium | Centralize helpers; single creation path chosen early |
| PropertyChanged threading issues | Low | Document single-thread assumption; add dispatcher abstraction in later phase |
| GPU-dependent test instability | Low | Skip/mark inconclusive if device create fails |

## 10. Reference List (Updated)
- WindowsAppSDK Samples (structure & DX patterns)
- WinUI 3 Testing docs
- Windows.Foundation.Diagnostics Logging samples
- LoggingChannel API (https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.loggingchannel?view=winrt-26100)
- LoggingSession API (https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.loggingsession?view=winrt-26100)
- C++/WinRT MVVM community guidance
- DirectML & DirectX samples for resource management patterns

## 10.1 Logging Strategy (New)
Goals: Structured, module-isolated logging with minimal abstraction overhead.

Components:
- Single `FileLoggingSession` (App1-owned) started at app launch; file path documented for diagnostics.
- Distinct `LoggingChannel` per concern (initial proposal):
  - `App1.Core`
  - `App1.ViewModel`
  - `App1.DX`
  - `App1.Persistence`
  (Extend cautiously; avoid per-class channels.)
- Shared1 will consume `Windows::Foundation::Diagnostics::ILoggingChannel` directly, enabling injection of:
  - Real platform `LoggingChannel` instances
  - `NullLoggingChannel` (no-op implementation) for silent mode
  - `StreamLoggingChannel` (redirects messages to `std::wostream`, e.g., `std::wcout` or test buffer)

Guidelines:
- Channel acquisition: App1 constructs channels and passes references (or wrappers) to Shared1 constructors/factories.
- Event Categories: encode fine-grained categories via keywords or event name strings instead of creating new channels.
- Severity Mapping: Informational (state transitions), Warning (recoverable anomalies), Error (operation failed), Critical (unrecoverable; precedes graceful shutdown path).
- Performance: Batch high-frequency logs or guard behind conditional flags to avoid excessive session size.
- Rotation (Future): Add size monitoring & archival; out-of-scope for PoC.
- Testing: Unit tests may instantiate ephemeral session + channel; verify code paths accept channel reference but not assert on file contents yet (parsing file deferred).

Open Questions:
- Do we need a dedicated Telemetry channel separate from Core? (Defer until analytics integration.)

## 11. Additional Concerns (2025-10-05 Update)

These clarifications refine scope for the immediate Shared1 implementation phase.

1. Application-Owned Channels & Optional Logging
  - All channel instances (system or custom) are created and lifetime-managed by `App1` or test harness.
  - Shared1 receives `ILoggingChannel` (interface) values; no global lookup.
  - Null Object: `NullLoggingChannel` implements `ILoggingChannel` with no-op `LogMessage` methods.
  - Stream redirection: `StreamLoggingChannel` implements `ILoggingChannel` writing to an injected `std::wostream` (diagnostics/tests).
  - Definitions will initially live in Shared1 (likely `pch.cpp` or dedicated logging source) until stabilized.

2. Null Object Pattern (Preview)
  - Implemented via `NullLoggingChannel` deriving from `winrt::implements<..., ILoggingChannel>`.
  - Eliminates conditional branches in consumer code (interface always callable).
  - Detailed sketches & alternative options documented in `developer-concerns.md`.

3. Logging Channel Separation (Deferral)
  - Current practical set remains: `App1.Core`, `App1.ViewModel`, `App1.DX`, `App1.Persistence` (Telemetry TBD).
  - Evaluation criteria for adding/removing channels (volume, semantic isolation, consumer filtering) will be elaborated in `developer-concerns.md`.
  - Work-note only records the decision state; design exploration transferred out to reduce churn here.

4. DirectX (`namespace DX`) Sources Untouched
  - Existing `DX`-prefixed or namespaced utility code is a placeholder validating build, linkage, and SwapChainPanel integration feasibility.
  - Planned *replacement* or significant consolidation is expected in a future phase (likely when Shared2 or a dedicated graphics module emerges).
  - Therefore: No refactors or relocations of these DX helpers during the current Shared1 extraction to avoid wasted effort.

5. Documentation File Role Separation
  - `work-note.md`: High-level requirements, decisions, finalized architecture & risk record.
  - `developer-concerns.md`: In-depth, evolving design sketches (null logger pattern, channel taxonomy proposals, adapter API drafts, future refactors under consideration).
  - `TODO.md`: Forward-looking task tracking for future Shared2 (projection, COM interop, DX consolidation) — intentionally excluded from near-term execution scope.

6. Test Impact
  - Unit tests inject `NullLoggingChannel` or `StreamLoggingChannel` to verify logging interaction without requiring a real session.
  - No log file parsing yet; tests may optionally capture stream output for simple assertions.

7. Out-of-Scope Confirmation (for this immediate sprint)
  - Implementing channel rotation, size governance, or telemetry uplink.
  - Refactoring or relocating DX helper code.
  - Introducing performance counters into logging (may be considered after basic Shared1 integration).

References Added (see README too):
- SwapChainPanel interop: `ISwapChainPanelNative` (DX interop) – https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/win32/microsoft.ui.xaml.media.dxinterop/nn-microsoft-ui-xaml-media-dxinterop-iswapchainpanelnative
- WinUI Controls overview – https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.ui.xaml.controls?view=windows-app-sdk-1.7

## 12. Shared1 Implementation Plan (2025-10-05)

### Baseline Verification
✅ **Build Test Completed**: `MSBuild windows-experiment.sln /p:platform="x64" /p:configuration="Debug" /p:VcpkgEnableManifest=true /Verbosity:Minimal` succeeded with warnings only (no errors).

### Implementation Sequence
1. **Project Structure Setup**
   - Create `Shared1` static library project (.vcxproj) matching existing configurations (Debug/Release x64/ARM64)
   - Add to solution and configure project references in App1 and UnitTestApp1
   - Create minimal PCH structure with logging adapter null object

2. **Logging Infrastructure** 
   - Implement `logging_adapter` struct in Shared1 pch.h/pch.cpp per developer-concerns.md design
   - Add free functions: `log_info()`, `log_warn()`, `log_error()` with null-object behavior
   - Test compilation without breaking existing code

3. **Class Migration Priority**
   a. **BasicItem**: Pure data model, minimal XAML dependencies
   b. **BasicViewModel**: Moderate complexity, demonstrates PropertyChanged pattern
   c. **DeviceResources**: Complex DirectX utilities, validates DirectX build integration

4. **Migration Process per Class**
   - Copy .h/.cpp files to Shared1 with flat structure
   - Remove dependency on .idl (convert to plain C++ classes)
   - Update namespace references and logging calls to use adapter
   - Remove original files from App1 project
   - Update App1 to reference Shared1 types

5. **Testing Integration**
   - Add BasicViewModel unit tests to UnitTests.cpp
   - Verify PropertyChanged event behavior
   - Test logging adapter injection (real vs null)

6. **Validation**
   - Full solution build after each migration
   - Run existing tests to ensure no regression
   - Final test execution via vstest.console.exe

### Risk Mitigation
- **Incremental approach**: One class at a time with build verification
- **Rollback plan**: Keep original files until migration validated
- **Dependency tracking**: Update includes systematically to avoid linker errors

### Success Criteria
- All projects build without errors
- Existing functionality preserved (UI navigation, settings persistence)
- New tests pass with both real and null logging adapters
- Clear separation achieved: App1 (UI/binding), Shared1 (logic/models), UnitTestApp1 (validation)

- Introduce keyword constants (bitmask) for filtering? (Add when first filtering use case appears.)

## 11. Pending Decisions / Open Questions (Revised)
1. Threshold for introducing Shared2 (e.g., >=3 wrapper types or cross-language demand)?
2. Implement PropertyChanged base now vs after first migration? (Recommend now.)
3. Immediate placement of DirectX device utilities (Shared1 vs delay)?
4. Standard logging categories & naming (Core, ViewModel, DX, Persistence, Telemetry) — finalize soon?
5. Formal DI container necessity & timing.

## 12. Next Steps (Updated Roadmap)
1. Scaffold `Shared1` static library project (flat layout; add `pch.h/pch.cpp`).
2. Add `PropertyChangedBase` helper (header-only or minimal .cpp) & basic unit tests.
3. Implement `SettingsState` (core persistence + change raise using helper) and adapt `SettingsViewModel` to delegate.
4. Introduce initial LoggingChannel setup in App1 (create session + channels) and thread channel reference into `SettingsState` (optional parameter for now).
5. Add unit test verifying PropertyChanged semantics for `SettingsState` (no channel) and with dummy channel.
6. Draft DirectX helper stub (optional placeholder) or defer based on priority.
7. Define readiness criteria & checklist for starting Shared2 (add to notes section).
8. Add logging category constants in `LoggingChannels.h` (strings only, no heavy logic).

## 12.1 Shared1 Implementation Plan (October 2025)

### Build Verification Status
✓ Baseline build confirmed working (MSBuild commands from README.md)
✓ NuGet packages restored successfully
✓ Both App1 and UnitTestApp1 compile to .exe without errors
Note: Non-critical MSB3106 warnings about AI.Foundation.winmd path format (known issue with WindowsAppSDK 1.7)

### Phase 1: Project Scaffolding
1. **Create Shared1.vcxproj**
   - Static library (ConfigurationType=StaticLibrary)
   - Match App1 configurations: Debug/Release x64/ARM64
   - PlatformToolset=v143, C++20, Unicode
   - Include vcpkg manifest support (VcpkgEnableManifest=true)
   - Reference same NuGet packages as App1 for consistency

2. **Add to Solution**
   - Insert Shared1 project into windows-experiment.sln
   - Add project dependencies: App1 → Shared1, UnitTestApp1 → Shared1
   - Verify all configuration/platform combinations build

3. **Create PCH Structure**
   - Shared1/pch.h: Core WinRT includes + ILoggingChannel implementations
   - Shared1/pch.cpp: Implementation of NullLoggingChannel & StreamLoggingChannel
   - Avoid heavy XAML includes in PCH (keep minimal for shared logic)

### Phase 2: Core Logic Migration
4. **BasicItem Migration**
   - Copy BasicItem.h/.cpp to Shared1 (remove projected WinRT portions)
   - Remove IDL dependency; create plain C++ class with PropertyChanged helper
   - Add ILoggingChannel parameter for constructor (optional, defaults to null)

5. **BasicViewModel Migration** 
   - Copy BasicViewModel.h/.cpp to Shared1
   - Replace spdlog calls with ILoggingChannel::LogMessage usage
   - Remove async/WinRT collection projections; use STL containers
   - Add factory method accepting ILoggingChannel

6. **DeviceResources Migration**
   - Copy DeviceResources.h/.cpp to Shared1
   - Keep DX namespace but add logging integration
   - No breaking changes to public API initially

### Phase 3: Integration & Testing
7. **Update App1 References**
   - Add #include paths to Shared1 headers
   - Remove BasicItem.*, BasicViewModel.*, DeviceResources.* from App1.vcxproj
   - Create thin WinRT wrapper classes in App1 if needed for XAML binding
   - Remove corresponding .idl files

8. **Create Tests**
   - Add BasicViewModel tests in UnitTests.cpp
   - Test PropertyChanged events using StreamLoggingChannel
   - Test null logging behavior with NullLoggingChannel
   - Verify container operations work without WinRT projections

9. **Build Verification**
   - Incremental builds after each migration step
   - Full solution build test
   - Unit test execution verification

### Migration Sequence (Build-Test-Fix Cycle)
```
Step 1: Scaffold Shared1 + PCH → Build test → Fix issues
Step 2: Migrate BasicItem → Build test → Fix issues  
Step 3: Migrate BasicViewModel → Build test → Fix issues
Step 4: Migrate DeviceResources → Build test → Fix issues
Step 5: Update App1 references → Build test → Fix issues
Step 6: Add tests → Test execution → Fix issues
```

### Risk Mitigation
- Keep original files until full migration verified
- Maintain backup of working baseline before each step
- Use incremental approach (one class per step)
- Verify build success before moving to next component

### Validation Criteria  
- [ ] Shared1.lib builds successfully in all configurations
- [ ] App1 links with Shared1 and launches normally
- [ ] UnitTestApp1 links with Shared1 and runs existing tests
- [ ] New BasicViewModel tests pass
- [ ] No regression in existing functionality

## 13. Items Requiring Validation
- Flat folder structure for Shared1 (confirm no subdirectories).
- Initial channel set: Core, ViewModel, DX, Persistence (add Telemetry?).
- Timing for introducing DirectX helper (now vs after `SettingsState`).
- Accept postponement of file rotation logic.
- Readiness threshold for Shared2 (define numeric or qualitative trigger).

---
 (End of updated session notes)
