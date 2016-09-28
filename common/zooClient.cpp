/*
 * =====================================================================================
 * 
 *       Filename:  zooClient.cpp
 * 
 *    Description:  common client
 * 
 *        Version:  1.0
 *        Created:  2015-04-13 14:28
 *       Revision:  none
 *       Compiler:  g++
 * 
 *         Author:  lizhengping@pwrd.com
 *        Company:  Perfect World
 * 
 * =====================================================================================
 */

#define _WEBSOCKETPP_NOEXCEPT_
#define WEBSOCKETPP_CPP11_CHRONO


#include "zooClient.h"
#include "common.h"
#include <string.h>
#include "zk_adaptor.h"
#include <zookeeper/zookeeper.h>
#include <assert.h>
#include "writeRunInfo.h"
#include <sstream>
using namespace std;
/*********************                   基本 C API              **********************************************/
FILE* fp=NULL;
zookClient::zookClient(const HostInfo& localInfo, const string& serverList, const string_callFunc_map& callFunc /* = string_callFunc_map */)
{ 
	_localInfo			= localInfo;
	_g_wfn				= zookClient::g_watcher_init_function;
	_zooServerList		= serverList;
	_session_timeout	= 3000;
	_zhandle_t			= NULL;
	string_callFunc_map::const_iterator itor = callFunc.begin();
	for (; itor != callFunc.end(); ++itor)
	{
		TCallBackObj tmpObj;
		tmpObj.callBack = itor->second;
		_mapCallBack.insert(make_pair(itor->first, tmpObj));
	}

	init();
}

int zookClient::init()
{
	WRITE_INFO(("%s \t zookeeper_init", __FUNCTION__));
	if (_zhandle_t)
	{
		WRITE_INFO(("delete _zhandle_t first!"));
		::zookeeper_close(_zhandle_t);
	}

	zoo_set_debug_level(ZOO_LOG_LEVEL_INFO);
	fp = fopen("log/zooClient.log", "a+");
	zoo_set_log_stream(fp);

	_zhandle_t = ::zookeeper_init(_zooServerList.c_str(), _g_wfn, _session_timeout, 0, this, 0);
	
	if (_zhandle_t == NULL)
	{
		return -1;
	}
	WRITE_INFO(("address= %#p", &(_zhandle_t->to_send)));
	WRITE_INFO(("address= %#p", &(_zhandle_t->client_id)));

	return 0;
}

int zookClient::zoo_delete(const string& path, int version /* = -1 */)
{
	return ::zoo_delete(_zhandle_t, path.c_str(), version);
}

int zookClient::zoo_wexists(const string& path, watcher_fn watcher, void* watcherCtx, struct Stat* stat)
{
	return ::zoo_wexists(_zhandle_t, path.c_str(), watcher, watcherCtx, stat);
}

int zookClient::zoo_exists(const string& path, int watch, struct Stat* stat)
{
	return ::zoo_exists(_zhandle_t, path.c_str(), watch, stat); 
}

int zookClient::zoo_wget_children(const string& path, watcher_fn watcher, void* watcherCtx, string_vector& node_list)
{
	struct String_vector vecStr = {0, NULL};
	int ret= ::zoo_wget_children(_zhandle_t, path.c_str(), watcher, watcherCtx, &vecStr);

	if (!ret)
	{
		for (uint8_t i = 0; i< vecStr.count; i++)
		{
			node_list.push_back(string(vecStr.data[i]));
		}
	}
	
	// delete the vecStr by user
	::deallocate_String_vector(&vecStr);
	return ret;
}

int zookClient::zoo_acreate(const string& path, const string& node_data, const struct ACL_vector& acl, int flags, string_completion_t completion, const void* data)
{
	return ::zoo_acreate(_zhandle_t, path.c_str(), node_data.c_str(), node_data.size(), &acl, flags, completion, data);
}

int zookClient::zoo_adelete(const string& path, int version,void_completion_t completion, const void* data)
{
	return ::zoo_adelete(_zhandle_t, path.c_str(), -1, completion, data);
}

