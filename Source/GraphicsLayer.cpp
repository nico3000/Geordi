#include "StdAfx.h"
#include "GraphicsLayer.h"
#include "RenderTarget.h"
#include <FW1FontWrapper.h>
#pragma comment(lib, "FW1FontWrapper.lib")

#define LI_LOGGER_TAG "Direct3D"


GraphicsLayer::GraphicsLayer(void):
m_pContext(0), m_pDevice(0), m_pSwapChain(0), m_pDebug(0), m_pOutput(0), m_hWnd(0), m_initialized(false), m_onShutdown(false), m_vsync(false), m_pBackbuffer(new RenderTarget)
{
    m_pDefaultBlendState = 0;
    m_pDefaultDepthStencilState = 0;
    m_pDefaultRasterizerState = 0;
    m_pFontWrapper = 0;
}


GraphicsLayer::~GraphicsLayer(void)
{
    m_onShutdown = true;
    if(m_initialized)
    {
        this->SetFullscreen(false);
    }
    m_initialized = false;
    for each(auto blobPair in m_vertexShaderBlobs)
    {
        SAFE_RELEASE(blobPair.second);
    }
    for each(auto shaderPair in m_vertexShaders)
    {
        SAFE_RELEASE(shaderPair.second);
    }
    for each(auto shaderPair in m_geometryShaders)
    {
        SAFE_RELEASE(shaderPair.second);
    }
    for each(auto shaderPair in m_pixelShaders)
    {
        SAFE_RELEASE(shaderPair.second);
    }
    for each(auto shaderPair in m_computeShaders)
    {
        SAFE_RELEASE(shaderPair.second);
    }

    if(m_ppDefaultSamplers)
    {
        for(unsigned int i=0; i < m_samplerCount; ++i)
        {
            SAFE_RELEASE(m_ppDefaultSamplers[i]);
        }
        SAFE_DELETE(m_ppDefaultSamplers);
    }
    SAFE_RELEASE(m_pDefaultBlendState);
    SAFE_RELEASE(m_pDefaultRasterizerState);
    SAFE_RELEASE(m_pDefaultDepthStencilState);
    
    if(m_pSwapChain)
    {
        m_pSwapChain->SetFullscreenState(FALSE, 0);
        SAFE_RELEASE(m_pSwapChain);
    }
    SAFE_RELEASE(m_pFontWrapper);
    SAFE_RELEASE(m_pOutput);
    SAFE_RELEASE(m_pDebug);
    SAFE_RELEASE(m_pContext);
    SAFE_RELEASE(m_pDevice);
    
    
    if(m_hWnd)
    {
        DestroyWindow(m_hWnd);
    }
}



bool GraphicsLayer::InitTest(void)
{
    //ID3D11ShaderResourceView* pTextureSRV;
    //HRESULT hr = D3DX11CreateShaderResourceViewFromFileA(m_pDevice, "./Texture/nicotopia.png", 0, 0, &pTextureSRV, 0);
    //RETURN_IF_FAILED(hr);

    //m_pContext->PSSetShaderResources(1, 1, &pTextureSRV);
    
    
    return true;
}


bool GraphicsLayer::Init(HINSTANCE hInstance)
{
    m_width = LostIsland::g_pApp->GetConfig()->GetIntAttribute("graphics", "display", "width");
    m_height = LostIsland::g_pApp->GetConfig()->GetIntAttribute("graphics", "display", "height");
    m_vsync = LostIsland::g_pApp->GetConfig()->GetBoolAttribute("graphics", "display", "vsync");
    if(!this->CreateAppWindow(hInstance))
    {
        return false;
    }    
    UpdateWindow(m_hWnd);
    ShowWindow(m_hWnd, SW_SHOW);
    if(!this->CreateAppGraphics())
    {
        return false;
    }
    if(!this->InitDefaultStates())
    {
        return false;
    }
    this->SetDefaultStates();

    return InitTest();

    //return true;
}


void GraphicsLayer::Clear(void)
{
    static const float pClearColor[] = { 0.0f, 0.2f, 0.2f, 1.0f };
    m_pBackbuffer->ClearColor(pClearColor);
    m_pBackbuffer->ClearDepthStencil(1.0f, 0);
}

void GraphicsLayer::Present(void)
{
    static bool occluded = false;
    HRESULT hr = m_pSwapChain->Present(m_vsync, occluded ? DXGI_PRESENT_TEST : 0);
    occluded = hr == DXGI_STATUS_OCCLUDED;
    this->SetDefaultStates();
}


