//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <uiautomation.h>
#include <iostream>
#include <vector>
#include <string>
#include <comdef.h> // For _bstr_t and _com_error

std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen);
using namespace std;
#include <ostream>
#include <fstream>

#if 1
// Global variables
HHOOK g_hMouseHook = NULL;
IUIAutomation* g_pAutomation = NULL;
IUIAutomationTreeWalker* g_pControlWalker = NULL;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc_2(int nCode, WPARAM wParam, LPARAM lParam);
void ProcessClick(POINT pt);
void PrintElementAndAncestors(IUIAutomationElement* pClickedElement);
std::wstring GetElementDescription(IUIAutomationElement* pElement);

// Helper to Safely Release COM Objects
template<class T> void SafeRelease(T** ppT) {
  if (*ppT) {
    (*ppT)->Release();
    *ppT = NULL;
  }
}

int mainGemini() {
  // Get the instance handle manually
  HINSTANCE hInstance = GetModuleHandle(NULL);
  // nCmdShow is not available directly, use SW_SHOWDEFAULT or SW_HIDE if needed,
  // but it's less relevant for a background hook/console app.
  // int nCmdShow = SW_SHOWDEFAULT; // Or SW_HIDE

  // --- The rest of your WinMain code stays largely the same ---

  // 1. Initialize COM
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if FAILED(hr) { /* ... error handling ... */ return 1; }

  // 2. Initialize UI Automation
  hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER,
    __uuidof(IUIAutomation), (void**)&g_pAutomation);
  if FAILED(hr) { /* ... error handling ... */ CoUninitialize(); return 1; }

  // 3. Get the Control View Walker
  hr = g_pAutomation->get_ControlViewWalker(&g_pControlWalker);
  if FAILED(hr) { /* ... error handling ... */ SafeRelease(&g_pAutomation); CoUninitialize(); return 1; }

  // 4. Register the window class (still needs hInstance)
  const wchar_t CLASS_NAME[] = L"UIA Ancestor Finder Class";
  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance; // Use the hInstance we got from GetModuleHandle
  wc.lpszClassName = CLASS_NAME;
  RegisterClass(&wc);

  // 5. Create the window (still needs hInstance)
  HWND hwnd = CreateWindowEx(
    0, CLASS_NAME, L"UIA Ancestor Finder", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, 200, 100,
    NULL, NULL, hInstance, NULL // Use the hInstance we got
  );
  // HWND hwnd = CreateWindowEx(0, CLASS_NAME, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL); // For message-only

  if (hwnd == NULL) { /* ... error handling ... */ SafeRelease(&g_pControlWalker); SafeRelease(&g_pAutomation); CoUninitialize(); return 1; }

  // ShowWindow(hwnd, SW_HIDE); // Example: Explicitly hide if desired

  // 6. Set the low-level mouse hook (still needs hInstance)
  g_hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc_2, hInstance, 0); // Use the hInstance we got
  if (g_hMouseHook == NULL) { /* ... error handling ... */ DestroyWindow(hwnd); SafeRelease(&g_pControlWalker); SafeRelease(&g_pAutomation); CoUninitialize(); return 1; }

  std::wcout << L"UIA Ancestor Finder running..." << std::endl; // This will now print to the console

  // 7. Message loop (Revised - Debugger friendly)
  MSG msg = {};
  bool bQuit = false;
  while (!bQuit)
  {
    // Process all pending messages without blocking
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      // Check for WM_QUIT specifically, as PeekMessage doesn't filter it
      // like GetMessage's > 0 condition does implicitly.
      if (msg.message == WM_QUIT)
      {
        bQuit = true; // Set flag to exit outer loop
        break;        // Exit inner PeekMessage loop
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } // End of inner PeekMessage loop

    // If we received WM_QUIT, exit the outer loop immediately
    if (bQuit)
    {
      break;
    }

    // If there were no messages, wait efficiently for the next one.
    // This yields CPU time like GetMessage, but the loop structure
    // allows the debugger to interrupt more easily *between* calls.
    WaitMessage();
  } // End of outer loop

  // The final message's wParam usually contains the exit code from PostQuitMessage
  int exitCode = (int)msg.wParam;

  // 8. Cleanup
  std::wcout << L"Exiting..." << std::endl;
  UnhookWindowsHookEx(g_hMouseHook);
  if (hwnd) // Only destroy if creation succeeded
  {
    DestroyWindow(hwnd);
  }
  SafeRelease(&g_pControlWalker);
  SafeRelease(&g_pAutomation);
  CoUninitialize();

  return exitCode; // Return the exit code
}


