#include <Windows.h>
#include <iostream>
#include <atlbase.h>
#include <UIAutomation.h>

// Global variables
HWINEVENTHOOK g_eventHook;
CComPtr<IUIAutomation> g_uiAutomation;

// Callback for handling WinEvent
void CALLBACK WinEventProc(
  HWINEVENTHOOK hWinEventHook,
  DWORD event,
  HWND hwnd,
  LONG idObject,
  LONG idChild,
  DWORD idEventThread,
  DWORD dwmsEventTime)
{
  if (!g_uiAutomation || hwnd == nullptr) {
    return;
  }

  CComPtr<IUIAutomationElement> pElement;
  VARIANT varChild;
  varChild.vt = VT_I4;
  varChild.lVal = idChild;

  // Get the UI Automation element
  HRESULT hr = g_uiAutomation->ElementFromHandle(hwnd, &pElement);
  if (FAILED(hr) || !pElement) {
    std::wcout << L"Failed to retrieve UI Automation element.\n";
    return;
  }

  // Retrieve element properties
  BSTR name, value, controlType;
  if (SUCCEEDED(pElement->get_CurrentName(&name))) {
    if (name != nullptr)
    {
      std::wcout << L"Name: " << name << std::endl;
      SysFreeString(name);
    }
  }
  VARIANT temp;
  if (SUCCEEDED(pElement->GetCurrentPropertyValue(EVENT_OBJECT_VALUECHANGE , &temp))) {
    value = temp.bstrVal;
    std::wcout << L"Value: " << value << std::endl;
    SysFreeString(value);
  }

  if (SUCCEEDED(pElement->get_CurrentLocalizedControlType(&controlType))) {
    std::wcout << L"Control Type: " << controlType << std::endl;
    SysFreeString(controlType);
  }

  // Log additional details if needed
  std::wcout << L"Event: EVENT_OBJECT_VALUECHANGE\n";
  std::wcout << L"Window Handle: " << hwnd << L"\n";
  std::wcout << L"ID Object: " << idObject << L"\n";
  std::wcout << L"ID Child: " << idChild << L"\n\n";
}

int EVENT_OBJECT_VALUECHANGE_()
{
  // Initialize COM and UI Automation
  CoInitialize(NULL);
  g_uiAutomation.CoCreateInstance(CLSID_CUIAutomation);

  // Set up WinEvent hook for EVENT_OBJECT_VALUECHANGE
  g_eventHook = SetWinEventHook(
    EVENT_OBJECT_VALUECHANGE, // Min event
    EVENT_OBJECT_VALUECHANGE, // Max event
    NULL,                     // Module handle
    WinEventProc,             // Callback
    0,                        // Process ID (0 = all processes)
    0,                        // Thread ID (0 = all threads)
    WINEVENT_OUTOFCONTEXT     // Flags
  );

  if (!g_eventHook) {
    std::cerr << "Failed to set event hook.\n";
    return 1;
  }

  std::cout << "Listening for EVENT_OBJECT_VALUECHANGE...\n";

  // Enter message loop
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Clean up
  UnhookWinEvent(g_eventHook);
  g_uiAutomation.Release();
  CoUninitialize();

  return 0;
}
