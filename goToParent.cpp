#include <windows.h>
#include <UIAutomation.h>
#include <iostream>

#include "Header.h"

extern IUIAutomation* g_pAutomation2;

IUIAutomationElement* GetContainingWindow(IUIAutomationElement* pChild)
{
  IUIAutomationElement* pDesktop = NULL;
  BOOL same;
  HRESULT hr;
  IUIAutomationElement* pParent = NULL;
  IUIAutomationElement* pNode = pChild;
  IUIAutomationTreeWalker* pWalker = NULL;
  if (pChild == NULL)
    return NULL;

  if (g_pAutomation2 == nullptr) {
    HRESULT hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&g_pAutomation2);
    if (FAILED(hr)) { std::cerr << "CoCreateInstance failed: " << std::hex << hr << std::endl; goto cleanup; }
  }
  hr = g_pAutomation2->GetRootElement(&pDesktop);
  if (FAILED(hr))
    return NULL;

  g_pAutomation2->CompareElements(pChild, pDesktop, &same);
  if (same)
  {
    pDesktop->Release();
    return NULL; // No parent, so return NULL.
  }


  // Create the treewalker.
  g_pAutomation2->get_ControlViewWalker(&pWalker);
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
    {
      BSTR name;//get_CurrentAutomationId
      HRESULT hr2 = pNode->get_CurrentAutomationId(&name);
      if (SUCCEEDED(hr2) && (name != NULL) && (SysStringLen(name) != 0)) {
        wprintf(L"1:%s", name); printf("/");
        SysFreeString(name);
      }
      else
      {
        CONTROLTYPEID controlType;
        HRESULT hr3 = pNode->get_CurrentControlType(&controlType);
        hr2 = pNode->get_CurrentName(&name);
        if (SUCCEEDED(hr2) && (name != NULL) && (SysStringLen(name) != 0)) {
          wprintf(name); printf("/");
          SysFreeString(name);
        }
        else
        {
          hr2 = pNode->get_CurrentClassName(&name);
          if (SUCCEEDED(hr2) && (name != NULL) && (SysStringLen(name) != 0)) {
            wprintf(name); printf("/");
            SysFreeString(name);
          }
        }
      }
    }
    g_pAutomation2->CompareElements(pParent, pDesktop, &same);
    if (same)
    {
      pDesktop->Release();
      pParent->Release();
      pWalker->Release();
      // Reached desktop, so return next element below it.
      printf("/n");
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
  printf("/n");
  return NULL;
}

IUIAutomation* g_pAutomation2 = nullptr;
IUIAutomationTreeWalker* treeWalker = nullptr;
ControlLevel desktop;

void DescendantsIterative()
{
  IUIAutomationElement* pCalculatorWindow = nullptr;
  IUIAutomationElement* pRootElement = nullptr;
  if (PrepareAutomationEnvironment(&pRootElement) != 0) goto CLEAN_UP;
  if (GetCalculatorElement(&pCalculatorWindow, pRootElement) != 0) goto CLEAN_UP;
  //CacheAndRetrieveMultiplePropertiesMine(pCalculatorWindow);

  ListDescendantsIterative(pCalculatorWindow);//pRootElement);// 
  ProcessJson(desktop);
  CLEAN_UP:
  CleanUg_pAutomation(&pRootElement);
}

