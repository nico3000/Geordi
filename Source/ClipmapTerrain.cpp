#include "StdAfx.h"
#include "ClipmapTerrain.h"
#include "Geometry.h"
#include "ShaderProgram.h"
#include "ConstantBuffer.h"
#include "DisplacementData.h"


ClipmapTerrain::ClipmapTerrain(void):
m_mx3Block(0), m_mxmBlock(0), m_initialized(false), m_pProgram(0), m_pBuffer(0), m_pLShapeToDraw(0), m_pTexture(0), m_pSRV(0), m_pLevels(0)
{
}


ClipmapTerrain::~ClipmapTerrain(void)
{
    this->Cleanup();
}


void ClipmapTerrain::Cleanup(void)
{
    SAFE_DELETE_ARRAY(m_pLShapeToDraw);
    SAFE_DELETE_ARRAY(m_pLevels);
    SAFE_RELEASE(m_pSRV);
    SAFE_RELEASE(m_pTexture);
    m_currentX = INT_MAX;
    m_currentZ = INT_MAX;
}


bool ClipmapTerrain::Init(unsigned int p_gridSizePerLevel, unsigned int p_levelCount)
{
    this->Cleanup();

    m_pData.reset(new DisplacementData);
	//m_pData->Init("./Displacements/test.ndm", 16);
    m_pData->GenerateTestData();

    float *pFloatData = new float[p_levelCount * (p_gridSizePerLevel + 1) * (p_gridSizePerLevel + 1)];
    ZeroMemory(pFloatData, p_levelCount * (p_gridSizePerLevel + 1) * (p_gridSizePerLevel + 1) * sizeof(float));
    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = pFloatData;
    data.SysMemPitch = (p_gridSizePerLevel + 1) * sizeof(float);
    data.SysMemSlicePitch = (p_gridSizePerLevel + 1) * (p_gridSizePerLevel + 1) * sizeof(float);

    D3D11_TEXTURE2D_DESC desc;
    desc.ArraySize = p_levelCount;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.Format = DXGI_FORMAT_R32_FLOAT;
    desc.Width = p_gridSizePerLevel + 1;
    desc.Height = p_gridSizePerLevel + 1;
    desc.MipLevels = 1;
    desc.MiscFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    HRESULT hr = LostIsland::g_pGraphics->GetDevice()->CreateTexture2D(&desc, 0, &m_pTexture);
    RETURN_IF_FAILED(hr);
    SAFE_DELETE(data.pSysMem);
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = desc.Format;
    srvDesc.Texture2DArray.ArraySize = p_levelCount;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.MipLevels = 1;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    hr = LostIsland::g_pGraphics->GetDevice()->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pSRV);
    RETURN_IF_FAILED(hr);

    m_scale = 0.125f;
    m_levelCount = p_levelCount;
    m_paramN = p_gridSizePerLevel;
    m_paramM = (m_paramN + 1) / 4;
    m_pLShapeToDraw = new std::shared_ptr<Geometry>[p_levelCount];
    m_pLevels = new ClipmapLevel[p_levelCount];
    for(unsigned int i=0; i < p_levelCount; ++i)
    {
        m_pLevels[i].Init(m_paramN, i, m_pTexture);
    }

    m_fullBlock = CreateRectGrid(m_paramN, m_paramN, 1.0f);
    m_mxmBlock = CreateRectGrid(m_paramM, m_paramM, 1.0f);
    m_mx3Block = CreateRectGrid(m_paramM, 3, 1.0f);
    m_3xmBlock = CreateRectGrid(3, m_paramM, 1.0f);
    m_degenerateTriangles = CreateDegenerateTriangles(m_paramN, m_paramN, 1.0f);
    m_pLShape[NORTH_EAST] = CreateLShapedGrid(2 * m_paramM + 1, NORTH_EAST, 1.0f);
    m_pLShape[NORTH_WEST] = CreateLShapedGrid(2 * m_paramM + 1, NORTH_WEST, 1.0f);
    m_pLShape[SOUTH_WEST] = CreateLShapedGrid(2 * m_paramM + 1, SOUTH_WEST, 1.0f);
    m_pLShape[SOUTH_EAST] = CreateLShapedGrid(2 * m_paramM + 1, SOUTH_EAST, 1.0f);
    for(unsigned int i=0; i < p_levelCount; ++i) {
        m_pLShapeToDraw[i] = m_pLShape[NORTH_EAST];
    }
    float trans0 = 0.5f * (float)(m_paramN - 1) - 0.5f * (float)(m_paramM - 1);
    float trans1 = 0.5f * (float)(m_paramN - 1) - 1.5f * (float)(m_paramM - 1);
    m_mxmTranslations[0] = XMFLOAT3(-trans0, 0.0f, +trans0);
    m_mxmTranslations[1] = XMFLOAT3(-trans1, 0.0f, +trans0);
    m_mxmTranslations[2] = XMFLOAT3(+trans1, 0.0f, +trans0);
    m_mxmTranslations[3] = XMFLOAT3(+trans0, 0.0f, +trans0);
    m_mxmTranslations[4] = XMFLOAT3(-trans0, 0.0f, +trans1);
    m_mxmTranslations[5] = XMFLOAT3(+trans0, 0.0f, +trans1);
    m_mxmTranslations[6] = XMFLOAT3(-trans0, 0.0f, -trans1);
    m_mxmTranslations[7] = XMFLOAT3(+trans0, 0.0f, -trans1);
    m_mxmTranslations[8] = XMFLOAT3(-trans0, 0.0f, -trans0);
    m_mxmTranslations[9] = XMFLOAT3(-trans1, 0.0f, -trans0);
    m_mxmTranslations[10] = XMFLOAT3(+trans1, 0.0f, -trans0);
    m_mxmTranslations[11] = XMFLOAT3(+trans0, 0.0f, -trans0);
    m_mx3Translations[0] = XMFLOAT3(-trans0, 0.0f, 0.0f);
    m_mx3Translations[1] = XMFLOAT3(+trans0, 0.0f, 0.0f);
    m_3xmTranslations[0] = XMFLOAT3(0.0f, 0.0f, +trans0);
    m_3xmTranslations[1] = XMFLOAT3(0.0f, 0.0f, -trans0);

    if(!m_pProgram)
    {
        m_pProgram.reset(new ShaderProgram);
        if(!m_pProgram->Load(L"./Shader/Clipmap.hlsl", "ClipmapVS", 0, "ClipmapPS"))
        {
            return false;
        }
        if(!m_pProgram->CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements))
        {
            return false;
        }
    }
    if(!m_pBuffer)
    {
        TileProperties properties;
        m_pBuffer.reset(new ConstantBuffer);
        if(!m_pBuffer->CopyDataAndBuild(&properties, sizeof(TileProperties)))
        {
            return false;
        }
    }
    m_initialized = true;
    return m_initialized;
}


