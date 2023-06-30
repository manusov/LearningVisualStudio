/*

Context,
data types, initialization, visualization
for vector data and intrinsics.

m128i, m256i, m512i.
Integer data, 8/16/32/64 bits.
Can be initialized as constant array of up to 64 BYTES.

m128, m256, m512.
Single-precision floating point data,
4 x 32-bit numbers per 128-bit vector, 8 x 32-bit numbers per 256-bit vector, 16 x 32-bit numbers per 512-bit vector.
Can be initialized as constant array of up to 4/8/16 FLOATS.

m128d, m256d, m512d.
Double-precision floating point data,
2 x 64-bit numbers per 128-bit vector, 4 x 64-bit numbers per 256-bit vector, 8 x 64-bit numbers per 512-bit vector.
Can be initialized as constant array of up to 2/4/8 DOUBLES.

*/

#include "Task_20.h"

int Task_20::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[Context for vector data intrinsics.]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	__m128i a{ -1,-1,-1, -1,-1,-1,-1,-1,9,10,11,12,13,14,15,16 };
	hexPrint128x8(&a);
	hexPrint128x16(&a);
	hexPrint128x32(&a);
	hexPrint128x64(&a);
	decimalPrint128x8(&a);
	decimalPrint128x16(&a);
	decimalPrint128x32(&a);
	decimalPrint128x64(&a);
	__m128 b{ 1.0f, -1.0f, 15.5f, 17.6f };
	floatPrint128x32(&b);
	__m128d c{-1.0, 1.0};
	doublePrint128x64(&c);

	AppLib::write("\r\n");
	__m256i d{ 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16, -1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15,-16 };
	hexPrint256x8(&d);
	hexPrint256x8(&d, 32);
	hexPrint256x8(&d, 32, APPCONST::TABLE_COLOR);
	hexPrint256x16(&d);
	hexPrint256x32(&d);
	hexPrint256x64(&d);
	decimalPrint256x8(&d);
	decimalPrint256x16(&d);
	decimalPrint256x32(&d);
	decimalPrint256x64(&d);
	__m256 e{ 1.0f, -1.0f, 15.5f, 17.6f, 10.0f, 20.0f, 30.1f, 40.1f };
	floatPrint256x32(&e);
	__m256d f{ -1.0, 1.0, 10.5, -20.5 };
	doublePrint256x64(&f);

	AppLib::write("\r\n");
	__m512i g{ 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16, -1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15,-16, 9 };
	hexPrint512x8(&g, 32, APPCONST::GROUP_COLOR);
	hexPrint512x16(&g);
	hexPrint512x32(&g);
	hexPrint512x64(&g);
	decimalPrint512x8(&g);
	decimalPrint512x16(&g);
	decimalPrint512x32(&g);
	decimalPrint512x64(&g);
	__m512 h{ 1.0f, -1.0f, 15.5f, 17.6f, 10.0f, 20.0f, 30.1f, 40.1f, 3.33f };
	floatPrint512x32(&h);
	__m512d i{ -1.0, 1.0, 10.5, -20.5, 3.34 };
	doublePrint512x64(&i);

	return exitCode;
}

void Task_20::hexPrint128x8(const __m128i* data, int columns, WORD color)
{
	helperHexBytes(reinterpret_cast<const BYTE*>(data), 16, columns, color);
}
void Task_20::hexPrint128x16(const __m128i* data, int columns, WORD color)
{
	helperHexWords(reinterpret_cast<const WORD*>(data), 8, columns, color);
}
void Task_20::hexPrint128x32(const __m128i* data, int columns, WORD color)
{
	helperHexDwords(reinterpret_cast<const DWORD*>(data), 4, columns, color);
}
void Task_20::hexPrint128x64(const __m128i* data, int columns, WORD color)
{
	helperHexQwords(reinterpret_cast<const DWORD64*>(data), 2, columns, color);
}
void Task_20::decimalPrint128x8(const __m128i* data, int columns, WORD color)
{
	helperDecimalBytes(reinterpret_cast<const BYTE*>(data), 16, columns, color);
}
void Task_20::decimalPrint128x16(const __m128i* data, int columns, WORD color)
{
	helperDecimalWords(reinterpret_cast<const WORD*>(data), 8, columns, color);
}
void Task_20::decimalPrint128x32(const __m128i* data, int columns, WORD color)
{
	helperDecimalDwords(reinterpret_cast<const DWORD*>(data), 4, columns, color);
}
void Task_20::decimalPrint128x64(const __m128i* data, int columns, WORD color)
{
	helperDecimalQwords(reinterpret_cast<const DWORD64*>(data), 2, columns, color);
}
void Task_20::floatPrint128x32(const __m128* data, int columns, WORD color)
{
	helperFloat(reinterpret_cast<const float*>(data), 4, columns, color);
}
void Task_20::doublePrint128x64(const __m128d* data, int columns, WORD color)
{
	helperDouble(reinterpret_cast<const double*>(data), 2, columns, color);
}

