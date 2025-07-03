#pragma once
// Header.h

#include <string>
#include <map>
#include <vector>
#include <UIAnimation.h>
#include <UIAutomationClient.h>
#include <atlbase.h>
#include <nlohmann/json.hpp>

//struct DataPerProperty_old
//{
//  std::string description;//Shortcut key combinations invoke an action. For example, CTRL+O is often used to invoke the Open file common dialog box.
//  std::string ToPrintGlobal;
//  bool isToPrint;
//  DataPerProperty_old(std::string _description, std::string _ToPrintGlobal, bool _isToPrint) : description(_description), ToPrintGlobal(_ToPrintGlobal), isToPrint(_isToPrint){}
//  DataPerProperty_old() {}
//};


typedef double POS_TYPE;
struct ClickablePoint
{
  POS_TYPE X;
  POS_TYPE Y;
  bool isValid;
  ClickablePoint() : isValid(false), X(0), Y(0) {}
};

//https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-supportbuttoncontroltype
//struct dataPerButton
//{

  //VT_UNKNOWN LabeledBy = NULL; //UIA_LabeledByPropertyId
//  dataPerButton() : clickablePoint() {}
//};
struct BoundingRect
{
  POS_TYPE left;
  POS_TYPE top;
  POS_TYPE bottom;
  POS_TYPE right;
  bool isValid;
  BoundingRect() : left(0), top(0), bottom(0), right(0), isValid(false){}
  std::vector<POS_TYPE> asVector() { return std::vector<POS_TYPE>{left, top, bottom, right}; }
};
struct DataPerControlType
{
  std::string name;                 // UIA_NamePropertyId
  std::string ControlTypeStr;       // UIA_LocalizedControlTypePropertyId
  CONTROLTYPEID controlType;        // UIA_ControlTypePropertyId
  BoundingRect boundingRect;        // UIA_BoundingRectanglePropertyId
  std::string automationID;           // UIA_AutomationIdPropertyId;
  int32_t processId;                // UIA_ProcessIdPropertyId
  int32_t cultureId;                // UIA_CulturePropertyId
  int32_t Orientation;              // UIA_OrientationPropertyId
  LONG propertyId;
  std::vector<int32_t> RuntimeId;   // UIA_RuntimeIdPropertyId
  std::string accessPropertyId;     // UIA_AccessKeyPropertyId
  std::string className;            // UIA_ClassNamePropertyId
  std::string acceleratorKey;            // UIA_AcceleratorKeyPropertyId
  std::string FrameworkId;               // UIA_FrameworkIdPropertyId
  std::string HelpText;                  // UIA_HelpTextPropertyId
  std::string ItemType;             // UIA_ItemTypePropertyId
  std::string ItemStatus;           // UIA_ItemStatusPropertyId
  std::string ValueValue;           // UIA_ValueValuePropertyId
  ClickablePoint clickablePoint;          //UIA_ClickablePointPropertyId
  bool IsContentElement;          //UIA_IsContentElementPropertyId
  bool IsControlElement;          //UIA_IsControlElementPropertyId
  bool HasKeyboardFocus;          // UIA_HasKeyboardFocusPropertyId
  bool IsKeyboardFocusable;       // UIA_IsKeyboardFocusablePropertyId
  bool IsPassword;                // UIA_IsPasswordPropertyId
  bool IsNAtiveWindowHandle;      // UIA_IsPasswordPropertyId
  bool IsRequiredForForm;         // UIA_IsRequiredForFormPropertyId
  bool IsEnabled;       // UIA_IsEnabledPropertyId
  bool IsOffscreen;     // UIA_IsOffscreenPropertyId
  bool IsDockPatternAvailable; //UIA_IsDockPatternAvailablePropertyId
  bool IsExpandCollapsePatternAvailable; // UIA_IsExpandCollapsePatternAvailablePropertyId
  bool IsGridItemPatternAvailable;        // UIA_IsGridItemPatternAvailablePropertyId
  bool IsGridPatternAvailable;            // UIA_IsGridPatternAvailablePropertyId
  bool IsInvokePatternAvailable;          // UIA_IsInvokePatternAvailablePropertyId
  bool IsMultipleViewPatternAvailable;    // UIA_IsMultipleViewPatternAvailablePropertyId
  bool IsRangeValuePatternAvailable;      // UIA_IsRangeValuePatternAvailablePropertyId
  bool IsScrollPatternAvailable;          // UIA_IsScrollPatternAvailablePropertyId
  bool IsScrollItemPatternAvailable;      // UIA_IsScrollItemPatternAvailablePropertyId
  bool IsSelectionItemPatternAvailable;   // UIA_IsSelectionItemPatternAvailablePropertyId
  bool IsSelectionPatternAvailable;     // UIA_IsSelectionPatternAvailablePropertyId
  bool IsTablePatternAvailable;         // UIA_IsTablePatternAvailablePropertyId
  bool IsTableItemPatternAvailable;     // UIA_IsTableItemPatternAvailablePropertyId
  bool IsTextPatternAvailable;          // UIA_IsTextPatternAvailablePropertyId
  bool IsTogglePatternAvailable;        // UIA_IsTogglePatternAvailablePropertyId
  bool IsTransformPatternAvailable;     // UIA_IsTransformPatternAvailablePropertyId
  bool IsValuePatternAvailable;         // UIA_IsValuePatternAvailablePropertyId
  bool IsWindowPatternAvailable;        // UIA_IsWindowPatternAvailablePropertyId
  bool ValueIsReadOnly;                 // UIA_ValueIsReadOnlyPropertyId
  std::string LocalizedControlType;       //UIA_LocalizedControlTypePropertyId
  //friend void to_json(nlohmann::ordered_json& dataOut, const DataPerControlType& dataIn);
  friend void to_json(nlohmann::json& dataOut, const DataPerControlType& dataIn);
  friend void to_json(nlohmann::ordered_json& dataOut, const DataPerControlType& dataIn);
  friend void from_json(const nlohmann::json& dataIn, DataPerControlType& dataOut)
  {
    dataIn.at("name").get_to(dataOut.name);
    dataIn.at("ControlTypeStr").get_to(dataOut.ControlTypeStr);
    dataIn.at("boundingRect").get_to(dataOut.boundingRect.left);
    dataIn.at("automationID").get_to(dataOut.automationID);
  }
};