void GraphicsLayer::ReleaseRenderTarget(void) const
{
    static ID3D11RenderTargetView* pNull = 0;
    m_pContext->OMSetRenderTargets(1, &pNull, 0);
}


void GraphicsLayer::ReleaseUnorderedAccess(unsigned int p_startSlot, unsigned int p_count) const
{
    ID3D11UnorderedAccessView** ppNull = new ID3D11UnorderedAccessView*[p_count];
    ZeroMemory(ppNull, p_count * sizeof(ID3D11UnorderedAccessView*));
    unsigned int* pCounts = new unsigned int[p_count];
    ZeroMemory(pCounts, p_count * sizeof(unsigned int));
    m_pContext->CSSetUnorderedAccessViews(p_startSlot, p_count, ppNull, pCounts);
    SAFE_DELETE(ppNull);
    SAFE_DELETE(pCounts);
}


void GraphicsLayer::ReleaseShaderResources(unsigned int p_startSlot, unsigned int p_count, ShaderTarget p_target /* = TARGET_ALL */) const
{
    ID3D11ShaderResourceView** ppNull = new ID3D11ShaderResourceView*[p_count];
    ZeroMemory(ppNull, p_count * sizeof(ID3D11ShaderResourceView*));
    if(p_target & TARGET_VS)
    {
        m_pContext->VSSetShaderResources(p_startSlot, p_count, ppNull);
    }
    if(p_target & TARGET_GS)
    {
        m_pContext->GSSetShaderResources(p_startSlot, p_count, ppNull);
    }
    if(p_target & TARGET_PS)
    {
        m_pContext->PSSetShaderResources(p_startSlot, p_count, ppNull);
    }
    if(p_target & TARGET_CS)
    {
        m_pContext->CSSetShaderResources(p_startSlot, p_count, ppNull);
    }
    SAFE_DELETE(ppNull);
}


bool GraphicsLayer::CreateAppGraphics(void)
{
    m_vertexShaderProfiles[SHADER_VERSION_3_0] = "vs_3_0";
    m_vertexShaderProfiles[SHADER_VERSION_4_0] = "vs_4_0";
    m_vertexShaderProfiles[SHADER_VERSION_5_0] = "vs_5_0";
    m_geometryShaderProfiles[SHADER_VERSION_4_0] = "gs_4_0";
    m_geometryShaderProfiles[SHADER_VERSION_5_0] = "gs_5_0";
    m_pixelShaderProfiles[SHADER_VERSION_3_0] = "ps_3_0";
    m_pixelShaderProfiles[SHADER_VERSION_4_0] = "ps_4_0";
    m_pixelShaderProfiles[SHADER_VERSION_5_0] = "ps_5_0";
    m_computeShaderProfiles[SHADER_VERSION_4_0] = "vs_4_0";
    m_computeShaderProfiles[SHADER_VERSION_5_0] = "cs_5_0";

    unsigned int flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL pFeatureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    IDXGIFactory1* pFactory;
    RETURN_IF_FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory));
    RETURN_IF_FAILED(pFactory->MakeWindowAssociation(m_hWnd, 0));

    IDXGIAdapter1* pAdapter;
    for(int i=0; 1; ++i)
    {
        if(SUCCEEDED(pFactory->EnumAdapters1(i, &pAdapter)))
        {
            this->PrintAdapterString(pAdapter);
        }
        else
        {
            break;
        }
    }

    RETURN_IF_FAILED(pFactory->EnumAdapters1(0, &pAdapter));    // TODO: use default adapter
    RETURN_IF_FAILED(D3D11CreateDevice(pAdapter,
                                       D3D_DRIVER_TYPE_UNKNOWN,
                                       0,
                                       flags,
                                       pFeatureLevels,
                                       ARRAYSIZE(pFeatureLevels),
                                       D3D11_SDK_VERSION,
                                       &m_pDevice,
                                       &m_featureLevel,
                                       &m_pContext));
#ifdef _DEBUG
    RETURN_IF_FAILED(m_pDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_pDebug));
