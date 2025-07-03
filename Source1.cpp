#include <windows.h>
#include "Header.h"
#include <map>
#include <UIAutomation.h>
#include <comutil.h>
#include <iostream>
#include <comdef.h>
#include <stack>
#include <atlbase.h>
#include "log.h"

using namespace std;


extern IUIAutomation* g_pAutomation2;
extern IUIAutomationTreeWalker* treeWalker;
extern ControlLevel desktop;
int PrepareAutomationEnvironment(IUIAutomationElement** pRootElement)
{
  // Initialize COM library
  HRESULT hr = CoInitialize(NULL);
  if (FAILED(hr)) {
    wprintf(L"Failed to initialize COM library. Error code: 0x%x\n", hr);
    return 1;
  }

  // Create IUIAutomation object
  g_pAutomation2 = NULL;
  hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void**)&g_pAutomation2);
  if (FAILED(hr)) {
    _com_error err(hr);
    LPCTSTR errMsg = err.ErrorMessage();
    wprintf(L"Failed to create IUIAutomation object. Error code: 0x%x messsage:%s\n", hr, errMsg);
    //CoUninitialize();
    return 1;
  }

  // Get the root automationID element (desktop)
  *pRootElement = nullptr;
  hr = g_pAutomation2->GetRootElement(pRootElement);
  if (FAILED(hr)) {
    wprintf(L"Failed to get root element. Error code: 0x%x\n", hr);
    g_pAutomation2->Release();
    //CoUninitialize();
    return 1;
  }
  
  hr = g_pAutomation2->get_ControlViewWalker(&treeWalker);
  if (FAILED(hr) || treeWalker == nullptr) {
    std::cerr << "Failed to get tree walker." << std::endl;
    return 1;
  }
  return 0;
}

void CleanUg_pAutomation(IUIAutomationElement** pRootElement)
{
  if ((*pRootElement) != nullptr)  {(*pRootElement)->Release(); (*pRootElement) = nullptr;}
  if (g_pAutomation2 != nullptr){g_pAutomation2->Release(); g_pAutomation2 = nullptr;}
  if (treeWalker != nullptr){treeWalker->Release();treeWalker = nullptr;}
  CoUninitialize();
}

int GetCalculatorElement(IUIAutomationElement** pCalculatorWindow, IUIAutomationElement* pRootElement)
{
  IUIAutomationCondition* pCondition = NULL;
  IUIAutomationCacheRequest* pCacheRequest = NULL;

  HRESULT hr = g_pAutomation2->CreatePropertyCondition(UIA_NamePropertyId, _variant_t(L"Calculator"), &pCondition);
  if (FAILED(hr)) {wprintf(L"Failed to create property condition. Error code: 0x%x\n", hr); goto CLEAN_RESOURCES;}

  // Create a cache request
  hr = g_pAutomation2->CreateCacheRequest(&pCacheRequest);
  if (FAILED(hr)) { std::cerr << "CreateCacheRequest failed: " << std::hex << hr << std::endl; goto CLEAN_RESOURCES; }

  for (long propertyId = UIA_RuntimeIdPropertyId; propertyId < UIA_IsDialogPropertyId; propertyId++)
  {
    pCacheRequest->AddProperty(propertyId);
  }

  *pCalculatorWindow = NULL;
  hr = pRootElement->FindFirstBuildCache(TreeScope_Children, pCondition, pCacheRequest, pCalculatorWindow);
  if (FAILED(hr) || *pCalculatorWindow == NULL)
  {
    wprintf(L"Failed to find Calculator window. Error code: 0x%x\n", hr);
    LaunchCalculator();
    hr = pRootElement->FindFirstBuildCache(TreeScope_Children, pCondition, pCacheRequest, pCalculatorWindow);
    if (FAILED(hr) || *pCalculatorWindow == NULL)
    {
      goto CLEAN_RESOURCES;
    }
  }

CLEAN_RESOURCES:
  if (pCondition != NULL) { pCondition->Release(); pCondition = NULL; }
  if (pCacheRequest != NULL) { pCacheRequest->Release(); pCacheRequest = NULL; }

  return 0;
}