struct ControlLevel
{
  DataPerControlType self;
  std::vector<ControlLevel> children;
  ControlLevel* parent;
  ControlLevel(ControlLevel* _parent = NULL) : parent(_parent) {}
  friend void to_json(nlohmann::ordered_json& dataOut, const ControlLevel& dataIn);
  friend void to_json(nlohmann::json& dataOut, const ControlLevel& dataIn);
  friend void from_json(const nlohmann::json& dataIn, ControlLevel& dataOut)
  {
    dataIn.at("self").get_to(dataOut.self);
    dataIn.at("children").get_to(dataOut.children);
  }
};


struct ListDescendantsStackEntry
{
  IUIAutomationElement* pElem;
  ControlLevel* pParent;
  ListDescendantsStackEntry(IUIAutomationElement* _pElem, ControlLevel* _pParent) : pElem(_pElem), pParent(_pParent) {}
};


struct ListDescendantsVectorEntry
{
  IUIAutomationElement* pElem;
  ControlLevel* pParent;
  ListDescendantsVectorEntry(IUIAutomationElement* _pElem, ControlLevel* _pParent) : pElem(_pElem), pParent(_pParent) {}
};
int Click_one(void);
void LaunchCalculator(void);
int GetCalculatorElement(IUIAutomationElement** pCalculatorWindow, IUIAutomationElement* pRootElement);
void CleanUg_pAutomation(IUIAutomationElement** pRootElement);
int PrepareAutomationEnvironment(IUIAutomationElement** pRootElement);
void Fill_BOOL_Property(IUIAutomationElement* element, VARIANT_BOOL* pDst, long PropertyId);
void Fill_BSTR_Property(IUIAutomationElement* element, BSTR* pDst, long PropertyId);
void FillClickablePoint(IUIAutomationElement* element, ClickablePoint* clickablePoint);
void FillBoundingRect(IUIAutomationElement* element, BoundingRect* boundingRect);
void FillName(IUIAutomationElement* element, std::string name);
void FillPropertyId(IUIAutomationElement* element, LONG* pPropertyId);
DataPerControlType FillElementData(IUIAutomationElement* element);
void ListDescendantsIterative(IUIAutomationElement* pRootElement);
int FocusChange();
int intevent_location_changed();

// Caching
HRESULT CacheAndRetrieveMultipleProperties(void);
HRESULT CacheAndRetrieveMultiplePropertiesMine(IUIAutomationElement* pItem);
HRESULT CacheAndRetrieveMultiplePropertiesMine(IUIAutomationElement* pInElement, DataPerControlType* pOut);

HRESULT CacheAndRetrievePropertiesSimple(void);

void FillRuntimeId(IUIAutomationElement* element, std::vector<int32_t>& out);

int GlobalHookKeyStrokes();
int GlobalHookKeyStrokesSpecificHandle(UIA_HWND* handle);
int notification2();
int TestChangedEvent();
int TextPattern();

int EventHandlerTest(void);

int json_sample(void);
void ProcessJson(const ControlLevel& desktop);

IUIAutomationElement* GetContainingWindow(IUIAutomationElement* pChild);

int TakeScreenshot(void);
int TakeScreenshot_old(void);
bool CaptureActiveWindow(LPCWSTR filePath);
int TakeScreenshotActiveWindow(void);
int EVENT_OBJECT_VALUECHANGE_();
int SetVaueNotepad();
void DescendantsIterative();

void Capture_user();

std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen);
void FillElementData_by_property_id(DataPerControlType* pDst, long propertyId, VARIANT* pSrc);
DWORD WINAPI MtaThread(LPVOID lpParam);

extern IUIAutomation* _automation;