void Task_20::hexPrint256x8(const __m256i* data, int columns, WORD color)
{
	helperHexBytes(reinterpret_cast<const BYTE*>(data), 32, columns, color);
}
void Task_20::hexPrint256x16(const __m256i* data, int columns, WORD color)
{
	helperHexWords(reinterpret_cast<const WORD*>(data), 16, columns, color);
}
void Task_20::hexPrint256x32(const __m256i* data, int columns, WORD color)
{
	helperHexDwords(reinterpret_cast<const DWORD*>(data), 8, columns, color);
}
void Task_20::hexPrint256x64(const __m256i* data, int columns, WORD color)
{
	helperHexQwords(reinterpret_cast<const DWORD64*>(data), 4, columns, color);
}
void Task_20::decimalPrint256x8(const __m256i* data, int columns, WORD color)
{
	helperDecimalBytes(reinterpret_cast<const BYTE*>(data), 32, columns, color);
}
void Task_20::decimalPrint256x16(const __m256i* data, int columns, WORD color)
{
	helperDecimalWords(reinterpret_cast<const WORD*>(data), 16, columns, color);
}
void Task_20::decimalPrint256x32(const __m256i* data, int columns, WORD color)
{
	helperDecimalDwords(reinterpret_cast<const DWORD*>(data), 8, columns, color);
}
void Task_20::decimalPrint256x64(const __m256i* data, int columns, WORD color)
{
	helperDecimalQwords(reinterpret_cast<const DWORD64*>(data), 4, columns, color);
}
void Task_20::floatPrint256x32(const __m256* data, int columns, WORD color)
{
	helperFloat(reinterpret_cast<const float*>(data), 8, columns, color);
}
void Task_20::doublePrint256x64(const __m256d* data, int columns, WORD color)
{
	helperDouble(reinterpret_cast<const double*>(data), 4, columns, color);
}

void Task_20::hexPrint512x8(const __m512i* data, int columns, WORD color)
{
	helperHexBytes(reinterpret_cast<const BYTE*>(data), 64, columns, color);
}
void Task_20::hexPrint512x16(const __m512i* data, int columns, WORD color)
{
	helperHexWords(reinterpret_cast<const WORD*>(data), 32, columns, color);
}
void Task_20::hexPrint512x32(const __m512i* data, int columns, WORD color)
{
	helperHexDwords(reinterpret_cast<const DWORD*>(data), 16, columns, color);
}
void Task_20::hexPrint512x64(const __m512i* data, int columns, WORD color)
{
	helperHexQwords(reinterpret_cast<const DWORD64*>(data), 8, columns, color);
}
void Task_20::decimalPrint512x8(const __m512i* data, int columns, WORD color)
{
	helperDecimalBytes(reinterpret_cast<const BYTE*>(data), 64, columns, color);
}
void Task_20::decimalPrint512x16(const __m512i* data, int columns, WORD color)
{
	helperDecimalWords(reinterpret_cast<const WORD*>(data), 32, columns, color);
}
void Task_20::decimalPrint512x32(const __m512i* data, int columns, WORD color)
{
	helperDecimalDwords(reinterpret_cast<const DWORD*>(data), 16, columns, color);
}
void Task_20::decimalPrint512x64(const __m512i* data, int columns, WORD color)
{
	helperDecimalQwords(reinterpret_cast<const DWORD64*>(data), 2, columns, color);
}
void Task_20::floatPrint512x32(const __m512* data, int columns, WORD color)
{
	helperFloat(reinterpret_cast<const float*>(data), 16, columns, color);
}
void Task_20::doublePrint512x64(const __m512d* data, int columns, WORD color)
{
	helperDouble(reinterpret_cast<const double*>(data), 8, columns, color);
}

void Task_20::helperHexBytes(const BYTE* p, unsigned int n, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = n - 1; i >= 0; i--)
	{
		BYTE a = p[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%02X", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_20::helperHexWords(const WORD* p, unsigned int n, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = n - 1; i >= 0; i--)
	{
		WORD a = p[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%04X", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_20::helperHexDwords(const DWORD* p, unsigned int n, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = n - 1; i >= 0; i--)
	{
		DWORD a = p[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%08X", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_20::helperHexQwords(const DWORD64* p, unsigned int n, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = n - 1; i >= 0; i--)
	{
		DWORD64 a = p[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%016llX", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_20::helperDecimalBytes(const BYTE* p, unsigned int n, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = n - 1; i >= 0; i--)
	{
		int a = static_cast<int>(static_cast<char>(p[i]));
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%4d", a);
		AppLib::writeColor(msg, color);
		if (i % 16)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_20::helperDecimalWords(const WORD* p, unsigned int n, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = n - 1; i >= 0; i--)
	{
		int a = static_cast<int>(static_cast<short>(p[i]));
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%6d", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_20::helperDecimalDwords(const DWORD* p, unsigned int n, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = n - 1; i >= 0; i--)
	{
		int a = static_cast<int>(static_cast<int>(p[i]));
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%11d", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_20::helperDecimalQwords(const DWORD64* p, unsigned int n, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = n - 1; i >= 0; i--)
	{
		long long a = static_cast<long long>(static_cast<long long>(p[i]));
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%21lld", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_20::helperFloat(const float* p, unsigned int n, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = n - 1; i >= 0; i--)
	{
		float a = p[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%7.1f", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_20::helperDouble(const double* p, unsigned int n, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = n - 1; i >= 0; i--)
	{
		double a = p[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%7.1f", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