void LaunchCalculator(void)
{
  //return;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  if (!CreateProcess(
    L"C:\\Windows\\System32\\calc.exe", // Path to the executable
    NULL,                               // Command line
    NULL,                               // Process handle not inheritable
    NULL,                               // Thread handle not inheritable
    FALSE,                              // Set handle inheritance to FALSE
    0,                                  // No creation flags
    NULL,                               // Use parent's environment block
    NULL,                               // Use parent's starting directory 
    &si,                                // Pointer to STARTUPINFO structure
    &pi)                                // Pointer to PROCESS_INFORMATION structure
    ) {
    wprintf(L"CreateProcess failed (%d).\n", GetLastError());
    exit(1);
  }

  // Wait a bit for the calculator to launch
  Sleep(2000);
}



//UIA_LocalizedControlTypePropertyId
void to_json(nlohmann::ordered_json& dataOut, const DataPerControlType& dataIn) {
  dataOut = nlohmann::ordered_json{
    { "name", dataIn.name },
    { "ControlTypeStr", dataIn.ControlTypeStr },
    { "boundingRect", {dataIn.boundingRect.left, dataIn.boundingRect.top, dataIn.boundingRect.bottom, dataIn.boundingRect.right} },
    { "automationId", dataIn.automationID }
  };
}
void to_json(nlohmann::json& dataOut, const DataPerControlType& dataIn) {
  dataOut = nlohmann::json{
    { "name", dataIn.name },
    { "ControlTypeStr", dataIn.ControlTypeStr },
    { "boundingRect", {dataIn.boundingRect.left, dataIn.boundingRect.top, dataIn.boundingRect.bottom, dataIn.boundingRect.right} },
    { "automationId", dataIn.automationID }
  };
}

//void from_json(nlohmann::json& dataIn, const DataPerControlType& dataOut)
//{
//  dataIn.at("name").get_to(dataOut.name);
//  dataIn.at("ControlTypeStr").get_to(dataOut.ControlTypeStr);
//  dataIn.at("boundingRect").get_to(dataOut.boundingRect);
//}

void to_json(nlohmann::ordered_json& dataOut, const ControlLevel& dataIn) {
  dataOut = nlohmann::ordered_json{
    { "self", dataIn.self },
    { "children", dataIn.children }
  };
}

void to_json(nlohmann::json& dataOut, const ControlLevel& dataIn) {
  dataOut = nlohmann::json{
    { "self", dataIn.self },
    { "children", dataIn.children }
  };
}


int Click_one(void)
{
  // Initialize COM
  CoInitialize(NULL);

  // Create the IUIAutomation interface
  IUIAutomation* g_pAutomation = nullptr;
  HRESULT hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void**)&g_pAutomation);

  if (FAILED(hr) || g_pAutomation == nullptr) {
    std::cerr << "Failed to initialize UI Automation." << std::endl;
    CoUninitialize();
    return -1;
  }

  // Get the root element of the desktop
  IUIAutomationElement* pRootElement = nullptr;
  hr = g_pAutomation->GetRootElement(&pRootElement);

  if (FAILED(hr) || pRootElement == nullptr) {
    std::cerr << "Failed to get the root element." << std::endl;
    g_pAutomation->Release();
    CoUninitialize();
    return -1;
  }

  // Find the Calculator window
  IUIAutomationCondition* pCondition = nullptr;
  hr = g_pAutomation->CreatePropertyCondition(UIA_NamePropertyId, _variant_t(L"Calculator"), &pCondition);

  if (SUCCEEDED(hr) && pCondition != nullptr) {
    IUIAutomationElement* pCalculatorWindow = nullptr;
    hr = pRootElement->FindFirst(TreeScope_Children, pCondition, &pCalculatorWindow);

    if (SUCCEEDED(hr) && pCalculatorWindow != nullptr) {
      // Find the "1" button within the Calculator window
      IUIAutomationCondition* pButtonCondition = nullptr;
      hr = g_pAutomation->CreatePropertyCondition(UIA_NamePropertyId, _variant_t(L"One"), &pButtonCondition);

      if (SUCCEEDED(hr) && pButtonCondition != nullptr) {
        IUIAutomationElement* pButtonElement = nullptr;
        hr = pCalculatorWindow->FindFirst(TreeScope_Descendants, pButtonCondition, &pButtonElement);

        if (SUCCEEDED(hr) && pButtonElement != nullptr) {
          // Get the Invoke pattern for the button
          IUIAutomationInvokePattern* pInvokePattern = nullptr;
          hr = pButtonElement->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern), (void**)&pInvokePattern);

          if (SUCCEEDED(hr) && pInvokePattern != nullptr) {
            // Invoke the "1" button
            hr = pInvokePattern->Invoke();
            if (SUCCEEDED(hr)) {
              std::cout << "Successfully clicked the '1' button!" << std::endl;
            }
            else {
              std::cerr << "Failed to invoke the '1' button." << std::endl;
            }

            pInvokePattern->Release();
          }
          else {
            std::cerr << "Failed to get Invoke pattern." << std::endl;
          }

          pButtonElement->Release();
        }
        else {
          std::cerr << "Failed to find the '1' button." << std::endl;
        }

        pButtonCondition->Release();
      }
      else {
        std::cerr << "Failed to create button condition." << std::endl;
      }

      pCalculatorWindow->Release();
    }
    else {
      std::cerr << "Failed to find the Calculator window." << std::endl;
    }

    pCondition->Release();
  }
  else {
    std::cerr << "Failed to create window condition." << std::endl;
  }

  pRootElement->Release();
  g_pAutomation->Release();
  CoUninitialize();

  return 0;
}

