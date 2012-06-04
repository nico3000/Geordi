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


VOID Octree::SetValue(USHORT p_x, USHORT p_y, USHORT p_z, CHAR p_value)
{
    if(this->GetSize() == 1)
    {
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
                    m_pSons[i].Init(this, i);
                }
            }
            else 
            {
                return;
            }
        }
        INT index = this->GetSonIndex(p_x, p_y, p_z);
        m_pSons[index].SetValue(p_x, p_y, p_z, p_value);
    }
    if(m_pSons != NULL)
    {
        this->CheckSons();
    }
    if(m_pSons == NULL && m_pFather != NULL)
    {
        m_pFather->CheckSons();
    }
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
            this->Clear();
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
    m_level = 0;
    while(p_size > 1)
    {
        ++m_level;
        p_size >>= 1;
    }
    m_value = DEFAULT_VALUE;
    m_flags = 0;
    m_pFather = NULL;
    m_pSons = NULL;
}


VOID Octree::Init(Octree *p_pFather, CHAR p_sonIndex)
{
    //std::cout << p_pFather->m_minX << " " << p_pFather->m_minY << " " << p_pFather->m_minZ << " " << p_pFather->m_size << std::endl;
    m_size = p_pFather->m_size/2;
    m_minX = p_pFather->m_minX + (p_sonIndex % 2) * m_size;
    m_minY = p_pFather->m_minY + ((p_sonIndex >> 1) % 2) * m_size;
    m_minZ = p_pFather->m_minZ + ((p_sonIndex >> 2) % 2) * m_size;
    m_level = p_pFather->m_level - 1;
    m_sonIndex = p_sonIndex;
    m_value = p_pFather->m_value;
    m_flags = 0;
    m_pFather = p_pFather;
    m_pSons = NULL;
    
}


BOOL Octree::Init(std::fstream& p_stream)
{
    this->Clear();

    USHORT size;
    INT numInnerNodes;
    INT numLeafNodes;
    p_stream.read((CHAR*)&size, sizeof(USHORT));
    p_stream.read((CHAR*)&numInnerNodes, sizeof(INT));
    p_stream.read((CHAR*)&numLeafNodes, sizeof(INT));
    
    this->Init(size);
    BOOL result;
#ifdef _DEBUG_OR_PROFILE
    //std::cout << "loading octree..." << std::endl;
    //INT id = g_timer.Tick(IMMEDIATE);
    result = this->InitIntern(p_stream, numInnerNodes == 0, numInnerNodes);
    //std::cout << "loading took " << (1e-3 * (DOUBLE)g_timer.Tock(id, ERASE)) << " secs" << std::endl;
#else
    result = this->InitIntern(p_stream, numInnerNodes == 0, numInnerNodes);
#endif
    return result;
}


#define LEAF_SIZE (2 * sizeof(CHAR))
#define INNER_SIZE (2 * sizeof(CHAR) + 8 * sizeof(INT))
#define OFFSET (sizeof(USHORT) + 2 * sizeof(INT))


BOOL Octree::InitIntern(std::fstream& p_stream, BOOL p_isLeaf, INT p_firstLeafIndex)
{
    p_stream.read(&m_value, sizeof(CHAR));
    p_stream.read(&m_flags, sizeof(CHAR));

    if(!p_isLeaf)
    {
        INT pSons[8];
        p_stream.read((CHAR*)pSons, 8 * sizeof(INT));
        m_pSons = (Octree*)sm_pool.Alloc();
        for(INT i=0; i < 8; ++i)
        {
            m_pSons[i].Init(this, i);
            INT innerCount = min(pSons[i], p_firstLeafIndex);
            INT leafCount = max(0, pSons[i] - innerCount);
            p_stream.seekg(OFFSET + innerCount * INNER_SIZE + leafCount * LEAF_SIZE, std::ios::beg);
            m_pSons[i].InitIntern(p_stream, innerCount == p_firstLeafIndex, p_firstLeafIndex);
        }
    }

    return TRUE;
}


static INT g_currentNode = 0;
static INT g_currentLeaf = 0;


VOID Octree::Save(std::fstream& p_stream) CONST
{
#ifdef _DEBUG_OR_PROFILE
    //std::cout << "saving octree..." << std::endl;
    //INT id = g_timer.Tick(IMMEDIATE);
    this->SaveIntern(p_stream);
    //std::cout << "saving took " << (1e-3 * (DOUBLE)g_timer.Tock(id, ERASE)) << " secs" << std::endl;
#else
    this->SaveIntern(p_stream);
#endif
    
}


VOID Octree::SaveIntern(std::fstream& p_stream) CONST
{
    ULONG numNodes = this->GetNumNodes();
    ULONG numLeafs = this->GetNumLeafs();
    ULONG numInner = numNodes - numLeafs;
    USHORT size = this->GetSize();

    p_stream.write((CHAR*)&size, sizeof(USHORT));
    p_stream.write((CHAR*)&numInner, sizeof(INT));
    p_stream.write((CHAR*)&numLeafs, sizeof(INT));

    std::queue<Octree CONST*> innerQueue, leafQueue;
    (this->IsLeaf() ? leafQueue : innerQueue).push(this);
    
    INT qInner = (INT)innerQueue.size();
    INT qLeaf = (INT)leafQueue.size();

    while(!innerQueue.empty())
    {
        Octree CONST* pToSave = innerQueue.front();
        innerQueue.pop();

        p_stream.write(&pToSave->m_value, sizeof(CHAR));
        p_stream.write(&pToSave->m_flags, sizeof(CHAR));

        INT pSons[8];
        for(INT i=0; i < 8; ++i) 
        {
            Octree CONST* pSon = pToSave->m_pSons + i;
            (pSon->IsLeaf() ? leafQueue : innerQueue).push(pSon);
            pSons[i] = pSon->IsLeaf() ? (numInner + qLeaf++) : qInner++;
        }

        p_stream.write((CHAR*)pSons, 8 * sizeof(INT));
    }
    while(!leafQueue.empty())
    {
        Octree CONST* pToSave = leafQueue.front();
        leafQueue.pop();

        p_stream.write(&pToSave->m_value, sizeof(CHAR));
        p_stream.write(&pToSave->m_flags, sizeof(CHAR));
    }
}


BOOL Octree::operator==(Octree CONST& second) CONST
{
    if(this->GetSize() != second.GetSize())
    {
#ifdef _DEBUG_OR_PROFILE
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
#ifdef _DEBUG_OR_PROFILE
                    std::cout << "val mismatch: " << x << " " << y << " " << z << std::endl;
#endif
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}


VOID Octree::Clear(VOID)
{
    if(!this->IsLeaf())
    {
        for(INT i=0; i < 8; ++i)
        {
            m_pSons[i].Clear();
        }
        sm_pool.Free(m_pSons);
        m_pSons = NULL;
    }
    
}