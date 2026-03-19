#include "CPacket.h"

CPacket::CPacket(int iBufferSize)
{
    m_iBufferSize = iBufferSize;
    m_chpBuffer = new char[m_iBufferSize];
    Clear();
}

CPacket::~CPacket()
{
    if (m_chpBuffer != nullptr)
    {
        delete[] m_chpBuffer;
        m_chpBuffer = nullptr;
    }
}

void CPacket::Clear(void)
{
    m_iWritePos = 0;
    m_iReadPos = 0;
}

int CPacket::MoveWritePos(int iSize)
{
    if (iSize <= 0) return 0;

    if (m_iWritePos + iSize > m_iBufferSize) return 0;

    m_iWritePos += iSize;
    return iSize;
}

int CPacket::MoveReadPos(int iSize)
{
    if (iSize <= 0) return 0;

    if (m_iReadPos + iSize > m_iWritePos) return 0;

    m_iReadPos += iSize;
    return iSize;
}

CPacket& CPacket::operator =(CPacket& clSrcPacket)
{
    // 자기 자신을 대입하는 경우 방지
    if (this == &clSrcPacket)
        return *this;

    // 현재 버퍼 크기가 복사해 올 데이터보다 작다면 재할당
    if (m_iBufferSize < clSrcPacket.m_iBufferSize)
    {
        delete[] m_chpBuffer;
        m_iBufferSize = clSrcPacket.m_iBufferSize;
        m_chpBuffer = new char[m_iBufferSize];
    }

    // 데이터 복사 및 위치 동기화
    m_iWritePos = clSrcPacket.m_iWritePos;
    m_iReadPos = clSrcPacket.m_iReadPos;
    memcpy(m_chpBuffer, clSrcPacket.m_chpBuffer, m_iWritePos);

    return *this;
}