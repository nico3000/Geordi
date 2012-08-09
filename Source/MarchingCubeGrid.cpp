#include "StdAfx.h"
#include "MarchingCubeGrid.h"

#define MATERIAL_MAX 16


unsigned int MarchingCubeGrid::sm_terrainVertexNumElements = 6;
D3D11_INPUT_ELEMENT_DESC MarchingCubeGrid::sm_pTerrainVertexElementDesc[6] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "MATERIAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "MATERIAL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "MATERIAL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "MATERIAL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};


void MarchingCubeGrid::ResetCubeCodes(short p_cubes)
{ 
    if(m_cubes != p_cubes)
    {
        m_cubes = p_cubes;
        SAFE_DELETE(m_pCubes);
        m_pCubes = new CubeInfo[p_cubes * p_cubes * p_cubes];
    }
    ZeroMemory(m_pCubes, p_cubes * p_cubes * p_cubes * sizeof(CubeInfo));
}


void MarchingCubeGrid::SetBitIfExists(short p_x, short p_y, short p_z, char p_bit)
{
    if(this->IsIn(p_x, p_y, p_z))
    {
        this->GetCube(p_x, p_y, p_z).code |= 1 << p_bit;
    }
}


void MarchingCubeGrid::SetBits(short p_x, short p_y, short p_z)
{
    this->SetBitIfExists(p_x,     p_y,     p_z,     0);
    this->SetBitIfExists(p_x - 1, p_y,     p_z,     1);
    this->SetBitIfExists(p_x - 1, p_y - 1, p_z,     2);
    this->SetBitIfExists(p_x,     p_y - 1, p_z,     3);
    this->SetBitIfExists(p_x,     p_y,     p_z - 1, 4);
    this->SetBitIfExists(p_x - 1, p_y,     p_z - 1, 5);
    this->SetBitIfExists(p_x - 1, p_y - 1, p_z - 1, 6);
    this->SetBitIfExists(p_x,     p_y - 1, p_z - 1, 7);
}


void MarchingCubeGrid::AddEdgeIfExists(char p_edge, unsigned short p_index, short p_x, short p_y, short p_z)
{
    if(this->IsIn(p_x, p_y, p_z))
    {
        CubeInfo& info = this->GetCube(p_x, p_y, p_z);
        info.pEdgeIndices[p_edge] = p_index;
    }
}


void MarchingCubeGrid::AddEdge(char p_edge, unsigned short p_index, short p_x, short p_y, short p_z)
{
    this->AddEdgeIfExists(p_edge, p_index, p_x, p_y, p_z);
    switch(p_edge)
    {
    case 0:
        this->AddEdgeIfExists(1, p_index, p_x, p_y, p_z - 1);
        this->AddEdgeIfExists(2, p_index, p_x, p_y - 1, p_z - 1);
        this->AddEdgeIfExists(3, p_index, p_x, p_y - 1, p_z);
        break;
    case 4:
        this->AddEdgeIfExists(5, p_index, p_x - 1, p_y, p_z);
        this->AddEdgeIfExists(6, p_index, p_x - 1, p_y, p_z - 1);
        this->AddEdgeIfExists(7, p_index, p_x, p_y, p_z - 1);
        break;
    case 8:
        this->AddEdgeIfExists(9,  p_index, p_x - 1, p_y, p_z);
        this->AddEdgeIfExists(10, p_index, p_x - 1, p_y - 1, p_z);
        this->AddEdgeIfExists(11, p_index, p_x, p_y - 1, p_z);
        
        break;
    }
}


void SetMaterial(int p_material1, int p_material2, float p_t, XMFLOAT4X4& p_target)
{
    //p_target.m[p_material1 / 4][p_material1 % 4] += 1.0f - p_t;
    //p_target.m[p_material2 / 4][p_material2 % 4] += p_t;
}