#endif
    this->PrepareFeatureLevel();

    DXGI_SWAP_CHAIN_DESC scDesc;
    scDesc.BufferCount = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT| DXGI_USAGE_UNORDERED_ACCESS;
    scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    scDesc.OutputWindow = m_hWnd;
    scDesc.SampleDesc.Count = 1;
    scDesc.SampleDesc.Quality = 0;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scDesc.Windowed = LostIsland::g_pApp->GetConfig()->GetBoolAttribute("graphics", "display", "windowed");

    DXGI_MODE_DESC desiredMode;
    ZeroMemory(&desiredMode, sizeof(DXGI_MODE_DESC));
    desiredMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desiredMode.Width = m_width;
    desiredMode.Height = m_height;
    desiredMode.RefreshRate.Numerator = LostIsland::g_pApp->GetConfig()->GetIntAttribute("graphics", "display", "refresh");
    desiredMode.RefreshRate.Denominator = 1;

    RETURN_IF_FAILED(pAdapter->EnumOutputs(0, &m_pOutput));
    RETURN_IF_FAILED(m_pOutput->FindClosestMatchingMode(&desiredMode, &scDesc.BufferDesc, m_pDevice));

    std::ostringstream str;
    str << "Using DXGI mode " << scDesc.BufferDesc.Width << "x" << scDesc.BufferDesc.Height << "@" << (scDesc.BufferDesc.RefreshRate.Numerator / scDesc.BufferDesc.RefreshRate.Denominator) << "Hz";
    LI_LOG_WITH_TAG(str.str());

//     unsigned int numModes;
//     RETURN_IF_FAILED(m_pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH, &numModes, 0));
//     DXGI_MODE_DESC *pDesc = new DXGI_MODE_DESC[numModes];
//     RETURN_IF_FAILED(m_pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH, &numModes, pDesc));
//     for(unsigned int i=0; i < numModes; ++i)
//     {
//         std::ostringstream str;
//         str << pDesc[i].Width << "x" << pDesc[i].Height << " {" << pDesc[i].RefreshRate.Numerator << ", " << pDesc[i].RefreshRate.Denominator << "}";
//         LI_INFO(str.str());
//     }
//     SAFE_DELETE(pDesc);
    
    RETURN_IF_FAILED(pFactory->CreateSwapChain(m_pDevice,
                                               &scDesc,
                                               &m_pSwapChain));
    //m_pSwapChain->GetContainingOutput(&m_pOutput);
    //m_pSwapChain->

    //m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

    IFW1Factory* pFontFactory;
    RETURN_IF_FAILED(FW1CreateFactory(FW1_VERSION, &pFontFactory));
    RETURN_IF_FAILED(pFontFactory->CreateFontWrapper(m_pDevice, L"Arial", &m_pFontWrapper));

    m_initialized = true;

    SAFE_RELEASE(pAdapter);
    SAFE_RELEASE(pFactory);

    this->ReleaseBackbuffer();
    return this->LoadBackbuffer();
}


void GraphicsLayer::PrepareFeatureLevel(void)
{
    switch(m_featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_0:
        m_vertexShaderProfiles[SHADER_VERSION_MAX] = m_vertexShaderProfiles[SHADER_VERSION_5_0];
        m_geometryShaderProfiles[SHADER_VERSION_MAX] = m_geometryShaderProfiles[SHADER_VERSION_5_0];
        m_pixelShaderProfiles[SHADER_VERSION_MAX] = m_pixelShaderProfiles[SHADER_VERSION_5_0];
        m_computeShaderProfiles[SHADER_VERSION_MAX] = m_computeShaderProfiles[SHADER_VERSION_5_0];
        LI_LOG_WITH_TAG("Your GPU supports Direct3D11.");
        break;
    case D3D_FEATURE_LEVEL_10_1:
        break;
    case D3D_FEATURE_LEVEL_10_0:
        m_vertexShaderProfiles[SHADER_VERSION_MAX] = m_vertexShaderProfiles[SHADER_VERSION_4_0];
        m_geometryShaderProfiles[SHADER_VERSION_MAX] = m_geometryShaderProfiles[SHADER_VERSION_4_0];
        m_pixelShaderProfiles[SHADER_VERSION_MAX] = m_pixelShaderProfiles[SHADER_VERSION_4_0];
        m_computeShaderProfiles[SHADER_VERSION_MAX] = m_computeShaderProfiles[SHADER_VERSION_4_0];
        LI_LOG_WITH_TAG("Your GPU does not support Direct3D 11. Falling back to Direct3D10.");
        break;
    case D3D_FEATURE_LEVEL_9_3:
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
        m_vertexShaderProfiles[SHADER_VERSION_MAX] = m_vertexShaderProfiles[SHADER_VERSION_3_0];
        m_pixelShaderProfiles[SHADER_VERSION_MAX] = m_pixelShaderProfiles[SHADER_VERSION_3_0];
        LI_LOG_WITH_TAG("Poor bastard! Your GPU does not support Direct3D 11. Not even Direct3D 10. Falling back to Direct3D9.");
        break;
    }
}


