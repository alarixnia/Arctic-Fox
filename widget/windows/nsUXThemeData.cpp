/* vim: se cin sw=2 ts=2 et : */
/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/ArrayUtils.h"
#include "mozilla/WindowsVersion.h"

#include "nsUXThemeData.h"
#include "nsDebug.h"
#include "nsToolkit.h"
#include "nsUXThemeConstants.h"

using namespace mozilla;
using namespace mozilla::widget;

const wchar_t
nsUXThemeData::kThemeLibraryName[] = L"uxtheme.dll";

HANDLE
nsUXThemeData::sThemes[eUXNumClasses];

HMODULE
nsUXThemeData::sThemeDLL = nullptr;

bool
nsUXThemeData::sFlatMenus = false;

bool nsUXThemeData::sTitlebarInfoPopulatedAero = false;
bool nsUXThemeData::sTitlebarInfoPopulatedThemed = false;
SIZE nsUXThemeData::sCommandButtons[4];

void
nsUXThemeData::Teardown() {
  Invalidate();
  if(sThemeDLL)
    FreeLibrary(sThemeDLL);
}

void
nsUXThemeData::Initialize()
{
  ::ZeroMemory(sThemes, sizeof(sThemes));
  NS_ASSERTION(!sThemeDLL, "nsUXThemeData being initialized twice!");

  CheckForCompositor(true);
  Invalidate();
}

void
nsUXThemeData::Invalidate() {
  for(int i = 0; i < eUXNumClasses; i++) {
    if(sThemes[i]) {
      CloseThemeData(sThemes[i]);
      sThemes[i] = nullptr;
    }
  }
  BOOL useFlat = FALSE;
  sFlatMenus = ::SystemParametersInfo(SPI_GETFLATMENU, 0, &useFlat, 0) ?
                   useFlat : false;
}

HANDLE
nsUXThemeData::GetTheme(nsUXThemeClass cls) {
  NS_ASSERTION(cls < eUXNumClasses, "Invalid theme class!");
  if(!sThemes[cls])
  {
    sThemes[cls] = OpenThemeData(nullptr, GetClassName(cls));
  }
  return sThemes[cls];
}

HMODULE
nsUXThemeData::GetThemeDLL() {
  if (!sThemeDLL)
    sThemeDLL = ::LoadLibraryW(kThemeLibraryName);
  return sThemeDLL;
}

const wchar_t *nsUXThemeData::GetClassName(nsUXThemeClass cls) {
  switch(cls) {
    case eUXButton:
      return L"Button";
    case eUXEdit:
      return L"Edit";
    case eUXTooltip:
      return L"Tooltip";
    case eUXRebar:
      return L"Rebar";
    case eUXMediaRebar:
      return L"Media::Rebar";
    case eUXCommunicationsRebar:
      return L"Communications::Rebar";
    case eUXBrowserTabBarRebar:
      return L"BrowserTabBar::Rebar";
    case eUXToolbar:
      return L"Toolbar";
    case eUXMediaToolbar:
      return L"Media::Toolbar";
    case eUXCommunicationsToolbar:
      return L"Communications::Toolbar";
    case eUXProgress:
      return L"Progress";
    case eUXTab:
      return L"Tab";
    case eUXScrollbar:
      return L"Scrollbar";
    case eUXTrackbar:
      return L"Trackbar";
    case eUXSpin:
      return L"Spin";
    case eUXStatus:
      return L"Status";
    case eUXCombobox:
      return L"Combobox";
    case eUXHeader:
      return L"Header";
    case eUXListview:
      return L"Listview";
    case eUXMenu:
      return L"Menu";
    case eUXWindowFrame:
      return L"Window";
    default:
      NS_NOTREACHED("unknown uxtheme class");
      return L"";
  }
}