void Fill_BOOL_Property(IUIAutomationElement* element, VARIANT_BOOL* pDst, long PropertyId)
{
  VARIANT retVal;
  HRESULT hr = element->GetCurrentPropertyValue(PropertyId, &retVal);
  if (!FAILED(hr))
  {
    *pDst = V_BOOL(&retVal);
    VariantClear(&retVal);
  }
}

void ProcessDescendentsSingleElem(IUIAutomationElement* pNode, std::vector<ListDescendantsVectorEntry>& elementStack, ControlLevel* pParentLevel)
{
  IUIAutomationTreeWalker* pControlWalker = NULL;
  g_pAutomation2->get_ControlViewWalker(&pControlWalker);
  if (pControlWalker == NULL)
    return;

  IUIAutomationElementArray* pAllChildred;
  IUIAutomationCondition* spCond;     g_pAutomation2->CreateTrueCondition(&spCond);
  HRESULT hr = pNode->FindAll(TreeScope_Children, spCond, &pAllChildred);

  int count = 0;
  pAllChildred->get_Length(&count);
  for (int i = 0; i < count; i++)
  {
    IUIAutomationElement* pChild;
    pAllChildred->GetElement(i, &pChild);
    elementStack.push_back(ListDescendantsVectorEntry(pChild, pParentLevel));
  }
}

void FillControlLevel(ControlLevel* pControlLevel, IUIAutomationElement* pElement, ControlLevel* pParent)
{
  pControlLevel->parent = nullptr;
  pControlLevel->self = FillElementData(pElement);
  IUIAutomationElement* child = nullptr;
  HRESULT hr = treeWalker->GetFirstChildElement(pElement, &child);
  if (!FAILED(hr))
  {
    do
    {
      pControlLevel->children.push_back(ControlLevel(pControlLevel));
      IUIAutomationElement* nextSibling = nullptr;
      hr = treeWalker->GetNextSiblingElement(child, &nextSibling);

      if (!FAILED(hr))
      {
        pControlLevel->children.push_back(ControlLevel(pControlLevel));
        //newLevel.self = FillElementData(child);
        //TraverseTree(nextSibling, pRootElement);
        if (nextSibling != nullptr)
          nextSibling->Release();
      }
      if (child != nullptr)
        child->Release();
    } while (child != nullptr);
  }
}

