#ifndef __Curl_HttpCurl__
#define __Curl_HttpCurl__

#include "stdio.h"
#include <curl/curl.h>
#include <string>
class CHttpClient
{
public:
    CHttpClient(void);
    ~CHttpClient(void);
    
    /**
     *@brief HTTP POST请求
     *@param strUrl 输入参数，请求Url地址，如:http://www.baidu.com
     *@param strPost 输入参数，使用如下格式:para1=val1¶2=val2
     *@param strResponse 输出参数，返回内容
     *@return 返回是否Post成功
     */
    static int Post(const std::string& strUrl,const std::string& strPost,std::string& strResponse);
    
    /**
     *@brief HTTP GET请求
     *@param strUrl 输入参数，请求的Url地址，如:https://www.alipay.com
     *@param strResponse 输出参数，返回的内容
     *@return 返回是否成功
     */
    static int Get(const std::string& strUrl,std::string& strResponse);
    
    /**
     *@brief HTTPS POST请求，无证书版本
     *@param strUrl 输入参数，请求的Url地址，如：https://www.alipay.com
     *@param strPost 输入参数，使用格式如下：para1=val1¶2=val2
     *@param strResponse 输出参数，返回的内容
     *@param pCaPath 输入参数，为CA证书的路径，如果输入为NULL，则不验证服务器端证书的有效性
     *@return 返回是否Posts成功
     */
    static int Posts(const std::string& strUrl,const std::string& strPost,std::string& strResponse,const char* pCaPath = NULL);
    
    /**
     *@brief HTTPS GET请求，无证书版
     *@param strUrl 输入参数，请求的Url地址，如https://www.alipay.com
     *@param strResponse 输出参数，返回的内容
     *@param pCaPath 输入参数，为CA证书的路径，如果输入为NULL，则不验证服务端证书的有效性
     *@return 返回是否Gets成功
     */
    static int Gets(const std::string& strUrl,std::string& strResponse,const char* pCaPath = NULL);
    
public:
    static void setDebug(bool bDebug);
    static bool m_Debug;
};

#endif /* defined(__Curl_HttpCurl__) */
