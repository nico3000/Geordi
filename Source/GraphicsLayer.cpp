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


GraphicsLayer::GraphicsLayer(void):
m_pContext(0), m_pDevice(0), m_pSwapChain(0), m_pBackBuffer(0), m_pOutput(0), m_hWnd(0), m_initialized(false), m_fullscreen(false)
{
}


GraphicsLayer::~GraphicsLayer(void)
{
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


ShaderProgram g_program;
VertexBuffer g_vertices;
IndexBuffer g_indices;
Camera g_cam;


bool GraphicsLayer::Init(HINSTANCE hInstance)
{
    // TODO: Static testing stuff goes here and only here.
    //TerrainData terrain;
    //terrain.Init(32, 8, 4, 8, 8);
    //terrain.Test();

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
        { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(+0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-0.5f, +0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(+0.5f, +0.5f, 0.5f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
    };
    if(!g_vertices.Build(vertices, ARRAYSIZE(vertices), sizeof(VertexBuffer::SimpleVertex)))
    {
        return false;
    }

    unsigned int indices[] = {
        0, 1, 2, 3,
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
    g_cam.Move(-5.0f, 0.0f, 0.0f);

    return true;
}


void GraphicsLayer::Clear(void)
{
    static const float pClearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    m_pContext->ClearRenderTargetView(m_pBackBuffer, pClearColor);
}


void GraphicsLayer::Present(void)
{
    static int id = LostIsland::g_pTimer->Tick(REALTIME);
    long elapsed = LostIsland::g_pTimer->Tock(id, KEEPRUNNING);
    g_cam.SetPosition(sin((float)elapsed * 1e-2f), 0.0f, -3.0f + sin((float)elapsed * 0.5e-2f));

    g_program.Bind();
    g_vertices.Bind();
    g_indices.Bind();
    g_cam.Update();
    //m_pContext->DrawIndexed(4, 0, 0);

    m_pSwapChain->Present(0, 0);
}


bool GraphicsLayer::CreateAppGraphics(void)
{
    HRESULT hr = S_OK;

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

    IDXGIFactory* pFactory;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
    RETURN_IF_FAILED(hr);

    IDXGIAdapter* pAdapter;
    hr = pFactory->EnumAdapters(0, &pAdapter);
    RETURN_IF_FAILED(hr);
    RETURN_IF_FAILED(hr);

    DXGI_SWAP_CHAIN_DESC scDesc;
    scDesc.BufferCount = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    scDesc.OutputWindow = m_hWnd;
    scDesc.SampleDesc.Count = 1;
    scDesc.SampleDesc.Quality = 0;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scDesc.Windowed = !m_fullscreen;

    DXGI_MODE_DESC desiredMode;
    ZeroMemory(&desiredMode, sizeof(DXGI_MODE_DESC));
    desiredMode.Width = SCREEN_WIDTH;
    desiredMode.Height = SCREEN_HEIGHT;
    desiredMode.RefreshRate.Numerator = 120;
    desiredMode.RefreshRate.Denominator = 1;

    hr = pAdapter->EnumOutputs(0, &m_pOutput);
    RETURN_IF_FAILED(hr);
    hr = m_pOutput->FindClosestMatchingMode(&desiredMode, &scDesc.BufferDesc, m_pDevice);
    RETURN_IF_FAILED(hr);

    hr = D3D11CreateDeviceAndSwapChain(pAdapter,
                                       D3D_DRIVER_TYPE_UNKNOWN,
                                       0,
                                       flags,
                                       pFeatureLevels,
                                       ARRAYSIZE(pFeatureLevels),
                                       D3D11_SDK_VERSION,
                                       &scDesc,
                                       &m_pSwapChain,
                                       &m_pDevice,
                                       &m_featureLevel,
                                       &m_pContext);
    RETURN_IF_FAILED(hr);

    switch(m_featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_0:
        m_vertexShaderProfiles[SHADER_VERSION_MAX] = m_vertexShaderProfiles[SHADER_VERSION_5_0];
        m_geometryShaderProfiles[SHADER_VERSION_MAX] = m_geometryShaderProfiles[SHADER_VERSION_5_0];
        m_pixelShaderProfiles[SHADER_VERSION_MAX] = m_pixelShaderProfiles[SHADER_VERSION_5_0];
        LI_LOG("Direct3D", "Your GPU supports Direct3D11.");
        break;
    case D3D_FEATURE_LEVEL_10_1:
        break;
    case D3D_FEATURE_LEVEL_10_0:
        m_vertexShaderProfiles[SHADER_VERSION_MAX] = m_vertexShaderProfiles[SHADER_VERSION_4_0];
        m_geometryShaderProfiles[SHADER_VERSION_MAX] = m_geometryShaderProfiles[SHADER_VERSION_4_0];
        m_pixelShaderProfiles[SHADER_VERSION_MAX] = m_pixelShaderProfiles[SHADER_VERSION_4_0];
        LI_LOG("Direct3D", "Your GPU does not support Direct3D 11. Falling back to Direct3D10.");
        break;
    case D3D_FEATURE_LEVEL_9_3:
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
        m_vertexShaderProfiles[SHADER_VERSION_MAX] = m_vertexShaderProfiles[SHADER_VERSION_3_0];
        m_pixelShaderProfiles[SHADER_VERSION_MAX] = m_pixelShaderProfiles[SHADER_VERSION_3_0];
        LI_LOG("Direct3D", "Poor bastard! Your GPU does not support Direct3D 11. Not even Direct3D 10. Falling back to Direct3D9.");
        break;
    }

    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.FrontCounterClockwise = true;
    ID3D11RasterizerState* pRasterizerState;
    hr = m_pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState);
    RETURN_IF_FAILED(hr);
    m_pContext->RSSetState(pRasterizerState);
    SAFE_RELEASE(pRasterizerState);

    D3D11_VIEWPORT viewport;
    viewport.Width = SCREEN_WIDTH;
    viewport.Height = SCREEN_HEIGHT;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_pContext->RSSetViewports(1, &viewport);

    this->LoadBackbuffer();
    m_pContext->OMSetRenderTargets(1, &m_pBackBuffer, 0);

    m_initialized = true;

    return SUCCEEDED(hr);
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

    m_width = SCREEN_WIDTH;
    m_height = SCREEN_HEIGHT;
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
        str << "New size: " << p_width << "x" << p_height;
        LI_INFO(str.str());

        this->ReleaseBackbuffer();

        hr = m_pSwapChain->ResizeBuffers(0, p_width, p_height, DXGI_FORMAT_UNKNOWN, 0);
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

    return true;
}


bool GraphicsLayer::ToggleFullscreen(void)
{
    ReleaseBackbuffer();
    HRESULT hr = S_OK;
    if(m_fullscreen)
    {
        hr = m_pSwapChain->SetFullscreenState(false, 0);
        RETURN_IF_FAILED(hr);
        m_fullscreen = false;
    }
    else
    {
        DXGI_MODE_DESC desc, matching;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.Width = 1920;
        desc.Height = 1080;
        desc.RefreshRate.Numerator = 120;
        desc.RefreshRate.Denominator = 1;
        desc.Scaling = DXGI_MODE_SCALING_CENTERED;
        desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
        m_pOutput->FindClosestMatchingMode(&desc, &matching, m_pDevice);

        hr = m_pSwapChain->ResizeTarget(&desc);
        RETURN_IF_FAILED(hr);
        hr = m_pSwapChain->SetFullscreenState(true, m_pOutput);
        RETURN_IF_FAILED(hr);

        m_fullscreen = true;
    }
    LoadBackbuffer();
    return true;
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