/*int ListDescendantsAtlIterative(ControlLevel* desktop)
{
  if (CoInitialize(nullptr) != S_OK) return -1;
  CComPtr<IUIAutomation> spUI;                if (spUI.CoCreateInstance(__uuidof(CUIAutomation)) != S_OK) return -1;
  CComPtr<IUIAutomationElement> spRoot;       if (spUI->GetRootElement(&spRoot) != S_OK) return -1;
  vector<ListDescendantsVectorEntry> elementVector;
  desktop->parent = nullptr;
  if (ProcessDescendentsSingleElem(spRoot, &elementVector, desktop, spUI) != 0)
    return -1;
  while (!elementVector.empty())
  {
    ListDescendantsVectorEntry& current = elementVector.back();
    if (ProcessDescendentsSingleElem(current.pElem, &elementVector, current.pParent, spUI) != 0)
      return -1;
    elementVector.pop_back();
  }
  return 0;
}*/

void ListDescendantsIterative(IUIAutomationElement* pRootElement)
{
  if (pRootElement == NULL)
    return;

  IUIAutomationTreeWalker* pControlWalker = NULL;
  g_pAutomation2->get_ControlViewWalker(&pControlWalker);
  if (pControlWalker == NULL)
    return;

  std::vector<ListDescendantsVectorEntry> elementStack;
  desktop.parent = nullptr;
  desktop.self = FillElementData(pRootElement);
  ProcessDescendentsSingleElem(pRootElement, elementStack, &desktop);
  
  while (!elementStack.empty())
  {
    // Get the top element and its indent level from the stack
    IUIAutomationElement* pNode = elementStack.back().pElem;
    ControlLevel* pParentLevel = elementStack.back().pParent;
    elementStack.pop_back();

    if (pNode == NULL)
      continue;

    ControlLevel newLevel(pParentLevel);
    newLevel.self = FillElementData(pNode);
    pParentLevel->children.push_back(newLevel);
    ProcessDescendentsSingleElem(pNode, elementStack, &pParentLevel->children.back());
    pNode->Release();
  }

  // Clean up the tree walker
  if (pControlWalker != NULL)
    pControlWalker->Release();
}
void Fill_string_from_VARIANT(std::string* pDst, VARIANT* pSrc)
{
  if (V_VT(pSrc) == VT_BSTR)
  {
    BSTR bstrValue = V_BSTR(pSrc);
    if ((bstrValue != nullptr) && (SysStringLen(bstrValue) != 0))
    {
      *pDst = _bstr_t(bstrValue, true);
    }
  }
  else
  {
    LOGGER_ERROR("Wrong type in Fill_string_from_VARIANT");
  }
}
void Fill_bool_from_VARIANT(bool* pDst, VARIANT* pSrc)
{
  if (V_VT(pSrc) == VT_BOOL)
  {
    VARIANT_BOOL bstrValue = V_BOOL(pSrc);
    *pDst = (bstrValue == 0) ? false : true;
  }
  else
  {
    LOGGER_ERROR("Wrong type in Fill_bool_from_VARIANT");
  }
}
void Fill_string_Property(IUIAutomationElement* element, std::string* pDst, long PropertyId)
{
  VARIANT retVal;
  HRESULT hr = element->GetCachedPropertyValue(PropertyId, &retVal);
  if (!FAILED(hr))
  {
      Fill_string_from_VARIANT(pDst, &retVal);
  }
  else
  {
    HRESULT hr2 = element->GetCurrentPropertyValue(PropertyId, &retVal);
    VARIANT retVal2;
    if (!FAILED(hr2))
    {
      Fill_string_from_VARIANT(pDst, &retVal2);
    }
    VariantClear(&retVal2);
  }
  VariantClear(&retVal);
}

void Fill_BSTR_Property(IUIAutomationElement* element, BSTR* pDst, long PropertyId)
{
  VARIANT retVal;
  HRESULT hr = element->GetCurrentPropertyValue(PropertyId, &retVal);
  if (!FAILED(hr))
  {
    if (V_VT(&retVal) == VT_EMPTY)
    {
      VariantClear(&retVal);
    }
    else
    {
      BSTR bstrValue = V_BSTR(&retVal);
      if ((bstrValue != nullptr) && (SysStringLen(bstrValue) != 0))
      {
        static int ffff = 0;
        if (PropertyId == UIA_HelpTextPropertyId)
          ffff++;

        //if (PropertyId == UIA_HelpTextPropertyId)
        //  while (1) {} // DEBUG TO MANUALLY INSPECT IT
        *pDst = bstrValue;
      }
      else
      {
        VariantClear(&retVal);
      }
    }
  }
}