void ClipmapTerrain::DrawLevel(unsigned int p_level) const
{
    m_pProgram->Bind();
    TileProperties& props = *((TileProperties*)m_pBuffer->GetData());
    props.scale = m_scale * (float)(1 << (p_level));
    props.globalTranslation = m_pLevels[p_level].GetGlobalTranslation();
    props.tileTranslation = XMFLOAT3(0.0f, 0.0f, 0.0f);
    props.level = (float)p_level;
    props.gridSize = (float)(m_paramN);

    m_pBuffer->UpdateAndBind(3, TARGET_VS_PS);
    m_degenerateTriangles->Draw();

    if(p_level == 0)
    {
        m_fullBlock->Draw();
    }
    else
    {
        m_pLShapeToDraw[p_level]->Draw();
        for(int i=0; i < 12; ++i)
        {
            props.tileTranslation = m_mxmTranslations[i];
            m_pBuffer->UpdateAndBind(3, TARGET_VS_PS);
            m_mxmBlock->Draw();
        }
        for(int i=0; i < 2; ++i)
        {
            props.tileTranslation = m_mx3Translations[i];
            m_pBuffer->UpdateAndBind(3, TARGET_VS_PS);
            m_mx3Block->Draw();
            props.tileTranslation = m_3xmTranslations[i];
            m_pBuffer->UpdateAndBind(3, TARGET_VS_PS);
            m_3xmBlock->Draw();
        }
    }
}