bool MarchingCubeGrid::ConstructData(Grid3D& p_weightGrid, Grid3D& p_materialGrid, const XMFLOAT3& m_position, float p_scale, int p_level)
{
    static const XMFLOAT4 color(0.5f, 1.5f, 0.5f, 1.0f);

    this->ResetCubeCodes(p_weightGrid.GetSize() - 3);
    m_vertices.clear();
    m_indices.clear();
    for(short x=0; x < m_cubes + 1; ++x)
    {
        for(short y=0; y < m_cubes + 1; ++y)
        {
            for(short z=0; z < m_cubes + 1; ++z)
            {
                float base = p_weightGrid.GetValue(x + 1, y + 1, z + 1);
                int baseMaterial = (int)p_materialGrid.GetValue(x + 1, y + 1, z + 1);
                if(base > 0)
                {
                    this->SetBits(x, y, z);
                }
                if(x != m_cubes)
                {
                    float w = p_weightGrid.GetValue(x + 2, y + 1, z + 1);
                    int m = (int)p_materialGrid.GetValue(x + 2, y + 1, z + 1);
                    if(w != base && w * base <= 0)
                    {
                        float t = base / (base - w);
                        TerrainVertex v =
                        {
                            XMFLOAT3(p_scale * (m_position.x + (float)(x + t)), p_scale * (m_position.y + (float)y), p_scale * (m_position.z + (float)z)),
                            XMFLOAT3(0.0f, 0.0f, 0.0f),
                            XMFLOAT4X4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
                        };
                        switch(p_level)
                        {
                        case 0: v.materialWeight._11 = 0.0f; v.materialWeight._21 = 1.0f; v.materialWeight._31 = 0.0f; break;
                        case 1: v.materialWeight._11 = 1.0f; v.materialWeight._21 = 1.0f; v.materialWeight._31 = 0.0f; break;
                        case 2: v.materialWeight._11 = 1.0f; v.materialWeight._21 = 0.0f; v.materialWeight._31 = 0.0f; break;
                        case 3: v.materialWeight._11 = 1.0f; v.materialWeight._21 = 0.0f; v.materialWeight._31 = 1.0f; break;
                        case 4: v.materialWeight._11 = 0.0f; v.materialWeight._21 = 0.0f; v.materialWeight._31 = 1.0f; break;
                        }
                        SetMaterial(baseMaterial, m, t, v.materialWeight);
                        p_weightGrid.GenerateGradient((float)(x + 1) + t, (float)(y + 1), (float)(z + 1), v.normalMC);
                        this->AddEdge(0, (unsigned short)m_vertices.size(), x, y, z);
                        m_vertices.push_back(v);
                    }
                }
                if(y != m_cubes)
                {
                    float w = p_weightGrid.GetValue(x + 1, y + 2, z + 1);
                    int m = (int)p_materialGrid.GetValue(x + 1, y + 2, z + 1);
                    if(w != base && w * base <= 0)
                    {
                        float t = base / (base - w);
                        TerrainVertex v =
                        {
                            XMFLOAT3(p_scale * (m_position.x + (float)x), p_scale * (m_position.y + (float)(y + t)), p_scale * (m_position.z + (float)z)),
                            XMFLOAT3(0.0f, 0.0f, 0.0f),
                            XMFLOAT4X4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
                        };
                        switch(p_level)
                        {
                        case 0: v.materialWeight._11 = 0.0f; v.materialWeight._21 = 1.0f; v.materialWeight._31 = 0.0f; break;
                        case 1: v.materialWeight._11 = 1.0f; v.materialWeight._21 = 1.0f; v.materialWeight._31 = 0.0f; break;
                        case 2: v.materialWeight._11 = 1.0f; v.materialWeight._21 = 0.0f; v.materialWeight._31 = 0.0f; break;
                        case 3: v.materialWeight._11 = 1.0f; v.materialWeight._21 = 0.0f; v.materialWeight._31 = 1.0f; break;
                        case 4: v.materialWeight._11 = 0.0f; v.materialWeight._21 = 0.0f; v.materialWeight._31 = 1.0f; break;
                        }
                        SetMaterial(baseMaterial, m, t, v.materialWeight);
                        p_weightGrid.GenerateGradient((float)(x + 1), (float)(y + 1) + t, (float)(z + 1), v.normalMC);
                        this->AddEdge(4, (unsigned short)m_vertices.size(), x, y, z);
                        m_vertices.push_back(v);
                    }
                }
                if(z != m_cubes)
                {
                    float w = p_weightGrid.GetValue(x + 1, y + 1, z + 2);
                    int m = (int)p_materialGrid.GetValue(x + 1, y + 1, z + 2);
                    if(w != base && w * base <= 0)
                    {
                        float t = base / (base - w);
                        TerrainVertex v =
                        {
                            XMFLOAT3(p_scale * (m_position.x + (float)x), p_scale * (m_position.y + (float)y), p_scale * (m_position.z + (float)(z + t))),
                            XMFLOAT3(0.0f, 0.0f, 0.0f),
                            XMFLOAT4X4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
                        };
                        switch(p_level)
                        {
                        case 0: v.materialWeight._11 = 0.0f; v.materialWeight._21 = 1.0f; v.materialWeight._31 = 0.0f; break;
                        case 1: v.materialWeight._11 = 1.0f; v.materialWeight._21 = 1.0f; v.materialWeight._31 = 0.0f; break;
                        case 2: v.materialWeight._11 = 1.0f; v.materialWeight._21 = 0.0f; v.materialWeight._31 = 0.0f; break;
                        case 3: v.materialWeight._11 = 1.0f; v.materialWeight._21 = 0.0f; v.materialWeight._31 = 1.0f; break;
                        case 4: v.materialWeight._11 = 0.0f; v.materialWeight._21 = 0.0f; v.materialWeight._31 = 1.0f; break;
                        }
                        SetMaterial(baseMaterial, m, t, v.materialWeight);
                        p_weightGrid.GenerateGradient((float)(x + 1), (float)(y + 1), (float)(z + 1) + t, v.normalMC);
                        this->AddEdge(8, (unsigned short)m_vertices.size(), x, y, z);
                        m_vertices.push_back(v);
                    }
                }
            }
        }
    }
    return !m_vertices.empty();
}