void FillClickablePoint(IUIAutomationElement* element, ClickablePoint* clickablePoint)
{
  VARIANT retVal;
  HRESULT hr = element->GetCurrentPropertyValue(UIA_ClickablePointPropertyId, &retVal);
  if (!FAILED(hr) && V_VT(&retVal) != VT_EMPTY)
  {
    // Get the SAFEARRAY from the VARIANT
    SAFEARRAY* pSafeArray = V_ARRAY(&retVal);

    // Get the number of elements in the array
    LONG lBound, uBound;
    SafeArrayGetLBound(pSafeArray, 1, &lBound);
    SafeArrayGetUBound(pSafeArray, 1, &uBound);
    LONG numElements = uBound - lBound + 1;

    if (numElements != 2)
      while (1);
    //clickablePoint

    // Access the data in the array
    double* pData = nullptr;
    SafeArrayAccessData(pSafeArray, (void**)&pData);

    clickablePoint->X = (POS_TYPE)pData[0];
    clickablePoint->Y = (POS_TYPE)pData[1];

    // Release the array data
    SafeArrayUnaccessData(pSafeArray);
    VariantClear(&retVal);
  }
}

void Fill_VT_I4__VT_ARRAY(VARIANT* pSrc, std::vector<int32_t>& out)
{
  if (V_VT(pSrc) == VT_EMPTY)
  {
    return;
  }
  SAFEARRAY* pSafeArray = V_ARRAY(pSrc);
  int32_t* pData = nullptr;
  SafeArrayAccessData(pSafeArray, (void**)&pData);
  UINT _dim = SafeArrayGetDim(pSafeArray);
  if (_dim != 1)
  {
    while (1)
    {
      perror("Fill_VT_I4__VT_ARRAY");
    }
  }

  //for (UINT dim = 0; dim < _dim; dim++)
  UINT dim = 0;
  {
    LONG low_bound; SafeArrayGetLBound(pSafeArray, dim+1, &low_bound);
    LONG up_bound; SafeArrayGetUBound(pSafeArray, dim + 1, &up_bound);
    for (LONG index = low_bound; index < up_bound; index++)
    {
      out.push_back(pData[index]);
    }
  }
  out.shrink_to_fit();

  // Release the array data
  SafeArrayUnaccessData(pSafeArray);
}
void FillRuntimeId(IUIAutomationElement* element, std::vector<int32_t>& out)
{
  VARIANT retVal;
  HRESULT hr = element->GetCurrentPropertyValue(UIA_RuntimeIdPropertyId, &retVal);
  if (!FAILED(hr))
  {
    Fill_VT_I4__VT_ARRAY(&retVal, out);
  }
  VariantClear(&retVal);
}

bool Fill_VT_R8__VT_ARRAY(double* pDst, VARIANT* pSrc, size_t preAllocLength)
{
  if (V_VT(pSrc) == (VT_R8 | VT_ARRAY))
  {
    SAFEARRAY* pSafeArray = V_ARRAY(pSrc);
    double* pData = nullptr;
    SafeArrayAccessData(pSafeArray, (void**)&pData);
    UINT dims = SafeArrayGetDim(pSafeArray);
    int indexDst = 0;
    for (UINT dim = 0; dim < dims; dim++)
    {
      LONG boundUp; LONG boundDown;
      SafeArrayGetUBound(pSafeArray, dim+1, &boundUp);
      SafeArrayGetLBound(pSafeArray, dim+1, &boundDown);
      for (int indexSrc = boundDown; indexSrc < boundUp && preAllocLength-- > 0; indexSrc++)
      {
        pDst[indexDst++] = (POS_TYPE)pData[indexSrc];
      }
    }
    SafeArrayUnaccessData(pSafeArray);

  }
  VariantClear(pSrc);

  return true;
}

