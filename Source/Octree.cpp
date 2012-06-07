#include "StdAfx.h"
#include "Octree.h"

using namespace LostIsland;

MemoryPool Octree::sm_pool;


Octree::Octree(VOID):
m_pFather(NULL), m_pSons(NULL), m_value(DEFAULT_VALUE)
{
}


Octree::~Octree(VOID)
{
    this->Clear();
}


BOOL Octree::SetValue(USHORT p_x, USHORT p_y, USHORT p_z, CHAR p_value)
{
    BOOL changed = FALSE;
    if(this->GetSize() == 1)
    {
        changed = m_value != p_value;
        m_value = p_value;
    }
    else
    {
        if(m_pSons == NULL)
        {
            if(p_value != m_value)
            {
                m_pSons = (Octree*)sm_pool.Alloc();
                for(INT i=0; i < 8; ++i)
                {
                    m_pSons[i].InitIntern(this, i);
                }
            }
            else 
            {
                return FALSE;
            }
        }
        INT index = this->GetSonIndex(p_x, p_y, p_z);
        if(index == -1)
        {
            return FALSE;
        }
        changed = m_pSons[index].SetValue(p_x, p_y, p_z, p_value);
    }
    if(changed)
    {
        if(!this->IsLeaf())
        {
            this->CheckSons();
        }
        if(this->IsLeaf() && m_pFather != NULL)
        {
            m_pFather->CheckSons();
        }
    }
    return changed;
}


CHAR Octree::GetValue(USHORT p_x, USHORT p_y, USHORT p_z) CONST
{
    if(m_pSons == NULL)
    {
        return m_value;
    }
    else
    {
        INT index = this->GetSonIndex(p_x, p_y, p_z);
        return m_pSons[index].GetValue(p_x, p_y, p_z);
    }
}


VOID Octree::CheckSons(VOID)
{
    if(m_pSons != NULL)
    {
        m_value = m_pSons[0].m_value;
        BOOL collapsable = TRUE;
        for(INT i=0; collapsable && i < 8; ++i) 
        {
            if(!m_pSons[i].IsLeaf() || m_pSons[i].m_value != m_value)
            {
                collapsable = FALSE;
            }
        }
        if(collapsable)
        {
            this->ClearSons();
        }
    }
}


INT Octree::GetSonIndex(USHORT p_x, USHORT p_y, USHORT p_z) CONST
{
    INT index = 0;
#ifdef _DEBUG
    if(!this->IsIn(p_x, p_y, p_z))
    {
        ERROR("called GetSonIndex() but position is not in tree");
        std::cerr << "error1: " << m_minX << " " << m_minY << " " << m_minZ << " " << m_size << std::endl;
        std::cerr << "error2: " << p_x << " " << p_y << " " << p_z << " " << std::endl;
        return -1;
    }
#endif
    index |= p_x >= (this->GetMinX() + this->GetSize()/2) ? 1 : 0;
    index |= p_y >= (this->GetMinY() + this->GetSize()/2) ? 2 : 0;
    index |= p_z >= (this->GetMinZ() + this->GetSize()/2) ? 4 : 0;
    return index;
}


BOOL Octree::IsIn(USHORT p_x, USHORT p_y, USHORT p_z) CONST
{
    return this->GetMinX() <= p_x && p_x < this->GetMaxX() &&
           this->GetMinY() <= p_y && p_y < this->GetMaxY() &&
           this->GetMinZ() <= p_z && p_z < this->GetMaxZ();
}


