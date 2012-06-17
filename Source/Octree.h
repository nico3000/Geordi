#pragma once
#include "MemoryPool.h"
#include <iostream>
#include <fstream>
#define DEFAULT_VALUE 0

class Octree
{
private:
    static MemoryPool sm_pool;

    Octree* m_pSons;
    Octree* m_pFather;
    USHORT m_minX;
    USHORT m_minY;
    USHORT m_minZ;
    USHORT m_size;
    char m_value;
    char m_flags;

    Octree(Octree const& toCopy) { /* no copy constructor */ }

    INT GetSonIndex(USHORT p_x, USHORT p_y, USHORT p_z) const;
    bool IsIn(USHORT p_x, USHORT p_y, USHORT p_z) const;
    void CheckSons(void);
    void InitIntern(Octree* p_pFather, char p_sonIndex);
    void InitIntern(char* p_pData, Octree* p_pFather, char p_sonIndex);
    char* SaveIntern(char* p_pData) const;
    void ClearSons(void);

public:
    Octree(void);
    ~Octree(void);

    void Init(USHORT p_size);
    bool Init(std::fstream& p_stream);
    void Save(std::fstream& p_stream) const;
    void Clear(void);
    bool SetValue(USHORT p_x, USHORT p_y, USHORT p_z, char p_value);
    char GetValue(USHORT p_x, USHORT p_y, USHORT p_z) const;
    ULONG GetNumNodes(void) const;
    ULONG GetMaxNumNodes(void) const;
    ULONG GetNumLeafs(void) const;
    void PrintTree(void) const;
    void PrintStructure(void) const;
    bool operator==(Octree const& second) const;
    char& GetFlags(void) { return m_flags; }
    char const& ReadFlag(void) const { return m_flags; }

    USHORT GetMinX(void) const { return m_minX; }
    USHORT GetMinY(void) const { return m_minY; }
    USHORT GetMinZ(void) const { return m_minZ; }
    USHORT GetSize(void) const { return m_size; }
    USHORT GetMaxX(void) const { return this->GetMinX() + this->GetSize(); }
    USHORT GetMaxY(void) const { return this->GetMinY() + this->GetSize(); }
    USHORT GetMaxZ(void) const { return this->GetMinZ() + this->GetSize(); }
    bool IsLeaf(void) const { return m_pSons == NULL; }
    void PrintUsage(void) const { std::cout << "tree usage: " << this->GetNumNodes() << " of " << this->GetMaxNumNodes() << " nodes (" << (UINT)(100.0 * (double)this->GetNumNodes() / (double)this->GetMaxNumNodes()) << "%)" << std::endl << std::endl; }

    static bool InitMemoryPool(INT p_numChunks) { return sm_pool.Init(8*sizeof(Octree), p_numChunks, true); }
    static MemoryPool& GetMemoryPool(void) { return sm_pool; }
};

