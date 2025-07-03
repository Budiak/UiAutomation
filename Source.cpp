#include <Windows.h>
#include <stdio.h>
#include <UIAutomation.h>
#include <iostream>
#include <comutil.h>
#include <map>
#include <vector>
#include <string>
#include "Header.h"
#include <stack>
#include <atlbase.h>
#include <fstream>
#include <Winuser.h>
#include <thread>


#include "log.h"
#include"logmanager.h"
// https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-howto-topics-for-uiautomation-clients
// https://github.com/microsoftarchive/msdn-code-gallery-microsoft/blob/411c271e537727d737a53fa2cbe99eaecac00cc0/Official%20Windows%20Platform%20Sample/UI%20Automation%20document%20content%20client%20sample/%5BC%2B%2B%5D-UI%20Automation%20document%20content%20client%20sample/C%2B%2B/UiaDocumentClient.cpp

IUIAutomation* _automation;

using namespace std;



LOGGER::managers::LogManager mLogManager;

bool Init()
{
  HRESULT hr = CoInitialize(NULL);
  if (FAILED(hr))
  {
    wprintf(L"CoInitialize failed, HR:0x%08x\n", hr);
    return false;
  }
  else
  {
    hr = CoCreateInstance(__uuidof(CUIAutomation8), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_automation));
    if (FAILED(hr))
    {
      wprintf(L"Failed to create a CUIAutomation8, HR: 0x%08x\n", hr);
      return false;
    }
  }

  mLogManager.Initialize();

  return true;
}

int mainGemini();

int main(int argc, char* argv[])
{
  mainGemini();

  //DescendantsIterative();
  //return 0;

    if (!Init()) return -1;
  //LOGGER_ERROR("sss");
  //LOGGER_INFO("YYY");

    // Create a thread for MTA
  //HANDLE hThread = CreateThread(
  //  NULL,                // Default security attributes
  //  0,                   // Default stack size
  //  MtaThread,           // Thread function
  //  NULL,                // Argument to pass to thread function
  //  0,                   // Default creation flags
  //  NULL);               // Thread identifier

  //if (hThread == NULL)
  //{
  //  std::cerr << "Failed to create MTA thread." << std::endl;
  //  return 1;
  //}

  mLogManager.Shutdown();

  // Wait for the thread to finish
  //WaitForSingleObject(hThread, INFINITE);
  //CloseHandle(hThread);

  //return 0;
  //return 0;
  // Fill json of clicked element
  // 

  //TakeScreenshot();
  //TakeScreenshot_old();
  //TakeScreenshotActiveWindow();
  //EventHandlerTest(); //Handles the 20K IDs
  //HRESULT retVal;
  //json_sample();
  //TextPattern();
  //TestChangedEvent();
  //notification2();
  //GlobalHookKeyStrokes();
  //intevent_location_changed();
  //SetVaueNotepad();
  //EVENT_OBJECT_VALUECHANGE_();
  //FocusChange();
  //LaunchCalculator();
  //Click_one();
  //GoToParent();
  //DescendantsIterative();
  
  return 0;
}