// Window Procedure (minimal, mainly for cleanup)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_DESTROY:
    PostQuitMessage(0); // Signal end of message loop
    return 0;
  case WM_CREATE:
    // Optional: Redirect cout/wcout to Debug Output if no console visible
    // AllocConsole(); // If you want a console window to pop up
    // freopen_s(...); // Redirect stdio if using AllocConsole
    break;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Low-Level Mouse Hook Procedure
LRESULT CALLBACK LowLevelMouseProc_2(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    // We are interested in Left Button Down events
    if (wParam == WM_LBUTTONDOWN) {
      MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;
      if (pMouseStruct != NULL) {
        // Process the click asynchronously or synchronously
        // For simplicity, doing it synchronously here.
        // Beware: Long operations in a hook can slow down the system.
        // Consider posting a message to your window thread for complex work.
        std::wcout << L"\n--- Left Click Detected at (" << pMouseStruct->pt.x << L", " << pMouseStruct->pt.y << L") ---" << std::endl;
        ProcessClick(pMouseStruct->pt);
      }
    }
  }
  // Pass the hook information to the next hook procedure in chain
  return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}

// Process the click using UI Automation
void ProcessClick(POINT pt) {
  if (!g_pAutomation) return;

  IUIAutomationElement* pElementUnderCursor = NULL;
  HRESULT hr = g_pAutomation->ElementFromPoint(pt, &pElementUnderCursor);

  if (SUCCEEDED(hr) && pElementUnderCursor != NULL) {
    PrintElementAndAncestors(pElementUnderCursor);
    pElementUnderCursor->Release(); // Release the element obtained from ElementFromPoint
  }
  else {
    if (hr == UIA_E_ELEMENTNOTAVAILABLE) {
      std::wcout << L"No UI element found at the specified point." << std::endl;
    }
    else {
      _com_error err(hr);
      std::wcout << L"Error getting element from point: " << err.ErrorMessage() << L" (Code: " << hr << L")" << std::endl;
    }
  }
  std::wcout << L"-----------------------------------------" << std::endl;
}

static std::string ConvertBSTRToMBS(BSTR bstr)
{
  int wslen = ::SysStringLen(bstr);
  return ConvertWCSToMBS((wchar_t*)bstr, wslen);
}

