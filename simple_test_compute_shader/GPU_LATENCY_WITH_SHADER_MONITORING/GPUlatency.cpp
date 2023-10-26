/*

Example how use compute shader for low-level GPU benchmarking.
GPU latency measurement. Version with shader execution monitoring for analysing aggressive optimizations.
Discrette GPU access to Video RAM (typically over GPU local bus ) and System RAM (typically over PCIe).
https://github.com/manusov/LearningVisualStudio/tree/master/simple_test_compute_shader
Fragment with isolation some code from MSDN example for learn it.
Original MSDN Compute Shaders example:
https://github.com/walbourn/directx-sdk-samples/tree/main/BasicCompute11
See also:
https://github.com/walbourn/directx-sdk-samples
https://learn.microsoft.com/ru-ru/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-create
https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader
Fundamental theory and specifications:
https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm
About hide memory delay cause wrong latency measurement results.
https://coremission.net/gamedev/vychislitelnye-sheidery-1/
https://coremission.net/gamedev/vychislitelnye-sheidery-2/
Popular:
https://www.overclockers.ua/video/gpu-evolution/
Special thanks to:
https://ravesli.com/uroki-po-opengl/
https://ravesli.com/uroki-cpp/

TODO.
1) How optimize shader for utilize maximum PCIe traffic (for example: 32GB/S PCIe 4.0 x16)?
2) How optimize shader for utilize maximum discrette local Video RAM traffic?
3) How measure latency and prevent GPU speculative access?
4) Errors reading-back buffer if simultaneously screen updates (by VS IDE for example). Depend on NUM_ELEMENTS.
5) Learn GPU topology and optimize threads. See Dispatch(X,Y,Z) and [numthreads(1, 1, 1)] at shader.
6) Parallel DRAM/PCIe traffic monitoring with NCRB and OpenHardwareMonitor.

IMPORTANT.
This engineering sample yet verified at NVIDIA GeForce RTX 3060 Ti only.
Latency tests at ASUS N750JK notebook (mobile NVIDIA GTX850M and INTEL HD4600) can fail depend on NUM_ELEMENTS.
Plus, GPU cache effects can occur, especially if reduce NUM_ELEMENTS.
Long latency chains can fail because driver can limit shader execution time?
Timings is very dependent on random numbers: latency test reports very different values (>10 times) if use hardware RNG.
Cause: GPU and shader compiler optimizations can corrupt for/while cycle logic at shader.
Required algorithm compatible with aggressive optimizations.

*/

#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <intrin.h>
#include <immintrin.h>

enum AddressType
{
    ADDR_ZERO,
    ADDR_INC,
    ADDR_DEC,
    ADDR_SRNG,
    ADDR_HRNG
};

// "Hot" parameters for adaptation.
   #define BIG_GPU
// #define SMALL_GPU
#ifdef BIG_GPU
constexpr UINT ADDRESS_MODE = ADDR_HRNG;          // Select address sequence generation or randomization method.
constexpr UINT LATENCY_CHAIN = 2 * 1024 * 1024;   // Independent settings for LATENCY_CHAIN and NUM_ELEMENTS at v0.5.3+.
constexpr BOOL SRC_PCIE_MODE = TRUE;              // This flag controls SOURCE data location for read, copy and latency. FALSE=VRAM, TRUE=DRAM.
constexpr BOOL DST_PCIE_MODE = TRUE;              // This flag controls DESTINATION data location for write and copy. FALSE=VRAM, TRUE=DRAM.
constexpr UINT MEASUREMENT_REPEATS = 10;          // Number of measurement repeats in the application.
#elif defined SMALL_GPU 
constexpr UINT ADDRESS_MODE = ADDR_INC;           // Select address sequence generation or randomization method.
constexpr UINT LATENCY_CHAIN = 512 * 1024;        // Independent settings for LATENCY_CHAIN and NUM_ELEMENTS at v0.5.3+.
constexpr BOOL SRC_PCIE_MODE = TRUE;              // This flag controls SOURCE data location for read, copy and latency. FALSE=VRAM, TRUE=DRAM.
constexpr BOOL DST_PCIE_MODE = TRUE;              // This flag controls DESTINATION data location for write and copy. FALSE=VRAM, TRUE=DRAM.
constexpr UINT MEASUREMENT_REPEATS = 100;         // Number of measurement repeats in the application.
#else
constexpr UINT ADDRESS_MODE  = ADDR_DEC;          // Select address sequence generation or randomization method.
constexpr UINT LATENCY_CHAIN = 4096;              // Independent settings for LATENCY_CHAIN and NUM_ELEMENTS at v0.5.3+.
constexpr BOOL SRC_PCIE_MODE = FALSE;             // This flag controls SOURCE data location for read, copy and latency. FALSE=VRAM, TRUE=DRAM.
constexpr BOOL DST_PCIE_MODE = FALSE;             // This flag controls DESTINATION data location for write and copy. FALSE=VRAM, TRUE=DRAM.
constexpr UINT MEASUREMENT_REPEATS = 100;         // Number of measurement repeats in the application.
#endif

