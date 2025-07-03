#include <windows.h>
#include <iostream>
#include <UIAutomation.h>
#include "Header.h"

// Global variable to store the hook handle
HHOOK hKeyboardHook = NULL;
HHOOK hMouseHook = NULL;

// Hook procedure to capture keyboard events
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
      std::cout << "Key Pressed: " << pKeyboard->vkCode << std::endl;
    }
    else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
      std::cout << "Key Released: " << pKeyboard->vkCode << std::endl;
    }
    TakeScreenshot();
  }

  // Call next hook in the chain
  return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    MSLLHOOKSTRUCT* pMouse = (MSLLHOOKSTRUCT*)lParam;
    switch (wParam)
    {
    case WM_LBUTTONDOWN:
      std::cout << "WM_LBUTTONDOWN";
      break;
    case WM_LBUTTONUP:
      std::cout << "WM_LBUTTONUP";
      break;
    case WM_MOUSEMOVE:
      std::cout << "WM_MOUSEMOVE";
      break;
    case WM_MOUSEWHEEL:
      std::cout << "WM_MOUSEWHEEL";
      break;
    case WM_RBUTTONDOWN:
      std::cout << "WM_RBUTTONDOWN";
      break;
    case WM_RBUTTONUP:
      std::cout << "WM_RBUTTONUP";
      break;
    case WM_MBUTTONDOWN:
      std::cout << "WM_MBUTTONDOWN";
      break;
    case WM_MBUTTONUP:
      std::cout << "WM_MBUTTONUP";
      break;
    default:
      std::cout << "Unknown type:" << wParam << std::endl;
      break;
    }
    std::cout << "Mouse pos :" << pMouse->pt.x << " : " << pMouse->pt.y << std::endl;
    //TakeScreenshot();
  }
  else
  {
    std::cout << "Unknown type:" << nCode << std::endl;
  }

  // Call next hook in the chain
  return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}
// Function to install the hook
void SetHook() {
  hKeyboardHook = SetWindowsHookExA(
    WH_KEYBOARD_LL,          // Set low-level keyboard hook
    LowLevelKeyboardProc,     // Hook procedure
    GetModuleHandle(NULL),    // Handle to the module
    0                         // 0 for global hook
  );

  if (hKeyboardHook == NULL) {
    std::cerr << "Failed to install hook!" << std::endl;
  }
  else {
    std::cout << "Hook installed!" << std::endl;
  }
}
void SetHook2() {
  hMouseHook = SetWindowsHookExA(
    WH_MOUSE_LL,          // Set low-level keyboard hook
    LowLevelMouseProc,     // Hook procedure
    GetModuleHandle(NULL),    // Handle to the module
    0                         // 0 for global hook
  );

  if (hMouseHook == NULL) {
    std::cerr << "Failed to install hook!" << std::endl;
  }
  else {
    std::cout << "Hook installed!" << std::endl;
  }
}
// Function to remove the hook
void Unhook() {
  if (hKeyboardHook != NULL) {
    UnhookWindowsHookEx(hKeyboardHook);
    std::cout << "Hook removed!" << std::endl;
  }
  if (hMouseHook != NULL) {
    UnhookWindowsHookEx(hMouseHook);
    std::cout << "Hook removed!" << std::endl;
  }
}

// Message loop to keep the hook runningss 
void MessageLoop() {
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

int GlobalHookKeyStrokes() {
  // Install the hook
  SetHook();
  //SetHook2();
  // Enter message loop to keep the hook alive
  MessageLoop();

  // Uninstall the hook before exiting
  Unhook();

  return 0;
}
