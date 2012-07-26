#include "StdAfx.h"
#include "ClipmapTerrain.h"
#include "Geometry.h"
#include "ShaderProgram.h"


ClipmapTerrain::ClipmapTerrain(void):
m_mx3Block(0), m_mxmBlock(0), m_initialized(false), m_pProgram(0)
{
    for(int i=0; i < 4; ++i)
    {
        m_pLShape[i] = 0;
    }
}


ClipmapTerrain::~ClipmapTerrain(void)
{
}


bool ClipmapTerrain::Init(unsigned int p_gridSizePerLevel, unsigned int p_levels)
{
    unsigned int& n = p_gridSizePerLevel;
    unsigned int m = (n + 1) / 4;
    m_mxmBlock = CreateRectGrid(m, m, 0.1f);
    m_mx3Block = CreateRectGrid(m, 3, 0.1f);
    m_pLShape[NORTH_EAST] = CreateLShapedGrid(m, NORTH_EAST, 1.0f);
    m_pLShape[NORTH_WEST] = CreateLShapedGrid(m, NORTH_WEST, 1.0f);
    m_pLShape[SOUTH_WEST] = CreateLShapedGrid(m, SOUTH_WEST, 1.0f);
    m_pLShape[SOUTH_EAST] = CreateLShapedGrid(m, SOUTH_EAST, 1.0f);
    if(!m_pProgram)
    {
        m_pProgram.reset(new ShaderProgram);
        m_pProgram->Load(L"./Shader/Clipmap.hlsl", "ClipmapVS", 0, "ClipmapPS");
        m_pProgram->CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements);
    }
    m_initialized = true;
    return m_initialized;
}


void ClipmapTerrain::Draw(void)
{
    m_pProgram->Bind();
    m_mxmBlock->Draw();
    m_mx3Block->Draw();
}


std::shared_ptr<Geometry> ClipmapTerrain::CreateRectGrid(unsigned char p_sizeX, unsigned char p_sizeY, float p_scale)
{
    VertexBuffer::SimpleVertex* pVertices = new VertexBuffer::SimpleVertex[p_sizeX * p_sizeY];
    unsigned int* pIndices = new unsigned int[2 * p_sizeX * (p_sizeY - 1) + (p_sizeY - 1)];
    unsigned int currentIndex = 0;

    for(unsigned char y=0; y < p_sizeY; ++y)
    {
        for(unsigned char x=0; x < p_sizeX; ++x)
        {
            VertexBuffer::SimpleVertex& v = pVertices[y * p_sizeX + x];
            v.positionMC.x = p_scale * (float)x;
            v.positionMC.y = 0.0f;
            v.positionMC.z = p_scale * (float)y;
            v.normalMC.x = 0.0f;
            v.normalMC.y = 1.0f;
            v.normalMC.z = 0.0f;
            if(p_sizeX == p_sizeY)
            {
                v.vertexColor = XMFLOAT4(0.905f, 0.910f, 0.918f, 1.0f);
            }
            else if(p_sizeY == 3)
            {
                v.vertexColor = XMFLOAT4(0.855f, 0.914f, 0.502f, 1.0f);
            }
            else if(p_sizeY == 2)
            {
                v.vertexColor = XMFLOAT4(0.800f, 0.882f, 0.988f, 1.0f);
            }
            else
            {
                v.vertexColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            }

            if(y != p_sizeY - 1)
            {
                pIndices[currentIndex++] = y * p_sizeX + x;
                pIndices[currentIndex++] = (y + 1) * p_sizeX + x;
            }
        }
        if(y != p_sizeY - 1)
        {
            pIndices[currentIndex++] = 0xffffffff;
        }
    }

    Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
    if(!pIndexBuffer->Build(pIndices, currentIndex, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP))
    {
        return 0;
    }
    SAFE_DELETE(pIndices);
    Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
    if(!pVertexBuffer->Build(pVertices, p_sizeX * p_sizeY, sizeof(VertexBuffer::SimpleVertex)))
    {
        return 0;
    }
    SAFE_DELETE(pVertices);

    std::shared_ptr<Geometry> pGeo(new Geometry);
    pGeo->SetIndices(pIndexBuffer);
    pGeo->SetVertices(pVertexBuffer);
    return pGeo;
}


std::shared_ptr<Geometry> ClipmapTerrain::CreateLShapedGrid(unsigned char p_size, Orientation p_orientation, float p_scale)
{
    return 0;
}