// Flags SRC_PCIE_MODE, DST_PCIE_MODE forces buffer allocation at system DRAM if this flag is TRUE (under-debug, can cause implementation-specific effects).
// Assumed typical scenario for discrette GPU-initiated traffic at PCIe-based platform: 
// FALSE = traffic GPU-VideoRAM (fast local path),  TRUE = traffic GPU-PCIe-SystemRAM (slow remote path, PCIe limited).

constexpr UINT NUM_X = 512;                            // This 3 topology parameters MUST BE INTEGER POWER OF 2.
constexpr UINT NUM_Y = 512;
constexpr UINT NUM_Z = 16;
constexpr UINT NUM_ELEMENTS = NUM_X * NUM_Y * NUM_Z;   // Original MSDN value is 1024. Changed for benchmark. Note about differrent limits for x64 and ia32 builds.
constexpr UINT SHADER_X = 1;        // This parameter means threads per wave front (?) Typical optimal is 32 (NVidia) and 64 (AMD) ?
constexpr UINT SHADER_Y = 1;        // But here set to { 1,1,1 } for prevent parallelism because latency test.
constexpr UINT SHADER_Z = 1;
constexpr UINT DISPATCH_X = 1;      // Set { 1,1,1 } for prevent parallelism because latency test.
constexpr UINT DISPATCH_Y = 1;
constexpr UINT DISPATCH_Z = 1;
constexpr UINT STEP_Y = SHADER_X * DISPATCH_X;
constexpr UINT STEP_Z = SHADER_X * DISPATCH_X * SHADER_Y * DISPATCH_Y;

constexpr int CHAIN_MARKER_FOR_CPU = -37;
constexpr int CHAIN_MARKER_FOR_GPU = 37;
constexpr UINT SHADER_RETURN_SIGNATURE = 0x5555AAAA;

#define DEFAULT_ADAPTER -1
constexpr int ADAPTER_INDEX = DEFAULT_ADAPTER;    // -1  or index overflow means default selection, otherwise index in the list, 0-based.
// Note test non-active adapter can fail, use adapter selection for application at Windows (left mouse button).

struct BufType
{
    int link;
};

constexpr DWORD64 SRC_ALLOCATED_BYTES = (DWORD64)NUM_ELEMENTS * sizeof(BufType);
constexpr DWORD64 DST_ALLOCATED_BYTES = 4096;                                       // Must be DST_ALLOCATED_BYTES >= size of ShaderStatusType structure.
constexpr double MEASURED_CYCLES = (DWORD64)LATENCY_CHAIN * MEASUREMENT_REPEATS;    // Total number of read cycles used for latency measurement.

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

struct ApplicationStatusType
{
    int count;
};

struct ShaderStatusType
{
    int count;
    int marker;
    int markerIndex;
    int error;
    int errorIndex;
    int signature;
};

ShaderStatusType shaderMonitor{ 0,0,0,0,0,0 };
ApplicationStatusType applicationMonitor{ 0 };

