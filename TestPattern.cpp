#include <windows.h>
#include <oleacc.h>
#include <iostream>
#include <UIAutomation.h>

int TextPattern() {
  // Initialize COM
  CoInitialize(NULL);

  // Initialize the UI Automation interface
  IUIAutomation* g_pAutomation = nullptr;
  HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&g_pAutomation);
  if (FAILED(hr)) {
    std::cerr << "Failed to initialize UI Automation." << std::endl;
    return 1;
  }

  // Get the root element (desktop)
  IUIAutomationElement* pRootElement = nullptr;
  hr = g_pAutomation->GetRootElement(&pRootElement);
  if (FAILED(hr) || pRootElement == nullptr) {
    std::cerr << "Failed to get the root element." << std::endl;
    g_pAutomation->Release();
    return 1;
  }

  // Find the target UI element (e.g., Notepad window)
  IUIAutomationElement* pTargetElement = nullptr;
  hr = pRootElement->FindFirst(TreeScope_Children, nullptr, &pTargetElement);
  if (FAILED(hr) || pTargetElement == nullptr) {
    std::cerr << "Failed to find target element." << std::endl;
    pRootElement->Release();
    g_pAutomation->Release();
    return 1;
  }

  // Get the TextPattern from the element
  IUIAutomationTextPattern* pTextPattern = nullptr;
  hr = pTargetElement->GetCurrentPatternAs(UIA_TextPatternId, IID_IUIAutomationTextPattern, (void**)&pTextPattern);
  if (FAILED(hr) || pTextPattern == nullptr) {
    std::cerr << "TextPattern not supported on this element." << std::endl;
    pTargetElement->Release();
    pRootElement->Release();
    g_pAutomation->Release();
    return 1;
  }

  // Get the document range (entire text)
  IUIAutomationTextRange* pTextRange = nullptr;
  hr = pTextPattern->get_DocumentRange(&pTextRange);
  if (FAILED(hr) || pTextRange == nullptr) {
    std::cerr << "Failed to get the text range." << std::endl;
    pTextPattern->Release();
    pTargetElement->Release();
    pRootElement->Release();
    g_pAutomation->Release();
    return 1;
  }

  // Extract the text from the range
  BSTR text;
  hr = pTextRange->GetText(-1, &text);  // -1 to get all text
  if (SUCCEEDED(hr)) {
    std::wcout << L"Text: " << text << std::endl;
    SysFreeString(text);
  }
  else {
    std::cerr << "Failed to extract text." << std::endl;
  }

  // Clean up
  pTextRange->Release();
  pTextPattern->Release();
  pTargetElement->Release();
  pRootElement->Release();
  g_pAutomation->Release();
  CoUninitialize();

  return 0;
}
