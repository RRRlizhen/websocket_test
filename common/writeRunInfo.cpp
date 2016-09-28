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

#define _WEBSOCKETPP_NOEXCEPT_  		


#define WEBSOCKETPP_CPP11_CHRONO 	


#include "writeRunInfo.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include "common.h"
#include <vector>
#include <errno.h>
#include <string.h>


string WriteRunInfo::m_sFileName = "test";
string WriteRunInfo::m_sDayFileName = "day/";
CThreadMutex WriteRunInfo::m_logMutex;
char WriteRunInfo::m_sStatFile[256];
CLogTypeArray  WriteRunInfo::LOGTYPEARRAY;

CLogTypeArray::CLogTypeArray()
{        
        TLogType tmpLogType = {LOGTYPE, 0, ".frame", 2};
        memcpy(&m_logTypeArray[LOGTYPE], &tmpLogType, sizeof(TLogType));
        
        TLogType tmpLogType1 = {FATALTYPE, 0, ".error", 1};        
        memcpy(&m_logTypeArray[FATALTYPE], &tmpLogType1, sizeof(TLogType));
        
        TLogType tmpLogType2 =  {ERRORTYPE, 0, ".error", 1};
        memcpy(&m_logTypeArray[ERRORTYPE], &tmpLogType2, sizeof(TLogType));
        
        TLogType tmpLogType3 =  {WARNTYPE, 0, ".warn", 1};
        memcpy(&m_logTypeArray[WARNTYPE], &tmpLogType3, sizeof(TLogType));
        
        TLogType tmpLogType4 = {INFOTYPE, 0, ".trace", 0};
        memcpy(&m_logTypeArray[INFOTYPE], &tmpLogType4, sizeof(TLogType));
        
        TLogType tmpLogType5 = {DEBUGTYPE, 0, ".trace", 0};
        memcpy(&m_logTypeArray[DEBUGTYPE], &tmpLogType5, sizeof(TLogType));

		TLogType tmpLogType6 = {TRACETYPE, 0, ".trace", 0};
		memcpy(&m_logTypeArray[TRACETYPE], &tmpLogType6, sizeof(TLogType));

		TLogType tmpLogType7 = {DATATYPE, 0, ".data", 1};
		memcpy(&m_logTypeArray[DATATYPE], &tmpLogType7, sizeof(TLogType));

	
        m_nLogTypeNum = 8;
}

TLogType& CLogTypeArray::operator[](int nTypeIndex)
{
        if (nTypeIndex >=  MAXLOGTYPE)
        {
                nTypeIndex = 0;
        }
        return m_logTypeArray[nTypeIndex];
}

CLogTypeArray::~CLogTypeArray()
{
}

void CLogTypeArray::InsertLogType(TLogType &logType)
{
        if (logType.nLogtype <= 7 || logType.nLogtype >= MAXLOGTYPE)
        {
                return;
        }
        memcpy(&m_logTypeArray[logType.nLogtype], &logType, sizeof(TLogType));
}



/***************************************writerunlog*******************************************************/
void WriteRunInfo::Init(string fileName)
{
        if (fileName.find("/") == string::npos)
        {
            m_sFileName = "logs/" +  fileName;               
        }
        else
        {
            m_sFileName = fileName;
        }
}


void  WriteRunInfo::WriteRunLog(const char * format, ...)
{
        if (LOGTYPEARRAY[INFOTYPE].nLogOff == 0)
        {
                return;
        }
        
        va_list ap;
        va_start(ap, format);
        LogWrite(format, ap, INFOTYPE);
        va_end(ap);
        return;
}


void  WriteRunInfo::WriteLog(const char *format, ...)
{
        va_list ap;
        va_start(ap, format);
        LogWrite(format, ap, LOGTYPE);
        va_end(ap);
        return;
}

void  WriteRunInfo::WriteStat(const char *format, ...)
{
        m_logMutex.Lock();
        FILE *fp;

        time_t nowTime = time(NULL);
        tm tmpTm;
        localtime_r((const time_t*)&nowTime, &tmpTm);        
        //sprintf(m_sStatFile, "stat/%s_%04d-%02d-%02d", m_sFileName.c_str(),  1900+tmpTm.tm_year, tmpTm.tm_mon+1, tmpTm.tm_mday);
        snprintf(m_sStatFile, sizeof(m_sStatFile), "stat/%s_%04d-%02d-%02d", m_sFileName.c_str(),  1900+tmpTm.tm_year, tmpTm.tm_mon+1, tmpTm.tm_mday);
        
                
        fp = fopen(m_sStatFile, "a+");
        if (fp== NULL)
        {                
                m_logMutex.UnLock();
                WriteLog("m_sStatFile is %s, error is %s",m_sStatFile,  strerror(errno));
                return;
        }               

        fprintf(fp, "[---%02d%02d%02d---]", tmpTm.tm_hour, tmpTm.tm_min, tmpTm.tm_sec);
        va_list ap;
        va_start(ap, format);
        vfprintf(fp, format, ap);
        va_end(ap);

        fprintf(fp, "\n");
        fclose(fp);               
        m_logMutex.UnLock();
        return;
}

