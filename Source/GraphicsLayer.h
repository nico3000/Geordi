#pragma once
#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>
#include <dxgi.h>
#include <xnamath.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "DxErr.lib")
#pragma comment(lib, "dxgi.lib")


#define SAFE_RELEASE(_resource) do  \
{                                   \
    if((_resource) != NULL)         \
    {                               \
        (_resource)->Release();     \
        (_resource) = NULL;         \
    }                               \
} while(0)

#define RETURN_IF_FAILED(_hr) do     \
{                                               \
    if(FAILED(_hr))                             \
    {                                           \
        LI_ERROR(DXGetErrorDescriptionA(_hr));  \
        return false;                           \
    }                                           \
} while (0)


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
    bool m_fullscreen;
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

    const D3D_FEATURE_LEVEL& GetFeatureLevel(void) const { return m_featureLevel; }
    ID3D11Device* GetDevice(void) { return m_pDevice; }
    ID3D11DeviceContext* GetContext(void) { return m_pContext; }
    int GetWidth(void) const { return m_width; }
    int GetHeight(void) const { return m_height; }
    const HWND GetWindowHandle(VOID) const { return m_hWnd; }
    bool IsFullscreen(void) const { return m_fullscreen; }
};