bool GraphicsLayer::PrintAdapterString(IDXGIAdapter1* p_pAdapter) const
{
    HRESULT hr = S_OK;
    DXGI_ADAPTER_DESC1 desc;
    hr = p_pAdapter->GetDesc1(&desc);
    RETURN_IF_FAILED(hr);

    std::ostringstream adapterStr;
    adapterStr << "GPU found: ";
    for(const wchar_t* c = desc.Description; *c != 0; ++c)
    {
        adapterStr << (char)*c;
    }
    adapterStr << ", Dedicated Video Memory: " << (desc.DedicatedVideoMemory / (1024*1024)) << "MB";
    adapterStr << ", Dedicated System Memory: " << (desc.DedicatedSystemMemory / (1024*1024)) << "MB";
    do 
    {
        static unsigned int i = 0;
        static IDXGIOutput* pOutput;
        hr = p_pAdapter->EnumOutputs(i++, &pOutput);
        if(SUCCEEDED(hr))
        {
            adapterStr << ", " << this->GetAdapterOutputString(pOutput);
        }
    } while (SUCCEEDED(hr));
    
    LI_LOG_WITH_TAG(adapterStr.str().c_str());

    return true;
}


std::string GraphicsLayer::GetAdapterOutputString(IDXGIOutput* p_pOutput) const
{
    HRESULT hr = S_OK;
    DXGI_OUTPUT_DESC desc;
    hr = p_pOutput->GetDesc(&desc);
    RETURN_IF_FAILED(hr);

    std::ostringstream outputStr;
    outputStr << "output: ";
    for(const wchar_t* c = desc.DeviceName; *c != 0; ++c)
    {
        outputStr << (char)*c;
    }

    return outputStr.str();
}


bool GraphicsLayer::CreateAppWindow(HINSTANCE hInstance)
{
    static LPCWSTR pWindowClass = L"LostIslandWindow";

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = LostIsland::WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = 0;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = 0; 
    wcex.lpszClassName  = pWindowClass;
    wcex.hIconSm        = 0;

    RegisterClassEx(&wcex);

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = m_width;
    rect.bottom = m_height;
    
    if(!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false))
    {
        LI_ERROR("AdjustWindowRect() failed");
        return false;
    }

    m_hWnd = CreateWindow(pWindowClass,
                          L"Lost Island",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          0,
                          rect.right - rect.left,
                          rect.bottom - rect.top,
                          0,
                          0,
                          hInstance,
                          0);

    if(m_hWnd == 0)
    {
        return false;
    }

    return true;
}


bool GraphicsLayer::OnWindowResized(int p_width, int p_height)
{
    HRESULT hr = S_OK;
    if(m_initialized)
    {
        std::ostringstream str;
        str << p_width << "x" << p_height;
        LI_LOG_WITH_TAG("New size " + str.str());

        this->ReleaseBackbuffer();
        
        hr = m_pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        RETURN_IF_FAILED(hr);

        m_width = p_width;
        m_height = p_height;

        if(!this->LoadBackbuffer())
        {
            return false;
        }
    }
    return true;
}


bool GraphicsLayer::ToggleFullscreen(void)
{
    return this->SetFullscreen(!this->IsFullscreen());
}


bool GraphicsLayer::SetFullscreen(bool p_fullscreen)
{
    HRESULT hr = S_OK;
    bool wasFullscreen = this->IsFullscreen();
    if(wasFullscreen != p_fullscreen)
    {
        hr = m_pSwapChain->SetFullscreenState(p_fullscreen, p_fullscreen ? m_pOutput : 0);
        RETURN_IF_FAILED(hr);
    }
    return true;
}


void GraphicsLayer::ReleaseBackbuffer(void)
{
    m_pBackbuffer->Destroy();
}