void WriteRunInfo::WriteHourStat(const char *name, const char *format, ...)
{
	m_logMutex.Lock();
	FILE *fp;
	time_t nowTime = time(NULL);
	tm tmpTm;
	localtime_r((const time_t*)&nowTime, &tmpTm);        
	snprintf(m_sStatFile, sizeof(m_sStatFile), "stat/%s_%s_%04d-%02d-%02d-%02d", m_sFileName.c_str(), name, 1900+tmpTm.tm_year, tmpTm.tm_mon+1, tmpTm.tm_mday, tmpTm.tm_hour);

	fp = fopen(m_sStatFile, "a+");
	if (NULL == fp)
	{
		m_logMutex.UnLock();
		WriteLog("m_sStatFile is %s, error is %s", m_sStatFile, strerror(errno));
		return;
	}

	//fprintf(fp, "[---%02d%02d%02d---]", tmpTm.tm_hour, tmpTm.tm_min, tmpTm.tm_sec);
	va_list ap;
	va_start(ap, format);
	vfprintf(fp, format, ap);
	va_end(ap);
	fprintf(fp, "\n");
	fclose(fp);               

	m_logMutex.UnLock();
	return ;
}


void WriteRunInfo::WritePrivateStat(const char *name, const char *format, ...)
{
	m_logMutex.Lock();
	FILE *fp;
	time_t nowTime = time(NULL);
	tm tmpTm;
	localtime_r((const time_t*)&nowTime, &tmpTm);        
	snprintf(m_sStatFile, sizeof(m_sStatFile), "stat/%s_%s_%04d-%02d-%02d", m_sFileName.c_str(), name, 1900+tmpTm.tm_year, tmpTm.tm_mon+1, tmpTm.tm_mday);

	fp = fopen(m_sStatFile, "a+");
	if (NULL == fp)
	{
		m_logMutex.UnLock();
		WriteLog("m_sStatFile is %s, error is %s", m_sStatFile, strerror(errno));
		return;
	}

	//fprintf(fp, "[---%02d%02d%02d---]", tmpTm.tm_hour, tmpTm.tm_min, tmpTm.tm_sec);
	va_list ap;
	va_start(ap, format);
	vfprintf(fp, format, ap);
	va_end(ap);
	fprintf(fp, "\n");
	fclose(fp);               

	m_logMutex.UnLock();
	return ;
}

void  WriteRunInfo::WriteInfo(const char *format, ...)
{
        va_list ap;
        va_start(ap, format);
        LogWrite(format, ap, INFOTYPE);
        va_end(ap);
        return;
}

void  WriteRunInfo::WriteInfoRun(const char *format, ...)
{     
        va_list ap;
        va_start(ap, format);
        LogWrite(format, ap, INFOTYPE);
        va_end(ap);           
        return;
}


void  WriteRunInfo::WriteTrace(const char *format, ...)
{
        if (LOGTYPEARRAY[TRACETYPE].nLogOff == 0)
        {
                return;
        }
        
        va_list ap;
        va_start(ap, format);
        LogWrite(format, ap, TRACETYPE);
        va_end(ap);
        return;
}


void  WriteRunInfo::WriteError(const char *format, ...)
{
        va_list ap;
        va_start(ap, format);
        LogWrite(format, ap, ERRORTYPE);
        va_end(ap);
}


void  WriteRunInfo::SetLogFlag(int iModLogType, int iFlag)
{
        if (iModLogType < 0 || iModLogType >= MAXLOGTYPE)
        {
                return;
        }

        if (LOGTYPEARRAY[iModLogType].nLogOff == 2) // 如果是2, 不允许改变的
        {
                return;
        }
        
        LOGTYPEARRAY[iModLogType].nLogOff = iFlag;
        return;
}


void WriteRunInfo::WriteNowDetailTime()
{
        if (LOGTYPEARRAY[TRACETYPE].nLogOff == 0)
        {
                return;
        }        

        
        timeval tm;
        gettimeofday(&tm, NULL);
        WriteTrace("Now is %d second and %d milsecond", tm.tv_sec, tm.tv_usec);
}

void WriteRunInfo::WriteDayLog(const char *name, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    DayLogWrite(name,format,ap);
    va_end(ap);
    return ;
}

void WriteRunInfo::DayLogWrite(const char *name, const char *format, va_list &ap)
{
    time_t i64Now = time(NULL);
    string sFileName = m_sDayFileName+"stat_day_"+string(name)+".log."+::Secd2Date(i64Now);
    m_logMutex.Lock();        
    FILE *fp;
    fp = fopen(sFileName.c_str(), "a+");
    if (fp== NULL) {
        m_logMutex.UnLock();
        return;
    }
    string dateTime = ::Secd2Time(time(NULL));
    fprintf(fp, "%s: ",::Secd2Time(i64Now).c_str());
    vfprintf(fp, format, ap);
    fprintf(fp, "\n");
    fclose(fp);   
    m_logMutex.UnLock();
}