// static
void
nsUXThemeData::InitTitlebarInfo()
{
  // Pre-populate with generic metrics. These likley will not match
  // the current theme, but they insure the buttons at least show up.
  sCommandButtons[0].cx = GetSystemMetrics(SM_CXSIZE);
  sCommandButtons[0].cy = GetSystemMetrics(SM_CYSIZE);
  sCommandButtons[1].cx = sCommandButtons[2].cx = sCommandButtons[0].cx;
  sCommandButtons[1].cy = sCommandButtons[2].cy = sCommandButtons[0].cy;
  sCommandButtons[3].cx = sCommandButtons[0].cx * 3;
  sCommandButtons[3].cy = sCommandButtons[0].cy;

  // Trigger a refresh on the next layout.
  sTitlebarInfoPopulatedAero = sTitlebarInfoPopulatedThemed = false;

}

// static
void
nsUXThemeData::UpdateTitlebarInfo(HWND aWnd)
{
  if (!aWnd)
    return;

  if (!sTitlebarInfoPopulatedAero && nsUXThemeData::CheckForCompositor()) {
    RECT captionButtons;
    if (SUCCEEDED(WinUtils::dwmGetWindowAttributePtr(aWnd,
                                                     DWMWA_CAPTION_BUTTON_BOUNDS,
                                                     &captionButtons,
                                                     sizeof(captionButtons)))) {
      sCommandButtons[CMDBUTTONIDX_BUTTONBOX].cx = captionButtons.right - captionButtons.left - 3;
      sCommandButtons[CMDBUTTONIDX_BUTTONBOX].cy = (captionButtons.bottom - captionButtons.top) - 1;
      sTitlebarInfoPopulatedAero = true;
    }
  }

  if (sTitlebarInfoPopulatedThemed)
    return;

  // Query a temporary, visible window with command buttons to get
  // the right metrics. 
  WNDCLASSW wc;
  wc.style         = 0;
  wc.lpfnWndProc   = ::DefWindowProcW;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = nsToolkit::mDllInstance;
  wc.hIcon         = nullptr;
  wc.hCursor       = nullptr;
  wc.hbrBackground = nullptr;
  wc.lpszMenuName  = nullptr;
  wc.lpszClassName = kClassNameTemp;
  ::RegisterClassW(&wc);

  // Create a transparent descendant of the window passed in. This
  // keeps the window from showing up on the desktop or the taskbar.
  // Note the parent (browser) window is usually still hidden, we
  // don't want to display it, so we can't query it directly.
  HWND hWnd = CreateWindowExW(WS_EX_LAYERED,
                              kClassNameTemp, L"",
                              WS_OVERLAPPEDWINDOW,
                              0, 0, 0, 0, aWnd, nullptr,
                              nsToolkit::mDllInstance, nullptr);
  NS_ASSERTION(hWnd, "UpdateTitlebarInfo window creation failed.");

  ShowWindow(hWnd, SW_SHOW);
  TITLEBARINFOEX info = {0};
  info.cbSize = sizeof(TITLEBARINFOEX);
  SendMessage(hWnd, WM_GETTITLEBARINFOEX, 0, (LPARAM)&info); 
  DestroyWindow(hWnd);

  // Only set if we have valid data for all three buttons we use.
  if ((info.rgrect[2].right - info.rgrect[2].left) == 0 ||
      (info.rgrect[3].right - info.rgrect[3].left) == 0 ||
      (info.rgrect[5].right - info.rgrect[5].left) == 0) {
    NS_WARNING("WM_GETTITLEBARINFOEX query failed to find usable metrics.");
    return;
  }
  // minimize
  sCommandButtons[0].cx = info.rgrect[2].right - info.rgrect[2].left;
  sCommandButtons[0].cy = info.rgrect[2].bottom - info.rgrect[2].top;
  // maximize/restore
  sCommandButtons[1].cx = info.rgrect[3].right - info.rgrect[3].left;
  sCommandButtons[1].cy = info.rgrect[3].bottom - info.rgrect[3].top;
  // close
  sCommandButtons[2].cx = info.rgrect[5].right - info.rgrect[5].left;
  sCommandButtons[2].cy = info.rgrect[5].bottom - info.rgrect[5].top;

  sTitlebarInfoPopulatedThemed = true;
}

