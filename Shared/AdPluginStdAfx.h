#ifndef _STDAFX_H
#define _STDAFX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#define _ATL_APARTMENT_THREADED

#define _AFX_ALL_WARNINGS
#include <afxwin.h>								// MFC core and standard components
#include <afxext.h>								// MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>							// MFC support for Internet Explorer 4 Common Controls
#endif

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>								// MFC support for Windows Common Controls
#endif

#include "afx.h" 

#define _CRT_SECURE_NO_DEPRECATE 1
#include <atlbase.h>
#include <atlstr.h>

extern CComModule _Module;
#include <comutil.h>
#include <atlcom.h>
#include <atlhost.h>
#include <stdio.h>
#include <assert.h>
#include <stdexcept>
#include <ExDisp.h>
#include <ExDispID.h>
#include <Mshtml.h>

#include <map>
#include <set>
#include <vector>
#include <fstream>
#include <sstream>
#include <commctrl.h>
#include <mshtmdid.h>
#include <Mlang.h>
#include <initguid.h>

// Win32
#include <shlguid.h>
#include <shlobj.h>
#include <iepmapi.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#include "AdPluginDebugMacros.h"
#include "AdPluginErrorCodes.h"

#if (defined PRODUCT_ADBLOCKER || defined PRODUCT_DOWNLOADHELPER || defined PRODUCT_DOWNLOADHELPER_APP)
 #include "Config.h"
#endif

#if (defined PRODUCT_ADBLOCKER)
 #include "..\ABlocker\Version.h"
#elif (defined PRODUCT_DOWNLOADHELPER)
 #include "..\DownloadHelper\Version.h"
#elif (defined PRODUCT_DOWNLOADHELPER_APP)
 #include "..\DownloadHelper\Version.h"
#endif

#if (defined PRODUCT_ADBLOCKER)
 #include "..\ABlocker\Resource.h"
#elif (defined PRODUCT_DOWNLOADHELPER)
 #include "..\DownloadHelper\Resource.h"
#elif (defined PRODUCT_DOWNLOADHELPER_APP)
 #include "..\DownloadHelperApp\Resource.h"
#endif

#if (defined PRODUCT_ADBLOCKER || defined PRODUCT_DOWNLOADHELPER || defined PRODUCT_DOWNLOADHELPER_APP)
 #include "AdPluginDebug.h"
#endif

#if (defined PRODUCT_ADBLOCKER || defined PRODUCT_DOWNLOADHELPER)
 #include "AdPluginSelftest.h"
#endif

#ifdef _MSC_VER
 #pragma warning(push)
 // warning C4996: function call with parameters that might be unsafe
 #pragma warning(disable : 4996)
#endif

#endif // not _STDAFX_H
