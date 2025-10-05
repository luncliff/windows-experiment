# Project Work Notes

## Session Log
- 2025-10-05: Initialized design exploration. Goal: introduce `Shared1` component to host Model & ViewModel for reuse by `App1` (UI) and `UnitTestApp1` (tests). No code added yet per instructions.

## 1. Requirements
### Explicit
- Create proof-of-concept Windows Store style application using C++20, C++/WinRT, WinUI 3.
- Support local run, debug, and unit testing.
- Introduce new shared component project `Shared1` for Model + ViewModel logic.
- `App1` stays as application UI project (XAML, pages, window shell).
- `App1Package` remains packaging project (MSIX / WAP packaging resources).
- `UnitTestApp1` executes tests against Model & ViewModel (and possibly some light UI thread dependent logic when required).
- Do not add implementation code yet; focus on design & references.
- Keep ability to integrate STL-heavy and Win32 API dependent modules.

### Implicit / Derived
- `Shared1` should minimize direct dependency on `Microsoft.UI.Xaml` types except where ViewModel or observable patterns require (e.g., `winrt::Microsoft::UI::Xaml::Data::INotifyPropertyChanged`). Prefer abstraction to allow headless testing.
- Favor separation between pure domain Model (POCO / plain C++ structs & classes) and ViewModel (WinRT projected types implementing notify/change, commands, async operations).
- Build configurations (x64, ARM64; Debug/Release) should work uniformly across new project.
- Reuse existing precompiled header strategy for compile speed (consider a `pch.h` within `Shared1`).
- Ensure no circular dependencies: `Shared1` must not reference `App1` or test project.
- Provide clear namespace segmentation (e.g., `App1::Core` / `App1::Model` / `App1::ViewModel`) inside `Shared1` to avoid scattering inside UI project.
- Consider whether `Shared1` is a WinUI Class Library (Windows Runtime Component producing .winmd) vs static library; choose based on consumption needs (XAML binding to WinRT types requires WinRT component types).
- Testing strategy: non-UI logic should be testable without UI thread; ViewModel properties requiring dispatcher should use injected dispatcher abstraction or conditional helpers.

## 2. Reference Findings
(Sourced via web searches; summarized)
- Microsoft guidance for WinUI 3 testing recommends factoring logic into a library referenced by both app and test project.
- Unit Test App (WinUI 3) supports `[UITestMethod]` for UI thread dependent tests.
- Windows Runtime Component vs static library trade-off: WinRT component exposes projected types & metadata (.winmd) enabling XAML binding across language boundaries; static library simpler but lacks metadata for binding unless consumed only internally by same project.
- For C++/WinRT MVVM, common pattern: Domain Model (no WinRT dependency) + WinRT ViewModel classes implementing `INotifyPropertyChanged` + optional command helpers.

