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
    CHAR m_value;
    CHAR m_flags;
    CHAR m_sonIndex;
    CHAR m_level;

    INT GetSonIndex(USHORT p_x, USHORT p_y, USHORT p_z) CONST;
    BOOL IsIn(USHORT p_x, USHORT p_y, USHORT p_z) CONST;
    VOID CheckSons(VOID);
    VOID Init(Octree *p_pFather, CHAR p_sonIndex);
    BOOL InitIntern(std::fstream& p_stream, BOOL p_isLeaf, INT p_firstLeafIndex);
    VOID SaveIntern(std::fstream& p_stream) CONST;

public:
    Octree(VOID);
    ~Octree(VOID);

    VOID Init(USHORT p_size);
    BOOL Init(std::fstream& p_stream);
    VOID Save(std::fstream& p_stream) CONST;
    VOID Clear(VOID);
    VOID SetValue(USHORT p_x, USHORT p_y, USHORT p_z, CHAR p_value);
    CHAR GetValue(USHORT p_x, USHORT p_y, USHORT p_z) CONST;
    ULONG GetNumNodes(VOID) CONST;
    ULONG GetMaxNumNodes(VOID) CONST;
    ULONG GetNumLeafs(VOID) CONST;
    VOID PrintTree(VOID) CONST;
    VOID PrintStructure(VOID) CONST;
    BOOL operator==(Octree CONST& second) CONST;
    CHAR& GetFlags(VOID) { return m_flags; }

    USHORT GetMinX(VOID) CONST { return m_minX; }
    USHORT GetMinY(VOID) CONST { return m_minY; }
    USHORT GetMinZ(VOID) CONST { return m_minZ; }
    USHORT GetSize(VOID) CONST { return m_size; }
    USHORT GetMaxX(VOID) CONST { return this->GetMinX() + this->GetSize(); }
    USHORT GetMaxY(VOID) CONST { return this->GetMinY() + this->GetSize(); }
    USHORT GetMaxZ(VOID) CONST { return this->GetMinZ() + this->GetSize(); }
    BOOL IsLeaf(VOID) CONST { return m_pSons == NULL; }
    VOID PrintUsage(VOID) CONST { std::cout << "tree usage: " << this->GetNumNodes() << " of " << this->GetMaxNumNodes() << " nodes (" << (UINT)(100.0 * (DOUBLE)this->GetNumNodes() / (DOUBLE)this->GetMaxNumNodes()) << "%)" << std::endl << std::endl; }

    static BOOL InitMemoryPool(INT p_numChunks) { return sm_pool.Init(8*sizeof(Octree), p_numChunks, TRUE); }
    static MemoryPool& GetMemoryPool(VOID) { return sm_pool; }
};

