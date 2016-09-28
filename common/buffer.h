
#ifndef _BUFFER_H
#define _BUFFER_H

#include <arpa/inet.h>
#include <string.h>
#define  MAX_CVBUFFER_SIZE 64*1024

class CVBuffer
{
public:
	uint64_t ntohll(uint64_t old)
	{
		unsigned H = old >> 32;
		unsigned L = old & 0x00000000ffffffff;
		H = ntohl(H);
		L = ntohl(L);
		uint64_t ret = L;
		ret <<= 32;
		ret |= H;
		return ret;
	}
	
	uint64_t htonll(uint64_t old)
	{
		unsigned H = old >> 32;
		unsigned L = old & 0x00000000ffffffff;
		H = htonl(H);
		L = htonl(L);
		uint64_t ret = L;
		ret <<= 32;
		ret |= H;
		return ret;
	}
	
	CVBuffer(void);
	CVBuffer(const char* pBuffer,const unsigned int uSize){setBuffer(pBuffer,uSize);}
	~CVBuffer(void);
	
    const unsigned int getSize() const;
    const char* getBuffer() const;
    char* getBegin();

	bool setBuffer(const char* pBuffer,const unsigned int uSize);
	bool setSize(unsigned int uSize);

    bool storageData(unsigned short usLen, CVBuffer& buf);
    bool storageData(unsigned char ucLen, CVBuffer& buf);

	template <typename type>
	bool Set(const type& tData)
	{
		unsigned int uSize =sizeof(type);

		if(m_uOffset+uSize>MAX_CVBUFFER_SIZE)return false;

		memcpy(m_Buffer+m_uOffset,&tData,uSize);

		m_uOffset+=uSize;

		return true;
	}

	template <typename type>
	bool Get(type& tData)
	{
		unsigned int uSize =sizeof(type);

		if(m_uGetOffset+uSize>m_uOffset)return false;

		memcpy(&tData,m_Buffer+m_uGetOffset,uSize);

		m_uGetOffset+=uSize;

		return true;
	}

	bool AlignBuf();
	bool AddDataSize(unsigned int uSize);
	bool Get(char* pData, const unsigned int uLength);
    bool Copy(char* pData,unsigned int uLength);
    bool Append(const char* pData, const unsigned int uLength);

	void Jump(unsigned int uLength)
	{
		m_uGetOffset+=uLength;

		if( m_uGetOffset > m_uOffset )m_uGetOffset = m_uOffset;
	}

	//获取读取位置内存
	char* JumpBuffer()
	{
		if( m_uOffset > m_uGetOffset )
		{
			return (char*)(m_Buffer+m_uGetOffset);
		}

		return m_Buffer;
	}

    const char* JumpBuffer() const
    {
        if( m_uOffset > m_uGetOffset )
        {
            return (char*)(m_Buffer+m_uGetOffset);
        }

        return m_Buffer;
    }

	//获取读取位置剩余大小
	int RemainSize() const 
	{
		if( m_uOffset > m_uGetOffset )
		{
			return m_uOffset-m_uGetOffset;
		}

		return 0;
	}

    //获取未使用空间大小
    int NotUsedSize() const 
    {
        return MAX_CVBUFFER_SIZE - m_uOffset;
    }

	void updastart()
	{
		Set((char)0x0a);
	}
	void start()
	{
        Set((unsigned short)0x0);
		Set((char)0x02);
	}
    void astart()
	{
        Set((unsigned short)0x0);
		Set((char)0x0a);
    }
    void bstart()
    {
        Set((unsigned short)0x0);
        Set((char)0x0b);
    }
	void end()
	{
		Set((char)0x03);
	}

    void setTcpMsgLen(unsigned short usLen)
    {
        unsigned short usTempLen = htons(usLen);
		memcpy(m_Buffer,&usTempLen, sizeof(unsigned short));
    }

    void setMsgLen(unsigned short usLen)
    {
        unsigned short usTempLen = htons(usLen);
        memcpy(m_Buffer,&usTempLen, sizeof(unsigned short));
    }

    void setTLVLen(unsigned char ucLen)
    {
        memcpy(m_Buffer, &ucLen, sizeof(unsigned char));
    }

	void Clear();

	template <typename type>
	CVBuffer& operator << (const type& v) 
	{                
		type nv = v;
		if(sizeof(type)==sizeof(uint16_t))
			nv = (type)htons((uint16_t)v);
		else if (sizeof(type)==sizeof(uint32_t))
			nv = (type)htonl((uint32_t)v);
		else if (sizeof(type)==sizeof(uint64_t))
			nv = (type)htonll((uint64_t)v);

		Append((char *)&nv,sizeof(type));
		return *this;  
	}

	template <typename type>
	CVBuffer& operator >> (type& v) 
	{
		if(sizeof(type) == 1 ||sizeof(type) == 2 || sizeof(type) == 4 || sizeof(type) == 8)
		{
			v = 0;
		}

		type nv;
		bool b = Get((char *)&nv,sizeof(type));

		if(b)
		{
			v = nv;
			if(sizeof(type)==sizeof(uint16_t))
				v = (type)ntohs((uint16_t)nv);
			else if ( sizeof(type) == sizeof(uint32_t) )
				v = (type)ntohl((uint32_t)nv);
			else if ( sizeof(type) == sizeof(uint64_t) )
				v = (type)ntohll((uint64_t)nv);
			
		}
		
		return *this;  
	}
private:
	char m_Buffer[MAX_CVBUFFER_SIZE];
	unsigned int m_uOffset;
	unsigned int m_uGetOffset;
};

#endif
