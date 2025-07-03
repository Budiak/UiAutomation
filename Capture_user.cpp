#include <windows.h>
#include <iostream>
#include <thread>
#include <string>



#include "Header.h"
#include "log.h"

using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

static HHOOK hMouseHook;

static IUIAutomationElementArray* GetElementProperties(IUIAutomationElement* pElement)
{
  if (pElement == NULL) return NULL;

  IUIAutomationCondition* pCondition = NULL;
  IUIAutomationCacheRequest* pCacheRequest = NULL;
  IUIAutomationElementArray* pFound = NULL;

  HRESULT hr = _automation->CreateTrueCondition(&pCondition);
  if (FAILED(hr)) { cerr << "Failed CreateTrueCondition" << endl; goto cleanup; }

  hr = _automation->CreateCacheRequest(&pCacheRequest);
  if (FAILED(hr)) { cerr << "Failed CreateCacheRequest" << endl; goto cleanup; }

  hr = pCacheRequest->AddPattern(UIA_SelectionItemPatternId);
  if (FAILED(hr)) { cerr << "Failed AddPattern(UIA_SelectionItemPatternId)" << endl; goto cleanup; }

  hr = pCacheRequest->AddProperty(UIA_NamePropertyId);
  if (FAILED(hr)) { cerr << "Failed AddProperty(UIA_NamePropertyId)" << endl; goto cleanup; }

  pElement->FindAllBuildCache(TreeScope_Element, pCondition, pCacheRequest, &pFound);

cleanup:
  if (pCondition != NULL) pCondition->Release();
  if (pCacheRequest != NULL) pCacheRequest->Release();

  return pFound;
}
using namespace std;
#include <ostream>
#include <fstream>
static std::string ConvertBSTRToMBS(BSTR bstr)
{
  int wslen = ::SysStringLen(bstr);
  return ConvertWCSToMBS((wchar_t*)bstr, wslen);
}

static IUIAutomationElement* GetContainingWindow(IUIAutomationElement* pChild)
{
  HRESULT hr;
  BSTR retVal;
  if (pChild == NULL) return NULL;
  DataPerControlType filledData;
  CacheAndRetrieveMultiplePropertiesMine(pChild, &filledData);

  nlohmann::ordered_json ordered_desktop(filledData);
  std::ofstream outFile("pretty.json");
  outFile << std::setw(2) << ordered_desktop.dump(1, ' ', false, nlohmann::ordered_json::error_handler_t::replace) << std::endl;

  return nullptr;

  hr = pChild->get_CurrentName(&retVal);
  if (FAILED(hr)) return NULL;
  string name = ConvertBSTRToMBS(retVal);
  cout << name << endl;
  SysFreeString(retVal);

  IUIAutomationElement* pDesktop = NULL;
  hr = _automation->GetRootElement(&pDesktop);
  if (FAILED(hr)) return NULL;

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
  cout << string("AAAAAAAA") << endl;
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
    if (pNode != pChild) pNode->Release();
    hr = pParent->get_CurrentName(&retVal);
    if (!FAILED(hr))
    {
      string name = ConvertBSTRToMBS(retVal);
      cout << name << endl;
      SysFreeString(retVal);
    }
    else
    {
      cout << string("AAAAAAAA") << endl;
    }

    pNode = pParent;
  }

cleanup:
  if ((pNode != NULL) && (pNode != pChild)) pNode->Release();
  if (pDesktop != NULL) pDesktop->Release();
  if (pWalker != NULL)  pWalker->Release();
  if (pParent != NULL)  pParent->Release();

  return NULL;
}

static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode >= 0)
  {
    if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN)
    {
      MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;
      if (pMouseStruct)
      {
        IUIAutomationElement* element = NULL;
        HRESULT hr = _automation->ElementFromPoint(pMouseStruct->pt, &element);
        if (FAILED(hr))
        {
          wprintf(L"Failed to ElementFromPoint, HR: 0x%08x\n\n", hr);
        }
        else
        {
          std::cout << "Mouse Click at: (" << pMouseStruct->pt.x << ", " << pMouseStruct->pt.y << ")" << std::endl;
          GetContainingWindow(element);
        }
      }
    }
  }
  return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

void Capture_user()
{
  hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);
  if (!hMouseHook) {
    LOGGER_FATAL("Failed to install mouse hook!");
    return;
  }

  LOGGER_INFO("Mouse click logger started. Press ESC to exit.");
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
    if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
    {
      break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnhookWindowsHookEx(hMouseHook);
}