void ClipmapTerrain::Draw(void) const
{
    static ID3D11RasterizerState* pState = 0;
    static ID3D11RasterizerState* pPrevState = 0;
    if(pState == 0)
    {
        D3D11_RASTERIZER_DESC desc;
        ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
        desc.AntialiasedLineEnable = true;
        desc.CullMode = D3D11_CULL_NONE;
        desc.FillMode = D3D11_FILL_WIREFRAME;
        desc.FrontCounterClockwise = true;
        LostIsland::g_pGraphics->GetDevice()->CreateRasterizerState(&desc, &pState);
    }
    LostIsland::g_pGraphics->GetContext()->RSGetState(&pPrevState);
    //LostIsland::g_pGraphics->GetContext()->RSSetState(pState);
    LostIsland::g_pGraphics->GetContext()->VSSetShaderResources(7, 1, &m_pSRV);
    LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(7, 1, &m_pSRV);
    for(unsigned int i=0; i < m_levelCount; ++i)
    {
        this->DrawLevel(i);
    }
    LostIsland::g_pGraphics->GetContext()->RSSetState(pPrevState);
    SAFE_RELEASE(pPrevState);
    //SAFE_RELEASE(pState); // memory leak
}


void ClipmapTerrain::Update(const XMFLOAT3& p_viewPoint)
{
    //XMFLOAT3 p_viewPoint(0,0,0);
    int lastX = (int)(2.0f * ROUND((p_viewPoint.x - 1.0f) / (2.0f * m_scale)) + 1.0f);
    int lastZ = (int)(2.0f * ROUND((p_viewPoint.z - 1.0f) / (2.0f * m_scale)) + 1.0f);
    if(abs(m_currentX - lastX) > (int)m_paramN / 8 || abs(m_currentZ - lastZ) > (int)m_paramN / 8)
    {
        m_pLevels[0].Update(m_currentX = lastX, m_currentZ = lastZ, m_pData);
        for(unsigned int level=1; level < m_levelCount; ++level)
        {
            lastX = (int)ROUND(0.5f * (float)lastX);
            lastZ = (int)ROUND(0.5f * (float)lastZ);
            m_pLShapeToDraw[level] = m_pLShape[(lastX % 2 == 0 ? WEST : EAST) | (lastZ % 2 == 0 ? SOUTH : NORTH)];
            if(lastX % 2 == 0) --lastX;
            if(lastZ % 2 == 0) --lastZ;
            m_pLevels[level].Update(lastX, lastZ, m_pData);
        }
    }
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
            v.positionMC = XMFLOAT3(p_scale * (float)x - p_scale * 0.5f * (float)(p_sizeX - 1), 0.0f, p_scale * (float)y - p_scale * 0.5f * (float)(p_sizeY - 1));
            v.normalMC = XMFLOAT3(0.0f, 1.0f, 0.0f);
            if(p_sizeX == p_sizeY)
            {
                v.vertexColor = XMFLOAT4(0.906f, 0.906f, 0.910f, 1.0f);
            }
            else if(p_sizeX == 3 || p_sizeY == 3)
            {
                v.vertexColor = XMFLOAT4(0.824f, 0.887f, 0.494f, 1.0f);
            }
            else
            {
                v.vertexColor = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            }

            if(y != p_sizeY - 1)
            {
                pIndices[currentIndex++] = (y + 1) * p_sizeX + x;
                pIndices[currentIndex++] = y * p_sizeX + x;
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


std::shared_ptr<Geometry> ClipmapTerrain::CreateDegenerateTriangles(unsigned char p_sizeX, unsigned char p_sizeY, float p_scale)
{
    std::vector<VertexBuffer::SimpleVertex> vertices;
    std::vector<unsigned int> indices;
    VertexBuffer::SimpleVertex v;

    for(unsigned char x=0; x < p_sizeX; ++x)
    {
        v.positionMC = XMFLOAT3(p_scale * (float)x - p_scale * 0.5f * (float)(p_sizeX - 1), 0.0f, p_scale * 0.5f * (float)(p_sizeY - 1));
        v.normalMC = XMFLOAT3(0.0f, 1.0f, 0.0f);
        v.vertexColor = XMFLOAT4(0.961f, 0.510f, 0.125f, 1.0f);
        vertices.push_back(v);

        v.positionMC = XMFLOAT3(p_scale * (float)x - p_scale * 0.5f * (float)(p_sizeX - 1), 0.0f, -p_scale * 0.5f * (float)(p_sizeY - 1));
        v.normalMC = XMFLOAT3(0.0f, 1.0f, 0.0f);
        v.vertexColor = XMFLOAT4(0.961f, 0.510f, 0.125f, 1.0f);
        vertices.push_back(v);

        if(x % 2 == 1)
        {
            indices.push_back((unsigned int)vertices.size());
            indices.push_back((unsigned int)vertices.size() - 4);
            indices.push_back((unsigned int)vertices.size() - 2);
            indices.push_back((unsigned int)vertices.size() + 1);
            indices.push_back((unsigned int)vertices.size() - 1);
            indices.push_back((unsigned int)vertices.size() - 3);
        }
    }
    for(unsigned char y=0; y < p_sizeX; ++y)
    {
        v.positionMC = XMFLOAT3(-p_scale * 0.5f * (float)(p_sizeX - 1), 0.0f, p_scale * (float)y - p_scale * 0.5f * (float)(p_sizeY - 1));
        v.normalMC = XMFLOAT3(0.0f, 1.0f, 0.0f);
        v.vertexColor = XMFLOAT4(0.961f, 0.510f, 0.125f, 1.0f);
        vertices.push_back(v);

        v.positionMC = XMFLOAT3(+p_scale * 0.5f * (float)(p_sizeX - 1), 0.0f, p_scale * (float)y - p_scale * 0.5f * (float)(p_sizeY - 1));
        v.normalMC = XMFLOAT3(0.0f, 1.0f, 0.0f);
        v.vertexColor = XMFLOAT4(0.961f, 0.510f, 0.125f, 1.0f);
        vertices.push_back(v);

        if(y % 2 == 1)
        {
            indices.push_back((unsigned int)vertices.size());
            indices.push_back((unsigned int)vertices.size() - 4);
            indices.push_back((unsigned int)vertices.size() - 2);
            indices.push_back((unsigned int)vertices.size() + 1);
            indices.push_back((unsigned int)vertices.size() - 1);
            indices.push_back((unsigned int)vertices.size() - 3);
        }
    }

    Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
    if(!pIndexBuffer->Build(&indices[0], (unsigned int)indices.size(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST))
    {
        return 0;
    }
    Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
    if(!pVertexBuffer->Build(&vertices[0], (unsigned int)vertices.size(), sizeof(VertexBuffer::SimpleVertex)))
    {
        return 0;
    }

    std::shared_ptr<Geometry> pGeo(new Geometry);
    pGeo->SetIndices(pIndexBuffer);
    pGeo->SetVertices(pVertexBuffer);
    return pGeo;
}


std::shared_ptr<Geometry> ClipmapTerrain::CreateLShapedGrid(unsigned char p_size, Orientation p_orientation, float p_scale)
{
    // p_size = 4 leads to:
    // O---1---2---3   3---2---1---0 11---10                1O---11
    // | / | / | / |   | / | / | / |  | / |  SOUTH    SOUTH  | / |
    // 4---5---6---7   7---6---5---4  9---8  WEST     EAST   8---9
    //         | / |   | / |          | / |                  | / |
    //  NORTH  8---9   9---8  NORTH   7---6---5---4  4---5---6---7
    //  EAST   | / |   | / |  WEST    | / | / | / |  | / | / | / |
    //        10---11 11---10         3---2---1---0  O---1---2---3

    int indexCount = 2 * p_size + (p_size - 1) * 2 + 1;
    unsigned int* pIndices = new unsigned int[indexCount];
    for(int i=0; i < p_size; ++i)
    {
        pIndices[2 * i + 0] = i;
        pIndices[2 * i + 1] = p_size + i;
        if(i != p_size - 1)
        {
            pIndices[2 * p_size + 1 + 2 * i + 0] = 2 * p_size - 1 + 2 * i;
            pIndices[2 * p_size + 1 + 2 * i + 1] = 2 * p_size - 2 + 2 * i;            
        }
        else
        {
            pIndices[2 * p_size] = 0xffffffff;
        }
    }

    int vertexCount = 2 * p_size + 2 * (p_size - 2);
    VertexBuffer::SimpleVertex* pVertices = new VertexBuffer::SimpleVertex[vertexCount];

    // NORTH_EAST
    for(int i=0; i < p_size; ++i)
    {
        pVertices[i].positionMC =          XMFLOAT3(p_scale * (float)i, 0.0f, p_scale * (float)(p_size - 1));
        pVertices[p_size + i].positionMC = XMFLOAT3(p_scale * (float)i, 0.0f, p_scale * (float)(p_size - 2));
        if(i < p_size - 2)
        {
            pVertices[2 * p_size + 2 * i + 0].positionMC = XMFLOAT3(p_scale * (float)(p_size - 2), 0.0f, p_scale * (float)(p_size - 3 - i));
            pVertices[2 * p_size + 2 * i + 1].positionMC = XMFLOAT3(p_scale * (float)(p_size - 1), 0.0f, p_scale * (float)(p_size - 3 - i));
        }
    }

    for(int i=0; i < vertexCount; ++i)
    {
        pVertices[i].positionMC.x -= p_scale * 0.5f * (float)(p_size - 1);
        pVertices[i].positionMC.z -= p_scale * 0.5f * (float)(p_size - 1);
        pVertices[i].normalMC = XMFLOAT3(0.0f, 1.0f, 0.0f);
        pVertices[i].vertexColor = XMFLOAT4(0.780f, 0.894f, 0.969f, 1.0f);

        // rotating
        XMFLOAT3& pos = pVertices[i].positionMC;
        float tx = pos.x;
        float tz = pos.z;
        switch(p_orientation)
        {
        case NORTH_EAST: /* do nothing */ break;
        case NORTH_WEST: pos.x = -tz; pos.z = tx; break;
        case SOUTH_WEST: pos.x = -tx; pos.z = -tz; break;
        case SOUTH_EAST: pos.x = tz; pos.z = -tx; break;
        }
    }

    Geometry::IndexBufferPtr pIndexBuffer(new IndexBuffer);
    if(!pIndexBuffer->Build(pIndices, indexCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP))
    {
        return 0;
    }
    SAFE_DELETE(pIndices);
    Geometry::VertexBufferPtr pVertexBuffer(new VertexBuffer);
    if(!pVertexBuffer->Build(pVertices, vertexCount, sizeof(VertexBuffer::SimpleVertex)))
    {
        return 0;
    }
    SAFE_DELETE(pVertices);

    std::shared_ptr<Geometry> pGeo(new Geometry);
    pGeo->SetIndices(pIndexBuffer);
    pGeo->SetVertices(pVertexBuffer);
    return pGeo;
}


ClipmapTerrain::ClipmapLevel::ClipmapLevel(void):
m_pData(0)
{
}


ClipmapTerrain::ClipmapLevel::~ClipmapLevel(void)
{
    SAFE_DELETE(m_pData);
}


void ClipmapTerrain::ClipmapLevel::Init(unsigned int p_paramN, unsigned int p_level, ID3D11Texture2D* p_pTexture)
{
    m_paramN = p_paramN;
    m_level = p_level;
    m_pTexture = p_pTexture;
    m_pData = new float[(p_paramN + 1) * (p_paramN + 1 )];
}


void ClipmapTerrain::ClipmapLevel::Update(int p_bottomLeftX, int p_bottomLeftY, std::shared_ptr<DisplacementData> p_pData)
{
    m_globalTranslation.x = (float)p_bottomLeftX;
    m_globalTranslation.y = 0.0f;
    m_globalTranslation.z = (float)p_bottomLeftY;

    for(int y=0; y < m_paramN + 1; ++y)
    {
        for(int x=0; x < m_paramN + 1; ++x)
        {
            int clipmapX = (int)(1 << m_level) * (p_bottomLeftX + x - (m_paramN - 1) / 2);
            int clipmapY = (int)(1 << m_level) * (p_bottomLeftY + y - (m_paramN - 1) / 2);
             if(m_level == 0)
            {
                int a = 0;
            }
            if(m_level == 2)
            {
                int a = 0;
            }
            m_pData[y * (m_paramN + 1) + x] = p_pData->GetHeight(clipmapX, clipmapY);
        }
    }
    D3D11_BOX box;
    box.front = box.left = box.top = 0;
    box.right = box.bottom = m_paramN + 1;
    box.back = 1;
    LostIsland::g_pGraphics->GetContext()->UpdateSubresource(m_pTexture, m_level, &box, m_pData, (m_paramN + 1) * sizeof(float), 0);
};