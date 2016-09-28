#include "http_client.h"
#include <string>
#include <curl/curl.h>

bool CHttpClient::m_Debug=false;
CHttpClient::CHttpClient(void)
{

}
CHttpClient::~CHttpClient(void)
{
    
}

static int OnDebug(CURL *,curl_infotype itype,char* pData,size_t size,void *)
{
    if(itype == CURLINFO_TEXT)
    {
        //printf("[TEXT]%s\n", pData);
    }
    else if(itype == CURLINFO_HEADER_IN)
    {
        printf("[HEADER_IN]%s\n", pData);
    }
    else if(itype == CURLINFO_HEADER_OUT)
    {
        printf("[HEADER_OUT]%s\n", pData);
    }
    else if(itype == CURLINFO_DATA_IN)
    {
        printf("[DATA_IN]%s\n", pData);
    }
    else if(itype == CURLINFO_DATA_OUT)
    {
        printf("[DATA_OUT]%s\n", pData);
    }
    return 0;
}

static size_t OnWirteData(void* buffer,size_t size,size_t nmemb,void* lpVoid)
{
    std::string* str =dynamic_cast<std::string*>((std::string *) lpVoid);
    if(NULL == str || NULL == buffer)
    {
        return -1;
    }
    
    char* pData = (char*)buffer;
    str->append(pData,size* nmemb);
    return nmemb;
}

int CHttpClient::Post(const std::string &strUrl, const std::string &strPost, std::string &strResponse)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    if(m_Debug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE,1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION,OnDebug);
    }
    curl_easy_setopt(curl, CURLOPT_URL,strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWirteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2);//连接时间
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

int CHttpClient::Get(const std::string &strUrl, std::string &strResponse)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    printf("%d",NULL == curl?0:1);
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    if(m_Debug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE,1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION,OnDebug);
    }
    curl_easy_setopt(curl, CURLOPT_URL,strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION,NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWirteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,(void*)& strResponse);
    
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL,1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,20);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,20);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed:%s\n",curl_easy_strerror(res));
    }
    curl_easy_perform(curl);
    return res;
}

int CHttpClient::Posts(const std::string &strUrl, const std::string &strPost, std::string &strResponse,const char* pCaPath)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    if(m_Debug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE,1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION,OnDebug);
    }
    
    curl_easy_setopt(curl, CURLOPT_URL,strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST,1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,strPost.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION,NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,OnWirteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,(void*)&strResponse);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL,1);
    
    if(NULL == pCaPath)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    }else{
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,true);
        curl_easy_setopt(curl, CURLOPT_CAINFO,pCaPath);
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,10);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,10);
    res = curl_easy_perform(curl);
    curl_easy_perform(curl);
    return res;
}

int CHttpClient::Gets(const std::string &strUrl, std::string &strResponse,const char* pCaPath)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    if(m_Debug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE,1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION,OnDebug);
    }
    curl_easy_setopt(curl, CURLOPT_URL,strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION,NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,OnWirteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,(void*)&strResponse);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL,1);
    if(NULL == pCaPath)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,false);
    }else{
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,true);
        curl_easy_setopt(curl, CURLOPT_CAINFO,pCaPath);
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}
void CHttpClient::setDebug(bool bDebug)
{
    m_Debug = bDebug;
}