void fill_bounding_rect_by_variant(BoundingRect* pDst, VARIANT* pSrc)
{
  if (V_VT(pSrc) == (VT_R8 | VT_ARRAY))
  {
    SAFEARRAY* pSafeArray = V_ARRAY(pSrc);
    double* pData = nullptr;
    SafeArrayAccessData(pSafeArray, (void**)&pData);

    if ((pData[0] != pData[1]) || (pData[1] != pData[2]) || (pData[2] != pData[3]) || (pData[3] != 0))
    {
      pDst->left = (POS_TYPE)pData[0];
      pDst->top = (POS_TYPE)pData[1];
      pDst->bottom = (POS_TYPE)pData[2];
      pDst->right = (POS_TYPE)pData[3];
      pDst->isValid = true;
    }

    // Release the array data
    SafeArrayUnaccessData(pSafeArray);
    VariantClear(pSrc);
  }
}

void Fill_VT_I4(int32_t* out, VARIANT* pSrc)
{
  if (V_VT(pSrc) == VT_I4)
  {
    *out = V_I4(pSrc);
  }
}

void FillPropertyId(IUIAutomationElement* element, LONG* propertId)
{
  VARIANT varControlType;
  HRESULT hr = element->GetCachedPropertyValue(UIA_ControlTypePropertyId, &varControlType);
  *propertId = varControlType.lVal;
  //if (SUCCEEDED(hr))
  //{
  //  std::wcout << L"Element " << i + 1 << L" - Cached Control Type: " << varControlType.lVal << std::endl;
  //}
  //else
  //{
  //  std::cerr << "Failed to get cached control type for element " << i + 1 << ": " << std::hex << hr << std::endl;
  //}
}