## 3. Architectural Options for `Shared1`
### Option A: Windows Runtime Component (recommended)
Pros:
- Produces .winmd enabling XAML binding to ViewModel types directly in `App1` XAML.
- Allows potential future reuse by other languages (C#, etc.).
- Natural fit for `INotifyPropertyChanged` & observable collections.
Cons:
- Slightly more build overhead; must conform to WinRT type restrictions (public types must be WinRT-friendly).

### Option B: Static Library (.lib)
Pros:
- Simplest; full unrestricted C++20 features internally.
- Faster incremental builds.
Cons:
- No metadata for direct XAML binding; would need adapter types inside `App1` or duplication.
- Harder to share across language projections.

Decision: Use Option A (Windows Runtime Component) for ViewModel + any models needing binding. For heavy algorithmic or STL/Win32 code that may not be WinRT-friendly, place into internal-only static sub-library later or internal namespaces with non-projected types (marked internal by not adding to IDL).

## 4. Proposed Project Responsibility Split
- Shared1 (new):
  - Public WinRT IDL for ViewModels, observable collections, service interfaces (e.g., settings service, data provider).
  - Internal C++ sources for domain logic not exposed via IDL.
  - PCH optimized to include only necessary Windows headers (avoid pulling full XAML unless needed).
  - Expose asynchronous operations using `IAsyncAction` / `IAsyncOperation<T>` where appropriate.
- App1:
  - UI composition: XAML pages, window shell, navigation, visual states.
  - Consumes `Shared1` ViewModels via XAML binding and code-behind.
  - App lifecycle & logging initialization.
- UnitTestApp1:
  - Tests domain logic (pure) directly.
  - Tests ViewModel notify semantics & async using `[UITestMethod]` when UI thread needed.
  - Potential mocking: Provide simple stub dispatcher or services.
- App1Package:
  - Packaging & assets only; references remain unchanged (still references App1 which transitively uses Shared1).

## 5. Namespace & Folder Plan within `Shared1`
```
Shared1/
  Shared1.vcxproj                (Windows Runtime Component)
  Shared1.idl                    (umbrella IDL includes others)
  idl/
    Model.idl                    (WinRT-exposed model types if needed)
    ViewModel.idl                (Primary ViewModel declarations)
    Services.idl                 (Service interfaces)
  include/                       (Public headers for internal-only helpers)
    Shared1/pch.h
    Shared1/internal/            (Not exposed via IDL)
  src/
    pch.cpp
    model/...
    viewmodel/...
    services/...
    internal/...
```

## 6. Dependency Boundaries
- `Shared1` may depend on: C++20 STL, WinRT base, limited XAML types for data binding, `spdlog` (optionally) for logging but consider interface injection instead.
- `App1` depends on `Shared1` for types; no reverse dependency.
- `UnitTestApp1` depends on `Shared1`; may include test doubles for services.

## 7. Build Integration Notes
- Add `Shared1` project to solution before `App1` & `UnitTestApp1` references.
- Ensure project configurations (Debug/Release + x64/ARM64) mirror existing ones.
- Use same vcpkg manifest (root) so transitive packages available; if `spdlog` needed in `Shared1`, no extra manifest needed.
- Update solution dependency graph so `App1` builds after `Shared1`.

## 8. Testing Strategy Outline
- Pure model tests: standard `[TestMethod]` (no UI thread).
- ViewModel property change tests requiring dispatcher: use `[UITestMethod]` and instantiate ViewModel directly.
- Asynchronous operations: test continuation behavior & state after await.
- Logging & error pathways: inject a test sink or stub logger.
- Future: Consider integration tests using WinAppDriver / UIA for E2E (out of scope for PoC phase).

## 9. Risk & Mitigation
| Risk | Impact | Mitigation |
|------|--------|------------|
| WinRT projection constraints limit advanced C++ features in public types | Medium | Keep complex templates internal; expose simple ABI-friendly wrappers via IDL |
| Build time increase with new component | Low | Optimize PCH; minimize includes in IDL |
| Unit tests flakiness due to UI thread requirements | Medium | Segregate pure logic; only use `[UITestMethod]` when absolutely required |
| Future need for non-WinRT consumption (CLI tools) | Low | Factor pure logic into internal static sources not requiring WinRT runtime |

## 10. Initial Reference List
- WindowsAppSDK Samples (project patterns)
- WinUI 3 Testing guidance (library extraction for testability)
- MVVM patterns for C++/WinRT (community blogs & docs; generalized)

## 11. Pending Decisions / Open Questions
- Do we need a second internal static library for heavy algorithmic code now or later? (Proposed: defer.)
- Will `Shared1` expose any custom XAML controls? (Not for initial PoC.)
- Introduce service locator vs explicit dependency injection pattern? (Lean toward constructor injection; avoid global state.)

## 12. Next Steps (Awaiting Confirmation)
1. Confirm selection of Windows Runtime Component for `Shared1`.
2. Define minimal initial IDL (e.g., a sample `ICounterViewModel`).
3. Create `Shared1` project scaffolding (no implementations beyond baseline templates).
4. Add project references from `App1` and `UnitTestApp1`.
5. Move existing ViewModel & model related files from `App1` into `Shared1` (update namespaces & includes).
6. Adjust unit tests to reference new namespace.

---
(End of current session notes)
