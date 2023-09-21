/*

Simple test for CPU instruction cache, this is demo for some operations:
 1) Instruction cache benchmark.
 2) Self-modifying code and instruction cache invalidation.
 3) Multi-byte NOPs.

Series of simple examples for Microsoft Visual Studio:
https://github.com/manusov/LearningVisualStudio
More projects at:
https://github.com/manusov?tab=repositories
Special thanks for C++ lessons:
https://ravesli.com/uroki-cpp/
More details about used memory management mechanisms:
https://learn.microsoft.com/ru-ru/windows/win32/api/memoryapi/nf-memoryapi-virtualallocex
https://learn.microsoft.com/ru-ru/windows/win32/api/memoryapi/nf-memoryapi-virtualfreeex
https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualprotectex
https://learn.microsoft.com/en-us/windows/win32/Memory/memory-protection-constants
https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-flushinstructioncache

*/

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <intrin.h>

// Definitions for platform configuration detect.
#define CPUID_TSC_MASK  0x20
// Definitions for dynamical code compile.
#define NEAR_RETURN     0xC3
#define MOV_ECX         0xB9
#define JNZ             0x0F, 0x85
#if _WIN64
#define DEC_ECX         0xFF, 0xC9
#define ARCH_NAME       "x64"
#elif _WIN32
#define DEC_ECX         0x49
#define ARCH_NAME       "ia32"
#endif
// Definitions for dynamical code compile, NOPs variants.
constexpr BYTE ONE_BYTE_NOP[]{ 0x90 };
constexpr BYTE TWO_BYTE_NOP[]{ 0x66, 0x90 };
constexpr BYTE FOUR_BYTE_NOP[]{ 0x0f, 0x1f, 0x40, 0x00 };
constexpr BYTE ELEVEN_BYTE_NOP[]{ 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };
constexpr BYTE FIFTEEN_BYTE_NOP[]{ 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };
// Definitions for dynamical code compile, instructions sequences.
constexpr BYTE prefixPattern[] = { MOV_ECX };
constexpr BYTE workPattern[] = { DEC_ECX, JNZ };
constexpr BYTE postfixPattern[] = { NEAR_RETURN };
constexpr size_t prefixSize = sizeof(prefixPattern);
constexpr size_t workSize = sizeof(workPattern);
constexpr size_t postfixSize = sizeof(postfixPattern);
// Block size for memory allocation.
constexpr SIZE_T blockSize = 64 * 1024;
// Definitions for measured block start size, end size, step.
constexpr SIZE_T countMin = 1024;
constexpr SIZE_T countMax = 48 * 1024;
constexpr SIZE_T countStep = 512;
// Measurement repeats.
constexpr unsigned int internalRepeats = 250000;
// Some alternative parameters for L2/L3 cache interrogation.
/*
constexpr SIZE_T blockSize = 1024 * 1024 * 2;
constexpr SIZE_T countMin = 64 * 1024;
constexpr SIZE_T countMax = 1024 * 1024;
constexpr SIZE_T countStep = 64 * 1024;
constexpr unsigned int internalRepeats = 5000;
*/
/*
constexpr SIZE_T blockSize = 32 * 1024 * 1024 + 4096;
constexpr SIZE_T countMin = 1024 * 1024;
constexpr SIZE_T countMax = 24 * 1024 * 1024;
constexpr SIZE_T countStep = 1024 * 1024;
constexpr unsigned int internalRepeats = 500;
*/

// Optimization flag: multi-byte NOPs usage.
constexpr BOOL useMultiByteNops = TRUE;

