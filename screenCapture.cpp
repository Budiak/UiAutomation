#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

bool SaveBitmapToFile(HBITMAP hBitmap, HDC hDC, LPCWSTR filePath) {
  BITMAP bmp;
  GetObject(hBitmap, sizeof(BITMAP), &bmp);

  BITMAPFILEHEADER bmfHeader = { 0 };
  BITMAPINFOHEADER bi = { 0 };

  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = bmp.bmWidth;
  bi.biHeight = -bmp.bmHeight; // Negative to ensure correct orientation
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;

  DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

  // Allocate memory for the bitmap data
  HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
  if (!hDIB) {
    std::wcerr << L"Failed to allocate memory for bitmap data.\n";
    return false;
  }

  char* lpbitmap = (char*)GlobalLock(hDIB);

  // Get the bitmap data
  BITMAPINFO biInfo = { 0 };
  biInfo.bmiHeader = bi;
  if (!GetDIBits(hDC, hBitmap, 0, (UINT)bmp.bmHeight, lpbitmap, &biInfo, DIB_RGB_COLORS)) {
    std::wcerr << L"Failed to get bitmap data.\n";
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    return false;
  }

  // Write the data to a file
  HANDLE hFile = CreateFile(filePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (!hFile || hFile == INVALID_HANDLE_VALUE) {
    std::wcerr << L"Failed to create file.\n";
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    return false;
  }

  DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  bmfHeader.bfSize = dwSizeofDIB;
  bmfHeader.bfType = 0x4D42; // BM

  DWORD dwBytesWritten;
  WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, nullptr);
  WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, nullptr);
  WriteFile(hFile, lpbitmap, dwBmpSize, &dwBytesWritten, nullptr);

  // Clean up
  CloseHandle(hFile);
  GlobalUnlock(hDIB);
  GlobalFree(hDIB);

  return true;
}

// Capture the active window
bool CaptureActiveWindow(LPCWSTR filePath) {
  HWND hwnd = GetForegroundWindow(); // Get the active window
  if (!hwnd) {
    std::wcerr << L"Failed to get active window.\n";
    return false;
  }

  RECT rc;
  GetWindowRect(hwnd, &rc);

  HDC hdcWindow = GetDC(hwnd);
  HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

  int width = rc.right - rc.left;
  int height = rc.bottom - rc.top;

  HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);

  if (!hbmScreen) {
    std::wcerr << L"Failed to create compatible bitmap.\n";
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);
    return false;
  }

  SelectObject(hdcMemDC, hbmScreen);

  if (!BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY)) {
    std::wcerr << L"BitBlt failed.\n";
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);
    return false;
  }

  bool result = SaveBitmapToFile(hbmScreen, hdcMemDC, filePath);

  DeleteObject(hbmScreen);
  DeleteDC(hdcMemDC);
  ReleaseDC(hwnd, hdcWindow);

  return result;
}

int TakeScreenshotActiveWindow(void)
{
  LPCWSTR filePath = L"active_window.bmp";
  if (CaptureActiveWindow(filePath)) {
    std::wcout << L"Captured active window successfully: " << filePath << L"\n";
  }
  else {
    std::wcerr << L"Failed to capture active window.\n";
  }
  return 0;
}

