/*
Fragment with isolation some code from MSDN example for learn it.
Original MSDN Compute Shaders example:
https://github.com/walbourn/directx-sdk-samples/tree/main/BasicCompute11
See also:
https://github.com/walbourn/directx-sdk-samples
https://learn.microsoft.com/ru-ru/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-create
https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader
TODO.
1)  + Make clean-up for error branches also.
2)  - Make helper functions if required for optimization.
3)  + Make memory allocation (dynamical arrays) for big buffers.
4)  How optimize shader for utilize full PCIe traffic (for example: 32GB/S PCIe 4.0 x16)?
    Change calculation type, allocation flags, traffic architecture (Rx/Tx ratio).
5)  How select between system DRAM and GPU memory for allocate buffers?
    Note about buffers can be cached in the GPU memory when measurement repeats used.
6)  Error reading back buffer if simultaneously output to screen (by VS IDE for example).
7)  Learn GPU topology and optimize threads.
    See Dispatch(X,Y,Z) and [numthreads(1, 1, 1)] at shader.
    Optimize work size per one shader call. 
    Too many shader calls at current variant?
    More work per one shader call is better for benchmarking?
    Otherwise overhead measurement instead really shader performance?
8)  Learn GPU features and select calculation scenario.
9)  See parallel NCRB DRAM bandwidth drawings. GPU access cached in the video memory?
*/

#include <windows.h>
#include <iostream>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=nullptr; } }
#endif

struct BufType
{
    int i;
    float f;
};

const UINT NUM_ELEMENTS = 1024 * 1024 * 16;   // Original MSDN value is 1024. Changed for benchmark. Note about differrent limits for x64 and ia32 builds.
const UINT NUM_ARRAYS = 2;                    // Two source arrays for sum. Read 2 source arrays and write 1 destination array. Read traffic analused.
const UINT NUM_REPEATS = 300;                 // Measurement repeats
const DWORD64 ALLOCATED_BYTES = (DWORD64)NUM_ELEMENTS * sizeof(BufType);
const double MEASURED_BYTES = (DWORD64)ALLOCATED_BYTES * NUM_ARRAYS * NUM_REPEATS;

BufType* g_vBuf0 = nullptr;
BufType* g_vBuf1 = nullptr;
ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pContext = nullptr;
ID3D11ComputeShader* g_pCS = nullptr;
ID3D11Buffer* g_pBuf0 = nullptr;
ID3D11Buffer* g_pBuf1 = nullptr;
ID3D11Buffer* g_pBufResult = nullptr;
ID3D11ShaderResourceView* g_pBuf0SRV = nullptr;
ID3D11ShaderResourceView* g_pBuf1SRV = nullptr;
ID3D11UnorderedAccessView* g_pBufResultUAV = nullptr;
ID3D11Buffer* readBackBuf = nullptr;

const char SHADER_SOURCE[] =
"struct BufType\r\n"
"{\r\n"
"int i;\r\n"
"float f;\r\n"
"};\r\n"
"StructuredBuffer<BufType> Buffer0 : register(t0);\r\n"
"StructuredBuffer<BufType> Buffer1 : register(t1);\r\n"
"RWStructuredBuffer<BufType> BufferOut : register(u0);\r\n"
"[numthreads(1, 1, 1)]\r\n"
"void CSMain(uint3 DTid : SV_DispatchThreadID)\r\n"
"{\r\n"
"BufferOut[DTid.x].i = Buffer0[DTid.x].i + Buffer1[DTid.x].i;\r\n"
"BufferOut[DTid.x].f = Buffer0[DTid.x].f + Buffer1[DTid.x].f;\r\n"
"}\0";
const UINT SHADER_SOURCE_LENGTH = sizeof(SHADER_SOURCE) - 1;

void cleaningUp()
{
    SAFE_RELEASE(readBackBuf);
    SAFE_RELEASE(g_pBuf0SRV);
    SAFE_RELEASE(g_pBuf1SRV);
    SAFE_RELEASE(g_pBufResultUAV);
    SAFE_RELEASE(g_pBuf0);
    SAFE_RELEASE(g_pBuf1);
    SAFE_RELEASE(g_pBufResult);
    SAFE_RELEASE(g_pCS);
    SAFE_RELEASE(g_pContext);
    SAFE_RELEASE(g_pDevice);
    if (g_vBuf1) { _aligned_free(g_vBuf1); }
    if (g_vBuf0) { _aligned_free(g_vBuf0); }
}

