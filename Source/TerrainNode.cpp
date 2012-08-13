#include "StdAfx.h"
#include "TerrainNode.h"
#include "Scene.h"

#define LOAD_MILLIS 20

TerrainNode::TerrainNode(std::shared_ptr<TerrainData> p_pData):
m_pTerrain(p_pData), m_pDiffuseTex(0), m_pBumpTex(0), m_pNormalTex(0)
{
    MarchingCubeGrid::Init();
    for(int x=0; x < NUM_BLOCKS; ++x)
    {
        for(int y=0; y < NUM_BLOCKS; ++y)
        {
            for(int z=0; z < NUM_BLOCKS; ++z)
            {
                m_pTerrain->CreateTerrainBlock(x - NUM_BLOCKS / 2, y - NUM_BLOCKS / 2, z - NUM_BLOCKS / 2);
            }
        }
    }
}



TerrainNode::~TerrainNode(void)
{
    m_pTerrain.reset((TerrainData*)0);
    SAFE_RELEASE(m_pDiffuseTex);
    SAFE_RELEASE(m_pBumpTex);
    SAFE_RELEASE(m_pNormalTex);
}


HRESULT TerrainNode::VOnRestore(void)
{
    if(!m_program.Load(L"./Shader/Terrain.hlsl", "TerrainVS", 0, "TerrainNormalMappingPS"))
    {
        return S_FALSE;
    }
    if(!m_program.CreateInputLayout(MarchingCubeGrid::sm_pTerrainVertexElementDesc, MarchingCubeGrid::sm_terrainVertexNumElements))
    {
        return S_FALSE;
    }
    D3DX11_IMAGE_LOAD_INFO info;
    info.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    info.CpuAccessFlags = D3DX11_DEFAULT;
    info.Width = D3DX11_DEFAULT;
    info.Height = D3DX11_DEFAULT;
    info.Depth = D3DX11_DEFAULT;
    info.Filter = D3DX11_DEFAULT;
    info.FirstMipLevel = D3DX11_DEFAULT;
    info.Format = DXGI_FORMAT_FROM_FILE;
    info.MipFilter = D3DX11_DEFAULT;
    info.MipLevels = D3DX11_DEFAULT;
    info.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    info.pSrcInfo = 0;       
    RETURN_IF_FAILED(D3DX11CreateShaderResourceViewFromFileA(LostIsland::g_pGraphics->GetDevice(), "../Assets/polishedstone_diffuse.jpg", &info, 0, &m_pDiffuseTex, 0));
    RETURN_IF_FAILED(D3DX11CreateShaderResourceViewFromFileA(LostIsland::g_pGraphics->GetDevice(), "../Assets/polishedstone_bump.jpg", &info, 0, &m_pBumpTex, 0));
    RETURN_IF_FAILED(D3DX11CreateShaderResourceViewFromFileA(LostIsland::g_pGraphics->GetDevice(), "../Assets/polishedstone_normal.jpg", &info, 0, &m_pNormalTex, 0));
    LostIsland::g_pGraphics->GetContext()->GenerateMips(m_pDiffuseTex);
    LostIsland::g_pGraphics->GetContext()->GenerateMips(m_pBumpTex);
    LostIsland::g_pGraphics->GetContext()->GenerateMips(m_pNormalTex);

    return S_OK;
}


HRESULT TerrainNode::VOnLostDevice(void)
{
    return S_OK;
}


HRESULT TerrainNode::VOnUpdate(Scene* p_pScene, unsigned long p_deltaMillis)
{
    //const XMFLOAT3& campos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    const XMFLOAT3& campos = p_pScene->GetCurrentCamera()->GetPosition();
    m_pTerrain->SetPointOfReference(campos.x, campos.y, campos.z);
    m_pTerrain->Update(LOAD_MILLIS);
    return S_OK;
}


HRESULT TerrainNode::VPreRender(Scene* p_pScene)
{
    LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(0, 1, &m_pDiffuseTex);
    LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(1, 1, &m_pBumpTex);
    LostIsland::g_pGraphics->GetContext()->PSSetShaderResources(2, 1, &m_pNormalTex);
    return S_OK;
}


HRESULT TerrainNode::VRender(Scene* p_pScene)
{
    m_program.Bind();
    Pose::ModelMatrixData data;
    XMStoreFloat4x4(&data.model, XMMatrixIdentity());
    XMStoreFloat4x4(&data.modelInv, XMMatrixIdentity());
    p_pScene->PushModelMatrices(data, true);

    auto iter = m_pTerrain->GetGeometryList().begin();
    while(iter != m_pTerrain->GetGeometryList().end()) {
        std::shared_ptr<Geometry> pGeo = (*iter).lock();
        if(pGeo)
        {
            pGeo->Draw();
            ++iter;
        }
        else
        {
            iter = m_pTerrain->GetGeometryList().erase(iter);
        }
    }
    static int frames = 0;
    if(++frames == 1000)
    {
        frames = 0;
        std::ostringstream info;
        info << m_pTerrain->GetGeometryList().size() << " blocks";
        LI_INFO(info.str());
    }

    p_pScene->PopModelMatrices(false);
    return S_OK;
}


HRESULT TerrainNode::VPostRender(Scene* p_pScene)
{
    return S_OK;
}