void TakeScreenshot(const char* fileName)
{
  // Get the device context of the screen
  HDC hScreenDC = GetDC(NULL);
  HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

  // Get screen dimensions
  HDC tmp = GetWindowDC(GetForegroundWindow());
  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);

  // Create a compatible bitmap from the screen DC
  HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);

  // Select the compatible bitmap into the memory DC
  HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);

  // Copy the screen content to the bitmap
  BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

  // Restore the original bitmap in the memory DC
  SelectObject(hMemoryDC, hOldBitmap);

  // Save the bitmap to a file
  BITMAPFILEHEADER bmfHeader;
  BITMAPINFOHEADER bi;

  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = screenWidth;
  bi.biHeight = -screenHeight; // Negative to indicate top-down DIB
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  DWORD dwBmpSize = ((screenWidth * bi.biBitCount + 31) / 32) * 4 * screenHeight;

  HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
  char* lpbitmap = (char*)GlobalLock(hDIB);

  GetDIBits(hMemoryDC, hBitmap, 0, (UINT)screenHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

  HANDLE hFile = CreateFileA(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  bmfHeader.bfSize = dwSizeofDIB;
  bmfHeader.bfType = 0x4D42; // BM

  DWORD dwBytesWritten;
  WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
  WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
  WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

  // Clean up
  CloseHandle(hFile);
  GlobalUnlock(hDIB);
  GlobalFree(hDIB);
  DeleteObject(hBitmap);
  DeleteDC(hMemoryDC);
  ReleaseDC(NULL, hScreenDC);

  printf("Screenshot saved to %s\n", fileName);
}

int TakeScreenshot_old(void)
{
   //printf(" The local time is: %02d:%02d %d.%04d\n", lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
  {
    SYSTEMTIME lt;
    GetLocalTime(&lt);
    std::string filename_tmp;
    filename_tmp += "screenshot_" + to_string(lt.wHour) + std::string("_") + to_string(lt.wMinute) + std::string("_") + to_string(lt.wSecond) + std::string("_") + to_string(lt.wSecond) + ".bmp";
    TakeScreenshot(filename_tmp.c_str());
  }
  {
    SYSTEMTIME lt;
    GetLocalTime(&lt);
    std::string filename_tmp;
    filename_tmp += "screenshot_" + to_string(lt.wHour) + std::string("_") + to_string(lt.wMinute) + std::string("_") + to_string(lt.wSecond) + std::string("_") + to_string(lt.wSecond) + ".bmp";
    TakeScreenshot(filename_tmp.c_str());
  }
  //TakeScreenshot("screenshot2.bmp");
  /*const char* filename = "C:\\Users\\bgano\\Documents\\GitHub\\UIAutomation_1\\screenshot1.bmp";
  HBITMAP bmp = (HBITMAP)LoadImageA(NULL, filename,
    IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
  if (bmp == NULL)
  {
    DWORD lastError = GetLastError();
    return 1;
  }

  
  HBITMAP hBitmap = (HBITMAP)LoadImage(
    NULL,
    L"C:\\Users\\bgano\\Documents\\GitHub\\UIAutomation_1\\screenshot1.bmp",
    IMAGE_BITMAP,
    0,
    0,
    LR_LOADFROMFILE
  );
  HBITMAP hBitmap2 = (HBITMAP)LoadImage(
    NULL,
    L"screenshot2.bmp",
    IMAGE_BITMAP,
    0,
    0,
    LR_LOADFROMFILE
  );gg
  */
  return 0;
}

/* Globals */
int ScreenX = 0;
int ScreenY = 0;
BYTE* ScreenData = 0;

void ScreenCap()
{
  HDC hScreen = GetDC(GetForegroundWindow());// NULL);
  ScreenX = GetDeviceCaps(hScreen, HORZRES);
  ScreenY = GetDeviceCaps(hScreen, VERTRES);

  HDC hdcMem = CreateCompatibleDC(hScreen);
  HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, ScreenX, ScreenY);
  HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);
  BitBlt(hdcMem, 0, 0, ScreenX, ScreenY, hScreen, 0, 0, SRCCOPY);
  SelectObject(hdcMem, hOld);

  BITMAPINFOHEADER bmi = { 0 };
  bmi.biSize = sizeof(BITMAPINFOHEADER);
  bmi.biPlanes = 1;
  bmi.biBitCount = 32;
  bmi.biWidth = ScreenX;
  bmi.biHeight = -ScreenY;
  bmi.biCompression = BI_RGB;
  bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;

  if (ScreenData)
    free(ScreenData);
  ScreenData = (BYTE*)malloc(4 * ScreenX * ScreenY);

  GetDIBits(hdcMem, hBitmap, 0, ScreenY, ScreenData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

  ReleaseDC(GetDesktopWindow(), hScreen);
  DeleteDC(hdcMem);
  DeleteObject(hBitmap);
}

inline int PosB(int x, int y)
{
  return ScreenData[4 * ((y * ScreenX) + x)];
}

inline int PosG(int x, int y)
{
  return ScreenData[4 * ((y * ScreenX) + x) + 1];
}

inline int PosR(int x, int y)
{
  return ScreenData[4 * ((y * ScreenX) + x) + 2];
}

bool ButtonPress(int Key)
{
  bool button_pressed = false;

  while (GetAsyncKeyState(Key))
    button_pressed = true;

  return button_pressed;
}


int TakeScreenshot()
{
  //while (true)
  {
    //if (ButtonPress(VK_SPACE))
    //{

    //  // Print out current cursor position
    //  POINT p;
    //  GetCursorPos(&p);
    //  printf("X:%d Y:%d \n", p.x, p.y);
    //  // Print out RGB value at that position
    //  std::cout << "Bitmap: r: " << PosR(p.x, p.y) << " g: " << PosG(p.x, p.y) << " b: " << PosB(p.x, p.y) << "\n";

    //}
    //else if (ButtonPress(VK_ESCAPE))
    //{
    //  printf("Quit\n");
    //  break;
    //}
    //else if (ButtonPress(VK_SHIFT))
    {
      {
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        std::string filename_tmp;
        filename_tmp += "screenshot_" + to_string(lt.wHour) + std::string("_") + to_string(lt.wMinute) + std::string("_") + to_string(lt.wSecond) + std::string("_") + to_string(lt.wSecond) + ".bmp";
        TakeScreenshot(filename_tmp.c_str());
      }
      //Sleep(5);
      //printf("Captured\n");
    }
  }
  
  system("PAUSE");
  return 0;
}