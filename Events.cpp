#include <Windows.h>
#include <UIAutomation.h>
#include <iostream>

#include <windows.h>
#include <stdio.h>
#include <UIAutomation.h>
#include <comutil.h>
#include "Header.h"

class EventHandler : public IUIAutomationEventHandler
{
private:
  LONG _refCount;

public:
  int _eventCount;

  // Constructor.
  EventHandler() : _refCount(1), _eventCount(0)
  {
  }

  // IUnknown methods.
  ULONG STDMETHODCALLTYPE AddRef()
  {
    ULONG ret = InterlockedIncrement(&_refCount);
    return ret;
  }

  ULONG STDMETHODCALLTYPE Release()
  {
    ULONG ret = InterlockedDecrement(&_refCount);
    if (ret == 0)
    {
      delete this;
      return 0;
    }
    return ret;
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface)
  {
    if (riid == __uuidof(IUnknown))
      *ppInterface = static_cast<IUIAutomationEventHandler*>(this);
    else if (riid == __uuidof(IUIAutomationEventHandler))
      *ppInterface = static_cast<IUIAutomationEventHandler*>(this);
    else
    {
      *ppInterface = NULL;
      return E_NOINTERFACE;
    }
    this->AddRef();
    return S_OK;
  }

  IUIAutomationElement* GetContainingWindow(IUIAutomationElement* pChild)
  {

    if (pChild == NULL)
      return NULL;

    IUIAutomationElement* pDesktop = NULL;
    HRESULT hr = _automation->GetRootElement(&pDesktop);
    if (FAILED(hr))
      return NULL;

    BOOL same;
    _automation->CompareElements(pChild, pDesktop, &same);
    if (same)
    {
      pDesktop->Release();
      return NULL; // No parent, so return NULL.
    }

    IUIAutomationElement* pParent = NULL;
    IUIAutomationElement* pNode = pChild;

    // Create the treewalker.
    IUIAutomationTreeWalker* pWalker = NULL;
    _automation->get_ControlViewWalker(&pWalker);
    if (pWalker == NULL)
      goto cleanup;

    // Walk up the tree.
    while (TRUE)
    {
      hr = pWalker->GetParentElement(pNode, &pParent);
      if (FAILED(hr) || pParent == NULL)
      {
        break;
      }
      _automation->CompareElements(pParent, pDesktop, &same);
      if (same)
      {
        pDesktop->Release();
        pParent->Release();
        pWalker->Release();
        // Reached desktop, so return next element below it.
        return pNode;
      }
      if (pNode != pChild) // Do not release the in-param.
        pNode->Release();
      pNode = pParent;
    }

  cleanup:
    if ((pNode != NULL) && (pNode != pChild))
      pNode->Release();

    if (pDesktop != NULL)
      pDesktop->Release();

    if (pWalker != NULL)
      pWalker->Release();

    if (pParent != NULL)
      pParent->Release();

    return NULL;
  }


