#include "common.h"
#include "http_client.h"
#include "json/json.h"
#include "writeRunInfo.h"

CMyserver *CMyserver::m_pMyServer=NULL;
 string Secd2Time(time_t lSecd)
{
	char psDateTime[50];
	int iYear, iMonth, iDay, iHour, iMin, iSec;
	struct tm tmstu;

	localtime_r(&lSecd, &tmstu);

	iYear = tmstu.tm_year+1900;
	iMonth = tmstu.tm_mon+1;
	iDay = tmstu.tm_mday;
	iHour = tmstu.tm_hour;
	iMin = tmstu.tm_min;
	iSec = tmstu.tm_sec;

	//sprintf(psDateTime, "%04d-%02d-%02d %02d:%02d:%02d", iYear, iMonth, iDay, iHour, iMin, iSec);
	snprintf(psDateTime, sizeof(psDateTime), "%04d-%02d-%02d %02d:%02d:%02d", iYear, iMonth, iDay, iHour, iMin, iSec);
	return string(psDateTime);
}

 string Secd2Date(time_t lSecd)
{
	char psDateTime[20] = {0};
	struct tm tmstu;
	localtime_r(&lSecd, &tmstu);
	//sprintf(psDateTime, "%04d%02d%02d", tmstu.tm_year+1900, tmstu.tm_mon + 1, tmstu.tm_mday);
	snprintf(psDateTime, sizeof(psDateTime), "%04d%02d%02d", tmstu.tm_year+1900, tmstu.tm_mon + 1, tmstu.tm_mday);
	return string(psDateTime);
}


 std::string ipToStr(unsigned int ip)
{
	struct in_addr inaddr;
	inaddr.s_addr = ip;
	return std::string(inet_ntoa(inaddr));
}

 std::string ipToStrHton(unsigned int ip)
{
	struct in_addr inaddr;
	inaddr.s_addr = htonl(ip);
	return std::string(inet_ntoa(inaddr));
}

int GetUdpServerAddr(const string& sDisPatchAddr, uint16_t iUpSrvType, uint16_t& iSelfType, string& sAddr, uint16_t& iPort)
{
	string sRsp;
	int iRet = CHttpClient::Get(sDisPatchAddr, sRsp);
	if (iRet)
	{
		WRITE_ERROR("get_udp_server_addr url:%s error:%d ", sDisPatchAddr.c_str(), iRet);
		return -1;
	}

	Json::Reader reader;
	Json::Value JRspData;
	if (!reader.parse(sRsp.c_str(), JRspData))
	{
		WRITE_ERROR(" parse json data error:%s", sRsp.c_str());
		return -1;
	}

	string sErrCode = JRspData["errno"].asString();
	if (sErrCode != "200")
	{
		WRITE_ERROR("%s %d sErrCode != 200", __FUNCTION__, __LINE__);
		return -1;
	}

	sAddr = JRspData["ip"].asString();
	iPort = atoi(JRspData["port"].asString().c_str());
	iUpSrvType = atoi(JRspData["type"].asString().c_str());
	return 0 ;
}

std::string TBroadCast::ToJson()
{
	Json::Value root;
	root["uuid"] = sUUID;
	root["roomid"] = sGroupId;
	root["alias"] = sNick;
	root["msgtype"] = sType;
	root["data"] = sMsgData;	

	Json::FastWriter writer;
	std::string  sRsp = writer.write(root);

	return sRsp;
}

std::string TRouteMsg::ToJson()
{
	Json::Value root;
	root["uid"] = sUUID;
	root["rid"] = sGroupId;
	root["rtype"] = sType;
	root["msg_data"] = sMsgData;
	root["msg_type"] = sMsgType;

	Json::FastWriter writer;
	std::string  sRsp = writer.write(root);

	return sRsp;
}