bool GraphicsLayer::LoadBackbuffer(void)
{
    HRESULT hr = S_OK;

    ID3D11Texture2D* pTexture;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pTexture);
    RETURN_IF_FAILED(hr);
    
    if(!m_pBackbuffer->Init2D(pTexture, RenderTarget::RTV_DSV_SRV_UAV))
    {
        return false;
    }
    SAFE_RELEASE(pTexture);

    m_pBackbuffer->BindAllRenderTargets();

    return true;
}


bool GraphicsLayer::IsFullscreen(void) const
{
    if(!m_initialized)
    {
        return false;
    }
    int fullscreen;
    m_pSwapChain->GetFullscreenState(&fullscreen, 0);
    return fullscreen != 0;
}


ID3D11VertexShader* GraphicsLayer::CompileVertexShader(LPCWSTR p_file, LPCSTR p_function, ID3D10Blob*& p_pShaderData, const D3D10_SHADER_MACRO* p_pDefines /* = 0 */, ShaderVersion p_version /* = SHADER_VERSION_MAX */)
{
    std::wstring id = p_file + std::wstring(L"|") + std::wstring((LPCTSTR)p_function);
    if(m_vertexShaders.find(id) == m_vertexShaders.end())
    {
        if(!this->CompileShader(p_file, p_function, m_vertexShaderProfiles[p_version], p_pShaderData, p_pDefines))
        {
            return 0;
        }
        m_vertexShaderBlobs[id] = p_pShaderData;
        RETURN_IF_FAILED(m_pDevice->CreateVertexShader(p_pShaderData->GetBufferPointer(), p_pShaderData->GetBufferSize(), 0, &m_vertexShaders[id]));
    }
    m_vertexShaderBlobs[id]->AddRef();
    m_vertexShaders[id]->AddRef();
    p_pShaderData = m_vertexShaderBlobs[id];
    return m_vertexShaders[id];
}


ID3D11GeometryShader* GraphicsLayer::CompileGeometryShader(LPCWSTR p_file, LPCSTR p_function, const D3D10_SHADER_MACRO* p_pDefines /* = 0 */, ShaderVersion p_version /* = SHADER_VERSION_MAX */)
{
    std::wstring id = p_file + std::wstring(L"|") + std::wstring((LPCTSTR)p_function);
    if(m_geometryShaders.find(id) == m_geometryShaders.end())
    {
        ID3D10Blob* pShaderData = 0;
        if(!this->CompileShader(p_file, p_function, m_geometryShaderProfiles[p_version], pShaderData, p_pDefines))
        {
            return 0;
        }
        HRESULT hr = m_pDevice->CreateGeometryShader(pShaderData->GetBufferPointer(), pShaderData->GetBufferSize(), 0, &m_geometryShaders[id]);
        SAFE_RELEASE(pShaderData);
        RETURN_IF_FAILED(hr);
    }
    m_geometryShaders[id]->AddRef();
    return m_geometryShaders[id];
}


ID3D11PixelShader* GraphicsLayer::CompilePixelShader(LPCWSTR p_file, LPCSTR p_function, const D3D10_SHADER_MACRO* p_pDefines /* = 0 */, ShaderVersion p_version /* = SHADER_VERSION_MAX */)
{
    std::wstring id = p_file + std::wstring(L"|") + std::wstring((LPCTSTR)p_function);
    if(m_pixelShaders.find(id) == m_pixelShaders.end())
    {
        ID3D10Blob* pShaderData = 0;
        if(!this->CompileShader(p_file, p_function, m_pixelShaderProfiles[p_version], pShaderData, p_pDefines))
        {
            return 0;
        }
        HRESULT hr = m_pDevice->CreatePixelShader(pShaderData->GetBufferPointer(), pShaderData->GetBufferSize(), 0, &m_pixelShaders[id]);
        SAFE_RELEASE(pShaderData);
        RETURN_IF_FAILED(hr);
    }
    m_pixelShaders[id]->AddRef();
    return m_pixelShaders[id];
}