void WriteRunInfo::WriteAllLog(int nLogType, const char *format, ...)
{
        if (nLogType < 0 || nLogType >= MAXLOGTYPE)
        {
                return;
        }
        va_list ap;
        va_start(ap, format);
        LogWrite(format, ap, nLogType);
        va_end(ap);
        return;
}

void WriteRunInfo::LogWrite(const char *format,  va_list &ap, int nLogType)
{               
        if (nLogType < 0 || nLogType >= MAXLOGTYPE)
        {
                return;
        }

        if (LOGTYPEARRAY[nLogType].nLogOff == 0)
        {
                return;
        }
        
        m_logMutex.Lock();        
        string sFileName = m_sFileName + string(LOGTYPEARRAY[nLogType].sFileNameSuffix);
        FILE *fp;
        struct stat statBuf;
        int ret = stat(sFileName.c_str(), &statBuf);        
        if (ret == 0)
        {
                if (statBuf.st_size > MAXLOGSIZE)
                {
                        LOGTYPEARRAY[nLogType].nFileNum ++;
                        LOGTYPEARRAY[nLogType].nFileNum = LOGTYPEARRAY[nLogType].nFileNum%10;
                        char newPath[1024];
                        //sprintf(newPath, "%s_%02d", sFileName.c_str(), LOGTYPEARRAY[nLogType].nFileNum);
                        snprintf(newPath, sizeof(newPath), "%s_%02d", sFileName.c_str(), LOGTYPEARRAY[nLogType].nFileNum);
                        rename(sFileName.c_str(), newPath);
                }
        }
                
        fp = fopen(sFileName.c_str(), "a+");
        if (fp== NULL)
        {
                m_logMutex.UnLock();
                return;
        }       
        string dateTime = ::Secd2Time(time(NULL));
        fprintf(fp, "%s: ", dateTime.c_str());
        vfprintf(fp, format, ap);
        fprintf(fp, "\n");
        fclose(fp);   
        m_logMutex.UnLock();
}

void  WriteRunInfo::WriteData(const char *pData,int iSize, const char *format, ...)
{
     	 va_list ap;
        va_start(ap, format);
        LogWriteData(pData, iSize, format, ap, DATATYPE);
        va_end(ap);
        return;
}

void WriteRunInfo::LogWriteData(const char *pData,int iSize, const char *format,  va_list &ap,  int nLogType)
{
	if (nLogType < 0 || nLogType >= MAXLOGTYPE)
        {
                return;
        }

        if (LOGTYPEARRAY[nLogType].nLogOff == 0)
        {
                return;
        }
        
        m_logMutex.Lock();        
        string sFileName = m_sFileName + string(LOGTYPEARRAY[nLogType].sFileNameSuffix);
        FILE *fp;
        struct stat statBuf;
        int ret = stat(sFileName.c_str(), &statBuf);        
        if (ret == 0)
        {
                if (statBuf.st_size > MAXLOGSIZE)
                {
                        LOGTYPEARRAY[nLogType].nFileNum ++;
                        LOGTYPEARRAY[nLogType].nFileNum = LOGTYPEARRAY[nLogType].nFileNum%10;
                        char newPath[1024];
                        //sprintf(newPath, "%s_%02d", sFileName.c_str(), LOGTYPEARRAY[nLogType].nFileNum);
                        snprintf(newPath, sizeof(newPath), "%s_%02d", sFileName.c_str(), LOGTYPEARRAY[nLogType].nFileNum);
                        rename(sFileName.c_str(), newPath);
                }
        }
                
        fp = fopen(sFileName.c_str(), "a+");
        if (fp== NULL)
        {
                m_logMutex.UnLock();
                return;
        }       
        string dateTime = ::Secd2Time(time(NULL));
        fprintf(fp, "%s: ", dateTime.c_str());
        vfprintf(fp, format, ap);
        fprintf(fp, "\n");
	// add data
	fwrite(pData,  sizeof(char), iSize, fp);
       fprintf(fp, "\n");
		
        fclose(fp);   
        m_logMutex.UnLock();
}
void WriteRunInfo::WriteBufToTrace(const char *pBuf, int nLen)
{
	
	int nCh = 0;
	char iConVert[10];	
	int nCnt = nLen%16 == 0 ? nLen/16 : nLen/16+1;	
	for(int i=0; i<nCnt; i++)
	{
		string sTmp;
		for(int j=0; j<16&&nCh<nLen; j++)
		{
			sprintf(iConVert, "%X%X ", (pBuf[nCh]>>4)&0xF, pBuf[nCh]&0xF);
			sTmp += iConVert, nCh++;
		}
		WriteRunInfo::WriteTrace("%s", sTmp.c_str());
		sTmp.clear();
	}
}


        
