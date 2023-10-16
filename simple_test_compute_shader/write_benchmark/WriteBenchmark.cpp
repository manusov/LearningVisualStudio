/*

Compute shader with write benchmark example.
https://github.com/manusov/LearningVisualStudio
Fragment with isolation some code from MSDN example for learn it.
Original MSDN Compute Shaders example:
https://github.com/walbourn/directx-sdk-samples/tree/main/BasicCompute11
See also:
https://github.com/walbourn/directx-sdk-samples
https://learn.microsoft.com/ru-ru/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-create
https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader
Special thanks to:
https://ravesli.com/uroki-po-opengl/
https://ravesli.com/uroki-cpp/

TODO.
1)  + Make clean-up for error branches also.
2)  - Make helper functions if required for optimization.
3)  + Make memory allocation (dynamical arrays) for big buffers.
4)   Add DISPATCH_Y and DISPATCH_Z.
5)   How optimize shader for utilize full PCIe traffic (for example: 32GB/S PCIe 4.0 x16)?
6)   How optimize shader for utilize full discrette local Video RAM traffic?
     Change calculation type, allocation flags, traffic architecture (Rx/Tx ratio).
7)   How select between system DRAM and GPU memory for allocate buffers?
     Note about buffers can be cached in the GPU memory when measurement repeats used.
8)   Error reading back buffer if simultaneously output to screen (by VS IDE for example).
9)   Learn GPU topology and optimize threads.
     See Dispatch(X,Y,Z) and [numthreads(1, 1, 1)] at shader.
     Optimize work size per one shader call.
     Too many shader calls at current variant?
     More work per one shader call is better for benchmarking?
     Otherwise overhead measurement instead really shader performance?
10)  Learn GPU features and select calculation scenario.
11)  See parallel NCRB DRAM bandwidth drawings. GPU access cached in the video memory?
12)  Strategy target: measure video memory bandwidth and latency.
13)  Strategy target: measure PCIe operations bandwidth and latency.
     For CPU- and GPU-initiated memory access.

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
    int x;
    int y;
    int z;
    int pad;
};

constexpr UINT NUM_X = 512;
constexpr UINT NUM_Y = 512;
constexpr UINT NUM_Z = 64;

constexpr UINT NUM_ELEMENTS = NUM_X * NUM_Y * NUM_Z;   // Original MSDN value is 1024. Changed for benchmark. Note about differrent limits for x64 and ia32 builds.
constexpr UINT NUM_ARRAYS = 1;                         // One destination array for write, write traffic bandwidth measured at this example.
constexpr UINT NUM_REPEATS = 5000;                     // Measurement repeats.

constexpr UINT SHADER_X = 32;                          // This parameter means threads per wave front (?) Typical optimal is 32 (NVidia) and 64 (AMD) ?
constexpr UINT SHADER_Y = 8;
constexpr UINT SHADER_Z = 2;

constexpr UINT DISPATCH_X = NUM_X / SHADER_X;
constexpr UINT DISPATCH_Y = NUM_Y / SHADER_Y;
constexpr UINT DISPATCH_Z = NUM_Z / SHADER_Z;

constexpr UINT STEP_Y = SHADER_X * DISPATCH_X;
constexpr UINT STEP_Z = SHADER_X * DISPATCH_X * SHADER_Y * DISPATCH_Y;

constexpr DWORD64 ALLOCATED_BYTES = (DWORD64)NUM_ELEMENTS * sizeof(BufType);
constexpr double MEASURED_BYTES = (DWORD64)ALLOCATED_BYTES * NUM_ARRAYS * NUM_REPEATS;

ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pContext = nullptr;
ID3D11ComputeShader* g_pCS = nullptr;
ID3D11Buffer* g_pBufResult = nullptr;
ID3D11UnorderedAccessView* g_pBufResultUAV = nullptr;
ID3D11Buffer* readBackBuf = nullptr;

const char SHADER_SOURCE[] =
"struct BufType\r\n"
"{\r\n"
"int x;\r\n"
"int y;\r\n"
"int z;\r\n"
"int pad;\r\n"
"};\r\n"
"StructuredBuffer<BufType> Buffer0 : register(t0);\r\n"
"StructuredBuffer<BufType> Buffer1 : register(t1);\r\n"
"RWStructuredBuffer<BufType> BufferOut : register(u0);\r\n"
"[numthreads(SHADER_X, SHADER_Y, SHADER_Z)]\r\n"
"void CSMain(uint3 DTid : SV_DispatchThreadID)\r\n"
"{\r\n"
"BufferOut[DTid.x + DTid.y * STEP_Y + DTid.z * STEP_Z].x = DTid.x;\r\n"
"BufferOut[DTid.x + DTid.y * STEP_Y + DTid.z * STEP_Z].y = DTid.y;\r\n"
"BufferOut[DTid.x + DTid.y * STEP_Y + DTid.z * STEP_Z].z = DTid.z;\r\n"
"BufferOut[DTid.x + DTid.y * STEP_Y + DTid.z * STEP_Z].pad = 0xFFFFFFFF;\r\n"
"}\0";
const UINT SHADER_SOURCE_LENGTH = sizeof(SHADER_SOURCE) - 1;

void cleaningUp()
{
    SAFE_RELEASE(readBackBuf);
    SAFE_RELEASE(g_pBufResultUAV);
    SAFE_RELEASE(g_pBufResult);
    SAFE_RELEASE(g_pCS);
    SAFE_RELEASE(g_pContext);
    SAFE_RELEASE(g_pDevice);
}

int main()
{
    std::cout << "Compute shader with write benchmark. v0.1.0." << std::endl;
    std::cout << "Based on MSDN information and sources." << std::endl << std::endl;

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
    double gigabytes = MEASURED_BYTES / 1E9;
    double seconds = 0.0;
    double gbps = 0.0;

    // (2) Create device.
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
        return 2;
    }

    // (3) Compile shader from constant text string and dynamically created defines string.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3dcompiler/nf-d3dcompiler-d3dcompile
    // https://learn.microsoft.com/ru-ru/windows/win32/api/d3dcompiler/nf-d3dcompiler-d3dcompilefromfile

    std::cout << "Compiling shader..." << std::endl;
    static char s1[10], s2[10], s3[10], s4[10], s5[10];
    const D3D_SHADER_MACRO defines[] =
    {
        "SHADER_X" , s1 ,
        "SHADER_Y" , s2 ,
        "SHADER_Z" , s3 ,
        "STEP_Y"   , s4 ,
        "STEP_Z"   , s5 ,
        nullptr, nullptr
    };
    snprintf(s1, 10, "%d", SHADER_X);
    snprintf(s2, 10, "%d", SHADER_Y);
    snprintf(s3, 10, "%d", SHADER_Z);
    snprintf(s4, 10, "%d", STEP_Y);
    snprintf(s5, 10, "%d", STEP_Z);

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
        return 3;
    }

    // (4) Create shader from compiled blob.
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
        return 4;
    }
    SAFE_RELEASE(pErrorBlob);
    SAFE_RELEASE(pBlob);

    // (5) Creating destination buffer.
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

    std::cout << "Creating destination buffer..." << std::endl;
    D3D11_BUFFER_DESC bufDesc = {};
    bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufDesc.ByteWidth = sizeof(BufType) * NUM_ELEMENTS;
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = sizeof(BufType);
    hr = g_pDevice->CreateBuffer(&bufDesc, nullptr, &g_pBufResult);
    if (FAILED(hr))
    {
        std::cout << "Error creating buffer objects, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUp();
        return 5;
    }

    // (6) Creating buffer views.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createshaderresourceview
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createunorderedaccessview
    // Shader resource view descriptor:
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_shader_resource_view_desc
    // Unordered access view descriptor:
    // https://learn.microsoft.com/ru-ru/windows/win32/api/d3d11/ns-d3d11-d3d11_unordered_access_view_desc

    std::cout << "Creating destination buffer view..." << std::endl;
    D3D11_BUFFER_DESC descBuf = {};
    g_pBufResult->GetDesc(&descBuf);
    bool inconsistent = true;
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
    if (FAILED(hr) || inconsistent)
    {
        std::cout << "Error creating buffer views, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUp();
        return 6;
    }

    // (7) Running compute shader.
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
    ID3D11ShaderResourceView* aRViews[2] = { nullptr, nullptr };

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
        g_pContext->Dispatch(DISPATCH_X, DISPATCH_Y, DISPATCH_Z);  // This call runs execution compute shader on GPU.
    }

    g_pContext->CSSetShader(nullptr, nullptr, 0);
    ID3D11UnorderedAccessView* ppUAViewnullptr[1] = { nullptr };
    g_pContext->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);
    ID3D11ShaderResourceView* ppSRVnullptr[2] = { nullptr, nullptr };
    g_pContext->CSSetShaderResources(0, 2, ppSRVnullptr);
    ID3D11Buffer* ppCBnullptr[1] = { nullptr };
    g_pContext->CSSetConstantBuffers(0, 1, ppCBnullptr);

    if ((!b1) || (!b2))
    {
        std::cout << "Error running shader, timer failed." << std::endl;
        cleaningUp();
        return 7;
    }

    // (8) Read back the data from GPU, verify its correctness against data computed by CPU.
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
        return 8;
    }

    // (9) Verify that if compute shader has done right

    std::cout << "Verifying against CPU result..." << std::endl;
    bool verifyError = false;
    int i = 0, j = 0, k = 0;
    for (int i = 0; i < NUM_Z; ++i)
    {
        for (int j = 0; j < NUM_Y; ++j)
        {
            for (int k = 0; k < NUM_X; ++k)
            {
                int checkX = p[k + j * STEP_Y + i * STEP_Z].x;
                int checkY = p[k + j * STEP_Y + i * STEP_Z].y;
                int checkZ = p[k + j * STEP_Y + i * STEP_Z].z;
                int checkP = p[k + j * STEP_Y + i * STEP_Z].pad;
                if ((checkX != k) || (checkY != j) || (checkZ != i) || (checkP != 0xFFFFFFFF))
                {
                    verifyError = true;
                    break;
                }
            }
        }
    }
    g_pContext->Unmap(readBackBuf, 0);

    if (verifyError)
    {
        std::cout << std::endl << "FAILED. GPU and CPU data mismatch. X=" << k << ", Y=" << j << ", Z=" << i << "." << std::endl;
    }
    else
    {
        std::cout << std::endl << "PASSED. GPU and CPU data match." << std::endl;
    }
 
    // (10) Check timings results.

    seconds = (t1.QuadPart - t2.QuadPart) * timerSeconds;
    gbps = gigabytes / seconds;
    std::cout << "Timer unit (seconds) = " << timerSeconds << ", shader dispatch (seconds) = " << seconds << ", gigabytes = " << gigabytes << "." << std::endl;
    std::cout << "GPU write bandwidth = " << gbps << " GBPS." << std::endl;

    // (11) Cleaning up.

    std::cout << "Cleaning up..." << std::endl;
    cleaningUp();
    return 0;
}
