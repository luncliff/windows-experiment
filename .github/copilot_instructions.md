

Use the [readme.md](../readme.md) for build/test instructions.

## Coding Convention

Global functions will use snake_case. See [pch.h](../App1/pch.h) and [pch.cpp](../App1/pch.cpp).

We use PascalCase for class names. However we mix PascalCase and snake_case for member functions.
For example,

- [SettingsPage](../App1/SettingsPage.xaml.h) class overrides XAML Control virtual functions like [Page.OnNavigatedFrom](https://learn.microsoft.com/en-us/uwp/api/windows.ui.xaml.controls.page.onnavigatedfrom) and [Page.OnNavigatedTo](https://learn.microsoft.com/en-us/uwp/api/windows.ui.xaml.controls.page.onnavigatedto). For those existing functions in XAML, it will use PascalCase.
- [MainWindow.xaml](../App1/MainWindow.xaml) defines custom event handlers in XAML file. To distinguish the handlers between XAML Control member functions, we will use snake_case. See [MainWindow.xaml.h](../App1/MainWindow.xaml.h)

## Tools

### MSVC

When more details are required for compiler options, see the following manuals.
Expect MSVC 170 or later. (Add `?view=msvc-170` after the following URLs)

- [Compiler options listed by category](https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category#header-unitsmodules)
- [Compiler options listed alphabetically](https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically)
