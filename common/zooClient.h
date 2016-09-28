/*
 * =====================================================================================
 * 
 *       Filename:  zooClient.h
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
#ifndef		_ZOOCLIENT_H_
#define		_ZOOCLIENT_H_


#include <zookeeper/zookeeper.h>
#include <zookeeper/zookeeper_log.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "common.h"
#include "writeRunInfo.h"
using namespace std;
using namespace boost;
class zookCallBack
{
public:

	boost::function<void (const vector<HostInfo>& vecInfo, const EOPT& opt) > func;

	zookCallBack(){}
	zookCallBack(boost::function<void (const vector<HostInfo>& vecInfo, const EOPT& opt) > f )
	{
		func = f;
	}

	void operator() (const vector<HostInfo>& vecInfo, const EOPT& opt)
	{
		if (func)
		{
			func(vecInfo, opt);
		}
	}
};

struct TCallBackObj 
{
	TCallBackObj()
	{

	}
	TCallBackObj(const zookCallBack& arg_func)
	{
		callBack = arg_func;
	}
	zookCallBack		callBack;
    vector<HostInfo>	vecHost;
};

typedef vector<string> string_vector;
typedef map<string, zookCallBack> string_callFunc_map;
typedef map<string, TCallBackObj> string_callBack_map;
typedef string_callBack_map::iterator str_call_itor;
class zookClient
{
public:
	explicit zookClient(const HostInfo& localInfo, const string& serverList, const string_callFunc_map& callFunc = string_callFunc_map()) ;
	int init();
	/*
		同步方法
	*/
	int zooCreate(const string& path, const string& value, int len, const struct ACL_vector& acl, int flags, string& new_path);
	int zoo_delete(const string& path, int version = -1);
	int zoo_wexists(const string& path, watcher_fn watcher, void* watcherCtx, struct Stat* stat);
	int zoo_exists(const string& path, int watch, struct Stat* stat);
	int zoo_wget(const string& path, watcher_fn watcher, void* watcherCtx, string& out_data, Stat* stat);
	int zoo_set(const string& path, const string& data, int version);
	int zoo_wget_children(const string& path, watcher_fn watcher, void* watcherCtx, string_vector& node_list);
	/*
		异步方法
	*/
	int zoo_acreate(const string& path, const string& node_data, const struct ACL_vector& acl, int flags, string_completion_t completion, const void* data);
	int zoo_adelete(const string& path, int version,void_completion_t completion, const void* data);
	int zoo_awexists(const string& path, watcher_fn watcher, void* watcherCtx, stat_completion_t completion, const void* data);
	int zoo_awget(const string& path, watcher_fn watcher, void* watcherCtx, data_completion_t completion, const void* data);
	int zoo_aset(const string& path, const string& node_data, int version, stat_completion_t completion, const void *data);
	int zoo_awget_children(const string& path, watcher_fn watcher, void* watcherCtx, strings_completion_t completion, const void* data);

	/*
		回调函数
	*/
	void static g_watcher_init_function(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);
	void static create_completion_function(int rc, const char *value, const void *data);
	void static delete_completion_function(int rc, const void *data);
	void static awexists_completion_function(int rc, const struct Stat *stat, const void *data);
	void static wget_completion_function(int rc, const char *value, int value_len, const struct Stat *stat, const void *data);
	void static awget_children_completion_function(int rc, const struct String_vector *strings, const void *data);
	void static watcher_process(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx);

	/*
		自定义方法
	*/

	void registerSelf();
	void observer();
	void updateServer(const string& path);
	void delServer(const string& path);
	void getServerList(const string& serverName, vector<HostInfo>& vecInfo);

protected:
private:
	HostInfo			_localInfo;				//zookeeper Client对用的模块的 信息
	string				_zooServerList;			// zookeeper 的机器列表
	watcher_fn			_g_wfn;
	int					_session_timeout;
	zhandle_t*			_zhandle_t ;
	string_callBack_map	_mapCallBack;		   // 有host变更时需要调用对用服务的回调函数
};
int SplitString(const string &srcStr,const string &delim, vector<string>& strList);
void parseHostInfo(const string& path, HostInfo& info);
string generateAddrInfo(const HostInfo& info);
void compareHostInfo(const vector<HostInfo>& newinfos, const vector<HostInfo>& oldinfos, vector<HostInfo>& addlist, vector<HostInfo>& minuslist);
#endif

