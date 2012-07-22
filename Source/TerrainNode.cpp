#include "StdAfx.h"
#include "TerrainNode.h"
#include "Scene.h"


TerrainNode::TerrainNode(std::shared_ptr<TerrainData> p_pTerrain, int p_chunksize, int p_smallradius) :
    m_pTerrain(p_pTerrain), m_chunksize(p_chunksize), m_smallradius(p_smallradius)
{
    m_tempGrid.Init(p_chunksize + 1);
    m_blockData.Init(max(m_pTerrain->GetSizeX(), max(m_pTerrain->GetSizeY(), m_pTerrain->GetSizeZ())));
}



TerrainNode::~TerrainNode(void)
{
}


HRESULT TerrainNode::VOnRestore(void)
{
    if(!m_program.Load(L"./Shader/NicotopiaTest.fx", "SimpleVS", 0, "SimplePS"))
    {
        return S_FALSE;
    }
    if(!m_program.CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements))
    {
        return S_FALSE;
    }

    return S_OK;
}


HRESULT TerrainNode::VOnLostDevice(void)
{
    return S_OK;
}


HRESULT TerrainNode::VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis)
{
    static unsigned long elapsed = 0;
    elapsed += p_deltaMillis;
    if(elapsed < 1000)
    {
        return S_OK;
    }
    elapsed = 0;

    const XMFLOAT3& pos = p_pScene->GetCurrentCamera()->GetPosition();
    int posX = (int)(10.0f * pos.x / (float)m_chunksize);
    int posY = (int)(10.0f * pos.y / (float)m_chunksize);
    int posZ = (int)(10.0f * pos.z / (float)m_chunksize);

    int distance = 0;
    int freeBlock = -1;
    for(int i=0; i < NUM_BLOCKS; ++i)
    {
        int currentDistance = !m_blocks[i].HasGeometry() ? INT_MAX : max(abs(m_blocks[i].GetX() - posX), max(abs(m_blocks[i].GetY() - posY), abs(m_blocks[i].GetZ() - posZ)));
        if(currentDistance > distance)
        {
            distance = currentDistance;
            freeBlock = i;
        }
    }
    if(freeBlock == -1)
    {
        return S_OK;
    }
    
    int toFillX = -1, toFillY = -1, toFillZ = -1;
    for(int dx=-(int)m_smallradius; dx <= m_smallradius; ++dx)
    {
        for(int dy=-(int)m_smallradius; dy <= m_smallradius; ++dy)
        {
            for(int dz=-(int)m_smallradius; dz <= m_smallradius; ++dz)
            {
                int currentDistance = max(abs(dx), max(abs(dy), abs(dz)));
                int x = posX + dx;
                int y = posY + dy;
                int z = posZ + dz;
                if(currentDistance < distance &&
                    0 <= x && x < m_pTerrain->GetSizeX() &&
                    0 <= y && y < m_pTerrain->GetSizeY() &&
                    0 <= z && z < m_pTerrain->GetSizeZ())
                {
                    short flags = m_blockData.GetValue(x, y, z);
                    if(flags == 0 || flags == 3) // undefined or (not empty and unused)
                    {
                        toFillX = x;
                        toFillY = y;
                        toFillZ = z;
                        distance = currentDistance;
                    }
                }
            }
        }
    }
    if(toFillX != -1)
    {
        bool hasGeometry = m_pTerrain->FillGrid(m_tempGrid, m_chunksize * toFillX, m_chunksize * toFillY, m_chunksize * toFillZ);
        if(hasGeometry)
        {
            if(m_blocks[freeBlock].GetGeometry().IsReady())
            {
                m_blockData.SetValue(m_blocks[freeBlock].GetX(), m_blocks[freeBlock].GetY(), m_blocks[freeBlock].GetZ(), 3); // not empty and unused
            }
            m_blockData.SetValue(toFillX, toFillY, toFillZ, 1); // not empty and used
            m_blocks[freeBlock].Build(toFillX, toFillY, toFillZ, m_tempGrid, 0.1f);
        }
        else
        {
            m_blockData.SetValue(toFillX, toFillY, toFillZ, 2); // empty (and unused)
            return S_OK;
        }

        // visualize chunks
        VertexBuffer::SimpleVertex pVertices[8] = {
            XMFLOAT3(toFillX * m_chunksize,       toFillY * m_chunksize,       toFillZ * m_chunksize),       XMFLOAT3(0,1,0), XMFLOAT4(1,0,0,1),
            XMFLOAT3((toFillX + 1) * m_chunksize, toFillY * m_chunksize,       toFillZ * m_chunksize),       XMFLOAT3(0,1,0), XMFLOAT4(1,0,0,1),
            XMFLOAT3((toFillX + 1) * m_chunksize, (toFillY + 1) * m_chunksize, toFillZ * m_chunksize),       XMFLOAT3(0,1,0), XMFLOAT4(1,0,0,1),
            XMFLOAT3(toFillX * m_chunksize,       (toFillY + 1) * m_chunksize, toFillZ * m_chunksize),       XMFLOAT3(0,1,0), XMFLOAT4(1,0,0,1),
            XMFLOAT3(toFillX * m_chunksize,       toFillY * m_chunksize,       (toFillZ + 1) * m_chunksize), XMFLOAT3(0,1,0), XMFLOAT4(1,0,0,1),
            XMFLOAT3((toFillX + 1) * m_chunksize, toFillY * m_chunksize,       (toFillZ + 1) * m_chunksize), XMFLOAT3(0,1,0), XMFLOAT4(1,0,0,1),
            XMFLOAT3((toFillX + 1) * m_chunksize, (toFillY + 1) * m_chunksize, (toFillZ + 1) * m_chunksize), XMFLOAT3(0,1,0), XMFLOAT4(1,0,0,1),
            XMFLOAT3(toFillX * m_chunksize,       (toFillY + 1) * m_chunksize, (toFillZ + 1) * m_chunksize), XMFLOAT3(0,1,0), XMFLOAT4(1,0,0,1),
        };
        unsigned int pIndices[] = {
            0, 1, 2, 3, 0, 4, 5, 6, 7, 4, 0xffffffff,
            3, 7, 0xffffffff,
            2, 6, 0xffffffff,
            1, 5, 
        };
        Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
        pVertexBuffer->Build(pVertices, ARRAYSIZE(pVertices), sizeof(VertexBuffer::SimpleVertex));
        Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
        pIndexBuffer->Build(pIndices, ARRAYSIZE(pIndices), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
        std::shared_ptr<Geometry> pGeo(new Geometry);
        pGeo->SetIndices(pIndexBuffer);
        pGeo->SetVertices(pVertexBuffer);
        m_empty.push_back(pGeo);
        // end visualize chunks
    }
    else
    {
        //LI_INFO("only empty blocks nearby");
    }

    return S_OK;
}


HRESULT TerrainNode::VPreRender(Scene* p_pScene)
{
    return S_OK;
}


HRESULT TerrainNode::VRender(Scene* p_pScene)
{
    m_program.Bind();
    Pose::ModelMatrixData data;
    XMVECTOR det;
    for(int i=0; i < NUM_BLOCKS; ++i)
    {
        XMStoreFloat4x4(&data.model, XMMatrixTranslation(
            0.1f * (float)(m_chunksize * m_blocks[i].GetX()),
            0.1f * (float)(m_chunksize * m_blocks[i].GetY()),
            0.1f * (float)(m_chunksize * m_blocks[i].GetZ())));
        XMStoreFloat4x4(&data.modelInv, XMMatrixIdentity());
        p_pScene->PushModelMatrices(data, true);

        m_blocks[i].Draw();

        p_pScene->PopModelMatrices(false);
    }

    XMStoreFloat4x4(&data.model, XMMatrixScaling(0.1f, 0.1f, 0.1f));
    XMStoreFloat4x4(&data.modelInv, XMMatrixInverse(&det, XMLoadFloat4x4(&data.modelInv)));
    p_pScene->PushModelMatrices(data, true);
    for(auto iter=m_empty.begin(); iter != m_empty.end(); ++iter)
    {
        (*iter)->Draw();
    }
    p_pScene->PopModelMatrices(false);
    return S_OK;
}


HRESULT TerrainNode::VPostRender(Scene* p_pScene)
{
    return S_OK;
}


static const int mcIndices[16][6] = {
    { -1, -1, -1,  -1, -1, -1 },    // 0
    {  0,  2,  1,  -1, -1, -1 },    // 1
    {  0,  2,  1,  -1, -1, -1 },    // 2
    {  0,  2,  1,   0,  1,  3 },    // 3
    {  0,  1,  2,  -1, -1, -1 },    // 4
    {  0,  1,  2,   0,  2,  3 },    // 5
    {  0,  1,  2,   0,  3,  1 },    // 6
    {  0,  1,  2,  -1, -1, -1 },    // 7
    {  0,  2,  1,  -1, -1, -1 },    // 8
    {  0,  2,  1,   0,  1,  3 },    // 9
    {  0,  2,  1,   0,  3,  2 },    // 10
    {  0,  2,  1,  -1, -1, -1 },    // 11
    {  0,  1,  2,   0,  3,  1 },    // 12
    {  0,  1,  2,  -1, -1, -1 },    // 13
    {  0,  1,  2,  -1, -1, -1 },    // 14
    { -1, -1, -1,  -1, -1, -1 },    // 15
};


unsigned int AppendVertex(const XMFLOAT3& p_v0, const XMFLOAT3& p_v1, float p_w0, float p_w1, std::vector<VertexBuffer::SimpleVertex>& p_vertices, float p_scale, Grid3D& p_grid)
{
    float t = p_w0 / (p_w0 - p_w1);
    XMFLOAT3 interpol;
    interpol.x = MIX(p_v0.x, p_v1.x, t);
    interpol.y = MIX(p_v0.y, p_v1.y, t);
    interpol.z = MIX(p_v0.z, p_v1.z, t);
    unsigned int index = (unsigned int)p_vertices.size();
//     for(int i=0; i < p_vertices.size() && index == p_vertices.size(); ++i)
//     {
//         if(p_vertices[i].positionMC.x == interpol.x && p_vertices[i].positionMC.y == interpol.y && p_vertices[i].positionMC.z == interpol.z)
//         {
//             index = i;
//         }
//     }
    if(index == p_vertices.size())
    {
//         float h = 0.3f;
//         float fdx = p_grid.SampleLinear(interpol.x + h, interpol.y, interpol.z) / h;
//         float fdy = p_grid.SampleLinear(interpol.x, interpol.y + h, interpol.z) / h;
//         float fdz = p_grid.SampleLinear(interpol.x, interpol.y, interpol.z + h) / h;
//         float len = sqrt(fdx * fdx + fdy * fdy + fdz * fdz);
//         VertexBuffer::SimpleVertex vertex = { interpol, XMFLOAT3(fdx / len, fdy / len, fdz / len), XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f) };
        interpol.x *= p_scale;
        interpol.y *= p_scale;
        interpol.z *= p_scale;
        VertexBuffer::SimpleVertex vertex = { interpol, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f) };
        p_vertices.push_back(vertex);
    }    
    return index;
}