  // IUIAutomationEventHandler methods
  HRESULT STDMETHODCALLTYPE HandleAutomationEvent(IUIAutomationElement* pSender, EVENTID eventID)
  {
    static int yyy = 0;
    if (eventID == UIA_Invoke_InvokedEventId)
    {
      GetContainingWindow(pSender);
      yyy++;
    }
    BSTR name;
    HRESULT hr = pSender->get_CurrentName(&name);
    //IUIAutomationElement* parent = NULL;
    //HRESULT hr2 = pSender->GetCachedParent(&parent);
    //if (SUCCEEDED(hr2) && parent)
    //{
    //  BSTR name2;
    //  hr2 = parent->get_CachedName(&name2);
    //  if (SUCCEEDED(hr2))
    //  {
    //    std::string stdstr(_bstr_t(name2, true));
    //    stdstr.shrink_to_fit();
    //    //std::wcout << L"Parent element " << i + 1 << L" - Cached Name: ";
    //    wprintf(name2); std::wcout << std::endl;
    //  }
    //  SysFreeString(name2);
    //}
    //else
    //{
    //  hr2 = pSender->GetParentElement(&parent);
    //  if (SUCCEEDED(hr2) && parent)
    //  {
    //    BSTR name2;
    //    hr2 = parent->get_CachedName(&name2);
    //    if (SUCCEEDED(hr2))
    //    {
    //      std::string stdstr(_bstr_t(name2, true));
    //      stdstr.shrink_to_fit();
    //      //std::wcout << L"Parent element " << i + 1 << L" - Cached Name: ";
    //      wprintf(name2); std::wcout << std::endl;
    //    }
    //    SysFreeString(name2);
    //}
    _eventCount++;
    switch (eventID)
    {
      // https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/WinAuto/uiauto-msaa.md
      // https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-event-ids
    case UIA_ToolTipOpenedEventId:      wprintf(L">> Event ToolTipOpened Received! (count: %d)\n", _eventCount);  break;
    case UIA_ToolTipClosedEventId:      wprintf(L">> Event ToolTipClosed Received! (count: %d)\n", _eventCount);  break;
    case UIA_StructureChangedEventId:   wprintf(L">> Event StructureChanged Received! (count: %d)\n", _eventCount);  break;
    case UIA_MenuOpenedEventId:         wprintf(L">> Event MenuOpened Received! (count: %d)\n", _eventCount);  break;
    case UIA_AutomationPropertyChangedEventId:wprintf(L">> Event AutomationPropertyChanged Received! (count: %d)\n", _eventCount);  break;
    //case UIA_AutomationFocusChangedEventId: wprintf(L">> Event AutomationFocus Received! (count: %d)\n", _eventCount);  break;
    case UIA_AsyncContentLoadedEventId: wprintf(L">> Event AsyncContentLoaded Received! (count: %d)\n", _eventCount);  break;
    case UIA_MenuClosedEventId: wprintf(L">> Event MenuClosed Received! (count: %d)\n", _eventCount);  break;
    case UIA_LayoutInvalidatedEventId: wprintf(L">> Event LayoutInvalidated Received! (count: %d)\n", _eventCount);  break;
    case UIA_Invoke_InvokedEventId:
      wprintf(L">> Event Invoke_Invoked for ");
      wprintf(name); wprintf(L" Received!(count: % d)\n", _eventCount);
      break;
    case UIA_SelectionItem_ElementAddedToSelectionEventId: wprintf(L">> Event SelectionItem_ElementAddedToSelection Received! (count: %d)\n", _eventCount);  break;
    case UIA_SelectionItem_ElementRemovedFromSelectionEventId: wprintf(L">> Event SelectionItem_ElementRemovedFromSelection Received! (count: %d)\n", _eventCount);  break;
    case UIA_SelectionItem_ElementSelectedEventId: wprintf(L">> Event SelectionItem_ElementSelected Received! (count: %d)\n", _eventCount);  break;
    case UIA_Selection_InvalidatedEventId: wprintf(L">> Event Selection_Invalidated Received! (count: %d)\n", _eventCount);  break;
    //case UIA_Text_TextSelectionChangedEventId: wprintf(L">> Event Text_TextSelectionChanged Received! (count: %d)\n", _eventCount);  break;
    //case UIA_Text_TextChangedEventId: wprintf(L">> Event Text_TextChanged Received! (count: %d)\n", _eventCount);  break;
    case UIA_Window_WindowOpenedEventId: wprintf(L">> Event Window_WindowOpened Received! (count: %d)\n", _eventCount);  break;
    case UIA_Window_WindowClosedEventId: wprintf(L">> Event Window_WindowClosed Received! (count: %d)\n", _eventCount);  break;
    case UIA_MenuModeStartEventId: wprintf(L">> Event MenuModeStart Received! (count: %d)\n", _eventCount);  break;
    case UIA_MenuModeEndEventId: wprintf(L">> Event MenuModeEnd Received! (count: %d)\n", _eventCount);  break;
    case UIA_InputReachedTargetEventId: wprintf(L">> Event InputReachedTarget Received! (count: %d)\n", _eventCount);  break;
    case UIA_InputReachedOtherElementEventId: wprintf(L">> Event InputReachedOtherElement Received! (count: %d)\n", _eventCount);  break;
    case UIA_InputDiscardedEventId: wprintf(L">> Event InputDiscarded Received! (count: %d)\n", _eventCount);  break;
    case UIA_SystemAlertEventId: wprintf(L">> Event SystemAlert Received! (count: %d)\n", _eventCount);  break;
    case UIA_LiveRegionChangedEventId: wprintf(L">> Event LiveRegionChanged Received! (count: %d)\n", _eventCount);  break;
    case UIA_HostedFragmentRootsInvalidatedEventId: wprintf(L">> Event HostedFragmentRootsInvalidated Received! (count: %d)\n", _eventCount);  break;
    case UIA_Drag_DragStartEventId: wprintf(L">> Event Drag_DragStart Received! (count: %d)\n", _eventCount);  break;
    case UIA_Drag_DragCancelEventId: wprintf(L">> Event Drag_DragCancel Received! (count: %d)\n", _eventCount);  break;
    case UIA_Drag_DragCompleteEventId: wprintf(L">> Event Drag_DragComplete Received! (count: %d)\n", _eventCount);  break;
    case UIA_DropTarget_DragEnterEventId: wprintf(L">> Event DropTarget_DragEnter Received! (count: %d)\n", _eventCount);  break;
    case UIA_DropTarget_DragLeaveEventId: wprintf(L">> Event DropTarget_DragLeave Received! (count: %d)\n", _eventCount);  break;
    case UIA_DropTarget_DroppedEventId: wprintf(L">> Event DropTarget_Dropped Received! (count: %d)\n", _eventCount);  break;
    case UIA_TextEdit_TextChangedEventId: wprintf(L">> Event TextEdit_TextChanged Received! (count: %d)\n", _eventCount);  break;
    case UIA_TextEdit_ConversionTargetChangedEventId: wprintf(L">> Event TextEdit_ConversionTarget Received! (count: %d)\n", _eventCount);  break;
    case UIA_ChangesEventId: wprintf(L">> Event Changes Received! (count: %d)\n", _eventCount);  break;
    case UIA_NotificationEventId:
      wprintf(L">> Event Notification for "); wprintf(name); wprintf(L" Received!(count: % d)\n", _eventCount);  break;
      break;
    case UIA_ActiveTextPositionChangedEventId: wprintf(L">> Event ActiveTextPositionChanged Received! (count: %d)\n", _eventCount);  break;
    default:  wprintf(L">> Event (%d) Received! (count: %d)\n", eventID, _eventCount); break;
    }
    if (SUCCEEDED(hr)) {
      SysFreeString(name);
    }

    return S_OK;
  }

};

