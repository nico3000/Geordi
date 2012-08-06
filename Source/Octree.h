#pragma once
#include "MemoryPool.h"

#define OCTREE_DEFAULT_VALUE -SHORT_MAX

class Octree
{
private:
    static MemoryPool sm_pool;

    Octree* m_pSons;
    Octree* m_pFather;
    short m_minX;
    short m_minY;
    short m_minZ;
    short m_size;
    int m_value;

    Octree(Octree const& toCopy) { /* no copy constructor */ }

    INT GetSonIndex(short p_x, short p_y, short p_z) const;
    
    void CheckSons(void);
    void InitIntern(Octree* p_pFather, char p_sonIndex);
    void InitIntern(char* p_pData, Octree* p_pFather, char p_sonIndex);
    char* SaveIntern(char* p_pData) const;
    void ClearSons(void);
    bool IsLeaf(void) const { return !m_pSons; }

public:
    Octree(void);
    ~Octree(void);

    void Init(short p_minX, short p_minY, short p_minZ, short p_size);
    bool Init(std::fstream& p_stream);
    bool Save(std::fstream& p_stream) const;
    void Clear(void);
    bool SetValue(short p_x, short p_y, short p_z, int p_value, bool p_autoOptimizeStructure = true);
    void OptimizeStructure(void);
    int GetValue(short p_x, short p_y, short p_z) const;
    unsigned long GetNumNodes(void) const;
    unsigned long GetMaxNumNodes(void) const;
    unsigned long GetNumLeafs(void) const;
    void PrintTree(void) const;
    void PrintStructure(void) const;
    bool operator==(Octree const& second) const;
    bool IsIn(short p_x, short p_y, short p_z) const;
    bool IsEmpty(void) const { return this->IsLeaf() && m_value == OCTREE_DEFAULT_VALUE; }

    short GetMinX(void) const { return m_minX; }
    short GetMinY(void) const { return m_minY; }
    short GetMinZ(void) const { return m_minZ; }
    short GetSize(void) const { return m_size; }
    short GetMaxX(void) const { return m_minX + m_size; }
    short GetMaxY(void) const { return m_minY + m_size; }
    short GetMaxZ(void) const { return m_minZ + m_size; }
    void PrintUsage(void) const { std::cout << "tree usage: " << this->GetNumNodes() << " of " << this->GetMaxNumNodes() << " nodes (" << (unsigned int)(100.0 * (double)this->GetNumNodes() / (double)this->GetMaxNumNodes()) << "%)" << std::endl << std::endl; }

    static bool InitMemoryPool(INT p_numChunks) { return sm_pool.Init(8*sizeof(Octree), p_numChunks, true); }
    static MemoryPool& GetMemoryPool(void) { return sm_pool; }
};