ID3D11ComputeShader* GraphicsLayer::CompileComputeShader(LPCWSTR p_file, LPCSTR p_function, const D3D10_SHADER_MACRO* p_pDefines /* = 0 */, ShaderVersion p_version /* = SHADER_VERSION_MAX */)
{
    std::wstring id = p_file + std::wstring(L"|") + std::wstring((LPCTSTR)p_function);
    if(m_computeShaders.find(id) == m_computeShaders.end())
    {
        ID3D10Blob* pShaderData = 0;
        if(!this->CompileShader(p_file, p_function, m_computeShaderProfiles[p_version], pShaderData, p_pDefines))
        {
            return 0;
        }
        HRESULT hr = m_pDevice->CreateComputeShader(pShaderData->GetBufferPointer(), pShaderData->GetBufferSize(), 0, &m_computeShaders[id]);
        SAFE_RELEASE(pShaderData);
        RETURN_IF_FAILED(hr);
    }
    m_computeShaders[id]->AddRef();
    return m_computeShaders[id];
}


bool GraphicsLayer::CompileShader(LPCWSTR p_file, LPCSTR p_function, LPCSTR p_pProfile, ID3D10Blob*& p_pShaderBlob, const D3D10_SHADER_MACRO* p_pDefines) const
{
    ID3D10Blob* pErrorBlob = 0;
    HRESULT hr = D3DX11CompileFromFileW(p_file, p_pDefines, 0, p_function, p_pProfile, 0, 0, 0, &p_pShaderBlob, &pErrorBlob, 0);
    if(pErrorBlob)
    {
        std::string errorMsg((char*)pErrorBlob->GetBufferPointer(), (char*)pErrorBlob->GetBufferPointer() + pErrorBlob->GetBufferSize());
        SAFE_RELEASE(pErrorBlob);
        if(FAILED(hr))
        {
            LI_ERROR(errorMsg);
            return false;
        }
        else
        {
            LI_WARNING(errorMsg);
        }
    }
    else
    {
        RETURN_IF_FAILED(hr);
    }
    return true;
}


bool GraphicsLayer::InitDefaultStates(void)
{
    m_samplerCount = 3;
    D3D11_SAMPLER_DESC pDesc[3];
    ZeroMemory(pDesc, 3 * sizeof(D3D11_SAMPLER_DESC));
    pDesc[0].AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc[0].AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc[0].AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc[0].Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

    pDesc[1].AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc[1].AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc[1].AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc[1].Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    pDesc[2].AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc[2].AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc[2].AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    pDesc[2].Filter = D3D11_FILTER_ANISOTROPIC;
    pDesc[2].MaxAnisotropy = 16;

    m_ppDefaultSamplers = new ID3D11SamplerState*[3];
    for(unsigned int i=0; i < 3; ++i)
    {
        RETURN_IF_FAILED(m_pDevice->CreateSamplerState(&pDesc[i], &m_ppDefaultSamplers[i]));
    }


    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.FrontCounterClockwise = true;
    RETURN_IF_FAILED(m_pDevice->CreateRasterizerState(&rasterizerDesc, &m_pDefaultRasterizerState));


    D3D11_DEPTH_STENCIL_DESC dsDesc;
    ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dsDesc.DepthEnable = true;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    RETURN_IF_FAILED(m_pDevice->CreateDepthStencilState(&dsDesc, &m_pDefaultDepthStencilState));


    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[2].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[3].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[4].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[5].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[6].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[7].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    RETURN_IF_FAILED(m_pDevice->CreateBlendState(&blendDesc, &m_pDefaultBlendState));


    return true;
}


void GraphicsLayer::SetDefaultStates(void)
{
    static float pBlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_pContext->OMSetDepthStencilState(m_pDefaultDepthStencilState, 0xffffffff);
    m_pContext->RSSetState(m_pDefaultRasterizerState);
    m_pContext->OMSetBlendState(m_pDefaultBlendState, pBlendFactor, 0xffffffff);
    
    m_pContext->VSSetSamplers(0, m_samplerCount, m_ppDefaultSamplers);
    m_pContext->PSSetSamplers(0, m_samplerCount, m_ppDefaultSamplers);
    if(m_featureLevel >= D3D_FEATURE_LEVEL_10_0)
    {
        m_pContext->GSSetSamplers(0, m_samplerCount, m_ppDefaultSamplers);
    }
    if(m_featureLevel >= D3D_FEATURE_LEVEL_11_0)
    {
        m_pContext->DSSetSamplers(0, m_samplerCount, m_ppDefaultSamplers);
        m_pContext->HSSetSamplers(0, m_samplerCount, m_ppDefaultSamplers);
    }
}



void GraphicsLayer::DrawText1(std::wstring p_text) const
{    
    m_pFontWrapper->DrawString(m_pContext, p_text.c_str(), 128.0f, 50.0f, 100.0f, 0xff0099ff, 0);
}