void FillName(IUIAutomationElement* element, std::string name)
{
  BSTR _name;
  HRESULT hr = element->get_CachedName(&_name);
  if (true)
  {
    if (SUCCEEDED(hr))
    {
      name = _bstr_t(_name, true);
      name.shrink_to_fit();
      //std::wcout << L"Element " << i + 1 << L" - Cached Name: ";
      //wprintf(_name); std::wcout << std::endl;
      SysFreeString(_name);
    }
    else
    {
      //std::cerr << "Failed to get cached name for element " << i + 1 << ": " << std::hex << hr << std::endl;
    }
  }
}
void FillBoundingRect(IUIAutomationElement* element, BoundingRect* boundingRect)
{
  VARIANT retValCached;
  HRESULT hrCached = element->GetCachedPropertyValue(UIA_BoundingRectanglePropertyId, &retValCached);

  //RECT retVal2; HRESULT hr2 = element->get_CurrentBoundingRectangle(&retVal2);
  //RECT retVal3; HRESULT hr3 = element->get_CachedBoundingRectangle(&retVal3);
  if (!FAILED(hrCached))
  {
    cout << string("Found bounding in cache") << endl;
    fill_bounding_rect_by_variant(boundingRect, &retValCached);
  }
  else
  {
    cout << string("Not found bounding in cache") << endl;
    VARIANT retValNonCached;
    HRESULT hrNonCached = element->GetCurrentPropertyValue(UIA_BoundingRectanglePropertyId, &retValNonCached);
    if (!FAILED(hrNonCached))
    {
      fill_bounding_rect_by_variant(boundingRect, &retValNonCached);
    }
  }
}
//void Fill_BSTR_Property_2(std::string* pDst, VARIANT* pSrc)
//{
//  VARIANT retVal;
//  HRESULT hr = element->GetCurrentPropertyValue(PropertyId, &retVal);
//  if (!FAILED(hr))
//  {
//    if (retVal.vt == VT_EMPTY)
//    {
//      VariantClear(&retVal);
//    }
//    else
//    {
//      BSTR bstrValue = V_BSTR(&retVal);
//      if ((bstrValue != nullptr) && (SysStringLen(bstrValue) != 0))
//      {
//        static int ffff = 0;
//        if (PropertyId == UIA_HelpTextPropertyId)
//          ffff++;
//
//        //if (PropertyId == UIA_HelpTextPropertyId)
//        //  while (1) {} // DEBUG TO MANUALLY INSPECT IT
//        *pDst = bstrValue;
//      }
//      else
//      {
//        VariantClear(&retVal);
//      }
//    }
//  }
//}
// https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-control-pattern-propids
void FillElementData_by_property_id(DataPerControlType* pDst, long propertyId, VARIANT* pSrc)
{
  switch (propertyId)
  {
    case UIA_RuntimeIdPropertyId:             { Fill_VT_I4__VT_ARRAY(pSrc, pDst->RuntimeId); break; }
    case UIA_BoundingRectanglePropertyId:     { Fill_VT_R8__VT_ARRAY(&pDst->boundingRect.left, pSrc, 4); break; }// fill_bounding_rect_by_variant(&pDst->boundingRect, pSrc); break;
    case UIA_ProcessIdPropertyId:             { Fill_VT_I4(&pDst->processId, pSrc); break; }
    case UIA_ControlTypePropertyId:           { Fill_VT_I4(&pDst->controlType, pSrc); break; }
    case UIA_LocalizedControlTypePropertyId:  { Fill_string_from_VARIANT(&pDst->ControlTypeStr, pSrc); break; }
    case UIA_NamePropertyId:                  { Fill_string_from_VARIANT(&pDst->name, pSrc); break; }
    case UIA_AcceleratorKeyPropertyId:        { Fill_string_from_VARIANT(&pDst->acceleratorKey, pSrc); break; }
    case UIA_AccessKeyPropertyId:             { Fill_string_from_VARIANT(&pDst->accessPropertyId, pSrc); break; }
    case UIA_HasKeyboardFocusPropertyId:      { Fill_bool_from_VARIANT(&pDst->HasKeyboardFocus, pSrc); break; }
    case UIA_IsKeyboardFocusablePropertyId:   { Fill_bool_from_VARIANT(&pDst->IsKeyboardFocusable, pSrc); break; }
    case UIA_IsEnabledPropertyId:             { Fill_bool_from_VARIANT(&pDst->IsEnabled, pSrc); break; }
    case UIA_AutomationIdPropertyId:          { Fill_string_from_VARIANT(&pDst->automationID, pSrc); break; }
    case UIA_ClassNamePropertyId:             { Fill_string_from_VARIANT(&pDst->className, pSrc); break; }
    case UIA_HelpTextPropertyId:              { Fill_string_from_VARIANT(&pDst->HelpText, pSrc); break; }
    case UIA_ClickablePointPropertyId:        { Fill_VT_R8__VT_ARRAY(&pDst->clickablePoint.X, pSrc, 2); break; }
    case UIA_CulturePropertyId:               { Fill_VT_I4(&pDst->cultureId, pSrc); break; }
    case UIA_IsControlElementPropertyId:      { Fill_bool_from_VARIANT(&pDst->IsControlElement, pSrc); break; }
    case UIA_IsContentElementPropertyId:      { Fill_bool_from_VARIANT(&pDst->IsContentElement, pSrc); break; }
    //case UIA_LabeledByPropertyId:           Type:Unkmowm
    case UIA_IsPasswordPropertyId:            { Fill_bool_from_VARIANT(&pDst->IsPassword, pSrc); break; }
    case UIA_NativeWindowHandlePropertyId:    { Fill_bool_from_VARIANT(&pDst->IsNAtiveWindowHandle, pSrc); break; }
    case UIA_ItemTypePropertyId:              { Fill_string_from_VARIANT(&pDst->ItemType, pSrc); break; }
    case UIA_IsOffscreenPropertyId:           { Fill_bool_from_VARIANT(&pDst->IsOffscreen, pSrc); break; }
    case UIA_OrientationPropertyId:           { Fill_VT_I4(&pDst->Orientation, pSrc); break; }
    case UIA_FrameworkIdPropertyId:           { Fill_string_from_VARIANT(&pDst->FrameworkId, pSrc); break; }
    case UIA_IsRequiredForFormPropertyId:     { Fill_bool_from_VARIANT(&pDst->IsRequiredForForm, pSrc); break; }
    case UIA_ItemStatusPropertyId:            { Fill_string_from_VARIANT(&pDst->ItemStatus, pSrc); break; }
    case UIA_IsDockPatternAvailablePropertyId:{ Fill_bool_from_VARIANT(&pDst->IsDockPatternAvailable, pSrc); break; }
    case UIA_IsExpandCollapsePatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsExpandCollapsePatternAvailable, pSrc); break; }
    case UIA_IsGridItemPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsGridItemPatternAvailable, pSrc); break; }
    case UIA_IsGridPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsGridPatternAvailable, pSrc); break; }
    case UIA_IsInvokePatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsInvokePatternAvailable, pSrc); break; }
    case UIA_IsMultipleViewPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsMultipleViewPatternAvailable, pSrc); break; }
    case UIA_IsRangeValuePatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsRangeValuePatternAvailable, pSrc); break; }
    case UIA_IsScrollPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsScrollPatternAvailable, pSrc); break; }
    case UIA_IsScrollItemPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsScrollItemPatternAvailable, pSrc); break; }
    case UIA_IsSelectionItemPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsSelectionItemPatternAvailable, pSrc); break; }
    case UIA_IsSelectionPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsSelectionPatternAvailable, pSrc); break; }
    case UIA_IsTablePatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsTablePatternAvailable, pSrc); break; }
    case UIA_IsTableItemPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsTableItemPatternAvailable, pSrc); break; }
    case UIA_IsTextPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsTextPatternAvailable, pSrc); break; }
    case UIA_IsTogglePatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsTogglePatternAvailable, pSrc); break; }
    case UIA_IsTransformPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsTransformPatternAvailable, pSrc); break; }
    case UIA_IsValuePatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsValuePatternAvailable, pSrc); break; }
    case UIA_IsWindowPatternAvailablePropertyId: { Fill_bool_from_VARIANT(&pDst->IsWindowPatternAvailable, pSrc); break; }
    case UIA_ValueValuePropertyId:            { Fill_string_from_VARIANT(&pDst->ValueValue, pSrc); break; }
    case UIA_ValueIsReadOnlyPropertyId:     { Fill_bool_from_VARIANT(&pDst->IsWindowPatternAvailable, pSrc); break; }
    //case UIA_RangeValueValuePropertyId: Fill_VT_R8__VT_ARRAY
    default: break;
  }
}

