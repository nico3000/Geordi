#include "StdAfx.h"
#include "Octree.h"

#define LI_LOGGER_TAG "Octree"

MemoryPool Octree::sm_pool;

Octree::Octree(void):
m_pFather(NULL), m_pSons(NULL), m_value(DEFAULT_VALUE)
{
}


Octree::~Octree(void)
{
    this->Clear();
}


bool Octree::SetValue(USHORT p_x, USHORT p_y, USHORT p_z, char p_value)
{
    bool changed = false;
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
                return false;
            }
        }
        INT index = this->GetSonIndex(p_x, p_y, p_z);
        if(index == -1)
        {
            return false;
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


char Octree::GetValue(USHORT p_x, USHORT p_y, USHORT p_z) const
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


void Octree::CheckSons(void)
{
    if(m_pSons != NULL)
    {
        m_value = m_pSons[0].m_value;
        bool collapsable = true;
        for(INT i=0; collapsable && i < 8; ++i) 
        {
            if(!m_pSons[i].IsLeaf() || m_pSons[i].m_value != m_value)
            {
                collapsable = false;
            }
        }
        if(collapsable)
        {
            this->ClearSons();
        }
    }
}


INT Octree::GetSonIndex(USHORT p_x, USHORT p_y, USHORT p_z) const
{
    INT index = 0;
#ifdef _DEBUG
    if(!this->IsIn(p_x, p_y, p_z))
    {
        LI_ERROR("called GetSonIndex() but position is not in tree");
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


bool Octree::IsIn(USHORT p_x, USHORT p_y, USHORT p_z) const
{
    return this->GetMinX() <= p_x && p_x < this->GetMaxX() &&
           this->GetMinY() <= p_y && p_y < this->GetMaxY() &&
           this->GetMinZ() <= p_z && p_z < this->GetMaxZ();
}


void Octree::PrintTree(void) const
{
    std::ostringstream str;
    str << "Octree size: " << this->GetSize() << std::endl;
    for(USHORT y=this->GetMinY(); y < this->GetMaxY(); ++y) 
    {
        str << "y=" << y << std::endl;
        for(USHORT z=this->GetMinZ(); z < this->GetMaxZ(); ++z) 
        {
            for(USHORT x=this->GetMinX(); x < this->GetMaxX(); ++x) 
            {
                str << (INT)this->GetValue(x, y, z) << " ";
            }
            str << std::endl;
        }
        str << std::endl;
    }
    LI_LOG_WITH_TAG(str.str());
}


void Octree::PrintStructure(void) const
{
    std::ostringstream str;
    str << "octree size: " << this->GetSize() << ", x dimension: " << this->GetMinX() << " - " << this->GetMaxX() << ", y dimension: " << this->GetMinY() << " - " << this->GetMaxY() << ", z dimension: " << this->GetMinZ() << " - " << this->GetMaxZ() << std::endl;
    LI_LOG_WITH_TAG(str.str());
    if(!this->IsLeaf())
    {
        for(INT i=0; i < 8; ++i)
        {
            m_pSons[i].PrintStructure();
        }
    }
}


ULONG Octree::GetNumNodes(void) const
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


ULONG Octree::GetNumLeafs(void) const
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


ULONG Octree::GetMaxNumNodes(void) const
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


void Octree::Init(USHORT p_size)
{
    this->Clear();

    m_size = p_size;
    m_minX = m_minY = m_minZ = 0;
    m_value = DEFAULT_VALUE;
    m_flags = 0;
    m_pFather = NULL;
    m_pSons = NULL;
}


void Octree::InitIntern(Octree *p_pFather, char p_sonIndex)
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


bool Octree::Init(std::fstream& p_stream)
{
    this->Clear();

    if(!p_stream.good())
    {
#if defined(_DEBUG) || defined(PROFILE)
        LI_ERROR("bad stream");
#endif
        return false;
    }
    p_stream.seekg(0, std::ios::end);
    UINT length = (UINT)p_stream.tellg();
    p_stream.seekg(0, std::ios::beg);
    char* pData = new char[length];
    p_stream.read(pData, length);

    this->Init(((USHORT*)pData)[0]);
#if defined(_DEBUG) || defined(PROFILE)
    //std::cout << "loading octree..." << std::endl;
    //INT id = g_pTimer->Tick(IMMEDIATE);
    this->InitIntern(pData + sizeof(USHORT), NULL, 0);
    //std::cout << "loading took " << (1e-3 * (double)g_pTimer->Tock(id, ERASE)) << " secs" << std::endl;
#else
    this->InitIntern(pData + sizeof(USHORT), NULL, 0);
#endif
    SAFE_DELETE_ARRAY(pData);
    return true;
}


void Octree::InitIntern(char* p_pData, Octree* p_pFather, char p_sonIndex)
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


#define NODE_SIZE_INNER (2 * sizeof(char) + 8 * sizeof(UINT))
#define NODE_SIZE_LEAF (2 * sizeof(char) + 1 * sizeof(UINT))
#define NODE_SIZE_HEAD (sizeof(USHORT))


void Octree::Save(std::fstream& p_stream) const
{
    ULONG numNodes = this->GetNumNodes();
    ULONG numLeafs = this->GetNumLeafs();
    UINT dataSize = (UINT)(NODE_SIZE_HEAD + (numNodes - numLeafs) * NODE_SIZE_INNER + numLeafs * NODE_SIZE_LEAF);
    char* pData = new char[dataSize];
    ((USHORT*)pData)[0] = m_size;
    
#if defined(_DEBUG) || defined(PROFILE)
    //std::cout << "saving octree..." << std::endl;
    //INT id = g_pTimer->Tick(IMMEDIATE);
    LONG usedSpace = (UINT)(this->SaveIntern(pData + sizeof(USHORT)) - pData);
    if(usedSpace != dataSize)
    {
        std::ostringstream str;
        str << "miscalculated space: " << usedSpace << " / " << dataSize << std::endl;
        LI_LOG_WITH_TAG(str.str());
    }
    //std::cout << "saving took " << (1e-3 * (double)g_pTimer->Tock(id, ERASE)) << " secs" << std::endl;
#else
    this->SaveIntern(pData + sizeof(USHORT));
#endif
    p_stream.write(pData, dataSize);
    delete pData;
}


char* Octree::SaveIntern(char* p_pData) const
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
        char* pNode = p_pData + NODE_SIZE_INNER;
        for(INT i=0; i < 8; ++i) 
        {
            ((UINT*)(p_pData + 2))[i] = (UINT)(pNode - p_pData);
            pNode = m_pSons[i].SaveIntern(pNode);
        }
        return pNode;
    }
}


bool Octree::operator==(Octree const& second) const
{
    if(this->GetSize() != second.GetSize())
    {
#if defined(_DEBUG) || defined(PROFILE)
        std::cout << this->GetSize() << " != " << second.GetSize() << std::endl;
#endif
        return false;
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
                char val1 = this->GetValue(minX + x, minY + y, minZ + z);
                char val2 = second.GetValue(secondMinX + x, secondMinY + y, secondMinZ + z);
                if(val1 != val2)
                {
#if defined(_DEBUG) || defined(PROFILE)
                    std::cout << "val mismatch: " << x << " " << y << " " << z << std::endl;
#endif
                    return false;
                }
            }
        }
    }
    return true;
}


void Octree::ClearSons(void)
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


void Octree::Clear(void)
{
    this->ClearSons();
    m_value = DEFAULT_VALUE;
    m_flags = 0;
}