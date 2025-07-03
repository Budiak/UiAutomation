#include <windows.h>
#include <uiautomation.h>
#include <iostream>

class NotificationEventHandler : public IUIAutomationNotificationEventHandler {
public:
  // IUnknown methods
  ULONG STDMETHODCALLTYPE AddRef() override {
    return InterlockedIncrement(&m_refCount);
  }

  ULONG STDMETHODCALLTYPE Release() override {
    ULONG ret = InterlockedDecrement(&m_refCount);
    if (ret == 0) {
      delete this;
    }
    return ret;
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface) override {
    if (riid == __uuidof(IUnknown) || riid == __uuidof(IUIAutomationNotificationEventHandler)) {
      *ppInterface = static_cast<IUIAutomationNotificationEventHandler*>(this);
    }
    else {
      *ppInterface = nullptr;
      return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
  }

  // IUIAutomationNotificationEventHandler method
  HRESULT STDMETHODCALLTYPE HandleNotificationEvent(IUIAutomationElement* pSender,
    NotificationKind notificationKind,
    NotificationProcessing notificationProcessing,
    BSTR displayString,
    BSTR activityId) override {
    // Handle the notification event
    wprintf(L"Notification received: %s\n", displayString);
    wprintf(L"Notification kind: %d\n", notificationKind);
    wprintf(L"Notification processing: %d\n", notificationProcessing);
    if (activityId != nullptr) {
      wprintf(L"Activity ID: %s\n", activityId);
    }
    return S_OK;
  }

private:
  LONG m_refCount = 1;
};

int notification2() {
  // Initialize COM
  CoInitialize(nullptr);

  // Create UI Automation object
  IUIAutomation6* g_pAutomation = nullptr;
  HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void**)&g_pAutomation);

  if (SUCCEEDED(hr)) {
    // Get the root element (e.g., the desktop)
    IUIAutomationElement* pRootElement = nullptr;
    hr = g_pAutomation->GetRootElement(&pRootElement);

    if (SUCCEEDED(hr)) {
      // Create the event handler group
      IUIAutomationEventHandlerGroup* pHandlerGroup = nullptr;
      hr = g_pAutomation->CreateEventHandlerGroup(&pHandlerGroup);

      if (SUCCEEDED(hr)) {
        // Create the notification event handler
        NotificationEventHandler* pNotificationHandler = new NotificationEventHandler();

        // Add the notification event handler to the group
        hr = pHandlerGroup->AddNotificationEventHandler(TreeScope_Subtree, nullptr, pNotificationHandler);

        if (SUCCEEDED(hr)) {
          // Add the event handler group to the automationID element (desktop)
          hr = g_pAutomation->AddEventHandlerGroup(pRootElement, pHandlerGroup);

          if (SUCCEEDED(hr)) {
            std::cout << "Notification event handler group registered successfully.\n";
          }
          else {
            std::cerr << "Failed to register event handler group.\n";
          }
        }
        else {
          std::cerr << "Failed to add notification event handler to group.\n";
        }

        // Release the handler group
        pHandlerGroup->Release();
      }
      else {
        std::cerr << "Failed to create event handler group.\n";
      }

      // Release the root element
      pRootElement->Release();
    }
    else {
      std::cerr << "Failed to get root element.\n";
    }

    // Release the UI Automation object
    g_pAutomation->Release();
  }

  // Uninitialize COM
  CoUninitialize();
  return 0;
}
