# Developer Concerns / Design Scratchpad

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

## 2. ILoggingChannel Implementations & Null Object

Goal: Consume `Windows::Foundation::Diagnostics::ILoggingChannel` directly so Shared1 code remains agnostic about whether the channel is the system `LoggingChannel` or a custom in-process implementation (null, stream redirection, test instrumentation).

Why pivot from adapter concept? The WinRT API already defines an interface (`ILoggingChannel`). Implementing small in-process types deriving from `winrt::implements<..., ILoggingChannel>` avoids inventing an additional abstraction while enabling polymorphism and a canonical null object.

Planned Implementations (non-projected, internal C++ classes):
| Type | Purpose | Notes |
|------|---------|-------|
| SystemLoggingChannel (alias) | Wrap existing `LoggingChannel` | Provided by platform; implicitly convertible to `ILoggingChannel` |
| NullLoggingChannel | Null object – no-op `LogMessage` | All other interface methods return defaults / benign values |
| StreamLoggingChannel | Redirects `LogMessage(hstring)` to `std::wostream` (e.g. `std::wcout`) | For tests & local diagnostics without session |
| FilteringLoggingChannel (future) | Conditional forwarding based on severity/keywords | Defer until need arises |

Key Interface Method (primary usage):
`void ILoggingChannel::LogMessage(hstring const& message) const;`
Reference: https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.iloggingchannel.logmessage

Usage Pattern in Shared1:
```cpp
void some_operation(Windows::Foundation::Diagnostics::ILoggingChannel const& ch) {
   ch.LogMessage(L"operation started");
   // ... work ...
   ch.LogMessage(L"operation finished");
}
```

Construction / Injection:
- App code supplies either a real `LoggingChannel` or a custom implementation (e.g., `StreamLoggingChannel`) wherever logic objects are created.
- For absence of logging, pass a shared static `NullLoggingChannel` instance.

Sketch: Null + Stream Implementations
```cpp
struct NullLoggingChannel final : winrt::implements<NullLoggingChannel, Windows::Foundation::Diagnostics::ILoggingChannel> {
   void LogMessage(winrt::hstring const&) const noexcept {}
   void LogMessage(winrt::hstring const&, Windows::Foundation::Diagnostics::LoggingLevel) const noexcept {}
   // Other interface members (if any) return default/benign values.
};

struct StreamLoggingChannel final : winrt::implements<StreamLoggingChannel, Windows::Foundation::Diagnostics::ILoggingChannel> {
   std::wostream* out{};
   explicit StreamLoggingChannel(std::wostream& os) noexcept : out(&os) {}
   void LogMessage(winrt::hstring const& msg) const noexcept {
      if (out) (*out) << msg.c_str() << L'\n';
   }
   void LogMessage(winrt::hstring const& msg, Windows::Foundation::Diagnostics::LoggingLevel) const noexcept {
      LogMessage(msg);
   }
};
```

Advantages:
- Removes need for custom adapter/wrapper naming.
- Naturally composes with any future WinRT projections (`Shared2`) since interface already part of metadata.
- Null object fully type-compatible (no `std::optional` or pointer null checks).

Edge Cases:
- Real `LoggingChannel` may throw if underlying session closed; Shared1 will treat logging as best-effort (catch & swallow non-critical exceptions if they appear—TBD after instrumentation).
- Performance: Additional virtual indirection per call is acceptable at current expected log frequency; revisit if hotspots emerge (possible inline fast path for `LoggingChannel`).

Testing Approach:
- Unit tests can inject `StreamLoggingChannel` capturing output into `std::wstringstream` and assert on contents, or `NullLoggingChannel` to ensure no side-effects.

## 3. ILoggingChannel Consumption Guidelines

1. Accept parameters as `ILoggingChannel const&` (never raw pointer) to enforce non-null semantic (Null object supplies behavior). 
2. Store as `Windows::Foundation::Diagnostics::ILoggingChannel` (value) inside objects; it's a projected interface smart pointer (reference-counted) and cheap to copy.
3. Do not cache `LoggingChannel` separately; rely solely on the interface type.
4. Severity filtering (future) handled by a decorated implementation; core code only calls `LogMessage` (non-severity overload) unless level adds value.
5. Avoid formatting allocations: prefer composing `std::wstring` using small buffers or streaming when necessary; micro-opt premature—defer.

Deferred Considerations:
- Introduce `LoggingFields` variants only when structured key-value data appears.
- Add severity-based overload usages after first set of warnings/errors are defined.

References:
- ILoggingChannel: https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.iloggingchannel
- ILoggingChannel::LogMessage: https://learn.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.iloggingchannel.logmessage

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

---
(End of developer-concerns scratchpad)
