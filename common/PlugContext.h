#ifndef __PlugContext_h__
#define __PlugContext_h__

#include <string>
#include <map>
#include <stdint.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "common.h"
#include "define.h"

struct SKeyRet
{
	std::string strKey;
	std::string strReqBody;
	std::string strRspBody;
	int iRet;

	SKeyRet(void)
	{
		iRet = 0;
	}

	std::string toString(void) const;
};
typedef std::map<std::string, SKeyRet> TKey2Ret;

class CDataOpBase
{
public:
	CDataOpBase(const string* pBuf)
	{
		m_sProtoBuf = pBuf;
	}

	virtual ~CDataOpBase()
	{

	}
	virtual int Process(string& sRsp) = 0; //主循环
	virtual std::string ToString(void) //对象字符串
	{
		std::stringstream oss;
		oss << "Req Json:" << m_oClientReq.toStyledString()
			<< "Rsp Json:" << m_oClientRsp.toStyledString()
			;
		return oss.str();
	}

	Json::Value &GetClientReq(void)
	{
		return m_oClientReq;
	}

	Json::Value &GetClientRsp(void)
	{
		return m_oClientRsp;
	}
	
	void SetCmd(unsigned int uiCmd)
	{
		m_uiCmd = uiCmd;
	}
	void SetType(uint16_t iType)
	{
		m_iUpType = iType;
	}

protected:
	unsigned int m_uiCmd;
	uint16_t m_iUpType;
	const string* m_sProtoBuf;
	Json::Value m_oClientReq;
	Json::Value m_oClientRsp;
};

class CDataObjectFactory
{
public:
	CDataObjectFactory(void)
	{
	}
	virtual ~CDataObjectFactory(void)
	{
	}

	virtual CDataOpBase *CreateDataObject(const string* pBuf)
	{
		return NULL;
	}
};

template <typename T>
class CFunctoryImp : public CDataObjectFactory
{
public:
	CFunctoryImp(void)
	{
	}
	virtual ~CFunctoryImp(void)
	{
	}

	virtual CDataOpBase *CreateDataObject(const string* pBuf)
	{
		return new T(pBuf);
	}
};

typedef struct SCmd
{
	unsigned int uiCmd;
	unsigned int uiSubCmd;

	SCmd(void)
	{
		clear();
	}
	SCmd(unsigned int _uiCmd, unsigned int _uiSubCmd = 0)
		: uiCmd(_uiCmd)
		, uiSubCmd(_uiSubCmd)
	{
	}

	void clear(void)
	{
		uiCmd = 0;
		uiSubCmd = 0;
	}

	friend bool operator == (const SCmd &lsh, const SCmd &rsh)
	{
		return ((lsh.uiCmd == rsh.uiCmd) && (lsh.uiSubCmd == rsh.uiSubCmd));
	}

	friend bool operator < (const SCmd &lsh, const SCmd &rsh)
	{
		if (lsh.uiCmd != rsh.uiCmd)
		{
			return lsh.uiCmd < rsh.uiCmd;
		}
		return lsh.uiSubCmd < rsh.uiSubCmd;
	}
} SCmd;

typedef struct SCallback
{
	unsigned int uiCmd;
	unsigned int uiSubCmd;
	CDataObjectFactory*	pFactory;

	SCallback(void)
	{
		uiCmd = 0;
		uiSubCmd = 0;
		pFactory = NULL;
	}
} SCallback;
typedef std::map<SCmd, SCallback> TCmdCallbackMap;

inline TCmdCallbackMap & GetGlobalCmdMap(void)
{
	static TCmdCallbackMap g_oCmdMap;
	return g_oCmdMap;
}

template <typename T>
class COpRegiHelper
{
public:
	COpRegiHelper(unsigned int uiCmd, unsigned int uiSubCmd = 0)
	{
		TCmdCallbackMap &oCallbackMap = GetGlobalCmdMap();
		TCmdCallbackMap::iterator it = oCallbackMap.find(SCmd(uiCmd, uiSubCmd));
		if (oCallbackMap.end() == it)
		{
			SCallback oCallback;
			oCallback.uiCmd = uiCmd;
			oCallback.uiSubCmd = uiSubCmd;
			oCallback.pFactory = new CFunctoryImp<T>();
			oCallbackMap.insert(std::make_pair(SCmd(uiCmd, uiSubCmd), oCallback));
		}
	}

	~COpRegiHelper(void)
	{

	}
};

#define IMP_MAP_DATA_BASE(class_name, cmd, subcmd); class class_name; static COpRegiHelper<class_name> xx_##class_name##_xx(cmd, subcmd);


#endif