// Get relevant properties of an element as a string
std::wstring GetElementDescription(IUIAutomationElement* pElement) {
  if (!pElement) return L"[Invalid Element]";

  std::wstring description = L"";
  HRESULT hr;

  // Name
  BSTR bstrName = NULL;
  //hr = pElement->get_CurrentName(&bstrName);
  //if (FAILED(hr)) return NULL;
  //std::string name = ConvertBSTRToMBS(bstrName);
  //std::cout << name << std::endl;
  //SysFreeString(bstrName);


  hr = pElement->get_CurrentName(&bstrName);
  if (SUCCEEDED(hr) && bstrName != NULL && SysStringLen(bstrName) > 0) {
    std::string name = ConvertBSTRToMBS(bstrName);
    std::cout << name << std::endl;
    SysFreeString(bstrName);
    //description += L"Name: \"" + std::wstring(bstrName) + L"\"";
    //SysFreeString(bstrName);
  }
  else {
    description += L"Name: [None]";
  }

  // Control Type
  CONTROLTYPEID controlTypeId;
  hr = pElement->get_CurrentControlType(&controlTypeId);
  if (SUCCEEDED(hr)) {
    description += L", TypeId: " + std::to_wstring(controlTypeId);

    // --- Removed the problematic call to GetLocalizedControlType ---
    // If you need localized names, you would typically implement a
    // manual mapping here using a switch or map based on common
    // UIA_...ControlTypeId constants defined in uiautomationclient.h
    /* Example manual mapping snippet:
    std::wstring controlTypeName = L"Unknown";
    switch(controlTypeId) {
        case UIA_ButtonControlTypeId: controlTypeName = L"Button"; break;
        case UIA_EditControlTypeId: controlTypeName = L"Edit"; break;
        case UIA_TextControlTypeId: controlTypeName = L"Text"; break;
        // ... add more mappings as needed ...
    }
    description += L" (" + controlTypeName + L")";
    */

  }
  else {
    description += L", TypeId: [Error]";
  }

  // Automation ID
  BSTR bstrAutomationId = NULL;
  hr = pElement->get_CurrentAutomationId(&bstrAutomationId);
  if (SUCCEEDED(hr) && bstrAutomationId != NULL && SysStringLen(bstrAutomationId) > 0) {
    description += L", AutomationId: \"" + std::wstring(bstrAutomationId) + L"\"";
    SysFreeString(bstrAutomationId);
  }
  else {
    // description += L", AutomationId: [None]"; // Often None, can omit
  }

  // Class Name (Win32 specific)
  BSTR bstrClassName = NULL;
  hr = pElement->get_CurrentClassName(&bstrClassName);
  if (SUCCEEDED(hr) && bstrClassName != NULL && SysStringLen(bstrClassName) > 0) {
    description += L", ClassName: \"" + std::wstring(bstrClassName) + L"\"";
    SysFreeString(bstrClassName);
  }
  else {
    // description += L", ClassName: [None]"; // Often None for non-HWND elements
  }

  return description;
}

// Print the clicked element and its ancestors up to the desktop
void PrintElementAndAncestors(IUIAutomationElement* pClickedElement) {
  if (!pClickedElement || !g_pControlWalker) return;

  std::wcout << L"Clicked Element: " << GetElementDescription(pClickedElement) << std::endl;
  std::wcout << L"Ancestors:" << std::endl;

  IUIAutomationElement* pCurrent = pClickedElement;
  pCurrent->AddRef(); // AddRef because we will release it inside the loop

  int level = 1;
  while (true) {
    IUIAutomationElement* pParent = NULL;
    HRESULT hr = g_pControlWalker->GetParentElement(pCurrent, &pParent);

    SafeRelease(&pCurrent); // Release the previous element (or the initial clicked one)

    if (FAILED(hr) || pParent == NULL) {
      if (hr == E_INVALIDARG) {
        std::wcout << L"  [" << level << L"] Error: GetParentElement received invalid argument (possibly desktop reached or element gone)." << std::endl;
      }
      else if FAILED(hr) {
        _com_error err(hr);
        std::wcout << L"  [" << level << L"] Error getting parent: " << err.ErrorMessage() << L" (Code: " << hr << L")" << std::endl;
      }
      else {
        std::wcout << L"  (Reached top of hierarchy or error)" << std::endl;
      }
      // pParent is already NULL or invalid, break loop
      break;
    }

    // Check if parent is the Desktop (optional check, loop condition handles it)
    IUIAutomationElement* pDesktopElement = NULL;
    g_pAutomation->GetRootElement(&pDesktopElement); // Get Desktop element
    BOOL areSame = FALSE;
    if (pDesktopElement) {
      g_pAutomation->CompareElements(pParent, pDesktopElement, &areSame);
      SafeRelease(&pDesktopElement);
    }


    std::wcout << L"  [" << level++ << L"] " << GetElementDescription(pParent);
    if (areSame) {
      std::wcout << L" (Desktop)";
    }
    std::wcout << std::endl;


    if (areSame) {
      SafeRelease(&pParent); // Release the desktop parent and stop
      break;
    }

    // Prepare for the next iteration
    pCurrent = pParent; // pParent becomes pCurrent (ownership transferred)
    // No AddRef needed here as we got it from GetParentElement
  }

  // Make sure pCurrent is released if the loop exited unexpectedly
  SafeRelease(&pCurrent);
}

#endif