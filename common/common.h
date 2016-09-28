#ifndef _H_COMMON_H_
#define _H_COMMON_H_

#include <string>
#include <time.h>
#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <set>
#include <map>
#include <cstdlib>
#include <cctype>

#include <stdexcept>

#include "json/json.h"

#define _WEBSOCKETPP_NOEXCEPT_
#define WEBSOCKETPP_CPP11_CHRONO

#include <vector>
#include <iostream>
#include <stdint.h>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include <set>
#include <queue>
typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
//using websocketpp::lib::bind;
using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;


using namespace boost;
using namespace std;
class zooKClient;
class HostInfo;
class CSendMsg;
enum EOPT
{
	E_ADD =	1,
	E_DEL = 2,
	E_UPDATE = 3
};


class CMyserver
{
public:
	CMyserver()
	{
	};     

	virtual ~CMyserver()
	{
		
	};

	static CMyserver *GetInstance()
	{        
		return m_pMyServer;
	};


	virtual int Init(){};

	static int SetServer(CMyserver *pMyserver)
	{
		m_pMyServer = pMyserver;
	}

	virtual string GetDispAddr(){};
	virtual uint16_t GetUpType(){};

	virtual void PutMsgToList(CSendMsg *pMsg){};

	virtual int Process() {};
	virtual int SendToUdpServer(uint32_t uiCmd, const string& msgBody, uint16_t iSrvType, string& sAddr, uint16_t& iPort){};
protected:
	static CMyserver * m_pMyServer;

};

struct HostInfo
{
	HostInfo():m_name(""), m_ip(""),m_port(0)
	{

	}
	HostInfo(string arg_name, string arg_ip, uint16_t arg_port)
	{
		m_name	= arg_name;
		m_ip		= arg_ip;
		m_port	= arg_port;
	}
	bool operator ==(const HostInfo& info) const
	{
		return (info.m_name == m_name &&
			info.m_ip == m_ip &&
			info.m_port == m_port);
	}

	bool operator !=(const HostInfo& info) const
	{
		return !(operator==(info));
	}

	string toString()const 
	{
		ostringstream ss;
		ss << m_name << " " << m_ip << " " << m_port ;
		return ss.str();
	}
	string			m_name;
	string			m_ip;
	uint16_t		m_port;
};


enum message_type 
{
	SUB_MSG,
	UNSUB_MSG,
	CLIENT_MSG,
	TIMER_MSG,
	SERVER_MSG
};


template<class T>
class CMySingleton
{
public:
	static T *GetInstance()
	{
		return m_pInstance;
	}

	static void SetInstance(T *instance)
	{
		if (m_pInstance != NULL)
		{
			delete m_pInstance;
		}

		m_pInstance = instance;
	}

private:
	static T *m_pInstance;
};

template<class T>
T* CMySingleton<T>::m_pInstance = NULL;


struct TClientMsg {
	TClientMsg(message_type t, connection_hdl h) : type(t), hdl(h) {}
	TClientMsg(message_type t, connection_hdl h, server::message_ptr m) : type(t), hdl(h), msg(m) {}
	TClientMsg(message_type t,const string buf): type(t), buffer(buf){}
	message_type type;
	websocketpp::connection_hdl hdl;
	server::message_ptr msg;
	string buffer;
};

struct TUserInfo
{
	std::string sUserId;
	std::string sAlias;
	std::string sRoom;

	Json::Value toJson()
	{
		Json::Value root;
		root["userid"]	= sUserId;
		root["alias"]	= sAlias;
		root["room"]	= sRoom;

		return root;
	}
};

struct TBroadCast
{
	std::string sUUID;
	std::string sGroupId;
	std::string sType;
	std::string sMsgData;
	std::string sNick;

	std::string ToJson();
};

struct TRouteMsg
{
	std::string sUUID;
	std::string sGroupId;
	std::string sType;
	std::string sMsgData;
	std::string sMsgType;

	std::string ToJson();
};

struct TRoomInfo
{
	std::string sRoomId;
	std::string sUserId;
	std::string sTime;
	std::set<TUserInfo> sUserList;

	Json::Value toJson()
	{
		Json::Value root;
		root["roomid"]	= sRoomId;
		root["userid"]	= sUserId;
		root["time"]	= sTime;
		
		Json::Value userList;
		std::set<TUserInfo>::iterator itor = sUserList.begin();
		for ( ; itor != sUserList.end(); itor++)
		{
			//userList.append(itor->toJson());		
		}

		root["userlist"] = userList;

		return root;
	}
};


 string Secd2Time(time_t lSecd);

 string Secd2Date(time_t lSecd);


 std::string ipToStr(unsigned int ip);

 std::string ipToStrHton(unsigned int ip);

int GetUdpServerAddr(const string& ssDisPatchAddr, uint16_t iUpSrvType, uint16_t& iSelfType, string& sAddr, uint16_t& iPort);

#endif
