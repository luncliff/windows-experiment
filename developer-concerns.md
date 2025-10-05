# Developer Concerns / Design Scratchpad

Purpose: Evolving, lower-level design notes not yet finalized for `work-note.md`.
Scope: Shared1 (static lib now) & preparatory thoughts for future Shared2.

---
## 1. Logging Channel Strategy (Deep Dive)

Canonical channels (initial set):
- App1.Core – App lifecycle, configuration, startup/shutdown, session creation
- App1.ViewModel – State transitions, property changes (guard against spam)
- App1.DX – Device creation, swap chain events, frame latency warnings
- App1.Persistence – Settings load/save, serialization anomalies
- (Deferred) App1.Telemetry – User/usage metrics once policy approved

Open Questions:
1. Do we collapse Core + Persistence? (Depends on volume; evaluate after instrumentation)
2. Should DX move to a separate future graphics module before projection? (Likely with Shared2)
3. Introduce keyword bitmask usage vs event name categorization? (Prototype both.)

Decision Recording Rules:
- Only finalized decisions migrate to `work-note.md`.
- Experiments & rejected options stay here for historical trace.

## 2. Null Logger / Adapter Pattern

Goal: Zero-branch consumer code in Shared1 when optional `LoggingChannel` not provided.

Patterns Considered:
A. Interface Class (pure virtual) + two implementations
   Pros: Polymorphic substitution. Cons: vtable overhead, ABI noise if later projected.
B. Template Policy (e.g., `template<typename Logger>` with `Logger` providing `info/warn/error`)
   Pros: Zero runtime overhead. Cons: Template bloat, couples call sites to template generic forms.
C. Lightweight Value Adapter wrapping `LoggingChannel` (holds nullable handle; methods no-op if null)
   Pros: Single type, branch is localized inside adapter, simple to inline. Cons: Branch per call.
D. Null Object + Free Functions (global `inline` null instance; overloads accept adapter ref)
   Pros: Simple injection, free functions allow ADL flexibility. Cons: Slight scattering of API surface.

Preliminary Choice: Hybrid C + D
- Define `struct logging_adapter { Windows::Foundation::Diagnostics::LoggingChannel m_channel; /* ... */ };`
- Provide free functions `log_info(logging_adapter&, std::wstring_view)` etc.
- Null variant: default constructed `logging_adapter{}` with empty `m_channel` → functions early return.
- Rationale: Minimizes templates while centralizing null check logic.

Edge Cases:
- Channel closed mid-operation: Windows API will return failure; adapter can ignore error for non-critical info-level logs.
- High-frequency property changes: Add optional `should_log_debug()` guard or call-site sampling later.

## 3. Adapter API Sketch

```cpp
// In Shared1 pch.h (tentative)
namespace winrt::App1 {
  struct logging_adapter {
    Windows::Foundation::Diagnostics::LoggingChannel channel{}; // empty means disabled
    logging_adapter() = default;
    explicit logging_adapter(Windows::Foundation::Diagnostics::LoggingChannel c) noexcept : channel(std::move(c)) {}
  };

  void log_info(logging_adapter const& lg, std::wstring_view msg) noexcept;
  void log_warn(logging_adapter const& lg, std::wstring_view msg) noexcept;
  void log_error(logging_adapter const& lg, std::wstring_view msg) noexcept;
}
```

Implementation Notes:
- Use `LoggingLevel::Information / Warning / Error` (Critical reserved for unrecoverable conditions).
- Avoid allocation: pass `std::wstring_view` or narrow `std::string_view` then widen only if necessary (TBD based on call sites).
- Potential extension: overload with `std::u16string_view` if UTF-16 sources are common.

## 4. Future Shared2 Projection Thoughts

Projected Types Candidates:
- Stable ViewModel wrappers exposing bindable properties with change notifications.
- Graphics device service (if exposing DX interop to other languages is desired).
- Settings service with async load/save operations returning `IAsyncAction` / `IAsyncOperation<T>`.

WinRT Surface Guidelines:
- Keep method count minimal; group related state modifications into atomic operations where practical.
- Prefer `hstring` and collection interfaces for projection-friendly signatures.

Interop / COM Considerations:
- Legacy COM wrappers should remain internal to Shared2 implementation namespace until stable.
- Evaluate need for `apartment_context` utilities if providing async cross-thread projections.

## 5. DX Namespace Transition Plan

Current State:
- `DX` utilities validate build flags & device creation in PoC.

Transition Triggers:
- Need for multi-project reuse.
- Need for test harness to simulate device loss paths.
- Introduction of advanced diagnostics (PIX markers, GPU counters) requiring structured injection.

Migration Steps Outline:
1. Document existing responsibilities (device creation, swap chain binding) – TODO
2. Introduce an interface-like abstract boundary in Shared1 (optional) – maybe skip until projection.
3. Move implementation to Shared2 with WinRT-friendly factory functions.

## 6. Task Backlog (Dev-Facing)

Short-Term (Shared1):
- Finalize logging adapter minimal surface.
- Implement adapter + null behavior.
- Extract `SettingsState` (logic) from `SettingsViewModel`.
- Provide unit tests for change notification using adapter stub.

Mid-Term (Pre-Shared2):
- Introduce DX helper consolidation design doc.
- Evaluate logging volume; decide if channel split adjustments needed.
- Add performance counters (timing) guarded by adapter feature flag.

Long-Term (Shared2):
- WinRT projection of stable surfaces.
- COM interop wrappers.
- Telemetry channel implementation (if approved).
- DX device service projection.

## 7. Open Questions (Keep Updated)
- Do we require structured logging (fields/keywords) or is formatted string sufficient for PoC? (Leaning: formatted only now.)
- Add `LoggingFields` usage for key-value pairs? (Defer until a use case emerges.)
- Introduce async-safe logging queue if high-frequency DX events appear? (Monitor first.)

---
(End of developer-concerns scratchpad)
