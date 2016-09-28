/*
 * FileName:       
 * Author:         wangqifeng  Version: 1.0  Date: 2004-12-31
 * Description:    
 * Version:        
 * Function List:  
 *                 1.
 * History:        
 *     <author>   <time>    <version >   <desc>
 */
#ifndef __WRITE_RUN_INFO_H
#define  __WRITE_RUN_INFO_H
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include "threadmutex.h"
#include <vector>
#include <sstream>
using namespace std;

#define  MAXLOGSIZE  5000000

#define ONLINE_LOG_TYPE 50
#define RSERVER_LOG_TYPE 51
#define QSERVER_LOG_TYPE 52
#define CONNECT_LOG_TYPE 53
#define VPEER_LOG_TYPE	54
#define NAT_LOG_TYPE 55

#define PC_LOG_TYPE 56  //dijie 09-5-20
#define NC_LOG_TYPE 57  //dijie 09-7-8
#define STRACK_LOG_TYPE 58  //dijie 09-7-16


enum WRITELOGTYPE
{
		LOGTYPE=0,
		FATALTYPE=1,
		ERRORTYPE=2,
		WARNTYPE=3,
		INFOTYPE=4,
		DEBUGTYPE=5,
		TRACETYPE=6,
		DATATYPE=7
};

struct TLogType
{
        int nLogtype;
        int nFileNum;
        char sFileNameSuffix[256];
        int nLogOff;  //0:允许开关 2:固定打开的 不允许开关的
};

const int MAXLOGTYPE=100;        
class CLogTypeArray
{
public:
        CLogTypeArray();
        ~CLogTypeArray();
        TLogType& operator[](int nTypeIndex);
        void InsertLogType(TLogType &logType);
        int Size()
        {
                return m_nLogTypeNum;
        }
private:
        TLogType m_logTypeArray[MAXLOGTYPE];
        int m_nLogTypeNum;
};


#define WRITE_COMMON(T, typeName,type, tmplt, ...) \
    if (WriteRunInfo::LOGTYPEARRAY[type].nLogOff)\
    {\
        stringstream tmp_ss; \
        tmp_ss <<"["<<typeName<<"]|"<<tmplt << " FILE[" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "]"; \
        T(type, tmp_ss.str().c_str(), ##__VA_ARGS__);\
    }


#define WRITE_DAY(name,tmplt, ...)  \
{ \
    stringstream tmp_ss; \
    tmp_ss<<"|"<<__LINE__<<"|"<<tmplt; \
    WriteRunInfo::WriteDayLog(name,tmp_ss.str().c_str(), ##__VA_ARGS__); \
}

#define WRITE_LOG(tmplt, ...) \
	WRITE_COMMON(WriteRunInfo::WriteAllLog,"LOG", LOGTYPE, tmplt, ##__VA_ARGS__)

#define WRITE_FATAL(tmplt, ...) \
	WRITE_COMMON(WriteRunInfo::WriteAllLog,"FATAL", FATALTYPE, tmplt, ##__VA_ARGS__)
#define IS_FATAL  WriteRunInfo::LOGTYPEARRAY[FATALTYPE].nLogOff

#define WRITE_ERROR(tmplt, ...) \
	WRITE_COMMON(WriteRunInfo::WriteAllLog,"ERROR", ERRORTYPE, tmplt, ##__VA_ARGS__)
#define IS_ERROR  WriteRunInfo::LOGTYPEARRAY[ERRORTYPE].nLogOff

#define WRITE_WARN(tmplt, ...) \
	WRITE_COMMON(WriteRunInfo::WriteAllLog,"WARN", WARNTYPE, tmplt, ##__VA_ARGS__)
#define IS_WARN  WriteRunInfo::LOGTYPEARRAY[WARNTYPE].nLogOff

#define WRITE_INFO(tmplt, ...) \
	WRITE_COMMON(WriteRunInfo::WriteAllLog,"INFO", INFOTYPE, tmplt, ##__VA_ARGS__)
#define IS_INFO  WriteRunInfo::LOGTYPEARRAY[INFOTYPE].nLogOff

#define WRITE_DEBUG(tmplt, ...) \
	WRITE_COMMON(WriteRunInfo::WriteAllLog,"DEBUG", DEBUGTYPE, tmplt, ##__VA_ARGS__)
#define IS_DEBUG  WriteRunInfo::LOGTYPEARRAY[DEBUGTYPE].nLogOff

#define WRITE_TRACE(tmplt, ...) \
	WRITE_COMMON(WriteRunInfo::WriteAllLog,"TARCE", TRACETYPE, tmplt, ##__VA_ARGS__)
#define IS_TRACE  WriteRunInfo::LOGTYPEARRAY[TRACETYPE].nLogOff

#define WRITE_BIN(pBuf, nLen) \
    if (WriteRunInfo::LOGTYPEARRAY[TRACETYPE].nLogOff)\
    {\
        WriteRunInfo::WriteBufToTrace(pBuf, nLen); \
    }

#define WRITE_DAT( pData , iSize , format , ... ) \
	if( WriteRunInfo::LOGTYPEARRAY[TRACETYPE].nLogOff)\
		WriteRunInfo::WriteData( pData , iSize , format , ##__VA_ARGS__)
		
#define WRITE_ALL_LOG( Logtype , format , ... ) \
	if(WriteRunInfo::LOGTYPEARRAY[Logtype].nLogOff)\
		WriteRunInfo::WriteAllLog( Logtype ,format , ##__VA_ARGS__)

class WriteRunInfo
{
public:
        static void  Init(string fileName);    
        static void  WriteRunLog(const char * format, ...);
        static void  WriteLog(const char *format, ...);
        static void  WriteInfo(const char *format, ...);
        static void  WriteInfoRun(const char *format, ...);
        static void  WriteTrace(const char *format, ...);
        static void  WriteStat(const char *format, ...);
		static void  WriteHourStat(const char *name, const char *format, ...);
        static void  WritePrivateStat(const char *name, const char *format, ...);
        static void  WriteError(const char *format, ...);

        static void WriteDayLog(const char *name, const char *format, ...);
        static void  WriteAllLog(int nLogType, const char *format, ...);
                
        static void  SetLogFlag(int iModLogType, int iFlag);
        static void  WriteNowDetailTime();

        static void  InsertLogType(TLogType &logType);

	    static void  WriteData(const char *pData,int iSize, const char *format, ...);
	
        static void WriteBufToTrace(const char *pBuf, int nLen);
	    static CLogTypeArray LOGTYPEARRAY;	
private:
        /**
         *
         *  \param   *format
         *  \param   &ap
         *  \param   *sFileName
         *  \param   &nFileNameIndex
         *  \return   static void
         */
        static void DayLogWrite(const char *name, const char *format, va_list &ap);
        static void LogWrite(const char *format,  va_list &ap,  int nLogType);  

        static void LogWriteData(const char *pData,int iSize, const char *format,  va_list &ap,  int nLogType);  

        static  string m_sFileName;
		static  string m_sDayFileName; //服务器记录按天相关统计信息

        static char m_sStatFile[256];
        static CThreadMutex m_logMutex;        
};

#endif

