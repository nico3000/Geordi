#pragma once

class MemoryPool
{
private:
    const static SIZE_T CHUNK_HEADER_SIZE;

    unsigned char** m_ppRawMemArray;
    UINT    m_memArraySize;
    UINT    m_chunkSize;
    UINT    m_numChunks;
    bool    m_resizable;
    unsigned char*  m_pHead;
    UINT    m_allocated;

    bool GrowMemory(void);
    unsigned char* CreateNewMemoryBlock(void);
    unsigned char* GetNext(unsigned char* p_pBlock) const;
    void SetNext(unsigned char* p_pBlock, unsigned char* p_pNext);

    static std::string FormatBytes(SIZE_T p_bytes);

public:
    MemoryPool(void);
    ~MemoryPool(void);

    bool Init(INT p_chunkSize, INT p_numChunks, bool p_resizeable);
    void* Alloc(void);
    void Free(void* p_pMem);
    void PrintInfo(void) const;

    SIZE_T GetSystemAllocatedBytes(void) const { return m_memArraySize * m_numChunks * (m_chunkSize + CHUNK_HEADER_SIZE); }
    SIZE_T GetPoolAllocatedChunks(void) const { return m_allocated; }
    SIZE_T GetPoolAllocatedBytes(void) const { return m_allocated * (m_chunkSize + CHUNK_HEADER_SIZE); }
    SIZE_T GetPoolFreeBytes(void) const { return (m_memArraySize * m_numChunks - m_allocated) * (m_chunkSize + CHUNK_HEADER_SIZE); }
    double GetPoolUsage(void) const { return (double)this->GetPoolAllocatedBytes() / (double)(this->GetPoolAllocatedBytes() + this->GetPoolFreeBytes()); }
    UINT GetChunkSize(void) const { return m_chunkSize; }
};
