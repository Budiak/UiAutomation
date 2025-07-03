#include <iostream>
#include <UIAutomation.h>

#include"Header.h"
#include <comdef.h>
#include <thread>

HRESULT CacheAndRetrieveMultipleProperties()
{
  // Initialize
  IUIAutomation* g_pAutomation = NULL;
  IUIAutomationCacheRequest* pCacheRequest = NULL;
  IUIAutomationElement* pRootElement = NULL;
  IUIAutomationCondition* pCondition = NULL;
  IUIAutomationElementArray* pElementArray = NULL;
  
  HRESULT hr = CoInitialize(NULL);
  if (FAILED(hr)) {std::cerr << "CoInitialize failed: " << std::hex << hr << std::endl; goto CLEAN_RESOURCES;}

  // Create the UI Automation object
  if (g_pAutomation == nullptr) {
    hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&g_pAutomation);
    if (FAILED(hr)) {std::cerr << "CoCreateInstance failed: " << std::hex << hr << std::endl; goto CLEAN_RESOURCES;}}

  // Create a cache request
  hr = g_pAutomation->CreateCacheRequest(&pCacheRequest);
  if (FAILED(hr)) {std::cerr << "CreateCacheRequest failed: " << std::hex << hr << std::endl; goto CLEAN_RESOURCES;}

  // Add properties to cache
  pCacheRequest->AddProperty(UIA_NamePropertyId);
  pCacheRequest->AddProperty(UIA_ControlTypePropertyId);

  // Retrieve the root element (desktop)
  hr = g_pAutomation->GetRootElement(&pRootElement);
  if (FAILED(hr)) {std::cerr << "GetRootElement failed: " << std::hex << hr << std::endl; goto CLEAN_RESOURCES;}

  // Use a true condition to find all elements
  hr = g_pAutomation->CreateTrueCondition(&pCondition);
  if (FAILED(hr)) {std::cerr << "CreateTrueCondition failed: " << std::hex << hr << std::endl; goto CLEAN_RESOURCES;}

  // Find all children of the desktop with caching
  hr = pRootElement->FindAllBuildCache(TreeScope_Descendants, pCondition, pCacheRequest, &pElementArray); //TreeScope_Subtree //TreeScope_Children
  if (SUCCEEDED(hr) && pElementArray)
  {
    int length = 0;
    hr = pElementArray->get_Length(&length);
    if (SUCCEEDED(hr)) {
      std::wcout << L"Number of elements found: " << length << std::endl;

      // Iterate through the elements and print cached properties
      for (int i = 0; i < length; ++i) {
        IUIAutomationElement* pElement = NULL;
        hr = pElementArray->GetElement(i, &pElement);
        if (SUCCEEDED(hr) && pElement) {
          // Retrieve cached properties
          BSTR name;
          VARIANT varControlType;

          hr = pElement->get_CachedName(&name);
          if (false)
          {
            if (SUCCEEDED(hr)) {
              std::wcout << L"Element " << i + 1 << L" - Cached Name: ";
              wprintf(name); std::wcout << std::endl;

            }
            else {
              std::cerr << "Failed to get cached name for element " << i + 1 << ": " << std::hex << hr << std::endl;
            }
          }
          SysFreeString(name);
          hr = pElement->GetCachedPropertyValue(UIA_ControlTypePropertyId, &varControlType);
          if (SUCCEEDED(hr)) {
            std::wcout << L"Element " << i + 1 << L" - Cached Control Type: " << varControlType.lVal << std::endl;
          }
          else {
            std::cerr << "Failed to get cached control type for element " << i + 1 << ": " << std::hex << hr << std::endl;
          }

          pElement->Release();  // Release the element after use
        }
      }
    }
    else {
      std::cerr << "Failed to get element array length: " << std::hex << hr << std::endl;
    }

    pElementArray->Release();  // Release the element array after use
  }
  else {
    std::cerr << "Failed to find child elements: " << std::hex << hr << std::endl;
  }

CLEAN_RESOURCES:
  // Release COM objects
  if (!pCondition)    pCondition->Release();    pCondition = NULL;
  if (!pRootElement)  pRootElement->Release();  pRootElement = NULL;
  if (!pCacheRequest) pCacheRequest->Release(); pCacheRequest = NULL;
  if (!g_pAutomation)   g_pAutomation->Release();   g_pAutomation = NULL;

  CoUninitialize();

  return hr;
}

