#include "StdAfx.h"
#include "GraphicsLayer.h"

#include "TerrainData.h"
#include "ShaderProgram.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Camera.h"


#define CHECK_ERROR_BLOB(_hr, _blob) do \
{ \
    if(FAILED(_hr)) \
    { \
        if(_blob != 0) \
        { \
            std::string error((char*)(_blob)->GetBufferPointer(), (char*)(_blob)->GetBufferPointer() + (_blob)->GetBufferSize()); \
            LI_ERROR(error); \
            SAFE_RELEASE(_blob); \
            return false; \
        } \
        else \
        { \
            RETURN_IF_FAILED(_hr); \
        } \
    } \
    else \
    { \
        SAFE_RELEASE(_blob); \
    } \
} while(0)

#define LI_LOGGER_TAG "Direct3D"


GraphicsLayer::GraphicsLayer(void):
m_pContext(0), m_pDevice(0), m_pSwapChain(0), m_pBackBuffer(0), m_pOutput(0), m_hWnd(0), m_initialized(false), m_vsync(0)
{
}


GraphicsLayer::~GraphicsLayer(void)
{
    if(m_initialized)
    {
        this->SetFullscreen(false);
    }
    m_initialized = false;
    for each(std::pair<std::string, ID3D11VertexShader*> shaderPair in m_vertexShaders)
    {
        SAFE_RELEASE(shaderPair.second);
    }
    for each(std::pair<std::string, ID3D11GeometryShader*> shaderPair in m_geometryShaders)
    {
        SAFE_RELEASE(shaderPair.second);
    }
    for each(std::pair<std::string, ID3D11PixelShader*> shaderPair in m_pixelShaders)
    {
        SAFE_RELEASE(shaderPair.second);
    }
    SAFE_RELEASE(m_pBackBuffer);
    SAFE_RELEASE(m_pContext);
    SAFE_RELEASE(m_pDevice);
    SAFE_RELEASE(m_pSwapChain);
    SAFE_RELEASE(m_pOutput);
    if(m_hWnd)
    {
        DestroyWindow(m_hWnd);
    }
}

struct ModelProperties
{
    XMFLOAT4X4 model;
    XMFLOAT4X4 modelIT;
};

ShaderProgram g_program;
VertexBuffer g_vertices;
IndexBuffer g_indices;
Camera g_cam;
ConstantBuffer g_model;
ModelProperties g_properties;


