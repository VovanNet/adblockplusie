#ifndef _SIMPLE_ADBLOCK_CLIENT_H_
#define _SIMPLE_ADBLOCK_CLIENT_H_


#include "PluginTypedef.h"
#include "PluginClientBase.h"
#include "AdblockPlus.h"


using namespace AdblockPlus;

class CPluginFilter;

#ifdef USE_CONSOLE
      CONSOLE(message.c_str());
#endif

    DEBUG_GENERAL(message.c_str());
class CAdblockPlusClient : public CPluginClientBase
{

private:

  std::auto_ptr<CPluginFilter> m_filter;
  std::auto_ptr<AdblockPlus::FilterEngine> filterEngine;

  CComAutoCriticalSection m_criticalSectionFilter;
  CComAutoCriticalSection m_criticalSectionCache;

  std::map<CString,bool> m_cacheBlockedSources;


  // Private constructor used by the singleton pattern
  CAdblockPlusClient();

public:

  static CAdblockPlusClient* s_instance;

  ~CAdblockPlusClient();

  static CAdblockPlusClient* GetInstance();

  AdblockPlus::FilterEngine* GetFilterEngine();

  // Removes the url from the list of whitelisted urls if present
  // Only called from ui thread
  bool ShouldBlock(CString src, int contentType, const CString& domain, bool addDebug=false);

  bool IsElementHidden(const CString& tag, IHTMLElement* pEl, const CString& domain, const CString& indent, CPluginFilter* filter);
  bool IsUrlWhiteListed(const CString& url);

  int GetIEVersion();

};

#endif // _SIMPLE_ADBLOCK_CLIENT_H_
