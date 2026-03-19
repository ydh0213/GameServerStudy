#pragma once
#ifndef __PACKET__
#define __PACKET__

#include <cstring>

class CPacket
{
public:
    CPacket(int iBufferSize);
    virtual ~CPacket();

    void Clear(void);

    int GetBufferSize(void) { return m_iBufferSize; }
    int GetDataSize(void) { return m_iWritePos; }
    char* GetBufferPtr(void) { return m_chpBuffer; }

    int MoveWritePos(int iSize);
    int MoveReadPos(int iSize);

    CPacket& operator =(CPacket& clSrcPacket);

    CPacket& operator <<(unsigned char byValue);
    CPacket& operator >>(unsigned char& byValue);

    CPacket& operator <<(char chValue);
    CPacket& operator >>(char& chValue);

    CPacket& operator <<(short shValue);
    CPacket& operator >>(short& shValue);

    CPacket& operator <<(unsigned short wValue);
    CPacket& operator >>(unsigned short& wValue);

    CPacket& operator <<(int iValue);
    CPacket& operator >>(int& iValue);

    CPacket& operator <<(long lValue);
    CPacket& operator >>(long& lValue);

    CPacket& operator <<(float fValue);
    CPacket& operator >>(float& fValue);

    CPacket& operator <<(long long llValue);
    CPacket& operator >>(long long& llValue);

    CPacket& operator <<(double dValue);
    CPacket& operator >>(double& dValue);

    int GetData(char* chpDest, int iSize);
    int PutData(char* chpSrc, int iSrcSize);

protected:
    char* m_chpBuffer;
    int m_iBufferSize;
    int m_iWritePos;
    int m_iReadPos;
};

inline int CPacket::PutData(char* chpSrc, int iSrcSize)
{
    if (m_iWritePos + iSrcSize > m_iBufferSize)
        return 0;

    memcpy(&m_chpBuffer[m_iWritePos], chpSrc, iSrcSize);
    m_iWritePos += iSrcSize;

    return iSrcSize;
}

inline int CPacket::GetData(char* chpDest, int iSize)
{
    if (m_iReadPos + iSize > m_iWritePos)
        return 0;

    memcpy(chpDest, &m_chpBuffer[m_iReadPos], iSize);
    m_iReadPos += iSize;

    return iSize;
}

// ========================================================================
// inline 연산자 오버로딩 구현부 (모든 자료형 추가)
// 매크로를 쓰면 코드를 줄일 수 있지만, 직관적인 이해를 위해 모두 풀어썼습니다.
// ========================================================================
inline CPacket& CPacket::operator <<(unsigned char byValue) { PutData((char*)&byValue, sizeof(unsigned char)); return *this; }
inline CPacket& CPacket::operator >>(unsigned char& byValue) { GetData((char*)&byValue, sizeof(unsigned char)); return *this; }

inline CPacket& CPacket::operator <<(char chValue) { PutData((char*)&chValue, sizeof(char)); return *this; }
inline CPacket& CPacket::operator >>(char& chValue) { GetData((char*)&chValue, sizeof(char)); return *this; }

inline CPacket& CPacket::operator <<(short shValue) { PutData((char*)&shValue, sizeof(short)); return *this; }
inline CPacket& CPacket::operator >>(short& shValue) { GetData((char*)&shValue, sizeof(short)); return *this; }

inline CPacket& CPacket::operator <<(unsigned short wValue) { PutData((char*)&wValue, sizeof(unsigned short)); return *this; }
inline CPacket& CPacket::operator >>(unsigned short& wValue) { GetData((char*)&wValue, sizeof(unsigned short)); return *this; }

inline CPacket& CPacket::operator <<(int iValue) { PutData((char*)&iValue, sizeof(int)); return *this; }
inline CPacket& CPacket::operator >>(int& iValue) { GetData((char*)&iValue, sizeof(int)); return *this; }

inline CPacket& CPacket::operator <<(long lValue) { PutData((char*)&lValue, sizeof(long)); return *this; }
inline CPacket& CPacket::operator >>(long& lValue) { GetData((char*)&lValue, sizeof(long)); return *this; }

inline CPacket& CPacket::operator <<(float fValue) { PutData((char*)&fValue, sizeof(float)); return *this; }
inline CPacket& CPacket::operator >>(float& fValue) { GetData((char*)&fValue, sizeof(float)); return *this; }

inline CPacket& CPacket::operator <<(long long llValue) { PutData((char*)&llValue, sizeof(long long)); return *this; }
inline CPacket& CPacket::operator >>(long long& llValue) { GetData((char*)&llValue, sizeof(long long)); return *this; }

inline CPacket& CPacket::operator <<(double dValue) { PutData((char*)&dValue, sizeof(double)); return *this; }
inline CPacket& CPacket::operator >>(double& dValue) { GetData((char*)&dValue, sizeof(double)); return *this; }

#endif // __PACKET__