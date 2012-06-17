#include "StdAfx.h"
#include "MemoryPool.h"

#define LI_LOGGER_TAG "MemoryPool"

const SIZE_T MemoryPool::CHUNK_HEADER_SIZE = sizeof(unsigned char*);


MemoryPool::MemoryPool(void):
m_ppRawMemArray(NULL), m_memArraySize(0), m_pHead(NULL), m_allocated(0)
{
}


MemoryPool::~MemoryPool(void)
{
#if defined(_DEBUG) || defined(PROFILE)
    if(m_allocated != 0)
    {
        LI_LOG_WITH_TAG("remaining allocated space!");
    }
#endif
    for(UINT i=0; i < m_memArraySize; ++i) {
        delete[] m_ppRawMemArray[i];
    }
    delete[] m_ppRawMemArray;
}


bool MemoryPool::Init(INT p_chunkSize, INT p_numChunks, bool p_resizable)
{
    m_chunkSize = p_chunkSize;
    m_numChunks = p_numChunks;
    m_resizable = p_resizable;
    return this->GrowMemory();
}


bool MemoryPool::GrowMemory(void)
{
    unsigned char** ppMemArray = new unsigned char*[m_memArraySize+1];
    for(UINT i=0; i < m_memArraySize; ++i)
    {
        ppMemArray[i] = m_ppRawMemArray[i];
    }
    ppMemArray[m_memArraySize] = this->CreateNewMemoryBlock();

    if(m_pHead != NULL)
    {
        unsigned char* pCurrent = m_pHead;
        unsigned char* pNext = this->GetNext(pCurrent);
        while(pNext != NULL) {
            pCurrent = pNext;
            pNext = this->GetNext(pCurrent);
        }
        this->SetNext(pCurrent, ppMemArray[m_memArraySize]);
    }
    else
    {
        m_pHead = ppMemArray[m_memArraySize];
    }

    ++m_memArraySize;
    delete[] m_ppRawMemArray;
    m_ppRawMemArray = ppMemArray;
    return true;
}


unsigned char* MemoryPool::CreateNewMemoryBlock(void)
{
    size_t blockSize = CHUNK_HEADER_SIZE + m_chunkSize;
    size_t memBlockSize = m_numChunks * blockSize;

    unsigned char* pMemBlock = new unsigned char[memBlockSize];
    unsigned char* pCurrent = pMemBlock;
    unsigned char* pEnd = pMemBlock + memBlockSize;
    while(pCurrent < pEnd)
    {
        unsigned char* pNext = pCurrent + blockSize;
        unsigned char** pChunkHeader = (unsigned char**)pCurrent;
        pChunkHeader[0] = (pNext < pEnd ? pNext : NULL);
        pCurrent += blockSize;
    }
    return pMemBlock;
}


unsigned char* MemoryPool::GetNext(unsigned char* p_pBlock) const
{
    unsigned char** pChunkHeader = (unsigned char**)p_pBlock;
    return pChunkHeader[0];
}


void MemoryPool::SetNext(unsigned char* p_pBlock, unsigned char* p_pNext)
{
    unsigned char** pChunkHeader = (unsigned char**)p_pBlock;
    pChunkHeader[0] = p_pNext;
}


void* MemoryPool::Alloc(void)
{
    if(m_pHead == NULL) 
    {
        if(m_resizable)
        {
            this->GrowMemory();
        }
        else
        {
            return NULL;
        }
    }
    ++m_allocated;
    unsigned char* pMem = m_pHead;
    m_pHead = this->GetNext(pMem);
    return pMem + CHUNK_HEADER_SIZE;
}


void MemoryPool::Free(void* p_pMem)
{
    if(p_pMem == NULL)
    {
        return;
    }
    unsigned char* pOldHead = m_pHead;
    m_pHead = (unsigned char*)p_pMem - CHUNK_HEADER_SIZE;
    this->SetNext(m_pHead, pOldHead);
    --m_allocated;
}


void MemoryPool::PrintInfo(void) const
{
    std::ostringstream str;
    str << "chunk size: " << FormatBytes(this->GetChunkSize()) << std::endl
        << "system memory used: " << FormatBytes(this->GetSystemAllocatedBytes()) << std::endl
        << "pool memory used: " << FormatBytes(this->GetPoolAllocatedBytes()) << std::endl
        << "pool memory free: " << FormatBytes(this->GetPoolFreeBytes()) << std::endl
        << "usage of allocated system memory: " << (UINT)(100.0 * this->GetPoolUsage()) << "%" << std::endl << std::endl;
    LI_LOG_WITH_TAG(str.str().c_str());
}


std::string MemoryPool::FormatBytes(SIZE_T p_bytes)
{
    INT unit = 0;
    while(p_bytes > 4096 && unit < 3)
    {
        p_bytes /= 1024;
        ++unit;
    }
    std::ostringstream str;
    str << p_bytes;
    switch(unit)
    {
    case 0: str << " Bytes"; break;
    case 1: str << " KB"; break;
    case 2: str << " MB"; break;
    case 3: str << " GB"; break;
    default: return "weird"; break;
    }
    return str.str();
}