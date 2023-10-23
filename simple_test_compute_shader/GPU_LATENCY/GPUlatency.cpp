/*

Example how use compute shader for low-level GPU benchmarking.
GPU latency measurement.
GPU access to Video RAM (typically over GPU local bus ) and System RAM (typically over PCIe)
https://github.com/manusov/LearningVisualStudio/tree/master/simple_test_compute_shader
Fragment with isolation some code from MSDN example for learn it.
Original MSDN Compute Shaders example:
https://github.com/walbourn/directx-sdk-samples/tree/main/BasicCompute11
See also:
https://github.com/walbourn/directx-sdk-samples
https://learn.microsoft.com/ru-ru/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-create
https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader
Fundamental theory:
https://www.overclockers.ua/video/gpu-evolution/
Special thanks to:
https://ravesli.com/uroki-po-opengl/
https://ravesli.com/uroki-cpp/

TODO.
1) How optimize shader for utilize full PCIe traffic (for example: 32GB/S PCIe 4.0 x16)?
2) How optimize shader for utilize full discrette local Video RAM traffic?
3) How measure latency and prevent GPU speculative access?
4) Errors reading back buffer if simultaneously screen updates (by VS IDE for example). Depend on NUM_ELEMENTS.
5) Learn GPU topology and optimize threads. See Dispatch(X,Y,Z) and [numthreads(1, 1, 1)] at shader.
6) Parallel DRAM/PCIe traffic monitoring with NCRB and OpenHardwareMonitor.

About hide memory delay cause wrong latency measurement results.
https://coremission.net/gamedev/vychislitelnye-sheidery-1/
https://coremission.net/gamedev/vychislitelnye-sheidery-2/

IMPORTANT.
This engineering sample yet verified at NVIDIA GeForce RTX 3060 Ti only.
Latency tests at ASUS N750JK notebook (mobile NVIDIA GTX850M and INTEL HD4600) can fail depend on NUM_ELEMENTS.
Plus, GPU cache effects can occur, especially if reduce NUM_ELEMENTS.
Can use long latency chain (linked list) with small read-back buffer? Reduce destination buffer size?
Long latency chains can fail because driver can limit shader execution time?

*/

#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=nullptr; } }
#endif

struct BufType
{
    int link;
};

constexpr UINT NUM_X = 512;                            // This 3 topology parameters MUST BE INTEGER POWER OF 2.
constexpr UINT NUM_Y = 512;
constexpr UINT NUM_Z = 16 * 2;                         // Twice buffer size at v0.5.3, old is 16.
constexpr UINT APP_REPEATS = 30;                       // Number of measurement repeats in the application.
constexpr UINT SHADER_REPEATS = 1;                     // Number of measurement repeats in the shader. Must be DST_ALLOCATED_BYTES >= SHADER_REPEATS * 4.
constexpr UINT NUM_ELEMENTS = NUM_X * NUM_Y * NUM_Z;   // Original MSDN value is 1024. Changed for benchmark. Note about differrent limits for x64 and ia32 builds.
constexpr UINT LATENCY_CHAIN = NUM_ELEMENTS / 2;       // Independent settings for LATENCY_CHAIN and NUM_ELEMENTS at v0.5.3.

constexpr UINT SHADER_X = 1;        // This parameter means threads per wave front (?) Typical optimal is 32 (NVidia) and 64 (AMD) ?
constexpr UINT SHADER_Y = 1;        // But here set to { 1,1,1 } for prevent parallelism because latency test.
constexpr UINT SHADER_Z = 1;
constexpr UINT DISPATCH_X = 1;      // Set { 1,1,1 } for prevent parallelism because latency test.
constexpr UINT DISPATCH_Y = 1;
constexpr UINT DISPATCH_Z = 1;
constexpr UINT STEP_Y = SHADER_X * DISPATCH_X;
constexpr UINT STEP_Z = SHADER_X * DISPATCH_X * SHADER_Y * DISPATCH_Y;

// Force buffer allocation at system DRAM if this flag is TRUE (under-debug, can cause implementation-specific effects).
// Assumed typical scenario for discrette GPU-initiated traffic at PCIe-based platform: 
// FALSE = traffic GPU-VideoRAM (fast local path),  TRUE = traffic GPU-PCIe-SystemRAM (slow remote path, PCIe limited).
constexpr BOOL SRC_PCIE_MODE = FALSE;    // This flag controls SOURCE data location for read, copy and latency. FALSE=VRAM, TRUE=DRAM.
constexpr BOOL DST_PCIE_MODE = FALSE;    // This flag controls DESTINATION data location for write and copy. FALSE=VRAM, TRUE=DRAM.