int EventHandlerTest(void)
{
  HRESULT hr;
  int ret = 0;
  IUIAutomationElement* pTargetElement = NULL;
  EventHandler* pEHTemp = NULL;

  CoInitializeEx(NULL, COINIT_MULTITHREADED);
  IUIAutomation* g_pAutomation = NULL;
  hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void**)&g_pAutomation);
  if (FAILED(hr) || g_pAutomation == NULL)
  {
    ret = 1;
    goto cleanup;
  }
  // Use root element for listening to window and tooltip creation and destruction.
  hr = g_pAutomation->GetRootElement(&pTargetElement);
  if (FAILED(hr) || pTargetElement == NULL)
  {
    ret = 1;
    goto cleanup;
  }

  pEHTemp = new EventHandler();
  if (pEHTemp == NULL)
  {
    ret = 1;
    goto cleanup;
  }

  wprintf(L"-Adding Event Handlers.\n");
  //hr = g_pAutomation->AddAutomationEventHandler(UIA_ToolTipOpenedEventId, pTargetElement, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEHTemp);
  //if (FAILED(hr)) {ret = 1; goto cleanup;}
  
  //hr = g_pAutomation->AddAutomationEventHandler(UIA_ToolTipClosedEventId, pTargetElement, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEHTemp);
  //if (FAILED(hr)){ret = 1; goto cleanup;}
  
  //hr = g_pAutomation->AddAutomationEventHandler(UIA_Window_WindowOpenedEventId, pTargetElement, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEHTemp);
  //if (FAILED(hr)){ret = 1; goto cleanup;}
  
  //hr = g_pAutomation->AddAutomationEventHandler(UIA_Window_WindowClosedEventId, pTargetElement, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEHTemp);
  //if (FAILED(hr)){ret = 1; goto cleanup; }
  //for (int i = UIA_ToolTipOpenedEventId; i <= UIA_ActiveTextPositionChangedEventId; i++)
  for (int i = UIA_ToolTipOpenedEventId; i <= UIA_ActiveTextPositionChangedEventId; i++)
  {
    //if ((i == UIA_AutomationFocusChangedEventId)) continue;// || (i == UIA_Text_TextSelectionChangedEventId) || (i == UIA_Text_TextChangedEventId)) continue;
    hr = g_pAutomation->AddAutomationEventHandler(i, pTargetElement, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEHTemp);
    if (FAILED(hr))
    {
      ret = 1; goto cleanup;
    }

  }
  wprintf(L"-Press any key to remove event handlers and exit\n");
  getchar();

  wprintf(L"-Removing Event Handlers.\n");

