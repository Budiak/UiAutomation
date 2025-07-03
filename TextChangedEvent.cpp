#include <windows.h>
#include <iostream>
#include <UIAutomation.h>

// Custom TextEditTextChanged event handler
class TextEditTextChangedHandler : public IUIAutomationTextEditTextChangedEventHandler {
public:
  // Constructor
  TextEditTextChangedHandler() : refCount(1) {}

  // IUnknown methods
  STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) {
    if (riid == IID_IUnknown || riid == IID_IUIAutomationTextEditTextChangedEventHandler) {
      *ppvObject = static_cast<IUIAutomationTextEditTextChangedEventHandler*>(this);
      AddRef();
      return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
  }

  STDMETHODIMP_(ULONG) AddRef() {
    return InterlockedIncrement(&refCount);
  }

  STDMETHODIMP_(ULONG) Release() {
    ULONG newRefCount = InterlockedDecrement(&refCount);
    if (newRefCount == 0) {
      delete this;
    }
    return newRefCount;
  }

  // IUIAutomationTextEditTextChangedEventHandler method
  STDMETHODIMP HandleTextEditTextChangedEvent(
    IUIAutomationElement* sender,
    TextEditChangeType changeType,
    SAFEARRAY* changedData)
  {
    // Handling the event
    std::wcout << L"TextEditTextChanged event received!" << std::endl;

    // Process changeType and changedData here
    if (changeType == TextEditChangeType_Composition) {
      std::wcout << L"Change type: Composition" << std::endl;
    }
    else if (changeType == TextEditChangeType_AutoCorrect) {
      std::wcout << L"Change type: AutoCorrect" << std::endl;
    }
    else {
      std::wcout << L"Change type: Unknown" << std::endl;
    }

    if (changedData != nullptr) {
      long lowerBound, upperBound;
      SafeArrayGetLBound(changedData, 1, &lowerBound);
      SafeArrayGetUBound(changedData, 1, &upperBound);

      BSTR* dataArray;
      SafeArrayAccessData(changedData, (void**)&dataArray);

      for (long i = lowerBound; i <= upperBound; i++) {
        std::wcout << L"Changed data: " << dataArray[i] << std::endl;
      }

      SafeArrayUnaccessData(changedData);
    }

    return S_OK;
  }

private:
  LONG refCount;
};

int TestChangedEvent() {
  CoInitialize(NULL);
  IUIAutomation6* automation = nullptr;
  IUIAutomationElement* rootElement = nullptr;

  // Initialize UI Automation
  HRESULT hr = CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&automation);
  if (FAILED(hr) || !automation) {
    std::cerr << "Failed to initialize UI Automation." << std::endl;
    return 1;
  }

  // Get the root element (desktop in this case)
  hr = automation->GetRootElement(&rootElement);
  if (FAILED(hr) || !rootElement) {
    std::cerr << "Failed to get root element." << std::endl;
    automation->Release();
    return 1;
  }

  // Create event handler instance
  TextEditTextChangedHandler* textEditEventHandler = new TextEditTextChangedHandler();

  // Register the event handler for text edit changed events (for all elements)
  hr = automation->AddTextEditTextChangedEventHandler(
    rootElement,
    TreeScope_Subtree,
    TextEditChangeType_AutoCorrect,
     nullptr,
   textEditEventHandler
  );

  if (SUCCEEDED(hr)) {
    std::cout << "TextEditTextChanged event handler registered successfully." << std::endl;
  }
  else {
    std::cerr << "Failed to register TextEditTextChanged event handler." << std::endl;
  }

  // Event loop to keep the program running and listening for events
  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Cleanup
  if (textEditEventHandler) {
    textEditEventHandler->Release();
  }
  if (rootElement) {
    rootElement->Release();
  }
  if (automation) {
    automation->Release();
  }

  CoUninitialize();
  return 0;
}
