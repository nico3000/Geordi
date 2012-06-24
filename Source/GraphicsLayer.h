#pragma once

#define SAFE_RELEASE(_resource) do  \
{                                   \
    if((_resource) != NULL)         \
    {                               \
        (_resource)->Release();     \
        (_resource) = NULL;         \
    }                               \
} while(0)

#define HRESULT_TO_ERROR(_hr) do            \
{                                           \
    LI_ERROR(DXGetErrorDescriptionA(_hr));  \
} while(0)

#define HRESULT_TO_WARNING(_hr) do          \
{                                           \
    LI_WARNING(DXGetErrorDescriptionA(_hr));\
} while(0)

#define RETURN_IF_FAILED(_hr) do    \
{                                   \
    HRESULT _result = _hr;          \
    if(FAILED(_result))             \
    {                               \
        HRESULT_TO_ERROR(_result);  \
        return false;               \
    }                               \
} while(0)


class GraphicsLayer
{
public:
    enum ShaderVersion
    {
        SHADER_VERSION_3_0,
        SHADER_VERSION_4_0,
        SHADER_VERSION_5_0,
        SHADER_VERSION_MAX,
    };

private:
    typedef std::map<std::string, ID3D11VertexShader*> VertexShaders;
    typedef std::map<std::string, ID3D11GeometryShader*> GeometryShaders;
    typedef std::map<std::string, ID3D11PixelShader*> PixelShaders;
    typedef std::map<ShaderVersion, LPCSTR> ShaderProfiles;

    bool m_initialized;
    bool m_vsync;
    HWND m_hWnd;
    IDXGISwapChain* m_pSwapChain;
    IDXGIOutput *m_pOutput;
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pContext;
    ID3D11RenderTargetView* m_pBackBuffer;
    D3D_FEATURE_LEVEL m_featureLevel;
    int m_width;
    int m_height;
    VertexShaders m_vertexShaders;
    GeometryShaders m_geometryShaders;
    PixelShaders m_pixelShaders;
    ShaderProfiles m_vertexShaderProfiles;
    ShaderProfiles m_geometryShaderProfiles;
    ShaderProfiles m_pixelShaderProfiles;

    bool CreateAppWindow(HINSTANCE p_hInstance);
    bool CreateAppGraphics(VOID);
    void ReleaseBackbuffer(void);
    bool LoadBackbuffer(void);
    void PrepareFeatureLevel(void);
    bool PrintAdapterString(IDXGIAdapter1* p_pAdapter) const;
    std::string GetAdapterOutputString(IDXGIOutput* p_pOutput) const;
    bool SetDefaultSamplers(void);

public:
    GraphicsLayer(void);
    ~GraphicsLayer(void);

    bool Init(HINSTANCE p_hInstance);
    void Clear(void);
    void Present(void);
    bool OnWindowResized(int p_width, int p_height);
    ID3D11VertexShader* CompileVertexShader(LPCSTR p_file, LPCSTR p_function, ID3D10Blob*& p_pShaderData, ShaderVersion p_version = SHADER_VERSION_MAX);
    ID3D11GeometryShader* CompileGeometryShader(LPCSTR p_file, LPCSTR p_function, ShaderVersion p_version = SHADER_VERSION_MAX);
    ID3D11PixelShader* CompilePixelShader(LPCSTR p_file, LPCSTR p_function, ShaderVersion p_version = SHADER_VERSION_MAX);
    bool ToggleFullscreen(void);
    bool SetFullscreen(bool p_fullscreen);
    bool IsFullscreen(void) const;

    const D3D_FEATURE_LEVEL& GetFeatureLevel(void) const { return m_featureLevel; }
    ID3D11Device* GetDevice(void) { return m_pDevice; }
    ID3D11DeviceContext* GetContext(void) { return m_pContext; }
    int GetWidth(void) const { return m_width; }
    int GetHeight(void) const { return m_height; }
    const HWND GetWindowHandle(VOID) const { return m_hWnd; }
};