#define DEFAULT_ADAPTER -1
constexpr int ADAPTER_INDEX = DEFAULT_ADAPTER;    // -1  or index overflow means default selection, otherwise index in the list, 0-based.
// Note test non-active adapter can fail, use adapter selection for application at Windows (left mouse button).

constexpr DWORD64 SRC_ALLOCATED_BYTES = (DWORD64)NUM_ELEMENTS * sizeof(BufType);
constexpr DWORD64 DST_ALLOCATED_BYTES = 4096;                                                // Must be DST_ALLOCATED_BYTES >= SHADER_REPEATS * 4.
constexpr double MEASURED_CYCLES = (DWORD64)LATENCY_CHAIN * APP_REPEATS * SHADER_REPEATS;    // Total number of read cycles used for latency measurement.

BufType* g_vBuf0 = nullptr;
BufType* g_vBuf1 = nullptr;
ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pContext = nullptr;
ID3D11ComputeShader* g_pCS = nullptr;
ID3D11Buffer* g_pBuf0 = nullptr;
ID3D11Buffer* g_pBuf1 = nullptr;
ID3D11Buffer* g_pBufResult = nullptr;
ID3D11Buffer* readBackBuf = nullptr;
ID3D11ShaderResourceView* g_pBuf0SRV = nullptr;
ID3D11ShaderResourceView* g_pBuf1SRV = nullptr;
ID3D11UnorderedAccessView* g_pBufResultUAV = nullptr;

const char SHADER_SOURCE_LATENCY[] =
"struct BufType\r\n"
"{\r\n"
"int link;\r\n"
"};\r\n"
"StructuredBuffer<BufType> Buffer0 : register(t0);\r\n"
"StructuredBuffer<BufType> Buffer1 : register(t1);\r\n"
"RWStructuredBuffer<BufType> BufferOut : register(u0);\r\n"
"[numthreads(SHADER_X, SHADER_Y, SHADER_Z)]\r\n"
"void CSMain()\r\n"
"{\r\n"
"for(int i=0; i<SHADER_REPEATS; i++)\r\n"
"   {\r\n"
"   int index = Buffer1[0].link;\r\n"
"   while(index >= 0)\r\n"
"      {\r\n"
"      index = Buffer0[index].link;\r\n"
"      }\r\n"
"   BufferOut[i].link = index + i;\r\n"
"   }\r\n"
"}\0";

constexpr UINT SHADER_SOURCE_LENGTH_LATENCY = sizeof(SHADER_SOURCE_LATENCY) - 1;

void cleaningUpAndWaitKey()
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
    std::cout << std::endl;
    system("pause");
}


