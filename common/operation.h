
#ifndef _operation_h_
#define _operation_h_

#define _WEBSOCKETPP_NOEXCEPT_
#define WEBSOCKETPP_CPP11_CHRONO


#include "redisCliMgr.h"
#include "common.h"
#include "writeRunInfo.h"
using namespace redis;
			


/************************************ defines *****************************************/
#define BAD_FILE_DESCRIPTOR		"Bad file descriptor"
#define SOCKET_BREAKE_PIPE		"Broken pipe"
#define REIDS_CONNECT_ERROR		"connection was closed"
#define REPLY_NOT_ONE			"expecting int reply of 1"
#define REPLY_NOT_INT			"unexpected prefix for integer reply"
#define REDIS_ERROR_NULL		""

#define REDIS_SET				"set"
#define REDIS_GET				"get"
#define REDIS_ZREVRANGE			"zvevrange"
#define REDIS_ZADD				"zadd"
#define REDIS_ZREM				"zrem"
#define REDIS_ZRANK				"zrank"
#define REDIS_ZCARD				"zcard"
#define REDIS_HMGET				"hmget"
#define REDIS_HDEL				"hdel"
#define REDIS_HSET				"hset"
#define REDIS_DEL				"del"
#define REDIS_ZRANGE			"zrange"
#define REDIS_ZCOUNT			"zcount"


#define  GPN_USR_PRE_KEY			"USR_"
#define  GPN_ROOM_PRE_KEY			"ROOM_"
/*------------------------------------------------------------------------------------*/



#define GET_REDIS_CLIENT(clientName) \
	RedisCliMgr* redisCliMgr = CMySingleton<RedisCliMgr>::GetInstance();\
	boost::shared_ptr<client> clientName = redisCliMgr->get();\
	RedisCliMgr::GiveBackGuard guard(*redisCliMgr, clientName);		

#define CATCH_REDIS_EXCEPTION(redisClient, func)\
	try\
	{\
		func;	\
	}\
	catch (std::exception &e)\
	{\
		if (string(e.what()) == SOCKET_BREAKE_PIPE || string(e.what()) == REIDS_CONNECT_ERROR || string(e.what()) == BAD_FILE_DESCRIPTOR)\
		{\
			WRITE_ERROR("connection_error|%s", e.what());\
			redisClient->reconnect();\
			func;\
		}\
		else\
		{\
			WRITE_ERROR(" error %s" << e.what());\
		}\
	}


#define GPN_OPERATION(clientName, func)\
	GET_REDIS_CLIENT(clientName);\
	CATCH_REDIS_EXCEPTION(clientName, func);

/************************************** ends ******************************************/


#endif