bool GraphicsLayer::Init(HINSTANCE hInstance)
{
    //TerrainData terrain;
    //terrain.Init(32, 8, 4, 8, 8);
    //terrain.Test();
    m_width = LostIsland::g_pConfig->GetIntAttribute("graphics", "display", "width");
    m_height = LostIsland::g_pConfig->GetIntAttribute("graphics", "display", "height");
    m_vsync = LostIsland::g_pConfig->GetBoolAttribute("graphics", "display", "vsync");
    if(!this->CreateAppWindow(hInstance))
    {
        return false;
    }
    if(!this->CreateAppGraphics())
    {
        return false;
    }    
    UpdateWindow(m_hWnd);
    ShowWindow(m_hWnd, SW_SHOW);

    // testing
    if(!g_program.Load("shader/NicotopiaTest.fx", "SimpleVS", 0, "SimplePS"))
    {
        return false;
    }
    if(!g_program.CreateInputLayout(VertexBuffer::sm_pSimpleVertexElementDesc, VertexBuffer::sm_simpleVertexNumElements))
    {
        return false;
    }

    VertexBuffer::SimpleVertex vertices[] = {
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(+0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(+0.5f, +0.5f, -0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-0.5f, +0.5f, -0.5f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-0.5f, -0.5f, +0.5f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(+0.5f, -0.5f, +0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-0.5f, +0.5f, +0.5f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
    };
    if(!g_vertices.Build(vertices, ARRAYSIZE(vertices), sizeof(VertexBuffer::SimpleVertex)))
    {
        return false;
    }

    unsigned int indices[] = {
        0, 1, 3, 2, 7, 6, 4, 5, 0, 1, 0xFFFFFFFF,
        1, 5, 2, 6, 0xFFFFFFFF,
        3, 7, 0, 4,
    };
    if(!g_indices.Build(indices, ARRAYSIZE(indices), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP))
    {
        return false;
    }

    if(!g_cam.Init())
    {
        return false;
    }
    g_cam.Bind();
    g_cam.Move(-2.0f, 0.0f, 0.0f);

    if(!g_model.Build(&g_properties, sizeof(ModelProperties)))
    {
        return false;
    }
    g_model.Bind(1, ConstantBuffer::TARGET_ALL);

    return true;
}


void GraphicsLayer::Clear(void)
{
    static const float pClearColor[] = { 0.0f, 0.2f, 0.2f, 1.0f };
    m_pContext->ClearRenderTargetView(m_pBackBuffer, pClearColor);
}


void GraphicsLayer::Present(void)
{
    static int id = LostIsland::g_pTimer->Tick(REALTIME);
    long elapsed = LostIsland::g_pTimer->Tock(id, KEEPRUNNING);
    //g_cam.SetPosition(sin((float)elapsed * 1e-2f), 0.0f, -3.0f + sin((float)elapsed * 0.5e-2f));
    XMStoreFloat4x4(&g_properties.model, XMMatrixRotationY(4e-3f * (float)elapsed));

    g_program.Bind();
    g_vertices.Bind();
    g_indices.Bind();
    g_cam.Update();
    g_model.Update();
    m_pContext->DrawIndexed(g_indices.GetIndexCount(), 0, 0);

    m_pSwapChain->Present(m_vsync, 0);
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

    UINT flags = 0;
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
    this->PrepareFeatureLevel();

    DXGI_SWAP_CHAIN_DESC scDesc;
    scDesc.BufferCount = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    scDesc.OutputWindow = m_hWnd;
    scDesc.SampleDesc.Count = 1;
    scDesc.SampleDesc.Quality = 0;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scDesc.Windowed = LostIsland::g_pConfig->GetBoolAttribute("graphics", "display", "windowed");

    DXGI_MODE_DESC desiredMode;
    ZeroMemory(&desiredMode, sizeof(DXGI_MODE_DESC));
    desiredMode.Width = m_width;
    desiredMode.Height = m_height;
    desiredMode.RefreshRate.Numerator = LostIsland::g_pConfig->GetIntAttribute("graphics", "display", "refresh");
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

    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.FrontCounterClockwise = true;
    ID3D11RasterizerState* pRasterizerState;
    HRESULT hr = m_pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState);
    if(FAILED(hr))
    {
        HRESULT_TO_WARNING(hr);
    }
    else
    {
        m_pContext->RSSetState(pRasterizerState);
        SAFE_RELEASE(pRasterizerState);   
    }

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
        LI_LOG_WITH_TAG("Your GPU supports Direct3D11.");
        break;
    case D3D_FEATURE_LEVEL_10_1:
        break;
    case D3D_FEATURE_LEVEL_10_0:
        m_vertexShaderProfiles[SHADER_VERSION_MAX] = m_vertexShaderProfiles[SHADER_VERSION_4_0];
        m_geometryShaderProfiles[SHADER_VERSION_MAX] = m_geometryShaderProfiles[SHADER_VERSION_4_0];
        m_pixelShaderProfiles[SHADER_VERSION_MAX] = m_pixelShaderProfiles[SHADER_VERSION_4_0];
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
    
    if(AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false))
    {
        m_width = rect.right - rect.left;
        m_height = rect.bottom - rect.top;
    }

    m_hWnd = CreateWindow(pWindowClass,
                          L"Lost Island",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          0,
                          m_width,
                          m_height,
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
    //return true;
    HRESULT hr = S_OK;
    if(m_initialized)
    {
        std::ostringstream str;
        str << p_width << "x" << p_height;
        LI_LOG_WITH_TAG("New size " + str.str());

        this->ReleaseBackbuffer();
        
        hr = m_pSwapChain->ResizeBuffers(0, p_width, p_height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        RETURN_IF_FAILED(hr);

        if(!this->LoadBackbuffer())
        {
            return false;
        }

        m_width = p_width;
        m_height = p_height;
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
        hr = m_pSwapChain->SetFullscreenState(p_fullscreen, 0);
        RETURN_IF_FAILED(hr);

        if(wasFullscreen)
        {
            ShowWindow(m_hWnd, SW_SHOW);
        }
    }
    return true;
}


void GraphicsLayer::ReleaseBackbuffer(void)
{
    SAFE_RELEASE(m_pBackBuffer);
}


bool GraphicsLayer::LoadBackbuffer(void)
{
    HRESULT hr = S_OK;

    ID3D11Texture2D* pTexture;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pTexture);
    RETURN_IF_FAILED(hr);

    hr = m_pDevice->CreateRenderTargetView(pTexture, NULL, &m_pBackBuffer);
    SAFE_RELEASE(pTexture);
    RETURN_IF_FAILED(hr);

    m_pContext->OMSetRenderTargets(1, &m_pBackBuffer, 0);

    D3D11_VIEWPORT viewport;
    viewport.Width = (float)m_width;
    viewport.Height = (float)m_height;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_pContext->RSSetViewports(1, &viewport);

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


ID3D11VertexShader* GraphicsLayer::CompileVertexShader(LPCSTR p_file, LPCSTR p_function, ID3D10Blob*& p_pShaderData, ShaderVersion p_version /* = SHADER_VERSION_MAX */)
{
    std::string id = p_file + std::string("|") + p_function;
    if(m_vertexShaders.find(id) == m_vertexShaders.end())
    {
        HRESULT hr = S_OK;
        ID3D10Blob* pErrorData = 0;
        hr = D3DX11CompileFromFileA(p_file, 0, 0, p_function, m_vertexShaderProfiles[p_version], 0, 0, 0, &p_pShaderData, &pErrorData, 0);
        CHECK_ERROR_BLOB(hr, pErrorData);
        hr = m_pDevice->CreateVertexShader(p_pShaderData->GetBufferPointer(), p_pShaderData->GetBufferSize(), 0, &m_vertexShaders[id]);
        RETURN_IF_FAILED(hr);
    }
    return m_vertexShaders[id];
}


ID3D11GeometryShader* GraphicsLayer::CompileGeometryShader(LPCSTR p_file, LPCSTR p_function, ShaderVersion p_version /* = SHADER_VERSION_MAX */)
{
    std::string id = p_file + std::string("|") + p_function;
    if(m_geometryShaders.find(id) == m_geometryShaders.end())
    {
        HRESULT hr = S_OK;
        ID3D10Blob* pShaderData = 0;
        ID3D10Blob* pErrorData = 0;
        hr = D3DX11CompileFromFileA(p_file, 0, 0, p_function, m_geometryShaderProfiles[p_version], 0, 0, 0, &pShaderData, &pErrorData, 0);
        CHECK_ERROR_BLOB(hr, pErrorData);
        hr = m_pDevice->CreateGeometryShader(pShaderData->GetBufferPointer(), pShaderData->GetBufferSize(), 0, &m_geometryShaders[id]);
        RETURN_IF_FAILED(hr);
    }
    return m_geometryShaders[id];
}


ID3D11PixelShader* GraphicsLayer::CompilePixelShader(LPCSTR p_file, LPCSTR p_function, ShaderVersion p_version /* = SHADER_VERSION_MAX */)
{
    std::string id = p_file + std::string("|") + p_function;
    if(m_pixelShaders.find(id) == m_pixelShaders.end())
    {
        HRESULT hr = S_OK;
        ID3D10Blob* pShaderData = 0;
        ID3D10Blob* pErrorData = 0;
        hr = D3DX11CompileFromFileA(p_file, 0, 0, p_function, m_pixelShaderProfiles[p_version], 0, 0, 0, &pShaderData, &pErrorData, 0);
        CHECK_ERROR_BLOB(hr, pErrorData);
        hr = m_pDevice->CreatePixelShader(pShaderData->GetBufferPointer(), pShaderData->GetBufferSize(), 0, &m_pixelShaders[id]);
        RETURN_IF_FAILED(hr);
    }
    return m_pixelShaders[id];
}