void AppendTetrahedron(
    int p_i0,
    int p_i1,
    int p_i2,
    int p_i3,
    std::vector<VertexBuffer::SimpleVertex>& p_vertices,
    std::vector<unsigned int>& p_indices,
    Grid3D& p_grid,
    float p_scale)
{
    int x, y, z;
    p_grid.SingleIndexToCoords(p_i0, x, y, z);
    XMFLOAT3 v0((float)x, (float)y, (float)z);
    p_grid.SingleIndexToCoords(p_i1, x, y, z);
    XMFLOAT3 v1((float)x, (float)y, (float)z);
    p_grid.SingleIndexToCoords(p_i2, x, y, z);
    XMFLOAT3 v2((float)x, (float)y, (float)z);
    p_grid.SingleIndexToCoords(p_i3, x, y, z);
    XMFLOAT3 v3((float)x, (float)y, (float)z);

    float w0 = p_grid.GetValue(p_i0);
    float w1 = p_grid.GetValue(p_i1);
    float w2 = p_grid.GetValue(p_i2);
    float w3 = p_grid.GetValue(p_i3);

    unsigned int baseIndex = (unsigned int)p_vertices.size();
    bool pCode[4] = { w0 >= 0, w1 >= 0, w2 >= 0, w3 >= 0, };
    char code = (pCode[0] ? 1 : 0) | (pCode[1] ? 2 : 0) | (pCode[2] ? 4 : 0) | (pCode[3] ? 8 : 0);
    std::vector<unsigned int> generatedVertices;
    if(pCode[0] != pCode[1])
    {
        generatedVertices.push_back(AppendVertex(v0, v1, w0, w1, p_vertices, p_scale, p_grid));
    }
    if(pCode[1] != pCode[2])
    {
        generatedVertices.push_back(AppendVertex(v1, v2, w1, w2, p_vertices, p_scale, p_grid));
    }
    if(pCode[2] != pCode[3])
    {
        generatedVertices.push_back(AppendVertex(v2, v3, w2, w3, p_vertices, p_scale, p_grid));
    }
    if(pCode[3] != pCode[0])
    {
        generatedVertices.push_back(AppendVertex(v3, v0, w3, w0, p_vertices, p_scale, p_grid));
    }
    if(pCode[0] != pCode[2])
    {
        generatedVertices.push_back(AppendVertex(v0, v2, w0, w2, p_vertices, p_scale, p_grid));
    }
    if(pCode[1] != pCode[3])
    {
        generatedVertices.push_back(AppendVertex(v1, v3, w1, w3, p_vertices, p_scale, p_grid));
    }
    for(int i=0; i < 6; ++i)
    {
        int offset = mcIndices[code][i];
        if(offset != -1)
        {
            p_indices.push_back(generatedVertices[offset]);
        }
    }
}


