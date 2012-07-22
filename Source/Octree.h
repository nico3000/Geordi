#pragma once
#include "MemoryPool.h"

#define DEFAULT_VALUE 0

class Octree
{
private:
    static MemoryPool sm_pool;

    Octree* m_pSons;
    Octree* m_pFather;
    unsigned short m_minX;
    unsigned short m_minY;
    unsigned short m_minZ;
    unsigned short m_size;
    short m_value;
    short m_flags;

    Octree(Octree const& toCopy) { /* no copy constructor */ }

    INT GetSonIndex(unsigned short p_x, unsigned short p_y, unsigned short p_z) const;
    bool IsIn(unsigned short p_x, unsigned short p_y, unsigned short p_z) const;
    void CheckSons(void);
    void InitIntern(Octree* p_pFather, char p_sonIndex);
    void InitIntern(char* p_pData, Octree* p_pFather, char p_sonIndex);
    char* SaveIntern(char* p_pData) const;
    void ClearSons(void);

public:
    Octree(void);
    ~Octree(void);

    void Init(unsigned short p_size);
    bool Init(std::fstream& p_stream);
    void Save(std::fstream& p_stream) const;
    void Clear(void);
    bool SetValue(unsigned short p_x, unsigned short p_y, unsigned short p_z, short p_value, bool p_autoOptimizeStructure = true);
    void OptimizeStructure(void);
    short GetValue(unsigned short p_x, unsigned short p_y, unsigned short p_z) const;
    unsigned long GetNumNodes(void) const;
    unsigned long GetMaxNumNodes(void) const;
    unsigned long GetNumLeafs(void) const;
    void PrintTree(void) const;
    void PrintStructure(void) const;
    bool operator==(Octree const& second) const;
    short& GetFlags(void) { return m_flags; }
    short const& ReadFlag(void) const { return m_flags; }

    unsigned short GetMinX(void) const { return m_minX; }
    unsigned short GetMinY(void) const { return m_minY; }
    unsigned short GetMinZ(void) const { return m_minZ; }
    unsigned short GetSize(void) const { return m_size; }
    unsigned short GetMaxX(void) const { return this->GetMinX() + this->GetSize(); }
    unsigned short GetMaxY(void) const { return this->GetMinY() + this->GetSize(); }
    unsigned short GetMaxZ(void) const { return this->GetMinZ() + this->GetSize(); }
    bool IsLeaf(void) const { return m_pSons == NULL; }
    void PrintUsage(void) const { std::cout << "tree usage: " << this->GetNumNodes() << " of " << this->GetMaxNumNodes() << " nodes (" << (unsigned int)(100.0 * (double)this->GetNumNodes() / (double)this->GetMaxNumNodes()) << "%)" << std::endl << std::endl; }

    static bool InitMemoryPool(INT p_numChunks) { return sm_pool.Init(8*sizeof(Octree), p_numChunks, true); }
    static MemoryPool& GetMemoryPool(void) { return sm_pool; }
};