cleanup:
  // Remove event handlers, release resources, and terminate
  if (g_pAutomation != NULL)
  {
    hr = g_pAutomation->RemoveAllEventHandlers();
    if (FAILED(hr))
      ret = 1;
    g_pAutomation->Release();
  }

  if (pEHTemp != NULL)
    pEHTemp->Release();

  if (pTargetElement != NULL)
    pTargetElement->Release();

  CoUninitialize();
  return ret;
}

// Define the event handler class
class FocusChangedEventHandler : public IUIAutomationFocusChangedEventHandler {
public:
  // IUnknown methods
  ULONG STDMETHODCALLTYPE AddRef() {
    return InterlockedIncrement(&m_refCount);
  }

  ULONG STDMETHODCALLTYPE Release() {
    LONG refCount = InterlockedDecrement(&m_refCount);
    if (refCount == 0) {
      delete this;
    }
    return refCount;
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) {
    if (riid == IID_IUnknown || riid == IID_IUIAutomationFocusChangedEventHandler) {
      *ppvObject = static_cast<IUIAutomationFocusChangedEventHandler*>(this);
      AddRef();
      return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
  }

  // IUIAutomationFocusChangedEventHandler method
  HRESULT STDMETHODCALLTYPE HandleFocusChangedEvent(IUIAutomationElement* sender) {
    BSTR name;
    HRESULT hr = sender->get_CurrentName(&name);
    if (SUCCEEDED(hr)) {
      std::wcout << L"Focus changed to: " << wprintf(name) << std::endl;
      SysFreeString(name);
    }
    return S_OK;
  }

private:
  LONG m_refCount = 1;
};

int FocusChange()
{
  // Initialize COM library
  CoInitialize(NULL);

  // Create UIAutomation object
  IUIAutomation* g_pAutomation = NULL;
  HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&g_pAutomation);

  if (FAILED(hr)) {
    std::cerr << "Failed to create UIAutomation object." << std::endl;
    return 1;
  }

  // Create and register event handler
  FocusChangedEventHandler* pFocusHandler = new FocusChangedEventHandler();
  hr = g_pAutomation->AddFocusChangedEventHandler(NULL, pFocusHandler);

  if (FAILED(hr)) {
    std::cerr << "Failed to register focus changed event handler." << std::endl;
    g_pAutomation->Release();
    return 1;
  }

  std::cout << "Listening for focus change events..." << std::endl;

  // Run a message loop to keep the application running and listen for events
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Clean up
  g_pAutomation->RemoveFocusChangedEventHandler(pFocusHandler);
  pFocusHandler->Release();
  g_pAutomation->Release();
  CoUninitialize();