int main()
{
    std::cout << "[ Measure DRAM/VRAM/PCIe latency by GPU. ]" << std::endl;
#if _WIN64
    std::cout << "[ Engineering sample v0.5.4 (x64).       ]" << std::endl;
#elif _WIN32
    std::cout << "[ Engineering sample v0.5.4 (ia32).      ]" << std::endl;
#elif
    std::cout << "[ Engineering sample v0.5.4 (unknown platform). ]" << std::endl;
#endif
    std::cout << "[ Based on MSDN information and sources. ]" << std::endl << std::endl;

    // (1) Read performance counter timer frequency.
    // https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency
    // https://learn.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter

    std::cout << "Reading performance counter timer frequency..." << std::endl;
    LARGE_INTEGER hz;
    if (!QueryPerformanceFrequency(&hz))
    {
        std::cout << "Performance counter failed." << std::endl;
        cleaningUpAndWaitKey();
        return 1;
    }
    double timerSeconds = 1.0 / hz.QuadPart;
    double cycles = MEASURED_CYCLES;
    double seconds = 0.0;
    double nanoseconds = 0.0;

    // (2) Allocate memory.
    // https://learn.microsoft.com/ru-ru/cpp/c-runtime-library/reference/malloc?view=msvc-170
    // https://learn.microsoft.com/ru-ru/cpp/c-runtime-library/reference/aligned-free?view=msvc-170

    std::cout << "Allocating memory..." << std::endl;
    g_vBuf0 = reinterpret_cast<BufType*>(_aligned_malloc(SRC_ALLOCATED_BYTES, 16));
    if (g_vBuf0)
    {
        g_vBuf1 = reinterpret_cast<BufType*>(_aligned_malloc(SRC_ALLOCATED_BYTES, 16));
    }
    if ((!g_vBuf0) || (!g_vBuf1))
    {
        std::cout << "Memory allocation error." << std::endl;
        cleaningUpAndWaitKey();
        return 2;
    }

    // (3) List video adapters devices.
    // https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgifactory
    // https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-createdxgifactory
    // https://learn.microsoft.com/ru-ru/windows/win32/api/dxgi/nn-dxgi-idxgiadapter
    // https://learn.microsoft.com/ru-ru/windows/win32/api/dxgi/ns-dxgi-dxgi_adapter_desc
    // https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/wcstombs-wcstombs-l?view=msvc-170

    IDXGIFactory* factory;
    std::vector<IDXGIAdapter*> adapters;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (SUCCEEDED(hr))
    {
        IDXGIAdapter* adapter;
        for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
            adapters.push_back(adapter);
        factory->Release();
        std::cout << std::endl << "Adapters list..." << std::endl;
        int index = 0;
        for (auto a : adapters)
        {
            DXGI_ADAPTER_DESC aDesc;
            if (SUCCEEDED(a->GetDesc(&aDesc)))
            {
                WCHAR* pWname = aDesc.Description;
                CHAR pName[128];
                size_t count = 0;
                wcstombs_s(&count, pName, pWname, 128);
                std::cout << "[ " << index << " ]" << "[ " << pName << " ]" << std::endl;
            }
            index++;
        }
        std::cout << std::endl;
    }
    else
    {
        std::cout << "Error creating DXGI factory, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUpAndWaitKey();
        return 3;
    }

    // (4) Creating device.
    // https://learn.microsoft.com/ru-ru/windows/win32/api/d3d11/nf-d3d11-d3d11createdevice
    // https://learn.microsoft.com/en-us/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_feature_level

    std::cout << "Creating device..." << std::endl;
    static const D3D_FEATURE_LEVEL featureLevelIn[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL featureLevelOut;

    IDXGIAdapter* pAdapter = nullptr;
    if ((ADAPTER_INDEX > 0) && (ADAPTER_INDEX < adapters.size()))
    {
        pAdapter = adapters[ADAPTER_INDEX];
    }

    hr = D3D11CreateDevice(
        pAdapter,                             // Pointer to video adapter interface (IDXGIAdapter*), or nullptr means use default graphics card.
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
        cleaningUpAndWaitKey();
        return 4;
    }

    // (5) Compiling shader from constant text string and dynamically created defines string.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3dcompiler/nf-d3dcompiler-d3dcompile
    // https://learn.microsoft.com/ru-ru/windows/win32/api/d3dcompiler/nf-d3dcompiler-d3dcompilefromfile

    std::cout << "Compiling shader..." << std::endl;
    static char s1[10], s2[10], s3[10], s4[10], s5[10], s6[10];
    const D3D_SHADER_MACRO defines[] =
    {
        "SHADER_X"       , s1 ,
        "SHADER_Y"       , s2 ,
        "SHADER_Z"       , s3 ,
        "STEP_Y"         , s4 ,
        "STEP_Z"         , s5 ,
        "SHADER_REPEATS" , s6 ,
        nullptr         , nullptr
    };
    snprintf(s1, 10, "%d", SHADER_X);
    snprintf(s2, 10, "%d", SHADER_Y);
    snprintf(s3, 10, "%d", SHADER_Z);
    snprintf(s4, 10, "%d", STEP_Y);
    snprintf(s5, 10, "%d", STEP_Z);
    snprintf(s6, 10, "%d", SHADER_REPEATS);

    LPCSTR pFunctionName = "CSMain";
    LPCSTR pProfile = (g_pDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
    ID3DBlob* pErrorBlob = nullptr;
    ID3DBlob* pBlob = nullptr;
    LPCVOID pSrcData = SHADER_SOURCE_LATENCY;
    SIZE_T srcDataSize = SHADER_SOURCE_LENGTH_LATENCY;

    hr = D3DCompile(
        pSrcData,                           // Pointer to shader source text string.
        srcDataSize,                        // Length of shader source text string, chars (chars=bytes for ASCII).
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
        cleaningUpAndWaitKey();
        return 5;
    }

    // (6) Creating shader from compiled blob.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createcomputeshader

    std::cout << "Creating shader..." << std::endl;
    hr = g_pDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &g_pCS);
    if (FAILED(hr))
    {
        std::cout << "Error creating shader, HRESULT=" << std::hex << hr << "h." << std::endl;
        if (pErrorBlob)
            std::cout << ((char*)pErrorBlob->GetBufferPointer()) << std::endl;
        SAFE_RELEASE(pErrorBlob);
        SAFE_RELEASE(pBlob);
        cleaningUpAndWaitKey();
        return 6;
    }
    SAFE_RELEASE(pErrorBlob);
    SAFE_RELEASE(pBlob);

    // (7) Generating test patterns.

    std::cout << "Generating initial data: test patterns..." << std::endl;
    for (int i = 0; i < NUM_ELEMENTS; i++)
    {
        g_vBuf0[i].link = 0;
        g_vBuf1[i].link = 0;
    }

    for (int i = 0; i < LATENCY_CHAIN; i++)
    {
        g_vBuf1[i].link = i;
    }

    constexpr UINT64 RANDOM_SEED = 0x3;
    constexpr UINT64 RANDOM_MULTIPLIER = 0x5DEECE66D;
    constexpr UINT64 RANDOM_ADDEND = 0xB;
    constexpr int CHAIN_END_MARKER = -5;
    UINT64 seed = RANDOM_SEED;
    int modValue = LATENCY_CHAIN;
    int modIndex = 0;
    for (int i = 0; i < LATENCY_CHAIN; i++)
    {
        seed = seed * RANDOM_MULTIPLIER + RANDOM_ADDEND;
        modIndex = seed % modValue;
        int temp = g_vBuf1[i].link;
        g_vBuf1[i].link = g_vBuf1[modIndex].link;
        g_vBuf1[modIndex].link = temp;
    }

    int entry = CHAIN_END_MARKER;
    for (int i = 0; i < LATENCY_CHAIN; i++)
    {
        int temp = g_vBuf1[i].link;
        g_vBuf0[temp].link = entry;
        entry = temp;
    }
    g_vBuf1[0].link = entry;

    for (int i = 1; i < LATENCY_CHAIN; i++)
    {
        g_vBuf1[i].link = 0;
    }

    int checkLength = 0;
    int checkIndex = g_vBuf1[0].link;
    bool checkBound = false;
    while (true)
    {
        checkIndex = g_vBuf0[checkIndex].link;
        checkLength++;
        if (checkIndex == CHAIN_END_MARKER)
        {
            break;
        }
        if (checkIndex >= LATENCY_CHAIN)
        {
            checkBound = true;
        }
    }

    if ((checkLength != LATENCY_CHAIN) || checkBound)
    {
        std::cout << "Internal error: randomization algorithm failed." << std::endl;
        cleaningUpAndWaitKey();
        return 7;
    }

    // (8) Creating source buffers with generated test patterns.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createbuffer

    std::cout << "Creating source buffers and filling them with initial data..." << std::endl;
    D3D11_BUFFER_DESC srcBufDesc = {};
    srcBufDesc.ByteWidth = SRC_ALLOCATED_BYTES;
    srcBufDesc.Usage = D3D11_USAGE_DEFAULT;
    srcBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    if (SRC_PCIE_MODE)
    {
        srcBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    }
    else
    {
        srcBufDesc.CPUAccessFlags = 0;
    }

    srcBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    srcBufDesc.StructureByteStride = sizeof(BufType);
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &g_vBuf0[0];
    hr = g_pDevice->CreateBuffer(&srcBufDesc, &initData, &g_pBuf0);
    if (SUCCEEDED(hr))
    {
        hr = g_pDevice->CreateBuffer(&srcBufDesc, &initData, &g_pBuf1);
    }
    if (FAILED(hr))
    {
        std::cout << "Error creating source buffer objects, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUpAndWaitKey();
        return 8;
    }

    // (9) Creating destination buffer.
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
    D3D11_BUFFER_DESC dstBufDesc = {};
    dstBufDesc.ByteWidth = DST_ALLOCATED_BYTES;
    dstBufDesc.Usage = D3D11_USAGE_DEFAULT;
    dstBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    if (DST_PCIE_MODE)
    {
        dstBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    }
    else
    {
        dstBufDesc.CPUAccessFlags = 0;
    }
    dstBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    dstBufDesc.StructureByteStride = sizeof(BufType);
    hr = g_pDevice->CreateBuffer(&dstBufDesc, nullptr, &g_pBufResult);
    if (FAILED(hr))
    {
        std::cout << "Error creating destination buffer objects, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUpAndWaitKey();
        return 9;
    }

    // (10) Creating source buffers views.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createshaderresourceview
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createunorderedaccessview
    // Shader resource view descriptor:
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_shader_resource_view_desc
    // Unordered access view descriptor:
    // https://learn.microsoft.com/ru-ru/windows/win32/api/d3d11/ns-d3d11-d3d11_unordered_access_view_desc

    std::cout << "Creating source buffers views..." << std::endl;
    D3D11_BUFFER_DESC descSrcBuf = {};
    g_pBuf0->GetDesc(&descSrcBuf);
    bool srcInconsistent = true;
    if (descSrcBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC resDesc = {};
        resDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        resDesc.BufferEx.FirstElement = 0;
        resDesc.Format = DXGI_FORMAT_UNKNOWN;
        resDesc.BufferEx.NumElements = descSrcBuf.ByteWidth / descSrcBuf.StructureByteStride;
        hr = g_pDevice->CreateShaderResourceView(g_pBuf0, &resDesc, &g_pBuf0SRV);
        if (SUCCEEDED(hr))
        {
            g_pBuf1->GetDesc(&descSrcBuf);
            if (descSrcBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC resDesc = {};
                resDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
                resDesc.BufferEx.FirstElement = 0;
                resDesc.Format = DXGI_FORMAT_UNKNOWN;
                resDesc.BufferEx.NumElements = descSrcBuf.ByteWidth / descSrcBuf.StructureByteStride;
                hr = g_pDevice->CreateShaderResourceView(g_pBuf1, &resDesc, &g_pBuf1SRV);
                srcInconsistent = false;
            }
        }
    }
    if (FAILED(hr) || srcInconsistent)
    {
        std::cout << "Error creating buffer views, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUpAndWaitKey();
        return 10;
    }

    // (11) Creating destination buffer view.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createshaderresourceview
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createunorderedaccessview
    // Shader resource view descriptor:
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_shader_resource_view_desc
    // Unordered access view descriptor:
    // https://learn.microsoft.com/ru-ru/windows/win32/api/d3d11/ns-d3d11-d3d11_unordered_access_view_desc

    std::cout << "Creating destination buffer view..." << std::endl;
    D3D11_BUFFER_DESC descDstBuf = {};
    g_pBufResult->GetDesc(&descDstBuf);
    bool dstInconsistent = true;
    if (descDstBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC resDesc = {};
        resDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        resDesc.Buffer.FirstElement = 0;
        resDesc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer.
        resDesc.Buffer.NumElements = descDstBuf.ByteWidth / descDstBuf.StructureByteStride;
        hr = g_pDevice->CreateUnorderedAccessView(g_pBufResult, &resDesc, &g_pBufResultUAV);
        dstInconsistent = false;
    }
    if (FAILED(hr) || dstInconsistent)
    {
        std::cout << "Error creating buffer views, HRESULT=" << std::hex << hr << "h." << std::endl;
        cleaningUpAndWaitKey();
        return 11;
    }

    // (12) Running compute shader.
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

    for (int i = 0; i < APP_REPEATS; i++)                     // Repeats for timings measurement.
    {
        g_pContext->Dispatch(DISPATCH_X, DISPATCH_Y, DISPATCH_Z);    // This call runs execution compute shader on GPU.
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
        cleaningUpAndWaitKey();
        return 12;
    }

    // (13) Reading back the data from GPU, verifying its correctness against data computed by CPU.
    // Time measurement interval ends at this step.
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createbuffer
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-copyresource
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-map

    std::cout << "Waiting shader execution and reading back the result from GPU..." << std::endl;
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
        cleaningUpAndWaitKey();
        return 13;
    }

    // (14) Check and calculate timings results.

    bool verifyError = false;
    for (int i = 0; i < SHADER_REPEATS; i++)
    {
        if (p->link != (CHAIN_END_MARKER + i))
        {
            verifyError = true;
        }
        p++;
    }
    
    g_pContext->Unmap(readBackBuf, 0);

    if (verifyError)
    {
        std::cout << "Error in the shader results." << std::endl;
        cleaningUpAndWaitKey();
        return 14;
    }

    seconds = (t1.QuadPart - t2.QuadPart) * timerSeconds;
    nanoseconds = 1E9 * seconds / cycles;
    std::cout << "Timer unit (seconds) = " << timerSeconds << ", shader dispatch (seconds) = " << seconds << ", read cycles = " << cycles << "." << std::endl;
    if (!SRC_PCIE_MODE)
    {
        std::cout << "GPU access latency (GPU to Video RAM) = " << nanoseconds << " ns." << std::endl;
    }
    else
    {
        std::cout << "GPU access latency (GPU to System DRAM) = " << nanoseconds << " ns." << std::endl;
    }

    // (15) Cleaning up.

    std::cout << "Cleaning up..." << std::endl;
    cleaningUpAndWaitKey();
    return 0;
}
