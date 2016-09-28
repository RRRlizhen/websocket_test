#include "work_thread.h"
#include "writeRunInfo.h"
#include "common.h"
#include "buffer.h"
#include "common.pb.h"
#include "PlugContext.h"
#include "basetask.h"
using namespace vipkid;

CWorkThread::CWorkThread()
{
	m_nRunFlag = 0;
}

CWorkThread::~CWorkThread()
{
	while (m_pMsgList.size())
	{
		CSendMsg * pCurMsg = m_pMsgList.get();

		delete pCurMsg;
	}
}

void *CWorkThread::ThreadEntry(void * arg)
{
	CWorkThread *workThread = (CWorkThread*)arg;
	int tmpRet = 0;

	workThread->Run();

	return NULL;
}

int CWorkThread::Start()
{
	int ret = pthread_create(&m_nThreadId, NULL, ThreadEntry, (void*)this);
	if (ret != 0)
	{
		WRITE_ERROR("Create thread fail, error is %d.\n", strerror(errno));
		return -1;
	}         

	return 0;
}


int CWorkThread::Run()
{
	while (m_nRunFlag)
	{

			//处理消息
			CSendMsg *pMsg =  m_pMsgList.get();
			if (pMsg != NULL)
			{
				Process(pMsg);
				delete pMsg;
			}
	}

	return 0;
}

int CWorkThread::PutMsgToInputList(CSendMsg * pMsg)
{
	m_pMsgList.put(pMsg);

	return 0;
}

int CWorkThread::Process(CSendMsg* pMsg)
{
	CVBuffer iRecvBuf(pMsg->GetBody(), pMsg->GetLen());
	unsigned short usHeadLen = 0, usBodyLen = 0;

	iRecvBuf >> usHeadLen;
	SSHead pbSSHead;
	if ( !pbSSHead.ParseFromArray(iRecvBuf.JumpBuffer(), usHeadLen) )
	{
		WRITE_ERROR("CWorkThread::Process %d pbSSHead.ParseFromArray error", __LINE__);
		return -1;
	}

	iRecvBuf.Jump(usHeadLen);
	iRecvBuf >> usBodyLen;

	if (iRecvBuf.RemainSize() != usBodyLen)
	{
		WRITE_ERROR("CWorkThread::Process:%d remainSize:%d usBodyLen:%d", __LINE__, iRecvBuf.RemainSize(), usBodyLen);
		return -1;
	}

	int uiCmd = pbSSHead.command();
	int uiSubCmd = 0;

	TCmdCallbackMap &oCallbackMap = GetGlobalCmdMap();
	TCmdCallbackMap::const_iterator cit = oCallbackMap.find(SCmd(uiCmd, uiSubCmd));
	if (oCallbackMap.end() == cit)
	{
		WRITE_ERROR("Main unkow dataop object. cmd: %d, subcmd: %d\n", uiCmd, uiSubCmd);
		return -1;
	}

	const SCallback &oCallback = cit->second;
	if (NULL == oCallback.pFactory)
	{
		WRITE_ERROR("CWorkThread::Process cmd: %d, subcmd: %d\n", uiCmd, uiSubCmd);
		return -1;
	}

	string sBuf = string(iRecvBuf.JumpBuffer(), iRecvBuf.RemainSize());
	std::auto_ptr<CDataOpBase> pObject;
	pObject.reset(oCallback.pFactory->CreateDataObject(&sBuf));
	if (NULL == pObject.get())
	{
		WRITE_ERROR("Main CreateDataObject failed. cmd: %d, subcmd: %d \n", uiCmd, uiSubCmd);
		return -1;
	}

	string strResult;
	pObject->SetCmd(uiCmd);
	int iRet = pObject->Process(strResult);
	if (iRet)
	{
		WRITE_ERROR("%s %d Process error", __FUNCTION__, __LINE__);
		return -1;
	}

	if (!strResult.empty())
	{
		std::string sIp;
		uint16_t iPort;
		string sDispAdr;
		uint16_t iSrvType, iSelfType;
		int iRet = GetUdpServerAddr(sDispAdr, iSrvType, iSelfType, sIp, iPort);
		if(iRet)
		{
			WRITE_ERROR("GetUdpServerAddr error");
			return -1;
		}

		CMyserver::GetInstance()->SendToUdpServer(uiCmd, strResult, iSelfType, sIp, iPort);		
	}
	
	return 0;

}