// visual style (aero glass, aero basic)
//    theme (aero, luna, zune)
//      theme color (silver, olive, blue)
//        system colors

struct THEMELIST {
  LPCWSTR name;
  int type;
};

const THEMELIST knownThemes[] = {
  { L"aero.msstyles", WINTHEME_AERO },
  { L"aerolite.msstyles", WINTHEME_AERO_LITE }
};

LookAndFeel::WindowsTheme
nsUXThemeData::sThemeId = LookAndFeel::eWindowsTheme_Generic;

bool
nsUXThemeData::sIsDefaultWindowsTheme = false;
bool
nsUXThemeData::sIsHighContrastOn = false;

// static
LookAndFeel::WindowsTheme
nsUXThemeData::GetNativeThemeId()
{
  return sThemeId;
}

// static
bool nsUXThemeData::IsDefaultWindowTheme()
{
  return sIsDefaultWindowsTheme;
}

bool nsUXThemeData::IsHighContrastOn()
{
  return sIsHighContrastOn;
}

// static
bool nsUXThemeData::CheckForCompositor(bool aUpdateCache)
{
  static BOOL sCachedValue = FALSE;
  if (aUpdateCache && WinUtils::dwmIsCompositionEnabledPtr) {
    WinUtils::dwmIsCompositionEnabledPtr(&sCachedValue);
  }
  return sCachedValue;
}

// static
void
nsUXThemeData::UpdateNativeThemeInfo()
{
  // Trigger a refresh of themed button metrics if needed
  sTitlebarInfoPopulatedThemed = false;

  sIsDefaultWindowsTheme = false;
  sThemeId = LookAndFeel::eWindowsTheme_Generic;

  HIGHCONTRAST highContrastInfo;
  highContrastInfo.cbSize = sizeof(HIGHCONTRAST);
  if (SystemParametersInfo(SPI_GETHIGHCONTRAST, 0, &highContrastInfo, 0)) {
    sIsHighContrastOn = ((highContrastInfo.dwFlags & HCF_HIGHCONTRASTON) != 0);
  } else {
    sIsHighContrastOn = false;
  }

  if (!IsAppThemed()) {
    sThemeId = LookAndFeel::eWindowsTheme_Classic;
    return;
  }

  WCHAR themeFileName[MAX_PATH + 1];
  WCHAR themeColor[MAX_PATH + 1];
  if (FAILED(GetCurrentThemeName(themeFileName,
                                 MAX_PATH,
                                 themeColor,
                                 MAX_PATH,
                                 nullptr, 0))) {
    sThemeId = LookAndFeel::eWindowsTheme_Classic;
    return;
  }

  LPCWSTR themeName = wcsrchr(themeFileName, L'\\');
  themeName = themeName ? themeName + 1 : themeFileName;

  WindowsTheme theme = WINTHEME_UNRECOGNIZED;
  for (size_t i = 0; i < ArrayLength(knownThemes); ++i) {
    if (!lstrcmpiW(themeName, knownThemes[i].name)) {
      theme = (WindowsTheme)knownThemes[i].type;
      break;
    }
  }

  if (theme == WINTHEME_UNRECOGNIZED)
    return;

  // We're using the default theme if we're using any of Aero or Aero Lite.
  // However, on Win8, GetCurrentThemeName (see above) returns
  // AeroLite.msstyles for the 4 builtin high contrast themes as well. Those
  // themes "don't count" as default themes, so we specifically check for high
  // contrast mode in that situation.
  if (!(IsWin8OrLater() && sIsHighContrastOn) &&
      (theme == WINTHEME_AERO || theme == WINTHEME_AERO_LITE)) {
    sIsDefaultWindowsTheme = true;
  }

  switch(theme) {
    case WINTHEME_AERO:
      sThemeId = LookAndFeel::eWindowsTheme_Aero;
      break;
    case WINTHEME_AERO_LITE:
      sThemeId = LookAndFeel::eWindowsTheme_AeroLite;
      break;
    default:
      NS_WARNING("unhandled theme type.");
  }
}