std::shared_ptr<Geometry> MarchingCubeGrid::CreateGeometry(bool p_withPhysics)
{
    if(m_vertices.empty())
    {
        return 0;
    }
    else
    {
        for(int c=0; c < m_cubes * m_cubes * m_cubes; ++c)
        {
            CubeInfo& info = m_pCubes[c];
            for(int i=0; i < 3 * sm_pTriangles[info.code][0]; ++i)
            {
                int edge = sm_pTriangles[info.code][i + 1];
                m_indices.push_back(info.pEdgeIndices[edge]);
            }
        }
        if(m_indices.empty())
        {
            return 0;
        }

        //Geometry::GenerateNormals(&m_vertices[0], &m_indices[0], m_indices.size(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
        pIndexBuffer->Build(&m_indices[0], (unsigned int)m_indices.size(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
        pVertexBuffer->Build(&m_vertices[0], (unsigned int)m_vertices.size(), sizeof(TerrainVertex));

        std::shared_ptr<Geometry> pGeo(new Geometry);
        pGeo->SetIndices(pIndexBuffer);
        pGeo->SetVertices(pVertexBuffer);
        
        if(true || p_withPhysics)
        {
            physx::PxU32* pIndices = new physx::PxU32[m_indices.size()];
            for(int i=0; i < m_indices.size(); i += 3)
            {
                pIndices[i] = m_indices[i];
                pIndices[i+1] = m_indices[i+2];
                pIndices[i+2] = m_indices[i+1];
            }
            physx::PxCooking* pCooking = LostIsland::g_pPhysics->GetCooking();
            physx::PxTriangleMeshDesc desc;
            ZeroMemory(&desc, sizeof(physx::PxTriangleMeshDesc));
            desc.setToDefault();
            desc.points.data = &m_vertices[0];
            desc.points.count = (unsigned int)m_vertices.size();
            desc.points.stride = sizeof(TerrainVertex);
            desc.triangles.data = pIndices;
            desc.triangles.count = (unsigned int)m_indices.size() / 3;
            desc.triangles.stride = 3 * sizeof(unsigned int);
            if(!desc.isValid())
            {
                LI_ERROR("PxTriangleMeshDesc invalid");
            }
            else
            {
                physx::PxDefaultMemoryOutputStream output;
                bool status = pCooking->cookTriangleMesh(desc, output);
                if(!status)
                {
                    LI_ERROR("cookTriangleMesh() failed");
                }
                else
                {
                    physx::PxDefaultMemoryInputData input(output.getData(), output.getSize());
                    physx::PxTriangleMesh* pMesh = LostIsland::g_pPhysics->GetPhysics()->createTriangleMesh(input);
                    physx::PxMaterial* pMaterial = LostIsland::g_pPhysics->GetPhysics()->createMaterial(0.5f, 0.5f, 0.1f);
                    physx::PxRigidStatic* pActor = LostIsland::g_pPhysics->GetPhysics()->createRigidStatic(physx::PxTransform::createIdentity());
                    physx::PxShape* pShape = pActor->createShape(physx::PxTriangleMeshGeometry(pMesh), *pMaterial);
                    LostIsland::g_pPhysics->GetScene()->addActor(*pActor);
                }
            }
            SAFE_DELETE(pIndices);
        }
        
        return pGeo;
    }
}


#define NUM_BASE_CUBES 22
#define ROT_X 0
#define ROT_Y 1
#define ROT_Z 2
#define MIRROR 3

void MarchingCubeGrid::Init(void)
{
    ZeroMemory(sm_pTriangles, 256 * 16 * sizeof(unsigned char));
    unsigned char pCodes[NUM_BASE_CUBES] = {
        0, 1, 3, 5, 7, 15, 20, 21, 23, 26, 27, 30, 31, 60, 61, 63, 90, 91, 95, 125, 127, 255,
    };
    unsigned char pTriangles[NUM_BASE_CUBES][16] = {
        { 0 },                                                      // 00000000 = 0
        { 1, 0, 8, 4, },                                            // 00000001 = 1
        { 2, 4, 5, 8, 5, 9, 8, },                                   // 00000011 = 3
        { 2, 0, 8, 4, 3, 10, 5, },                                  // 00000101 = 5
        { 3, 8, 10, 9, 8, 3, 10, 8, 4, 3, },                        // 00000111 = 7
        { 2, 8, 11, 9, 9, 11, 10, },                                // 00001111 = 15
        { 2, 1, 7, 8, 3, 10, 5, },                                  // 00010100 = 20
        { 3, 0, 1, 4, 4, 1, 7, 3, 10, 5, },                         // 00010101 = 21
        { 4, 4, 3, 7, 3, 10, 7, 10, 1, 7, 1, 10, 9  },              // 00010111 = 23
        { 3, 0, 5, 9, 1, 7, 8, 3, 4, 11, },                         // 00011010 = 26
        { 4, 1, 5, 9, 1, 3, 5, 1, 7, 3, 7, 11, 3, },                // 00011011 = 27
        { 4, 1, 7, 8, 9, 11, 10, 9, 0, 11, 0, 4, 11, },             // 00011110 = 30
        { 3, 9, 11, 10, 9, 1, 11, 1, 7, 11, },                      // 00011111 = 31
        { 4, 6, 7, 9, 7, 8, 9, 4, 11, 5, 11, 10, 5, },              // 00111100 = 60
        { 5, 10, 5, 11, 5, 0, 11, 0, 9, 11, 9, 7, 11, 9, 6, 7 },    // 00111101 = 61
        { 2, 11, 10, 7, 10, 6, 7, },                                // 00111111 = 63
        { 4, 0, 5, 9, 1, 7, 8, 2, 6, 10, 3, 4, 11, },               // 01011010 = 90
        { 5, 2, 6, 10, 9, 1, 5, 1, 7, 5, 5, 7, 3, 3, 7, 11, },      // 01011011 = 91
        { 4, 2, 6, 11, 6, 9, 11, 9, 1, 11, 1, 7, 11, },             // 01011111 = 95
        { 2, 0, 9, 5, 2, 7, 11, },                                  // 01111101 = 125
        { 1, 2, 7, 11, },                                           // 01111111 = 127
        { 0 },                                                      // 11111111 = 255
    };
    for(int i=0; i < NUM_BASE_CUBES; ++i)
    {
        ProcessBaseCube(pCodes[i], pTriangles[i]);
    }
}


void MarchingCubeGrid::ProcessBaseCube(unsigned char p_code, unsigned char* p_pTriangles)
{
    for(int mirror=0; mirror < 2; ++mirror)
    {
        for(int x=0; x < 4; ++x)
        {
            for(int y=0; y < 4; ++y)
            {
                for(int z=0; z < 4; ++z)
                {
                    CopyTriangles(p_code, p_pTriangles, mirror % 2 == 1);
                    Transform(p_code, p_pTriangles, ROT_Z);
                }
                Transform(p_code, p_pTriangles, ROT_Y);
            }
            Transform(p_code, p_pTriangles, ROT_X);
        }
        Transform(p_code, p_pTriangles, MIRROR);
    }
}


void MarchingCubeGrid::Transform(unsigned char& p_code, unsigned char* p_pTriangles, int p_transformIndex)
{
    char code = p_code;
    p_code = 0;
    for(int i=0; i < 8; ++i)
    {
        if(code & (1 << i))
        {
            p_code |= 1 << sm_pCodeTransforms[p_transformIndex][i];
        }
    }

    for(int i=1; i < 16; ++i)
    {
        p_pTriangles[i] = sm_pEdgeTransforms[p_transformIndex][p_pTriangles[i]];
    }
}


void MarchingCubeGrid::CopyTriangles(unsigned char p_code, unsigned char* p_pTriangles, bool mirrored)
{
    sm_pTriangles[p_code][0] = p_pTriangles[0];
    for(int i=0; i < 3 * sm_pTriangles[p_code][0]; i += 3)
    {
        sm_pTriangles[p_code][1 + i + 0] = p_pTriangles[1 + i + 0];
        sm_pTriangles[p_code][1 + i + 1] = p_pTriangles[1 + i + (mirrored ? 2 : 1)];
        sm_pTriangles[p_code][1 + i + 2] = p_pTriangles[1 + i + (mirrored ? 1 : 2)];
    }
}


const unsigned char MarchingCubeGrid::sm_pCodeTransforms[4][8] = {
    { 4, 5, 1, 0, 7, 6, 2, 3, }, // x axis rotation
    { 1, 5, 6, 2, 0, 4, 7, 3, }, // y axis rotation
    { 3, 0, 1, 2, 7, 4, 5, 6, }, // z axis rotation
    { 1, 0, 3, 2, 5, 4, 7, 6, }, // yz mirror
};


const unsigned char MarchingCubeGrid::sm_pEdgeTransforms[4][12] = {
    { 1, 2, 3, 0, 8, 9, 10, 11, 7, 6, 5, 4, }, // x axis rotation
    { 9, 8, 11, 10, 5, 6, 7, 4, 0, 1, 2, 3, }, // y axis rotation
    { 4, 7, 6, 5, 3, 0, 1, 2, 11, 8, 9, 10, }, // z axis rotation
    { 0, 1, 2, 3, 5, 4, 7, 6, 9, 8, 11, 10, }, // yz mirror
};


unsigned char MarchingCubeGrid::sm_pTriangles[256][16];