int zookClient::zoo_awexists(const string& path, watcher_fn watcher, void * watcherCtx, stat_completion_t completion, const void* data)
{
	return ::zoo_awexists(_zhandle_t, path.c_str(), watcher, watcherCtx, completion, data);
}

int zookClient::zoo_awget(const string& path, watcher_fn watcher, void* watcherCtx, data_completion_t completion, const void* data)
{
	return ::zoo_awget(_zhandle_t, path.c_str(), watcher, watcherCtx, completion, data);
}

int zookClient::zoo_aset(const string& path, const string& node_data, int version, stat_completion_t completion, const void *data)
{
	return ::zoo_aset(_zhandle_t, path.c_str(), node_data.c_str(), node_data.size(), version, completion, data);
}

int zookClient::zoo_awget_children(const string& path, watcher_fn watcher, void* watcherCtx, strings_completion_t completion, const void* data)
{
	return ::zoo_awget_children(_zhandle_t, path.c_str(), watcher, watcherCtx, completion, data);
}


/****************************             回调函数               ********************************************/
void zookClient::g_watcher_init_function(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
	zookClient* cli = (zookClient*)watcherCtx;
	if (type == ZOO_SESSION_EVENT) 
	{
		if (state == ZOO_CONNECTED_STATE) 
		{
			WRITE_INFO("%s \t Connected to Zookeeper Service Successfully!",__FUNCTION__);
			cli->registerSelf();
			cli->observer();
		}
		else if (state == ZOO_EXPIRED_SESSION_STATE) 
		{ 
			WRITE_INFO("%s Zookeeper Session Expired!", __FUNCTION__);
			if (cli)
			{
				cli->init();
			}	
		}
		else if (state == ZOO_CONNECTING_STATE)
		{
			WRITE_INFO("%s Host is Down! Will Reconnect!", __FUNCTION__);
		}
	}  

}
 /*
	param rc		error code	 0 ZOK, ZCONNECTIONLOSS = -4, ZSESSIONEXPIRED = -112, ZOPERATIONTIMEOUT -7.
	
	param value		the value of the string returned.

	param data		the pointer that was passed by the caller when the function	.

	param stat		a pointer to the stat information for the node	 if rc ！=0 the content of stat is undefined	.

	param strings	children list
 
 */
void zookClient::create_completion_function(int rc, const char *value, const void *data)
{
	WRITE_INFO("%s The Node %s Create Successfully!", __FUNCTION__, (char*)data);
}

void zookClient::delete_completion_function(int rc, const void *data)
{
   WRITE_INFO("%s rc=%d\n",__FUNCTION__, rc);
}

void zookClient::awexists_completion_function(int rc, const struct Stat *stat, const void *data)
{
   WRITE_INFO("%s rc=%d data =%s\n",__FUNCTION__, rc, data);
}

void zookClient::wget_completion_function(int rc, const char *value, int value_len, const struct Stat *stat, const void *data)
{
   WRITE_INFO("%s rc=%d\n",__FUNCTION__, rc);
}

void zookClient::awget_children_completion_function(int rc, const struct String_vector *strings, const void *data)
{
  WRITE_INFO("%s rc=%d\n",__FUNCTION__, rc);
}

/************************************************************************
标识监视事件的类型，他们通常用作监视器回调函数的第一个参数。
•ZOO_CREATED_EVENT; // 节点被创建(此前该节点不存在)，通过 zoo_exists() 设置监视。
•ZOO_DELETED_EVENT; // 节点被删除，通过 zoo_exists() 和 zoo_get() 设置监视。
•ZOO_CHANGED_EVENT; // 节点发生变化，通过 zoo_exists() 和 zoo_get() 设置监视。
•ZOO_CHILD_EVENT; // 子节点事件，通过zoo_get_children() 和 zoo_get_children2()设置监视。
•ZOO_SESSION_EVENT; // 会话丢失
•ZOO_NOTWATCHING_EVENT; // 监视被移除。

************************************************************************/
void zookClient::watcher_process(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx)
{
	zookClient *pClient = (zookClient*)watcherCtx;
	if (state == ZOO_CONNECTED_STATE)
	{
		switch (type)
		{	
				// Server  Down	    Not Use !!!!!!
			case DELETED_EVENT_DEF:
				pClient->delServer(path);
				break;
			
				// Up Or Down
			case CHILD_EVENT_DEF:
				pClient->updateServer(path);
				break;
			default:
				break;
		}
	}

}