// Compute shader source as text string.
// This source string compiled in runtime by GPU driver. Result is GPU-executable code, executed by GPU.
// https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-variable-register

const char SHADER_SOURCE_LATENCY[] =
"struct BufType\r\n"
"{\r\n"
"int link;\r\n"
"};\r\n"
"struct ShaderStatusType\r\n"
"{\r\n"
"int count;\r\n"
"int marker;\r\n"
"int markerIndex;\r\n"
"int error;\r\n"
"int errorIndex;\r\n"
"int signature;\r\n"
"};\r\n"
"StructuredBuffer<BufType> Buffer0 : register(t0);\r\n"
"StructuredBuffer<BufType> Buffer1 : register(t1);\r\n"
"RWStructuredBuffer<ShaderStatusType> BufferOut : register(u0);\r\n"
"[numthreads(SHADER_X, SHADER_Y, SHADER_Z)]\r\n"
"void CSMain()\r\n"
"{\r\n"
"int count = 0;\r\n"
"int marker = 0;\r\n"
"int markerIndex = 0;\r\n"
"int error = -1;\r\n"
"int errorIndex = -1;\r\n"
"int walk = Buffer1[0].link;\r\n"
"for(int i=0; i<LATENCY_CHAIN; i++)\r\n"
"   {\r\n"
"   walk = Buffer0[walk].link;\r\n"
"   if (walk == CHAIN_MARKER)                   { marker = walk; markerIndex = i; } \r\n"
"   if ((walk < 0) || (walk >= LATENCY_CHAIN )) { error = walk;  errorIndex = i;  } \r\n"
"   count++;\r\n"
"   }\r\n"
"BufferOut[0].count = count;\r\n"
"BufferOut[0].marker = marker;\r\n"
"BufferOut[0].markerIndex = markerIndex;\r\n"
"BufferOut[0].error = error;\r\n"
"BufferOut[0].errorIndex = errorIndex;\r\n"
"BufferOut[0].signature = SHADER_RETURN_SIGNATURE;\r\n"
"}\0";

constexpr UINT SHADER_SOURCE_LENGTH_LATENCY = sizeof(SHADER_SOURCE_LATENCY) - 1;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=nullptr; } }
#endif

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
    std::cout << "[ GPU to DRAM/VRAM/PCIe latency with shader execution monitoring. ]" << std::endl;
#if _WIN64
    std::cout << "[ Based on MSDN. Engineering sample v0.6.1 (x64).                 ]" << std::endl << std::endl;
#elif _WIN32
    std::cout << "[ Based on MSDN. Engineering sample v0.6.1 (ia32).                ]" << std::endl << std::endl;
#elif
    std::cout << "[ Based on MSDN. Engineering sample v0.6.1 (unknown platform).    ]" << std::endl << std::endl;
