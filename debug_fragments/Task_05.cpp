/*
Class for AVX512 matrix transpose.
Step 1 = debug for one cell 8x8 and 16x16, for x64 builds only.
*/

#include "Task_05.h"

extern "C" void __stdcall transposeBigToLittle(__m512d * src, __m512d * dst, size_t rowSize);
extern "C" void __stdcall transposeLittleToBig(__m512d * src, __m512d * dst, size_t rowSize);


int Task_05::execute(int argc, char** argv)
{
#ifdef NATIVE_WIDTH_64
	int exitCode = 0;
	AppLib::write("\r\n");
	if (detectAvx512())
	{
		constexpr DWORD64 BLOCK_SIZE = 32768;
		constexpr DWORD64 BLOCK_ALIGN = 64;
		constexpr DWORD64 REPEATS = 10000000;
/*
		DWORD64 dTSC = 0;
		DWORD64 sumTSC = 0;
		double dummyData = 0;
		double dummySum = 0;
*/
		LARGE_INTEGER hzPc;
		LARGE_INTEGER hzTsc;

		// char msg[APPCONST::MAX_TEXT_STRING];
		Timer timer;
		if (timer.precisionMeasure(hzPc, hzTsc))
		{
			double clockTime = 1.0 / static_cast<double>(hzTsc.QuadPart);
			double megabytes = static_cast<double>(BLOCK_SIZE * REPEATS) / 1E6;

			void* srcPtr = _aligned_malloc(BLOCK_SIZE, BLOCK_ALIGN);
			void* dstPtr = _aligned_malloc(BLOCK_SIZE, BLOCK_ALIGN);
			if ((srcPtr) && (dstPtr))
			{
				__m512d* p1 = reinterpret_cast<__m512d*>(srcPtr);
				__m512d* p2 = reinterpret_cast<__m512d*>(dstPtr);

				double* p3 = reinterpret_cast<double*>(p1);
				DWORD64 c = BLOCK_SIZE / sizeof(double);
				double a = 1.0;
				for (int i = 0; i < c; i++)
				{
					*p3 = a;
					p3++;
					a += 1.0;
				}
/*
				dump8x8(p1);
				AppLib::write("\r\n");
				// __debugbreak();
				// transposeBigToLittle(p1, p2, 64);
				transposeLittleToBig(p1, p2, 64);
				dump8x8(p2);
*/
/*
				dump16x16(p1);
				AppLib::write("\r\n");
				// __debugbreak();
				// transposeBigToLittle(p1, p2, 128);
				transposeLittleToBig(p1, p2, 128);
				dump16x16(p2);
*/
				dump16x16(p1);
				AppLib::write("\r\n");
				transposeBigToLittle(p1, p2, 128);
				transposeBigToLittle(p1 + 1, p2 + 16, 128);
				transposeBigToLittle(p1 + 16, p2 + 1, 128);
				transposeBigToLittle(p1 + 17, p2 + 17, 128);
				dump16x16(p2);
//
			}
			else
			{
				exitCode = 1;
			}
			if (srcPtr) _aligned_free(srcPtr);
			if (dstPtr) _aligned_free(dstPtr);
		}
		else
		{
			exitCode = 2;
		}
	}
	else
	{
		AppLib::writeColor("AVX512 or OS ZMM-context management is not supported.\r\n", APPCONST::ERROR_COLOR);
		exitCode = 3;
	}
	return exitCode;
#else
	AppLib::writeColor("\r\nAVX512 matrix transpose test is not supported at ia32 build.\r\n", APPCONST::ERROR_COLOR);
	return -1;
#endif
}


void Task_05::transposeAvx512(__m512d* src, __m512d* dst, DWORD32 cellsCount, size_t srcAdd, size_t dstAdd)
{

}


BOOL Task_05::detectAvx512()
{
	BOOL status = FALSE;
	int regs[4];
	__cpuid(regs, 0);
	if ((regs[0]) > 7)
	{
		__cpuid(regs, 1);
		constexpr int MASK_AVX256 = (1 << 27) | (1 << 28);
		if ((regs[2] & MASK_AVX256) == MASK_AVX256)
		{
			__cpuidex(regs, 7, 0);
			constexpr int MASK_AVX512 = 1 << 16;
			if ((regs[1] & MASK_AVX512) == MASK_AVX512)
			{
				DWORD64 xcr0 = _xgetbv(0);
				constexpr DWORD64 MASK_XCR0 = 0xE6;
				if ((xcr0 & MASK_XCR0) == MASK_XCR0)
				{
					status = TRUE;
				}
			}
		}
	}
	return status;
}

void Task_05::dump8x8(void* data)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	double* p = reinterpret_cast<double*>(data);
	for (int i = 0; i < 8; i++)
	{
		snprintf(msg, APPCONST::MAX_TEXT_STRING,
			"%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f\r\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
		p += 8;
	}
}


void Task_05::dump16x16(void* data)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	double* p = reinterpret_cast<double*>(data);
	for (int i = 0; i < 16; i++)
	{
		snprintf(msg, APPCONST::MAX_TEXT_STRING,
			"%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f\r\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		AppLib::writeColor(msg, APPCONST::TABLE_COLOR);
		p += 16;
	}
}