/****************************		    自定义函数        **************************************************/

void zookClient::registerSelf()
{
	WRITE_INFO("new %#llx pass %s ", _zhandle_t->client_id.client_id, _zhandle_t->client_id.passwd);

	string father_path = "/" + _localInfo.m_name;
	string full_path = father_path + "/" +  generateAddrInfo(_localInfo);
	int ret;
	struct Stat st1, st2;
	ret = zoo_exists(father_path, 0, &st1);
	if (ret == ZNONODE )
	{
		ret = zoo_acreate(father_path, father_path, ZOO_OPEN_ACL_UNSAFE, 0, create_completion_function, _localInfo.m_name.c_str());
		if (ret == ZOK)
		{
			WRITE_INFO("%s create node %s", __FUNCTION__, father_path.c_str());
		}
		
	}
	ret = zoo_exists(full_path, 0 , &st2);
	WRITE_INFO("old %#llx new addr=%p", st2.ephemeralOwner, &(_zhandle_t->client_id.client_id));
	FILE *pp = fopen("lzp.log", "w");
	fwrite(_zhandle_t->hostname, strlen(_zhandle_t->hostname), 1, pp);
	fwrite((void*)&_zhandle_t->client_id, sizeof(_zhandle_t->client_id), 1, pp);
	fclose(pp);
	WRITE_INFO("ctime %ld", st2.ctime);
	if (ret != ZNONODE && st2.ephemeralOwner != _zhandle_t->client_id.client_id)
	{
		zoo_delete(full_path);
		WRITE_INFO("%s the Old %s had exsits we will delete it first", __FUNCTION__, full_path.c_str());
	}

	ret = zoo_acreate(full_path, full_path, ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, create_completion_function, _localInfo.m_ip.c_str());
	if (ret == ZOK)
	{
	   WRITE_INFO("%s create node %s", __FUNCTION__, full_path.c_str());
	}
	
}

void zookClient::observer()
{
	str_call_itor itor = _mapCallBack.begin();
	for (; itor != _mapCallBack.end(); ++itor)
	{
		int ret = zoo_exists(itor->first, 0, NULL);
		if (ret == ZNONODE)
		{
			zoo_acreate(itor->first, itor->first, ZOO_OPEN_ACL_UNSAFE, 0, create_completion_function, itor->first.c_str());
			WRITE_INFO("%s \t %s Set Watcher path=%s", __FUNCTION__, _localInfo.m_name.c_str(), itor->first.c_str());
		}

		updateServer(itor->first);
	}

}

void zookClient::updateServer(const string& path)
{
	vector<HostInfo> vecInfo;
	str_call_itor itor = _mapCallBack.find(path);

	if (itor != _mapCallBack.end())
	{
		getServerList(path, vecInfo);
		
		if (vecInfo.size())
		{

			vector<HostInfo> addlist;
			vector<HostInfo> dellist;
			compareHostInfo(vecInfo, itor->second.vecHost, addlist, dellist);
			if(addlist.size())
			{
				WRITE_INFO("%s %s Server Add", __FUNCTION__, path.c_str());
				(itor->second.callBack)(addlist, E_ADD);
			}
			if(dellist.size()) 
			{
				WRITE_INFO("%s %s Server Del", __FUNCTION__, path.c_str());
				(itor->second.callBack)(dellist, E_DEL);
			}
		}
		else
		{
		   WRITE_INFO("%s %s Server Has Not Register Or All Down!", __FUNCTION__, path.c_str());
		   if (itor->second.vecHost.size())
		   {
			  WRITE_INFO("%s %s Server All Down", __FUNCTION__, path.c_str());
			  (itor->second.callBack)(itor->second.vecHost, E_DEL);
		   }

		} 

		itor->second.vecHost.swap(vecInfo);
	}
}