  return 0;
}


void CALLBACK HandleWinEvent(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
  // Check if the event is for the correct object and child
  if (event == EVENT_OBJECT_LOCATIONCHANGE && idObject == OBJID_WINDOW && idChild == CHILDID_SELF) {
    // Get window placement information
    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {
      printf("Window Moved: HWND: 0x%08p New Position: (%d, %d, %d, %d)\n", hwnd, rect.left, rect.top, rect.right, rect.bottom);
    }
  }
}

#include <Windows.h>
#include <UIAutomation.h>
#include <atlbase.h>
#include <iostream>

static CComPtr<IUIAutomation> g_uiAutomation;

HRESULT InitializeUIAutomation() {
  return CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_uiAutomation));
}

CComPtr<IUIAutomationElement> GetAutomationElement(HWND hwnd) {
  CComPtr<IUIAutomationElement> element;
  if (g_uiAutomation) {
    g_uiAutomation->ElementFromHandle(hwnd, &element);
  }
  return element;
}
bool SupportsValuePattern(CComPtr<IUIAutomationElement> element) {
  VARIANT_BOOL isSupported;
  if (element) {
    Fill_BOOL_Property(element, &isSupported, UIA_IsValuePatternAvailablePropertyId);
  }
  return isSupported == TRUE;
}
std::wstring GetValue(CComPtr<IUIAutomationElement> element) {
  CComPtr<IUIAutomationValuePattern> valuePattern;
  if (SUCCEEDED(element->GetCurrentPatternAs(UIA_ValuePatternId, IID_PPV_ARGS(&valuePattern)))) {
    BSTR value;
    if (SUCCEEDED(valuePattern->get_CurrentValue(&value))) {
      std::wstring result(value);
      SysFreeString(value);
      return result;
    }
  }
  return L"";
}
std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen)
{
  int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

  std::string dblstr(len, '\0');
  len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */,
    pstr, wslen /* not necessary NULL-terminated */,
    &dblstr[0], len,
    NULL, NULL /* no default char */);

  return dblstr;
}

HRESULT SetValue(CComPtr<IUIAutomationElement> element, const std::wstring& newValue) {
  CComPtr<IUIAutomationValuePattern> valuePattern;
  if (SUCCEEDED(element->GetCurrentPatternAs(UIA_ValuePatternId, IID_PPV_ARGS(&valuePattern)))) {
    //return valuePattern->SetValue(newValue.c_str());
  }
  return E_FAIL;
}