HRESULT CacheAndRetrieveMultiplePropertiesMine(IUIAutomationElement* pInElement, DataPerControlType* pOut)
{
  IUIAutomationCacheRequest* pCacheRequest = NULL;
  IUIAutomationCondition* pCondition = NULL;
  IUIAutomationElementArray* pElementArray = NULL;

  // Create a cache request
  HRESULT hr = _automation->CreateCacheRequest(&pCacheRequest);
  if (FAILED(hr)) { std::cerr << "CreateCacheRequest failed: " << std::hex << hr << std::endl; goto CLEAN_RESOURCES; }

  // Add properties to cache
  for (long propertyId = UIA_RuntimeIdPropertyId; propertyId <= UIA_IsDialogPropertyId ; propertyId++)
  {
    pCacheRequest->AddProperty(propertyId);
  }
  //pCacheRequest->AddPattern()

  // Use a true condition to find all elements
  hr = _automation->CreateTrueCondition(&pCondition);
  if (FAILED(hr)) { std::cerr << "CreateTrueCondition failed: " << std::hex << hr << std::endl; goto CLEAN_RESOURCES; }

  // Find all children of the desktop with caching
  hr = pInElement->FindAllBuildCache(TreeScope_Element, pCondition, pCacheRequest, &pElementArray); //TreeScope_Subtree //TreeScope_Descendants// TreeScope_Children
  if (SUCCEEDED(hr) && pElementArray)
  {
    int length = 0;
    hr = pElementArray->get_Length(&length);
    if (SUCCEEDED(hr)) {
      //std::wcout << L"Number of elements found: " << length << std::endl;

      // Iterate through the elements and print cached properties
      for (int i = 0; i < length; ++i)
      {
        IUIAutomationElement* pElement = NULL;
        hr = pElementArray->GetElement(i, &pElement);
        if (SUCCEEDED(hr) && pElement)
        {
          *pOut = FillElementData(pElement);
          // Retrieve cached properties
          //FillElementData_by_property_id(DataPerControlType * pDst, long propertyId, VARIANT * pSrc)
          //FillBoundingRect(pElement, &pOut->boundingRect);
          //FillName(pElement, pOut->name);
          //FillPropertyId(pElement, &pOut->propertyId);


          pElement->Release();  // Release the element after use
        }
      }
    }
    else {
      std::cerr << "Failed to get element array length: " << std::hex << hr << std::endl;
    }

    pElementArray->Release();  // Release the element array after use
  }
  else {
    std::cerr << "Failed to find child elements: " << std::hex << hr << std::endl;
  }

CLEAN_RESOURCES:
  // Release COM objects
  if (!pCondition)    pCondition->Release();    pCondition = NULL;
  if (!pInElement)  pInElement->Release();  pInElement = NULL;
  if (!pCacheRequest) pCacheRequest->Release(); pCacheRequest = NULL;
  //if (!g_pAutomation)   g_pAutomation->Release();   g_pAutomation = NULL;

  CoUninitialize();

  return hr;
}

// Function to simulate work on the COM MTA thread
DWORD WINAPI MtaThread(LPVOID lpParam)
{
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (FAILED(hr))
  {
    std::cerr << "Failed to initialize COM in MTA: " << std::hex << hr << std::endl;
    return 1;
  }

  // Now the thread is in the MTA, you can perform COM-related work here
  std::cout << "COM initialized in MTA." << std::endl;


  std::thread t_Capture_user(Capture_user);
  t_Capture_user.join();


  // Uninitialize COM
  CoUninitialize();
  std::cout << "COM uninitialized in MTA." << std::endl;

  return 0;
}

HRESULT CacheAndRetrievePropertiesSimple()
{
  HRESULT hr = CoInitialize(NULL);
  IUIAutomation* g_pAutomation = NULL;
  IUIAutomationCacheRequest* pCacheRequest = NULL;
  IUIAutomationElement* pRootElement = NULL;
  IUIAutomationCondition* pCondition = NULL;
  IUIAutomationElement* pElement = NULL;

  if (FAILED(hr)) {
    std::cerr << "CoInitialize failed: " << std::hex << hr << std::endl;
    goto CLEAN_RESOURCES;
  }

  // Create the UI Automation object
  hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&g_pAutomation);
  if (FAILED(hr)) {
    std::cerr << "CoCreateInstance failed: " << std::hex << hr << std::endl;
    goto CLEAN_RESOURCES;
  }

  // Create a cache request
  hr = g_pAutomation->CreateCacheRequest(&pCacheRequest);
  if (FAILED(hr)) {
    std::cerr << "CreateCacheRequest failed: " << std::hex << hr << std::endl;
    goto CLEAN_RESOURCES;
  }

  // Add properties to cache
  pCacheRequest->AddProperty(UIA_NamePropertyId);
  pCacheRequest->AddProperty(UIA_ControlTypePropertyId);

  // Retrieve the root element (desktop)
  hr = g_pAutomation->GetRootElement(&pRootElement);
  if (FAILED(hr)) {
    std::cerr << "GetRootElement failed: " << std::hex << hr << std::endl;
    goto CLEAN_RESOURCES;
  }

  // Use a true condition to find the first element
  hr = g_pAutomation->CreateTrueCondition(&pCondition);
  if (FAILED(hr)) {
    std::cerr << "CreateTrueCondition failed: " << std::hex << hr << std::endl;
    goto CLEAN_RESOURCES;
  }

  // Find the first child of the desktop with caching
  hr = pRootElement->FindFirstBuildCache(TreeScope_Children, pCondition, pCacheRequest, &pElement);
  if (SUCCEEDED(hr) && pElement) {
    // Retrieve cached properties
    BSTR name;
    VARIANT varControlType;
    pCacheRequest->Release();

    hr = pElement->get_CachedName(&name);
    if (SUCCEEDED(hr)) {
      std::wcout << L"Cached Name: " << name << std::endl;
      SysFreeString(name);
    }
    else {
      std::cerr << "Failed to get cached name: " << std::hex << hr << std::endl;
    }

    hr = pElement->GetCachedPropertyValue(UIA_ControlTypePropertyId, &varControlType);
    if (SUCCEEDED(hr)) {
      std::wcout << L"Cached Control Type: " << varControlType.lVal << std::endl;
    }
    else {
      std::cerr << "Failed to get cached control type: " << std::hex << hr << std::endl;
    }

    // Clean up
    pElement->Release();
  }
  else {
    std::cerr << "Failed to find first child element: " << std::hex << hr << std::endl;
  }

CLEAN_RESOURCES:
  // Release COM objects
  if (!pCondition)    pCondition->Release();    pCondition = NULL;
  if (!pRootElement)  pRootElement->Release();  pRootElement = NULL;
  if (!pCacheRequest) pCacheRequest->Release(); pCacheRequest = NULL;
  if (!g_pAutomation)   g_pAutomation->Release();   g_pAutomation = NULL;

  CoUninitialize();

  return hr;
}