void zookClient::delServer(const string& path)
{
	HostInfo host_info;
	parseHostInfo(path, host_info);
	vector<HostInfo> vecSrv;
	vecSrv.push_back(host_info);
}

void zookClient::getServerList(const string& serverName, vector<HostInfo>& vecInfo)
{
	string_vector temVecStr;
	string path = serverName;
	zoo_wget_children(path, watcher_process, this, temVecStr);

	for (uint8_t i = 0; i< temVecStr.size(); i++)
	{
		string spath = path + "/" + temVecStr[i];
		//如果不关心那个server 停掉 只是拉去一份最新的server 列表的话那就不用再关注子节点了， 有任何变化，会通知父节点。
		//zoo_awexists(spath, watcher_process, this, awexists_completion_function, __FUNCTION__);
		HostInfo temInfo;
		parseHostInfo(spath, temInfo);
		vecInfo.push_back(temInfo);
	}	
}

int SplitString(const string &srcStr,const string &delim, vector<string>& strList)
{     
	char *sBuf = new char[srcStr.length() + 1];
	char *sTmpBuf = new char[srcStr.length() + 1];

	strncpy(sBuf, srcStr.c_str(), srcStr.length());
	sBuf[srcStr.length()] = 0;

	char *tmpSrc = sBuf;
	char *tt = sTmpBuf;

	char *sToken = strtok_r(tmpSrc, delim.c_str(), &tt);
	while (sToken)
	{
		strList.push_back(sToken);
		sToken = strtok_r(NULL, delim.c_str(), &tt);
	}
	delete [] sBuf;
	delete [] sTmpBuf;
	return 0;
}

 void parseHostInfo(const string& path, HostInfo& info)
 {
	 vector<string> vecStr;
	 SplitString(path, "/", vecStr);
	 assert(vecStr.size() == 2);
	 string name = vecStr[0];
	 string host_info = vecStr[1];
	 vecStr.clear();
	 SplitString(host_info, ":", vecStr);
	 assert(vecStr.size() == 2);

	 info.m_name = name;
	 info.m_ip	 = vecStr[0];
	 info.m_port = atoi(vecStr[1].c_str());
 }

 string generateAddrInfo(const HostInfo& info)
 {
	 string tempstr = "";
	 char port[16] = {'\0'};
	 snprintf(port, 16, "%u", info.m_port);

	 tempstr = info.m_ip;
	 tempstr += ":";
	 tempstr += port;

	 return tempstr;
 }

 void compareHostInfo(const vector<HostInfo>& newinfos, const vector<HostInfo>& oldinfos, vector<HostInfo>& addlist, vector<HostInfo>& minuslist)
 {
	 vector<HostInfo>::const_iterator cmp1;
	 vector<HostInfo>::const_iterator cmp2;

	 //获取添加的
	 {
		 cmp1 = newinfos.begin();
		 while(cmp1 != newinfos.end())
		 {
			 bool flag = true;			//代表不存在
			 cmp2 = oldinfos.begin();
			 while(cmp2 != oldinfos.end())
			 {
				 if(*cmp1 == *cmp2)
				 {
					 flag = false;
					 break;
				 }
				 cmp2++;
			 }
			 if(flag)
				 addlist.push_back(*cmp1);
			 cmp1++;
		 }
	 }

	 //获取删除的
	 {
		 cmp1 = oldinfos.begin();
		 while(cmp1 != oldinfos.end())
		 {
			 bool flag = true;			//代表不存在
			 cmp2 = newinfos.begin();
			 while(cmp2 != newinfos.end())
			 {
				 if(*cmp1 == *cmp2)
				 {
					 flag = false;
					 break;
				 }
				 cmp2++;
			 }
			 if(flag)
				 minuslist.push_back(*cmp1);
			 cmp1++;
		 }
	 }
 }