#endif

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
    UINT srcCount = LATENCY_CHAIN;
    UINT dstBytes = sizeof(ShaderStatusType);
    int chainMarker = CHAIN_MARKER_FOR_GPU;
    if ((srcCount > NUM_ELEMENTS) || (dstBytes > DST_ALLOCATED_BYTES) || (chainMarker >= NUM_ELEMENTS) || (chainMarker <= 0))
    {
        std::cout << "Internal error: bad settings." << std::endl;
            cleaningUpAndWaitKey();
            return 2;
    }

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
    static char s1[12], s2[12], s3[12], s4[12], s5[12], s6[12], s7[12], s8[12];
    const D3D_SHADER_MACRO defines[] =
    {
        "SHADER_X"                , s1 ,
        "SHADER_Y"                , s2 ,
        "SHADER_Z"                , s3 ,
        "STEP_Y"                  , s4 ,
        "STEP_Z"                  , s5 ,
        "LATENCY_CHAIN"           , s6 ,
        "CHAIN_MARKER"            , s7 ,
        "SHADER_RETURN_SIGNATURE" , s8 ,
        nullptr                   , nullptr
    };
    snprintf(s1, 12, "%d", SHADER_X);
    snprintf(s2, 12, "%d", SHADER_Y);
    snprintf(s3, 12, "%d", SHADER_Z);
    snprintf(s4, 12, "%d", STEP_Y);
    snprintf(s5, 12, "%d", STEP_Z);
    snprintf(s6, 12, "%d", LATENCY_CHAIN);
    snprintf(s7, 12, "%d", CHAIN_MARKER_FOR_GPU);
    snprintf(s8, 12, "%d", SHADER_RETURN_SIGNATURE);

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
    constexpr UINT RDRAND_MASK = 0x40000000;
    bool hardwareRng = false;
    UINT addressMode = ADDRESS_MODE;
    if (ADDRESS_MODE == ADDR_HRNG)
    {   // If hardware random number generator selected by option, required check RDRAND instruction support.
        int cpudata[4];
        __cpuid(cpudata, 0);
        if (cpudata[0] >= 1)
        {
            __cpuid(cpudata, 1);
            if ((cpudata[2] & RDRAND_MASK))
            {
                hardwareRng = true;
            }
        }
        if (!hardwareRng)
        {
            std::cout << "Hardware RNG not supported or locked." << std::endl;
            cleaningUpAndWaitKey();
            return 7;
        }
    }

    for (int i = 0; i < NUM_ELEMENTS; i++)
    {
        g_vBuf0[i].link = 0;
        g_vBuf1[i].link = 0;
    }

    int entry = CHAIN_MARKER_FOR_CPU;
    if ((addressMode == ADDR_SRNG) || (addressMode == ADDR_HRNG))
    {
        for (int i = 0; i < LATENCY_CHAIN; i++)
        {
            g_vBuf1[i].link = i;
        }

        int modValue = LATENCY_CHAIN;
        if (hardwareRng)
        {
            for (int i = 0; i < LATENCY_CHAIN; i++)
            {
#if _WIN64
                UINT64 random = 0;
                while (!_rdrand64_step(&random));
#elif _WIN32
                UINT32 random = 0;
                while (!_rdrand32_step(&random));
#endif
                int modIndex = random % modValue;
                int temp = g_vBuf1[i].link;
                g_vBuf1[i].link = g_vBuf1[modIndex].link;
                g_vBuf1[modIndex].link = temp;
            }
        }
        else
        {
            constexpr UINT64 RANDOM_SEED = 0x3;
            constexpr UINT64 RANDOM_MULTIPLIER = 0x5DEECE66D;
            constexpr UINT64 RANDOM_ADDEND = 0xB;
            UINT64 seed = RANDOM_SEED;
            for (int i = 0; i < LATENCY_CHAIN; i++)
            {
                seed = seed * RANDOM_MULTIPLIER + RANDOM_ADDEND;
                int modIndex = seed % modValue;
                int temp = g_vBuf1[i].link;
                g_vBuf1[i].link = g_vBuf1[modIndex].link;
                g_vBuf1[modIndex].link = temp;
            }
        }

        for (int i = 0; i < LATENCY_CHAIN; i++)
        {
            int temp = g_vBuf1[i].link;
            g_vBuf0[temp].link = entry;
            entry = temp;
        }
    }
    
    else if (addressMode == ADDR_INC)
    {
        for (int i = LATENCY_CHAIN - 1; i >= 0; i--)
        {
            g_vBuf0[i].link = entry;
            entry = i;
        }
    }
    
    else if (addressMode == ADDR_DEC)
    {
        for (int i = 0; i < LATENCY_CHAIN; i++)
        {
            g_vBuf0[i].link = entry;
            entry = i;
        }
    }
    
    else if (addressMode == ADDR_ZERO)
    {
        for (int i = 0; i < LATENCY_CHAIN; i++)
        {
            g_vBuf0[i].link = 0;
        }
    }

    else
    {
        std::cout << "Internal error: wrong address mode selection." << std::endl;
        cleaningUpAndWaitKey();
        return 7;
    }

    g_vBuf1[0].link = entry;
    for (int i = 1; i < LATENCY_CHAIN; i++)
    {
        g_vBuf1[i].link = 0;
    }
    
    if (ADDRESS_MODE != ADDR_ZERO)
    {
        int checkLength = 0;
        int checkIndex = g_vBuf1[0].link;
        bool checkBound = false;
        while (true)
        {
            checkLength++;
            int temp = g_vBuf0[checkIndex].link;
            if (temp == CHAIN_MARKER_FOR_CPU)
            {
                g_vBuf0[checkIndex].link = CHAIN_MARKER_FOR_GPU;
                break;
            }
            else
            {
                checkIndex = temp;
            }
            if (checkIndex >= LATENCY_CHAIN)
            {
                checkBound = true;
            }
        }
        applicationMonitor.count = checkLength;
        if ((checkLength != LATENCY_CHAIN) || checkBound)
        {
            std::cout << "Internal error: randomization algorithm failed." << std::endl;
            cleaningUpAndWaitKey();
            return 7;
        }
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

    for (int i = 0; i < MEASUREMENT_REPEATS; i++)                    // Repeats for timings measurement.
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

    std::cout << "Waiting shader execution and reading back the results from GPU..." << std::endl;
    ShaderStatusType* p = nullptr;
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
            p = (ShaderStatusType*)mappedResource.pData;
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

    // (14) Check shader return and calculate timings results.
    
    shaderMonitor.count = p->count;
    shaderMonitor.marker = p->marker;
    shaderMonitor.markerIndex = p->markerIndex;
    shaderMonitor.error = p->error;
    shaderMonitor.errorIndex = p->errorIndex;
    shaderMonitor.signature = p->signature;
    g_pContext->Unmap(readBackBuf, 0);

    std::cout << std::endl << "Shader return block signature = " << std::hex << std::uppercase << shaderMonitor.signature << "h." << std::endl;
    std::cout << "Shader iterations counter     = " << std::dec << std::nouppercase << shaderMonitor.count;
    std::cout << " (CPU: " << applicationMonitor.count << ")." << std::endl;
    std::cout << "Shader walk list marker       = " << shaderMonitor.marker << "." << std::endl;
    std::cout << "Shader walk list marker index = " << shaderMonitor.markerIndex << "." << std::endl;
    std::cout << "Shader walk list error        = " << shaderMonitor.error << "." << std::endl;
    std::cout << "Shader walk list error index  = " << shaderMonitor.errorIndex << "." << std::endl;

    if (ADDRESS_MODE != ADDR_ZERO)
    {
        if ((shaderMonitor.signature != SHADER_RETURN_SIGNATURE) ||
            (shaderMonitor.count != applicationMonitor.count) ||
            (shaderMonitor.marker != CHAIN_MARKER_FOR_GPU) ||
            (shaderMonitor.error != -1) ||
            (shaderMonitor.errorIndex != -1))
        {
            // std::cout << "Error: shader results verification FAILED." << std::endl;
            // std::cout << "Timings is NOT VALID because speculative scenario detected." << std::endl;
            // cleaningUpAndWaitKey();
            // return 14;
            std::cout << std::endl << "Warning: timings is NOT VALID because speculative scenario detected." << std::endl;
        }
    }
    else
    {
        std::cout << std::endl << "Warning: shader results verification SKIPPED at zero-address mode." << std::endl;
    }
    
    seconds = (t1.QuadPart - t2.QuadPart) * timerSeconds;
    nanoseconds = 1E9 * seconds / cycles;
    std::cout << "Timer unit (seconds) = " << timerSeconds << ", shader dispatch (seconds) = " << seconds << ", read cycles = " << cycles << "." << std::endl;
    if (!SRC_PCIE_MODE)
    {
        std::cout << std::endl << "GPU access latency (GPU to Video RAM) = " << nanoseconds << " ns." << std::endl;
    }
    else
    {
        std::cout << std::endl << "GPU access latency (GPU to System DRAM) = " << nanoseconds << " ns." << std::endl;
    }

    // (15) Cleaning up.

    std::cout << "Cleaning up..." << std::endl;
    cleaningUpAndWaitKey();
    return 0;
}