void TerrainBlock::Build(int p_x, int p_y, int p_z, Grid3D& p_grid, float p_scale)
{
    static std::vector<VertexBuffer::SimpleVertex> vertices;
    static std::vector<unsigned int> indices;
    vertices.clear();
    indices.clear();
    for(int x=0; x < p_grid.GetSize() - 1; ++x)
    {
        for(int y=0; y < p_grid.GetSize() - 1; ++y)
        {
            for(int z=0; z < p_grid.GetSize() - 1; ++z)
            {
                int i0 = p_grid.GetSingleIndex(x,     y,     z);
                int i1 = p_grid.GetSingleIndex(x + 1, y,     z);
                int i2 = p_grid.GetSingleIndex(x + 1, y + 1, z);
                int i3 = p_grid.GetSingleIndex(x,     y + 1, z);
                int i4 = p_grid.GetSingleIndex(x,     y,     z + 1);
                int i5 = p_grid.GetSingleIndex(x + 1, y,     z + 1);
                int i6 = p_grid.GetSingleIndex(x + 1, y + 1, z + 1);
                int i7 = p_grid.GetSingleIndex(x,     y + 1, z + 1);
                
                AppendTetrahedron(i2, i4, i6, i7, vertices, indices, p_grid, p_scale);
                AppendTetrahedron(i2, i3, i4, i7, vertices, indices, p_grid, p_scale);
                AppendTetrahedron(i0, i1, i2, i4, vertices, indices, p_grid, p_scale);
                AppendTetrahedron(i1, i5, i2, i4, vertices, indices, p_grid, p_scale);
                AppendTetrahedron(i0, i2, i3, i4, vertices, indices, p_grid, p_scale);
                AppendTetrahedron(i5, i6, i2, i4, vertices, indices, p_grid, p_scale);
            }
        }
    }

    if(vertices.empty())
    {
        LI_INFO("should not happen: no geometry created");
        return;
    }
    
    if(!Geometry::GenerateNormals(&vertices[0], &indices[0], (unsigned int)indices.size(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST))
    {
        LI_ERROR("generating normals failed");
    }
    std::ostringstream str;
    str << "generated " << vertices.size() << " vertices";
    LI_INFO(str.str());
    
    Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
    pIndexBuffer->Build(&indices[0], (unsigned int)indices.size(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
    pVertexBuffer->Build(&vertices[0], (unsigned int)vertices.size(), sizeof(VertexBuffer::SimpleVertex));

    m_geometry.Destroy();    
    m_geometry.SetIndices(pIndexBuffer);
    m_geometry.SetVertices(pVertexBuffer);
    m_x = p_x;
    m_y = p_y;
    m_z = p_z;
}
