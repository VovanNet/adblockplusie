// Copyright 2007 Igor Tandetnik
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PASSTHROUGHAPP_PROTOCOLCF_INL
#define PASSTHROUGHAPP_PROTOCOLCF_INL

#if _MSC_VER > 1000
 #pragma once
#endif // _MSC_VER > 1000

#ifndef PASSTHROUGHAPP_PROTOCOLCF_H
 #error ProtocolCF.inl requires ProtocolCF.h to be included first
#endif

#include "PassthroughObject.h"


namespace PassthroughAPP
{

// ===== CComClassFactoryProtocol =====

inline STDMETHODIMP CComClassFactoryProtocol::CreateInstance(
	IUnknown* punkOuter, REFIID riid, void** ppvObj)
{
	ATLASSERT(ppvObj != 0);
	if (!ppvObj)
	{
		return E_POINTER;
	}
	*ppvObj = 0;

	CComPtr<IUnknown> spUnkTarget;
	HRESULT hr = CreateInstanceTarget(&spUnkTarget);
	ATLASSERT(SUCCEEDED(hr) && spUnkTarget != 0);

	CComPtr<IUnknown> spUnkObject;
	if (SUCCEEDED(hr))
	{
		hr = BaseClass::CreateInstance(punkOuter, riid,
			reinterpret_cast<void**>(&spUnkObject));
		ATLASSERT(SUCCEEDED(hr) && spUnkObject != 0);
	}

	if (SUCCEEDED(hr))
	{
		CComPtr<IPassthroughObject> spPassthroughObj;
		hr = spUnkObject->QueryInterface(&spPassthroughObj);
		ATLASSERT(SUCCEEDED(hr) && spPassthroughObj != 0);
		if (SUCCEEDED(hr))
		{
			hr = spPassthroughObj->SetTargetUnknown(spUnkTarget);
			ATLASSERT(SUCCEEDED(hr));
		}
	}

	if (SUCCEEDED(hr))
	{
		*ppvObj = spUnkObject.Detach();
	}
	return hr;
}

inline HRESULT CComClassFactoryProtocol::CreateInstanceTarget(
	IUnknown** ppTargetProtocol)
{
	ATLASSERT(ppTargetProtocol != 0);
	if (!ppTargetProtocol)
	{
		return E_POINTER;
	}
	*ppTargetProtocol = 0;

	CComPtr<IClassFactory> spTargetCF;
	HRESULT hr = GetTargetClassFactory(&spTargetCF);
	ATLASSERT(SUCCEEDED(hr) && spTargetCF != 0);
	if (SUCCEEDED(hr))
	{
		hr = spTargetCF->CreateInstance(0, IID_IInternetProtocolRoot,
			reinterpret_cast<void**>(ppTargetProtocol));
		ATLASSERT(SUCCEEDED(hr) && *ppTargetProtocol != 0);
	}
	return hr;
}

inline HRESULT CComClassFactoryProtocol::GetTargetClassFactory(
	IClassFactory** ppCF)
{
	ObjectLock lock(this);
	return m_spTargetCF.CopyTo(ppCF);
}

inline HRESULT CComClassFactoryProtocol::SetTargetClassFactory(
	IClassFactory* pCF)
{
	HRESULT hr = (pCF ? pCF->LockServer(TRUE) : S_OK);
	if (SUCCEEDED(hr))
	{
		ObjectLock lock(this);
		if (m_spTargetCF)
		{
			// LockServer(FALSE) is assumed to always succeed. Otherwise,
			// it is impossible to implement correct semantics
			HRESULT hr1 = m_spTargetCF->LockServer(FALSE);
			hr1;
			ATLASSERT(SUCCEEDED(hr1));
		}
		m_spTargetCF = pCF;
	}
	return hr;
}

inline HRESULT CComClassFactoryProtocol::SetTargetCLSID(REFCLSID clsid,
	DWORD clsContext)
{
	CComPtr<IClassFactory> spTargetCF;
	HRESULT hr = CoGetClassObject(clsid, clsContext, 0, IID_IClassFactory,
		reinterpret_cast<void**>(&spTargetCF));
	ATLASSERT(SUCCEEDED(hr) && spTargetCF != 0);
	if (SUCCEEDED(hr))
	{
		hr = SetTargetClassFactory(spTargetCF);
		ATLASSERT(SUCCEEDED(hr));
	}
	return hr;
}

inline void CComClassFactoryProtocol::FinalRelease()
{
	// No need to be thread safe here
	if (m_spTargetCF)
	{
		// LockServer(FALSE) is assumed to always succeed.
		HRESULT hr = m_spTargetCF->LockServer(FALSE);
		hr;
		ATLASSERT(SUCCEEDED(hr));

		m_spTargetCF.Release();
	}
}

// ===== CMetaFactory =====

template <class Factory, class Protocol, class FactoryComObject>
inline HRESULT CMetaFactory<Factory, Protocol, FactoryComObject>::
	CreateInstance(Factory** ppObj)
{
	ATLASSERT(ppObj != 0);
	if (!ppObj)
	{
		return E_POINTER;
	}

	HRESULT hr = E_OUTOFMEMORY;
	void* pv = static_cast<void*>(CreatorClass::CreateInstance);
	FactoryComObject* p = 0;
	ATLTRY(p = new FactoryComObject(pv))
	if (p != NULL)
	{
		p->SetVoid(pv);
		p->InternalFinalConstructAddRef();
#if _ATL_VER >= 0x800
		hr = p->_AtlInitialConstruct();
		if (SUCCEEDED(hr))
#endif
			hr = p->FinalConstruct();
#if _ATL_VER >= 0x800
		if (SUCCEEDED(hr))
			hr = p->_AtlFinalConstruct();
#endif
		p->InternalFinalConstructRelease();
		if (FAILED(hr))
		{
			delete p;
			p = 0;
		}
	}
	*ppObj = p;
	return hr;
}

template <class Factory, class Protocol, class FactoryComObject>
inline HRESULT CMetaFactory<Factory, Protocol, FactoryComObject>::
	CreateInstance(IClassFactory* pTargetCF, IClassFactory** ppCF)
{
	if (!ppCF)
	{
		return E_POINTER;
	}
	*ppCF = 0;

	Factory* pObj = 0;
	HRESULT hr = CreateInstance(&pObj);
	ATLASSERT(SUCCEEDED(hr) && pObj != 0);
	if (SUCCEEDED(hr))
	{
		pObj->_AtlInitialConstruct();
		pObj->AddRef();

		hr = pObj->SetTargetClassFactory(pTargetCF);
		ATLASSERT(SUCCEEDED(hr));

		if (SUCCEEDED(hr))
		{
			hr = pObj->QueryInterface(IID_IClassFactory,
				reinterpret_cast<void**>(ppCF));
			ATLASSERT(SUCCEEDED(hr) && *ppCF != 0);
		}

		pObj->Release();
	}
	return hr;
}

template <class Factory, class Protocol, class FactoryComObject>
inline HRESULT CMetaFactory<Factory, Protocol, FactoryComObject>::
	CreateInstance(REFCLSID clsidTarget, IClassFactory** ppCF)
{
	if (!ppCF)
	{
		return E_POINTER;
	}
	*ppCF = 0;

	Factory* pObj = 0;
	HRESULT hr = CreateInstance(&pObj);
	ATLASSERT(SUCCEEDED(hr) && pObj != 0);
	if (SUCCEEDED(hr))
	{
		//pObj->_AtlInitialConstruct();
		pObj->AddRef();

		hr = pObj->SetTargetCLSID(clsidTarget);
		ATLASSERT(SUCCEEDED(hr));

		if (SUCCEEDED(hr))
		{
			hr = pObj->QueryInterface(IID_IClassFactory,
				reinterpret_cast<void**>(ppCF));
			ATLASSERT(SUCCEEDED(hr) && *ppCF != 0);
		}

		pObj->Release();
	}
	return hr;
}

} // end namespace PassthroughAPP

#endif // PASSTHROUGHAPP_PROTOCOLCF_INL
