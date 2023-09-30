/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avx512techs=AVX512_BITALG
*/

#include "Task_31.h"

int Task_31::execute(int argc, char** argv)
{
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[AVX512 BITALG]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	int exitCode = 0;
	int selector = 23;  // -1;

	if ((selector == 0) || (selector == -1))
	{	// vpshufbitqmb k, xmm, xmm
		__m128i b;
		__m128i c;
		b.m128i_i64[0] = 0x00000000000000FF;   // 64-bit array.
		b.m128i_i64[1] = 0xAA55555555555555;   // 64-bit array.
		c.m128i_i64[0] = 0x3F00000000000101;   // b bytes-selectors.
		c.m128i_i64[1] = 0x3F00000000000101;   // b bytes-selectors.
		hexPrint128x64(&b);
		hexPrint128x64(&c);
		__mmask16 m = _mm_bitshuffle_epi64_mask(b, c);    // Each of two bytes of mask16 = bitmap of array bits, selected by selectors.
		hexPrintWord(&m);
		AppLib::write("\r\n");
	}
	if ((selector == 1) || (selector == -1))
	{	// vpshufbitqmb k {k}, xmm, xmm
		__m128i b;
		__m128i c;
		b.m128i_i64[0] = 0x00000000000000FF;   // 64-bit array.
		b.m128i_i64[1] = 0xAA55555555555555;   // 64-bit array.
		c.m128i_i64[0] = 0x3F00000000000101;   // b bytes-selectors.
		c.m128i_i64[1] = 0x3F00000000000101;   // b bytes-selectors.
		__mmask16 k = 0x0FFF;                  // Mask for zeroing result bits.
		hexPrint128x64(&b);
		hexPrint128x64(&c);
		hexPrintWord(&k);
		__mmask16 m = _mm_mask_bitshuffle_epi64_mask(k, b, c);    // Each of two bytes of mask16 = bitmap of array bits, selected by selectors.
		hexPrintWord(&m);
		AppLib::write("\r\n");
	}
	if ((selector == 2) || (selector == -1))
	{	// vpshufbitqmb k, ymm, ymm
		__m256i b;
		__m256i c;
		b.m256i_i64[0] = 0x00000000000000FF;   // 64-bit array.
		b.m256i_i64[1] = 0xAA55555555555555;   // 64-bit array.
		b.m256i_i64[2] = 0x0000000000000003;   // 64-bit array.
		b.m256i_i64[3] = 0x000000000000000C;   // 64-bit array.
		c.m256i_i64[0] = 0x3F00000000000101;   // b bytes-selectors.
		c.m256i_i64[1] = 0x3F00000000000101;   // b bytes-selectors.
		c.m256i_i64[2] = 0x0000000000000200;   // b bytes-selectors.
		c.m256i_i64[3] = 0x0000000003020100;   // b bytes-selectors.
		hexPrint256x64(&b);
		hexPrint256x64(&c);
		__mmask32 m = _mm256_bitshuffle_epi64_mask(b, c);    // Each of four bytes of mask32 = bitmap of array bits, selected by selectors.
		hexPrintDword(reinterpret_cast<DWORD*>(&m));
		AppLib::write("\r\n");
	}
	if ((selector == 3) || (selector == -1))
	{	// vpshufbitqmb k {k}, ymm, ymm
		__m256i b;
		__m256i c;
		b.m256i_i64[0] = 0x00000000000000FF;   // 64-bit array.
		b.m256i_i64[1] = 0xAA55555555555555;   // 64-bit array.
		b.m256i_i64[2] = 0x0000000000000003;   // 64-bit array.
		b.m256i_i64[3] = 0x000000000000000C;   // 64-bit array.
		c.m256i_i64[0] = 0x3F00000000000101;   // b bytes-selectors.
		c.m256i_i64[1] = 0x3F00000000000101;   // b bytes-selectors.
		c.m256i_i64[2] = 0x0000000000000200;   // b bytes-selectors.
		c.m256i_i64[3] = 0x0000000003020100;   // b bytes-selectors.
		__mmask32 k = 0xFF00FFFF;              // Mask for zeroing result bits.
		hexPrint256x64(&b);
		hexPrint256x64(&c);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__mmask32 m = _mm256_mask_bitshuffle_epi64_mask(k, b, c);    // Each of four bytes of mask32 = bitmap of array bits, selected by selectors.
		hexPrintDword(reinterpret_cast<DWORD*>(&m));
		AppLib::write("\r\n");
	}
	if ((selector == 4) || (selector == -1))
	{	// vpshufbitqmb k, zmm, zmm
		__m512i b;
		__m512i c;
		for (int i = 0; i < 8; i++)
		{
			b.m512i_i64[i] = static_cast<DWORD64>(-1) << i;
			c.m512i_i64[i] = 0x0001020304050607;
		}
		hexPrint512x64(&b);
		hexPrint512x64(&c);
		__mmask64 m = _mm512_bitshuffle_epi64_mask(b, c);
		hexPrintQword(reinterpret_cast<DWORD64*>(&m));
		AppLib::write("\r\n");
	}
	if ((selector == 5) || (selector == -1))
	{	// vpshufbitqmb k {k}, zmm, zmm
		__m512i b;
		__m512i c;
		for (int i = 0; i < 8; i++)
		{
			b.m512i_i64[i] = static_cast<DWORD64>(-1) << i;
			c.m512i_i64[i] = 0x0001020304050607;
		}
		__mmask64 k = 0xFF00FFFFFFFFFF00;
		hexPrint512x64(&b);
		hexPrint512x64(&c);
		__mmask64 m = _mm512_mask_bitshuffle_epi64_mask(k, b, c);
		hexPrintQword(reinterpret_cast<DWORD64*>(&m));
		AppLib::write("\r\n");
	}
	if ((selector == 6) || (selector == -1))
	{	// vpopcntw xmm {k}, xmm
		__m128i a;
		__m128i src;
		__mmask8 k = 0xBF;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = i;
			src.m128i_i16[i] = 0x1111;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&src);
		hexPrintByte(&k);
		__m128i dst = _mm_mask_popcnt_epi16(src, k, a);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 7) || (selector == -1))
	{	// vpopcntw xmm {z}, xmm
		__m128i a;
		__mmask8 k = 0x8F;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = i;
		}
		hexPrint128x16(&a);
		hexPrintByte(&k);
		__m128i dst = _mm_maskz_popcnt_epi16(k, a);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 8) || (selector == -1))
	{	// vpopcntw xmm, xmm
		__m128i a;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = i+10;
		}
		hexPrint128x16(&a);
		__m128i dst = _mm_popcnt_epi16(a);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 9) || (selector == -1))
	{	// vpopcntw ymm {k}, ymm
		__m256i a;
		__m256i src;
		__mmask16 k = 0xBFFF;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = i;
			src.m256i_i16[i] = 0x1111;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&src);
		hexPrintWord(&k);
		__m256i dst = _mm256_mask_popcnt_epi16(src, k, a);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 10) || (selector == -1))
	{	// vpopcntw ymm {z}, ymm
		__m256i a;
		__mmask16 k = 0xBFFF;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = i;
		}
		hexPrint256x16(&a);
		hexPrintWord(&k);
		__m256i dst = _mm256_maskz_popcnt_epi16(k, a);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 11) || (selector == -1))
	{	// vpopcntw ymm, ymm
		__m256i a;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = i;
		}
		hexPrint256x16(&a);
		__m256i dst = _mm256_popcnt_epi16(a);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 12) || (selector == -1))
	{	// vpopcntw zmm {k}, zmm
		__m512i a;
		__m512i src;
		__mmask32 k = 0xAFFFFFFF;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = i;
			src.m512i_i16[i] = 0x1111;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&src);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_mask_popcnt_epi16(src, k, a);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 13) || (selector == -1))
	{	// vpopcntw zmm {z}, zmm
		__m512i a;
		__mmask32 k = 0xAFFFFFFF;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = i;
		}
		hexPrint512x16(&a);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_maskz_popcnt_epi16(k, a);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 14) || (selector == -1))
	{	// vpopcntw zmm, zmm
		__m512i a;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = i;
		}
		hexPrint512x16(&a);
		__m512i dst = _mm512_popcnt_epi16(a);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 15) || (selector == -1))
	{	// vpopcntb xmm {k}, xmm
		__m128i a;
		__m128i src;
		__mmask16 k = 0xAFFF;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i;
			src.m128i_i8[i] = 0x11;
		}
		hexPrint128x8(&a);
		hexPrint128x8(&src);
		hexPrintWord(&k);
		__m128i dst = _mm_mask_popcnt_epi8(src, k, a);
		hexPrint128x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 16) || (selector == -1))
	{	// vpopcntb xmm {z}, xmm
		__m128i a;
		__mmask16 k = 0xAFFF;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i;
		}
		hexPrint128x8(&a);
		hexPrintWord(&k);
		__m128i dst = _mm_maskz_popcnt_epi8(k, a);
		hexPrint128x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 17) || (selector == -1))
	{	// vpopcntb xmm, xmm
		__m128i a;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i;
		}
		hexPrint128x8(&a);
		__m128i dst = _mm_popcnt_epi8(a);
		hexPrint128x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 18) || (selector == -1))
	{	// vpopcntb ymm {k}, ymm
		__m256i a;
		__m256i src;
		__mmask32 k = 0xAFFFFFFF;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i;
			src.m256i_i8[i] = 0x11;
		}
		hexPrint256x8(&a);
		hexPrint256x8(&src);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_mask_popcnt_epi8(src, k, a);
		hexPrint256x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 19) || (selector == -1))
	{	// vpopcntb ymm {z}, ymm
		__m256i a;
		__mmask32 k = 0xAFFFFFFF;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i;
		}
		hexPrint256x8(&a);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_maskz_popcnt_epi8(k, a);
		hexPrint256x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 20) || (selector == -1))
	{	// vpopcntb ymm, ymm
		__m256i a;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i;
		}
		hexPrint256x8(&a);
		__m256i dst = _mm256_popcnt_epi8(a);
		hexPrint256x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 21) || (selector == -1))
	{	// vpopcntb zmm {k}, zmm
		__m512i a;
		__m512i src;
		__mmask64 k = 0xAFFFFFFFFFFFFFFF;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i;
			src.m512i_i8[i] = 0x11;
		}
		hexPrint512x8(&a);
		hexPrint512x8(&src);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_mask_popcnt_epi8(src, k, a);
		hexPrint512x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 22) || (selector == -1))
	{	// vpopcntb zmm {z}, zmm
		__m512i a;
		__mmask64 k = 0xAFFFFFFFFFFFFFFF;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i;
		}
		hexPrint512x8(&a);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_maskz_popcnt_epi8(k, a);
		hexPrint512x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 23) || (selector == -1))
	{	// vpopcntb zmm, zmm
		__m512i a;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i;
		}
		hexPrint512x8(&a);
		__m512i dst = _mm512_popcnt_epi8(a);
		hexPrint512x8(&dst);
		AppLib::write("\r\n");
	}

	return exitCode;
}