int main()
{
    std::cout << "Run compute shader sample. v0.0.1. Based on MSDN examples." << std::endl << std::endl;

    // (1) Initializing timers.
    // https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency
    // https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter

    std::cout << "Initializing timers..." << std::endl;
    LARGE_INTEGER hz;
    if (!QueryPerformanceFrequency(&hz))
    {
        std::cout << "Performance counter failed." << std::endl;
        cleaningUp();
        return 1;
    }
    double timerSeconds = 1.0 / hz.QuadPart;
    double megabytes = MEASURED_BYTES / 1E6;
    double seconds = 0.0;
    double mbps = 0.0;

    // (2) Allocate memory.
    // https://learn.microsoft.com/ru-ru/cpp/c-runtime-library/reference/malloc?view=msvc-170
    // https://learn.microsoft.com/ru-ru/cpp/c-runtime-library/reference/aligned-free?view=msvc-170

    std::cout << "Allocate memory..." << std::endl;
    g_vBuf0 = reinterpret_cast<BufType*>(_aligned_malloc(ALLOCATED_BYTES, 16));
    if (g_vBuf0)
    {
        g_vBuf1 = reinterpret_cast<BufType*>(_aligned_malloc(ALLOCATED_BYTES, 16));
    }
    if ((!g_vBuf0) || (!g_vBuf1))
    {
        std::cout << "Memory allocation error." << std::endl;
        cleaningUp();
        return 2;
    }

    // (3) Create device.
    // https://learn.microsoft.com/ru-ru/windows/win32/api/d3d11/nf-d3d11-d3d11createdevice

    std::cout << "Create device..." << std::endl;
    static const D3D_FEATURE_LEVEL featureLevelIn[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL featureLevelOut;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                              // Pointer to video adapter interface (IDXGIAdapter*), nullptr means use default graphics card.
        D3D_DRIVER_TYPE_HARDWARE,             // Driver type, try to create a hardware accelerated device.
        nullptr,                              // DLL descriptor for software rasterizer, nullptr means do not use external software rasterizer module.
        D3D11_CREATE_DEVICE_SINGLETHREADED,   // Device creation flags.
        featureLevelIn,                       // Pointer to feature levels array.
        sizeof(featureLevelIn) / sizeof(D3D_FEATURE_LEVEL),  // Number of feature levels in the array.
        D3D11_SDK_VERSION,                    // SDK version.
        &g_pDevice,                           // Pointer for return pointer to device interface.
        &featureLevelOut,                     // Pointer for return actual feature level created.
        &g_pContext);                         // Pointer for return pointer to device context interface.

    if (FAILED(hr))
    {
        std::cout << "Error creating device, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUp();
        return 3;
    }

    // (4) Compile shader from constant text string.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3dcompiler/nf-d3dcompiler-d3dcompile
    // https://learn.microsoft.com/ru-ru/windows/win32/api/d3dcompiler/nf-d3dcompiler-d3dcompilefromfile

    std::cout << "Compiling shader..." << std::endl;
    const D3D_SHADER_MACRO defines[] =
    {
        "USE_STRUCTURED_BUFFERS", "1",
        nullptr, nullptr
    };
    LPCSTR pFunctionName = "CSMain";
    LPCSTR pProfile = (g_pDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
    ID3DBlob* pErrorBlob = nullptr;
    ID3DBlob* pBlob = nullptr;

    hr = D3DCompile(
        SHADER_SOURCE,                      // Pointer to shader source text string.
        SHADER_SOURCE_LENGTH,               // Length of shader source text string, chars (chars=bytes for ASCII).
        nullptr,                            // Pointer to shader name for error messages, not used here.
        defines,                            // Pointer to shaders definitions as macro sequence.
        D3D_COMPILE_STANDARD_FILE_INCLUDE,  // Default include mode.
        pFunctionName,                      // Pointer to function name for entry point at shader source.
        pProfile,                           // Pointer to shader feature description string.
        D3DCOMPILE_ENABLE_STRICTNESS,       // Flags #1: strictness mode for shader source, reject legacy syntax.
        0,                                  // Flags #2: none.
        &pBlob,                             // Pointer for return pointer to compiled code.
        &pErrorBlob);                       // Pointer for return pointer to error message string.

    if (FAILED(hr))
    {
        std::cout << "Error compiling shader, HRESULT=" << std::hex << hr << "h." << std::endl;
        if (pErrorBlob)
            std::cout << ((char*)pErrorBlob->GetBufferPointer()) << std::endl;
        SAFE_RELEASE(pErrorBlob);
        SAFE_RELEASE(pBlob);
        cleaningUp();
        return 4;
    }

    // (5) Create shader from compiled blob.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createcomputeshader

    hr = g_pDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &g_pCS);
    if (FAILED(hr))
    {
        std::cout << "Error creating shader, HRESULT=" << std::hex << hr << "h." << std::endl;
        if (pErrorBlob)
            std::cout << ((char*)pErrorBlob->GetBufferPointer()) << std::endl;
        SAFE_RELEASE(pErrorBlob);
        SAFE_RELEASE(pBlob);
        cleaningUp();
        return 5;
    }
    SAFE_RELEASE(pErrorBlob);
    SAFE_RELEASE(pBlob);

    // (6) Creating buffers and filling them with initial data...
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createbuffer
    // Buffer descriptor layout, for buffer creating options control:
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc
    // Setup optimal buffer usage model:
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage
    // Buffer bind flags, purpose options:
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_bind_flag
    // CPU access flags, enable read and/or write access from CPU to buffer:
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_cpu_access_flag
    // Miscellaneous flags for buffer creating options:
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_resource_misc_flag

    std::cout << "Creating buffers and filling them with initial data..." << std::endl;
    for (int i = 0; i < NUM_ELEMENTS; ++i)
    {
        g_vBuf0[i].i = i;
        g_vBuf0[i].f = (float)i;
        g_vBuf1[i].i = i;
        g_vBuf1[i].f = (float)i;
    }

    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufDesc.ByteWidth = sizeof(BufType) * NUM_ELEMENTS;
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = sizeof(BufType);
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &g_vBuf0[0];
    hr = g_pDevice->CreateBuffer(&bufDesc, &InitData, &g_pBuf0);
    if (SUCCEEDED(hr))
    {
        hr = g_pDevice->CreateBuffer(&bufDesc, &InitData, &g_pBuf1);
        if (SUCCEEDED(hr))
        {
            hr = g_pDevice->CreateBuffer(&bufDesc, nullptr, &g_pBufResult);
        }
    }
    if (FAILED(hr))
    {
        std::cout << "Error creating buffer objects, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUp();
        return 6;
    }

    // (7) Creating buffer views.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createshaderresourceview
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createunorderedaccessview
    // Shader resource view descriptor:
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_shader_resource_view_desc
    // Unordered access view descriptor:
    // https://learn.microsoft.com/ru-ru/windows/win32/api/d3d11/ns-d3d11-d3d11_unordered_access_view_desc

    std::cout << "Creating buffer views..." << std::endl;
    D3D11_BUFFER_DESC descBuf = {};
    g_pBuf0->GetDesc(&descBuf);
    bool inconsistent = true;
    if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC resDesc = {};
        resDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        resDesc.BufferEx.FirstElement = 0;
        resDesc.Format = DXGI_FORMAT_UNKNOWN;
        resDesc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
        hr = g_pDevice->CreateShaderResourceView(g_pBuf0, &resDesc, &g_pBuf0SRV);
        if (SUCCEEDED(hr))
        {
            g_pBuf1->GetDesc(&descBuf);
            if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC resDesc = {};
                resDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
                resDesc.BufferEx.FirstElement = 0;
                resDesc.Format = DXGI_FORMAT_UNKNOWN;
                resDesc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
                hr = g_pDevice->CreateShaderResourceView(g_pBuf1, &resDesc, &g_pBuf1SRV);
                if (SUCCEEDED(hr))
                {
                    D3D11_BUFFER_DESC descBuf = {};
                    g_pBufResult->GetDesc(&descBuf);
                    if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
                    {
                        D3D11_UNORDERED_ACCESS_VIEW_DESC resDesc = {};
                        resDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
                        resDesc.Buffer.FirstElement = 0;
                        resDesc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
                        resDesc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
                        hr = g_pDevice->CreateUnorderedAccessView(g_pBufResult, &resDesc, &g_pBufResultUAV);
                        inconsistent = false;
                    }
                }
            }
        }
    }
    if (FAILED(hr) || inconsistent)
    {
        std::cout << "Error creating buffer views, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUp();
        return 7;
    }

    // (8) Running compute shader.
    // Time measurement interval starts at this step.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-dispatch
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-cssetshader
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-cssetunorderedaccessviews
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-cssetshaderresources
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-cssetconstantbuffers
    // About threads optimization:
    // https://learn.microsoft.com/ru-ru/windows/win32/direct3dhlsl/sm5-attributes-numthreads
    // https://habr.com/ru/articles/248755/

    std::cout << "Running compute shader..." << std::endl;
    ID3D11ShaderResourceView* aRViews[2] = { g_pBuf0SRV, g_pBuf1SRV };

    LARGE_INTEGER t1, t2;  // Start of time measurement interval.
    t1.QuadPart = 0;
    BOOL b1 = QueryPerformanceCounter(&t1);
    t2.QuadPart = t1.QuadPart;
    BOOL b2 = TRUE;
    while (b1 && b2 && (t2.QuadPart == t1.QuadPart))
    {
        b2 = QueryPerformanceCounter(&t2);
    }

    g_pContext->CSSetShader(g_pCS, nullptr, 0);
    g_pContext->CSSetShaderResources(0, 2, aRViews);
    g_pContext->CSSetUnorderedAccessViews(0, 1, &g_pBufResultUAV, nullptr);

    for (int i = 0; i < NUM_REPEATS; i++)  // Repeats for timings measurement.
    {
        g_pContext->Dispatch(NUM_ELEMENTS, 1, 1);  // This call runs execution compute shader on GPU.
    }

    g_pContext->CSSetShader(nullptr, nullptr, 0);
    ID3D11UnorderedAccessView* ppUAViewnullptr[1] = { nullptr };
    g_pContext->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);
    ID3D11ShaderResourceView* ppSRVnullptr[2] = { nullptr, nullptr };
    g_pContext->CSSetShaderResources(0, 2, ppSRVnullptr);
    ID3D11Buffer* ppCBnullptr[1] = { nullptr };
    g_pContext->CSSetConstantBuffers(0, 1, ppCBnullptr);

    if((!b1) || (!b2))
    {
        std::cout << "Error running shader, timer failed." << std::endl;
        cleaningUp();
        return 8;
    }

    // (9) Read back the result from GPU, verify its correctness against result computed by CPU.
    // Time measurement interval ends at this step.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createbuffer
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-copyresource
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-map

    std::cout << "Wait shader execution and read back the result from GPU..." << std::endl;
    BufType* p = nullptr;
    D3D11_BUFFER_DESC rbufDesc = {};
    g_pBufResult->GetDesc(&rbufDesc);
    rbufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    rbufDesc.Usage = D3D11_USAGE_STAGING;
    rbufDesc.BindFlags = 0;
    rbufDesc.MiscFlags = 0;
    hr = g_pDevice->CreateBuffer(&rbufDesc, nullptr, &readBackBuf);
    if (SUCCEEDED(hr))
    {
        g_pContext->CopyResource(readBackBuf, g_pBufResult);
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        hr = g_pContext->Map(readBackBuf, 0, D3D11_MAP_READ, 0, &mappedResource);
        if (SUCCEEDED(hr))
        {
            p = (BufType*)mappedResource.pData;
        }
    }

    BOOL b3 = FALSE;  // End of time measurement interval.
    if (b1 && b2)
    {
        b3 = QueryPerformanceCounter(&t1);
    }

    if ((!p) || (!b3))
    {
        std::cout << "Error reading back buffer, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUp();
        return 9;
    }

    // (10) Verify that if compute shader has done right

    std::cout << "Verifying against CPU result..." << std::endl;

    bool bSuccess = true;
    for (int i = 0; i < NUM_ELEMENTS; ++i)
        if ((p[i].i != g_vBuf0[i].i + g_vBuf1[i].i) || (p[i].f != g_vBuf0[i].f + g_vBuf1[i].f))
        {
            std::cout << std::endl << "FAILED. GPU and CPU calculation mismatch." << std::endl;
            bSuccess = false;
            break;
        }
    if (bSuccess)
        std::cout << std::endl << "PASSED. GPU and CPU calculation match." << std::endl;
    g_pContext->Unmap(readBackBuf, 0);
    SAFE_RELEASE(readBackBuf);

    // (11) Check timings results.

    seconds = (t1.QuadPart - t2.QuadPart) * timerSeconds;
    mbps = megabytes / seconds;
    std::cout << "Timer unit, seconds = " << timerSeconds << ", dispatch seconds = " << seconds << ", megabytes = " << megabytes << "." << std::endl;
    std::cout << "MBPS = " << mbps << "." << std::endl;

    // (12) Cleaning up.

    std::cout << "Cleaning up..." << std::endl;
    cleaningUp();
    return 0;
}