// Application entry point.
int main(int argc, char** argv)
{
    using namespace std;
    cout << "Example with self-modifying code and instruction cache test (" << ARCH_NAME << ")." << endl;
    int cpudata[4];
    __cpuid(cpudata, 0);
    if (cpudata[0] >= 1)
    {
        __cpuid(cpudata, 1);
        if ((cpudata[3] & CPUID_TSC_MASK))  // Check RDTSC instruction available.
        {
            LPVOID blockPtr = VirtualAllocEx(GetCurrentProcess(), nullptr, blockSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);  // Allocate memory, set WRITEABLE attribute
            if (blockPtr)
            {
                cout << "Memory allocated at " << blockPtr << "h." << endl;
                BYTE* bytePtr = reinterpret_cast<BYTE*>(blockPtr);
                *bytePtr = NEAR_RETURN;  // Dynamical build executable code, here one NEAR RETURN instruction. This operation required WRITEABLE memory.
                DWORD oldProtect = 0;
                if (VirtualProtectEx(GetCurrentProcess(), blockPtr, blockSize, PAGE_EXECUTE_READ, &oldProtect))  // Change to EXECUTABLE attribute for run fragment.
                {
                    cout << "Change memory protection attribyte OK." << endl;
                    if (FlushInstructionCache(GetCurrentProcess(), blockPtr, blockSize))
                    {
                        cout << "Instruction cache flush OK." << endl;
                        void (*funcPtr)() = reinterpret_cast<void(*)()>(blockPtr);   // Set function pointer to allocated memory block with dynamically compiled code.
                        DWORD64 t = __rdtsc();
                        funcPtr();                   // Run test fragment (here used NEAR RETURN). This operation required EXECUTABLE memory.
                        DWORD64 dt = __rdtsc() - t;  // dt = number of TSC clocks per fragment.
                        cout << "Calling mechanism OK, dt is " << dt << "." << endl;
                        cout << "-----------------------------------" << endl;
                        cout << "   Bytes     dTSC/byte" << endl;
                        cout << "-----------------------------------" << endl;
                        BOOL error = FALSE;
                        for (SIZE_T i = countMin; i <= countMax; i += countStep)
                        {
                            if (!VirtualProtectEx(GetCurrentProcess(), blockPtr, blockSize, PAGE_READWRITE, &oldProtect))  // Set WRITEABLE memory mode, for dynamical compile.
                            {
                                error = TRUE;
                                break;
                            }
                            bytePtr = reinterpret_cast<BYTE*>(blockPtr);
                            memcpy(bytePtr, prefixPattern, prefixSize);    // Write first byte of "MOV ECX," instruction.
                            bytePtr += prefixSize;
                            memcpy(bytePtr, &internalRepeats, 4);          // Write 32-bit operand of "MOV ECX," instruction.
                            bytePtr += 4;
                            unsigned int nopBytes = static_cast<unsigned int>(i - workSize);
                            BYTE* overridePtr = bytePtr;
                            for (unsigned int i = 0; i < nopBytes; i++)    // Write series of one-byte NOP-s.
                            {
                                memcpy(bytePtr, ONE_BYTE_NOP, 1);
                                bytePtr += 1;
                            }
                            if (useMultiByteNops)  // Check optimization option.
                            {
                                unsigned int overrideNopBytes = nopBytes / 15;
                                for (unsigned int i = 0; i < overrideNopBytes; i++)  // Override one-byte NOPs by multi-byte NOPs.
                                {
                                    memcpy(overridePtr, FIFTEEN_BYTE_NOP, 15);
                                    overridePtr += 15;
                                }
                            }
                            memcpy(bytePtr, workPattern, workSize);             // Write "DEC ECX" instruction and two bytes of "JNZ ..." instruction code.
                            bytePtr += workSize;
                            int offset = static_cast<int>(workSize + 4 + nopBytes);
                            offset = -offset;
                            memcpy(bytePtr, &offset, 4);                        // Write 32-bit operand of "JNZ ..." instruction, signed offset.
                            bytePtr += 4;
                            memcpy(bytePtr, postfixPattern, postfixSize);       // Write "RET" instruction code.
                            if (!VirtualProtectEx(GetCurrentProcess(), blockPtr, blockSize, PAGE_EXECUTE_READ, &oldProtect))  // Set EXECUTABLE memory mode, for run compiled code.
                            {
                                error = TRUE;
                                break;
                            }
                            if (!FlushInstructionCache(GetCurrentProcess(), blockPtr, blockSize))  // Flush instruction cache because information updated.
                            {
                                error = TRUE;
                                break;
                            }
                            funcPtr();                   // This call for pre-heat.
                            DWORD64 t = __rdtsc();       // t = start Time Stamp Counter value (CPU clocks).
                            funcPtr();                   // This call for time measurement.
                            DWORD64 dt = __rdtsc() - t;  // dt = number of TSC clocks per measured fragment.
                            double delta = static_cast<double>(dt) / (i * internalRepeats);   // delta = number of TSC clocks per one byte at measured fragment.
                            cout << setw(9) << i << "    " << delta << endl;
                        }
                        cout << "-----------------------------------" << endl;
                        if (!error)
                        {
                            cout << "Instruction cache test passed." << endl;
                        }
                        else
                        {
                            cout << "Memory attributes failed during self-modifying code operations." << endl;
                        }
                    }
                    else
                    {
                        cout << "Instruction cache flush failed." << endl;
                    }
                }
                else
                {
                    cout << "Change attribyte failed." << endl;
                }
                if (VirtualFreeEx(GetCurrentProcess(), blockPtr, 0, MEM_RELEASE))   // Release memory block.
                {
                    cout << "Memory release OK." << endl;
                }
                else
                {
                    cout << "Memory release failed." << endl;
                }
            }
            else
            {
                cout << "Memory allocation failed." << endl;
            }
        }
        else
        {
            cout << "RDTSC instruction not supported by CPU or locked." << endl;
        }
    }
    else
    {
        cout << "CPUID instruction failed." << endl;
    }
    return 0;
}