DataPerControlType FillElementData(IUIAutomationElement* element)
{
  //CONTROLTYPEID controlType;
  DataPerControlType self;
  //HRESULT hr = element->get_CurrentControlType(&controlType);
  //if (!FAILED(hr))
  //{
    for (long propertyId = UIA_RuntimeIdPropertyId; propertyId <= UIA_IsDialogPropertyId; propertyId++)
    {
      VARIANT retVal;
      HRESULT hr = element->GetCachedPropertyValue(propertyId, &retVal);
      if (FAILED(hr))
      {
        VariantClear(&retVal);
        hr = element->GetCurrentPropertyValue(propertyId, &retVal);
        if (FAILED(hr))
        {
          VariantClear(&retVal);
          continue;
        }
      }

      FillElementData_by_property_id(&self, propertyId, &retVal);
      VariantClear(&retVal);
    }


    //FillBoundingRect(element, &self.boundingRect);
    //Fill_string_Property(element, &self.name, UIA_NamePropertyId); // Fill name
    //Fill_string_Property(element, &self.automationID, UIA_AutomationIdPropertyId);
    //self.controlType = controlType;
    //if (controlType == UIA_ButtonControlTypeId)
    //{
    //  Fill_BSTR_Property(element, &self.button.acceleratorKey, UIA_AcceleratorKeyPropertyId);
    //  Fill_BSTR_Property(element, &self.button.HelpText, UIA_HelpTextPropertyId);
    //  FillClickablePoint(element, &self.button.clickablePoint);
    //  self.button.IsContentElement = VARIANT_TRUE;
    //  self.button.IsControlElement = VARIANT_TRUE;
    //  Fill_BOOL_Property(element, &self.button.IsKeyboardFocusable, UIA_IsKeyboardFocusablePropertyId);
    //  Fill_BSTR_Property(element, &self.button.LocalizedControlType, UIA_LocalizedControlTypePropertyId);

    //}
    //else if (controlType == UIA_WindowControlTypeId)
    //{
    //  FillClickablePoint(element, &self.button.clickablePoint);
    //}
    //else if (controlType == UIA_PaneControlTypeId)
    //{

    //}
    //else
    //{
    //  static int ii = 0;
    //  ii++;
    //}
    //}
  
  return self;
}