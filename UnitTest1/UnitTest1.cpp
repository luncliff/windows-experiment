#include <CppUnitTest.h>

// https://learn.microsoft.com/en-us/visualstudio/test/microsoft-visualstudio-testtools-cppunittestframework-api-reference?view=vs-2022
#include <functional>

#include "DeviceResources.h"

using Microsoft::VisualStudio::CppUnitTestFramework::Assert;
using Microsoft::VisualStudio::CppUnitTestFramework::TestClass;

class DeviceResourcesTest : public TestClass<DeviceResourcesTest> {
    DX::DeviceResources res{};

    TEST_METHOD_INITIALIZE(Initialize) {
        res.CreateDeviceResources();
        res.WaitForGpu();
    }
    TEST_METHOD_CLEANUP(Cleanup) {
        res.WaitForGpu();
    }

    TEST_METHOD(TestInvalidHWND) {
        HWND windows = nullptr;
        res.SetWindow(windows, 0, 0);
        Assert::ExpectException<std::exception>( // invalid argument
            std::bind_front(&DX::DeviceResources::CreateWindowSizeDependentResources, &res));
    }
};