VOID Octree::PrintTree(VOID) CONST
{
    std::cout << "Octree size: " << this->GetSize() << std::endl;
    for(USHORT y=this->GetMinY(); y < this->GetMaxY(); ++y) 
    {
        std::cout << "y=" << y << std::endl;
        for(USHORT z=this->GetMinZ(); z < this->GetMaxZ(); ++z) 
        {
            for(USHORT x=this->GetMinX(); x < this->GetMaxX(); ++x) 
            {
                std::cout << (INT)this->GetValue(x, y, z) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}


VOID Octree::PrintStructure(VOID) CONST
{
    std::cout << "octree size: " << this->GetSize() << ", x dimension: " << this->GetMinX() << " - " << this->GetMaxX() << ", y dimension: " << this->GetMinY() << " - " << this->GetMaxY() << ", z dimension: " << this->GetMinZ() << " - " << this->GetMaxZ() << std::endl;
    if(!this->IsLeaf())
    {
        for(INT i=0; i < 8; ++i)
        {
            m_pSons[i].PrintStructure();
        }
    }
}


ULONG Octree::GetNumNodes(VOID) CONST
{
    if(m_pSons == NULL)
    {
        return 1;
    }
    else 
    {
        return 1 + m_pSons[0].GetNumNodes()
                 + m_pSons[1].GetNumNodes()
                 + m_pSons[2].GetNumNodes()
                 + m_pSons[3].GetNumNodes()
                 + m_pSons[4].GetNumNodes()
                 + m_pSons[5].GetNumNodes()
                 + m_pSons[6].GetNumNodes()
                 + m_pSons[7].GetNumNodes();
    }
}


ULONG Octree::GetNumLeafs(VOID) CONST
{
    if(m_pSons == NULL)
    {
        return 1;
    }
    else
    {
        return m_pSons[0].GetNumLeafs() + 
               m_pSons[1].GetNumLeafs() + 
               m_pSons[2].GetNumLeafs() + 
               m_pSons[3].GetNumLeafs() + 
               m_pSons[4].GetNumLeafs() + 
               m_pSons[5].GetNumLeafs() + 
               m_pSons[6].GetNumLeafs() + 
               m_pSons[7].GetNumLeafs();
    }
}


ULONG Octree::GetMaxNumNodes(VOID) CONST
{
    ULONG sum = 0;
    ULONG start = 1;
    USHORT size = this->GetSize();
    while(size > 0)
    {
        sum += start;
        start *= 8;
        size /= 2;
    }
    return sum;
}


VOID Octree::Init(USHORT p_size)
{
    this->Clear();

    m_size = p_size;
    m_minX = m_minY = m_minZ = 0;
    m_value = DEFAULT_VALUE;
    m_flags = 0;
    m_pFather = NULL;
    m_pSons = NULL;
}


VOID Octree::InitIntern(Octree *p_pFather, CHAR p_sonIndex)
{
    //std::cout << p_pFather->m_minX << " " << p_pFather->m_minY << " " << p_pFather->m_minZ << " " << p_pFather->m_size << std::endl;
    m_size = p_pFather->m_size / 2;
    m_minX = p_pFather->m_minX + (p_sonIndex % 2) * m_size;
    m_minY = p_pFather->m_minY + ((p_sonIndex >> 1) % 2) * m_size;
    m_minZ = p_pFather->m_minZ + ((p_sonIndex >> 2) % 2) * m_size;
    m_value = p_pFather->m_value;
    m_flags = 0;
    m_pFather = p_pFather;
    m_pSons = NULL;
    
}


BOOL Octree::Init(std::fstream& p_stream)
{
    this->Clear();

    if(!p_stream.good())
    {
#if defined(_DEBUG) || defined(PROFILE)
        ERROR("bad stream");
#endif
        return FALSE;
    }
    p_stream.seekg(0, std::ios::end);
    std::streamoff length = p_stream.tellg();
    p_stream.seekg(0, std::ios::beg);
    CHAR* pData = new CHAR[length];
    p_stream.read(pData, length);

    this->Init(((USHORT*)pData)[0]);
#if defined(_DEBUG) || defined(PROFILE)
    //std::cout << "loading octree..." << std::endl;
    //INT id = g_pTimer->Tick(IMMEDIATE);
    this->InitIntern(pData + sizeof(USHORT), NULL, 0);
    //std::cout << "loading took " << (1e-3 * (DOUBLE)g_pTimer->Tock(id, ERASE)) << " secs" << std::endl;
#else
    this->InitIntern(pData + sizeof(USHORT), NULL, 0);
#endif
    SAFE_DELETE_ARRAY(pData);
    return TRUE;
}


VOID Octree::InitIntern(CHAR* p_pData, Octree* p_pFather, CHAR p_sonIndex)
{
    if(p_pFather != NULL)
    {
        this->InitIntern(p_pFather, p_sonIndex);
    }

    m_value = p_pData[0];
    m_flags = p_pData[1];
    if(((UINT*)(p_pData + 2))[0] != 0)
    {
        m_pSons = (Octree*)sm_pool.Alloc();
        for(INT i=0; i < 8; ++i)
        {
            UINT offset = ((UINT*)(p_pData + 2))[i];
            m_pSons[i].InitIntern(p_pData + offset, this, i);
        }
    }
}


#define NODE_SIZE_INNER (2 * sizeof(CHAR) + 8 * sizeof(UINT))
#define NODE_SIZE_LEAF (2 * sizeof(CHAR) + 1 * sizeof(UINT))
#define NODE_SIZE_HEAD (sizeof(USHORT))


VOID Octree::Save(std::fstream& p_stream) CONST
{
    ULONG numNodes = this->GetNumNodes();
    ULONG numLeafs = this->GetNumLeafs();
    UINT dataSize = (UINT)(NODE_SIZE_HEAD + (numNodes - numLeafs) * NODE_SIZE_INNER + numLeafs * NODE_SIZE_LEAF);
    CHAR* pData = new CHAR[dataSize];
    ((USHORT*)pData)[0] = m_size;
    
#if defined(_DEBUG) || defined(PROFILE)
    //std::cout << "saving octree..." << std::endl;
    //INT id = g_pTimer->Tick(IMMEDIATE);
    LONG usedSpace = (UINT)(this->SaveIntern(pData + sizeof(USHORT)) - pData);
    if(usedSpace != dataSize)
    {
        std::cout << "miscalculated space: " << usedSpace << " / " << dataSize << std::endl;
    }
    //std::cout << "saving took " << (1e-3 * (DOUBLE)g_pTimer->Tock(id, ERASE)) << " secs" << std::endl;
#else
    this->SaveIntern(pData + sizeof(USHORT));
#endif
    p_stream.write(pData, dataSize);
    delete pData;
}


CHAR* Octree::SaveIntern(CHAR* p_pData) CONST
{
    p_pData[0] = m_value;
    p_pData[1] = m_flags;
    if(this->IsLeaf())
    {
        ((UINT*)(p_pData + 2))[0] = 0;
        return p_pData + NODE_SIZE_LEAF;
    }
    else
    {        
        CHAR* pNode = p_pData + NODE_SIZE_INNER;
        for(INT i=0; i < 8; ++i) 
        {
            ((UINT*)(p_pData + 2))[i] = (UINT)(pNode - p_pData);
            pNode = m_pSons[i].SaveIntern(pNode);
        }
        return pNode;
    }
}


BOOL Octree::operator==(Octree CONST& second) CONST
{
    if(this->GetSize() != second.GetSize())
    {
#if defined(_DEBUG) || defined(PROFILE)
        std::cout << this->GetSize() << " != " << second.GetSize() << std::endl;
#endif
        return FALSE;
    }
    USHORT minX = this->GetMinX();
    USHORT minY = this->GetMinY();
    USHORT minZ = this->GetMinZ();
    USHORT secondMinX = second.GetMinX();
    USHORT secondMinY = second.GetMinY();
    USHORT secondMinZ = second.GetMinZ();
    for(USHORT x=0; x < this->GetSize(); ++x)
    {
        for(USHORT y=0; y < this->GetSize(); ++y)
        {
            for(USHORT z=0; z < this->GetSize(); ++z)
            {
                CHAR val1 = this->GetValue(minX + x, minY + y, minZ + z);
                CHAR val2 = second.GetValue(secondMinX + x, secondMinY + y, secondMinZ + z);
                if(val1 != val2)
                {
#if defined(_DEBUG) || defined(PROFILE)
                    std::cout << "val mismatch: " << x << " " << y << " " << z << std::endl;
#endif
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}


VOID Octree::ClearSons(VOID)
{
    if(!this->IsLeaf())
    {
        for(INT i=0; i < 8; ++i)
        {
            m_pSons[i].ClearSons();
        }
        sm_pool.Free(m_pSons);
        m_pSons = NULL;
    }
}


VOID Octree::Clear(VOID)
{
    this->ClearSons();
    m_value = DEFAULT_VALUE;
    m_flags = 0;
}