int SetVaueNotepad() {
  CoInitialize(NULL);

  // Initialize UI Automation
  if (FAILED(InitializeUIAutomation())) {
    std::cerr << "Failed to initialize UI Automation.\n";
    return 1;
  }

  // Get the target window handle (replace with the actual HWND)
  HWND hwndTarget = FindWindow(NULL, L"Untitled - Notepad"); // Example: Notepad
  if (!hwndTarget) {
    std::cerr << "Target window not found.\n";
    return 1;
  }

  // Get the Automation Element
  CComPtr<IUIAutomationElement> element = GetAutomationElement(hwndTarget);
  if (!element) {
    std::cerr << "Failed to get automation element.\n";
    return 1;
  }

  // Check if the element supports ValuePattern
  if (!SupportsValuePattern(element)) {
    std::cerr << "Element does not support ValuePattern.\n";
    return 1;
  }

  // Get the current value
  std::wcout << L"Current Value: " << GetValue(element) << std::endl;

  // Set a new value (if the control allows setting values)
  if (SUCCEEDED(SetValue(element, L"New Value"))) {
    std::wcout << L"Value successfully updated.\n";
  }
  else {
    std::cerr << "Failed to set the value.\n";
  }

  CoUninitialize();
  return 0;
}
// It is deprected in favor of "UI Automation" according to https://learn.microsoft.com/en-us/windows/console/console-winevents
// https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-eventsoverview
void CALLBACK HandleWinEventGeneral(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
  // Check if the event is for the correct object and child
  //if (idObject == OBJID_WINDOW && idChild == CHILDID_SELF) {
    // Get window placement information
//    RECT rect;
    //if (GetWindowRect(hwnd, &rect)) {
  static int yyy = 0;
  //if (event != UIA_ValueValuePropertyId)
  //{
  //  IUIAutomationElement* pElement = GetAutomationElement(pAutomation, hwndTarget);
  //  if (!pElement) {
  //    pAutomation->lpVtbl->Release(pAutomation);
  //    CoUninitialize();
  //    return 1;
  //  }
  //}
  if (EVENT_OBJECT_VALUECHANGE)
  {
    yyy++;
  }
  if ((idObject == OBJID_WINDOW && idChild == CHILDID_SELF))
  {
    if (event == EVENT_OBJECT_LOCATIONCHANGE) {
      // Get window placement information
      RECT rect;
      if (GetWindowRect(hwnd, &rect))
      {
        printf("Window Moved: HWND: 0x%08p New Position: (%d, %d, %d, %d)\n",hwnd, rect.left, rect.top, rect.right, rect.bottom);
      }
    }
    else if (event == RI_MOUSE_RIGHT_BUTTON_UP)
    {
      printf("Button up\n");
    }
    else if (event == EVENT_OBJECT_SHOW)
    {
      printf("Object show\n");
    }
    else if (event == EVENT_OBJECT_HIDE)
    {
      printf("Object hide\n");
    }
    else if (event == EVENT_SYSTEM_CAPTUREEND)
    {
      printf("A window has lost mouse capture\n");
    }
    else if (event == EVENT_SYSTEM_MOVESIZEEND)
    {
      printf("The movement or resizing of a window has finished\n");
    }
    else if (event == EVENT_CONSOLE_LAYOUT)
    {
      printf("The console layout has changed\n");
    }
    else
    {
      printf("event:%d idObject:%d idChild:%d\n", event, idObject, idChild);
    }
  }
  else
  {
    if ((event != EVENT_CONSOLE_UPDATE_REGION) && (event != EVENT_CONSOLE_UPDATE_SCROLL))
      printf("event:%d idObject:%d idChild:%d\n", event, idObject, idChild);
  }
  //if ((event != EVENT_CONSOLE_UPDATE_REGION) && (event != EVENT_CONSOLE_UPDATE_SCROLL) && (event != EVENT_OBJECT_VALUECHANGE) &&
   //   /*(event != EVENT_OBJECT_LOCATIONCHANGE) && */(event != EVENT_CONSOLE_LAYOUT))
    //}
 // }
}

int intevent_location_changed() {
  // Hook for the location change event
  //HWINEVENTHOOK hEventHook = SetWinEventHook(
  //  EVENT_OBJECT_LOCATIONCHANGE, // Event type (min)
  //  EVENT_OBJECT_LOCATIONCHANGE, // Event type (max)
  //  NULL,                        // Handle to DLL (NULL for current process)
  //  HandleWinEvent,              // Callback function
  //  0,                           // Process ID (0 = all processes)
  //  0,                           // Thread ID (0 = all threads)
  //  WINEVENT_OUTOFCONTEXT        // Flags (run the hook out of context)
  //);
  HWINEVENTHOOK hEventHook = SetWinEventHook(
    EVENT_MIN, // Event type (min)
    EVENT_MAX, // Event type (max)
    NULL,                        // Handle to DLL (NULL for current process)
    HandleWinEventGeneral,              // Callback function
    0,                           // Process ID (0 = all processes)
    0,                           // Thread ID (0 = all threads)
    WINEVENT_OUTOFCONTEXT        // Flags (run the hook out of context)
  );
  if (hEventHook == NULL) {
    printf("Failed to set hook\n");
    return 1;
  }

  // Simple message loop to keep the program running
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Unhook when done
  UnhookWinEvent(hEventHook);
  return 0;
}
