/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avx512techs=AVX512_VBMI2&ig_expand=1317
*/

#include "Task_30.h"

int Task_30::execute(int argc, char** argv)
{
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[AVX512 VBMI2]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	int exitCode = 0;
	int selector = 149;  // -1;

	if ((selector == 0) || (selector == -1))
	{	// vpcompressw xmm {k}, xmm
		__m128i a;
		__m128i src;
		__mmask8 k = 0xB3;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0331 + i;
			src.m128i_i16[i] = 0x1111;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&src);
		hexPrintByte(&k);
		__m128i dst = _mm_mask_compress_epi16(src, k, a);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 1) || (selector == -1))
	{	// vpcompressw xmm {z}, xmm
		__m128i a;
		__mmask8 k = 0xB3;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0331 + i;
		}
		hexPrint128x16(&a);
		hexPrintByte(&k);
		__m128i dst = _mm_maskz_compress_epi16(k, a);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 2) || (selector == -1))
	{	// vpcompressw ymm {k}, ymm
		__m256i a;
		__m256i src;
		__mmask16 k = 0x8055;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0301 + i;
			src.m256i_i16[i] = 0x1111;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&src);
		hexPrintWord(&k);
		__m256i dst = _mm256_mask_compress_epi16(src, k, a);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 3) || (selector == -1))
	{	// vpcompressw ymm {z}, ymm
		__m256i a;
		__mmask16 k = 0x8055;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0301 + i;
		}
		hexPrint256x16(&a);
		hexPrintWord(&k);
		__m256i dst = _mm256_maskz_compress_epi16(k, a);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 4) || (selector == -1))
	{	// vpcompressw zmm {k}, zmm
		__m512i a;
		__m512i src;
		__mmask32 k = 0xC0008055;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 0x0301 + i;
			src.m512i_i16[i] = 0x1111;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&src);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_mask_compress_epi16(src, k, a);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 5) || (selector == -1))
	{	// vpcompressw zmm {z}, zmm
		__m512i a;
		__mmask32 k = 0xC0008055;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 0x0301 + i;
		}
		hexPrint512x16(&a);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_maskz_compress_epi16(k, a);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 6) || (selector == -1))
	{	// vpcompressb xmm {k}, xmm
		__m128i a;
		__m128i src;
		__mmask16 k = 0x80B3;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = 0x0A0 + i;
			src.m128i_i8[i] = 0x11;
		}
		hexPrint128x8(&a);
		hexPrint128x8(&src);
		hexPrintWord(&k);
		__m128i dst = _mm_mask_compress_epi8(src, k, a);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 7) || (selector == -1))
	{	// vpcompressb xmm {z}, xmm
		__m128i a;
		__mmask16 k = 0x80B3;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = 0x0A0 + i;
		}
		hexPrint128x8(&a);
		hexPrintWord(&k);
		__m128i dst = _mm_maskz_compress_epi8(k, a);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");

	}
	if ((selector == 8) || (selector == -1))
	{	// vpcompressb ymm {k}, ymm
		__m256i a;
		__m256i src;
		__mmask32 k = 0xE000C00F;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = 0x0A0 + i;
			src.m256i_i8[i] = 0x11;
		}
		hexPrint256x8(&a);
		hexPrint256x8(&src);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_mask_compress_epi8(src, k, a);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 9) || (selector == -1))
	{	// vpcompressb ymm {z}, ymm
		__m256i a;
		__mmask32 k = 0x80000001;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = 0x0A0 + i;
		}
		hexPrint256x8(&a);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_maskz_compress_epi8(k, a);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 10) || (selector == -1))
	{	// vpcompressb zmm {k}, zmm
		__m512i a;
		__m512i src;
		__mmask64 k = 0xC000000000000003;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = 0x0A0 + i;
			src.m512i_i8[i] = 0x11;
		}
		hexPrint512x8(&a);
		hexPrint512x8(&src);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_mask_compress_epi8(src, k, a);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 11) || (selector == -1))
	{	// vpcompressb zmm {z}, zmm
		__m512i a;
		__mmask64 k = 0xE000000000000005;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = 0x0A0 + i;
		}
		hexPrint512x8(&a);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_maskz_compress_epi8(k, a);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 12) || (selector == -1))
	{	// vpcompressw m128 {k}, xmm
		__m128i a;
		__m128i dst;
		__mmask8 k = 0x81;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0331 + i;
			dst.m128i_i16[i] = 0x1111;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&dst);
		hexPrintByte(&k);
		_mm_mask_compressstoreu_epi16(&dst, k, a);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 13) || (selector == -1))
	{	// vpcompressw m256 {k}, ymm
		__m256i a;
		__m256i dst;
		__mmask16 k = 0x8001;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0331 + i;
			dst.m256i_i16[i] = 0x1111;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&dst);
		hexPrintWord(&k);
		_mm256_mask_compressstoreu_epi16(&dst, k, a);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 14) || (selector == -1))
	{	// vpcompressw m512 {k}, zmm
		__m512i a;
		__m512i dst;
		__mmask32 k = 0x80000001;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 0x0331 + i;
			dst.m512i_i16[i] = 0x1111;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&dst);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		_mm512_mask_compressstoreu_epi16(&dst, k, a);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 15) || (selector == -1))
	{	// vpcompressb m128 {k}, xmm
		__m128i a;
		__m128i dst;
		__mmask16 k = 0x8001;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = 0x90 + i;
			dst.m128i_i8[i] = 0x11;
		}
		hexPrint128x8(&a);
		hexPrint128x8(&dst);
		hexPrintWord(&k);
		_mm_mask_compressstoreu_epi8(&dst, k, a);
		hexPrint128x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 16) || (selector == -1))
	{	// vpcompressb m256 {k}, ymm
		__m256i a;
		__m256i dst;
		__mmask32 k = 0x80000001;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = 0x90 + i;
			dst.m256i_i8[i] = 0x11;
		}
		hexPrint256x8(&a);
		hexPrint256x8(&dst);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		_mm256_mask_compressstoreu_epi8(&dst, k, a);
		hexPrint256x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 17) || (selector == -1))
	{	// vpcompressb m512 {k}, zmm
		__m512i a;
		__m512i dst;
		__mmask64 k = 0x8000000000000001;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = 0x90 + i;
			dst.m512i_i8[i] = 0x11;
		}
		hexPrint512x8(&a);
		hexPrint512x8(&dst);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		_mm512_mask_compressstoreu_epi8(&dst, k, a);
		hexPrint512x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 18) || (selector == -1))
	{	// vpexpandw xmm {k}, xmm
		__m128i a;
		__m128i src;
		__mmask8 k = 0x86;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0331 + i;
			src.m128i_i16[i] = 0x1111;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&src);
		hexPrintByte(&k);
		__m128i dst = _mm_mask_expand_epi16(src, k, a);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 19) || (selector == -1))
	{	// vpexpandw xmm {z}, xmm
		__m128i a;
		__mmask8 k = 0x86;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0331 + i;
		}
		hexPrint128x16(&a);
		hexPrintByte(&k);
		__m128i dst = _mm_maskz_expand_epi16(k, a);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 20) || (selector == -1))
	{	// vpexpandw ymm {k}, ymm
		__m256i a;
		__m256i src;
		__mmask16 k = 0x8006;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0331 + i;
			src.m256i_i16[i] = 0x1111;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&src);
		hexPrintWord(&k);
		__m256i dst = _mm256_mask_expand_epi16(src, k, a);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 21) || (selector == -1))
	{	// vpexpandw ymm {z}, ymm
		__m256i a;
		__mmask16 k = 0x8006;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0331 + i;
		}
		hexPrint256x16(&a);
		hexPrintWord(&k);
		__m256i dst = _mm256_maskz_expand_epi16(k, a);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 22) || (selector == -1))
	{	// vpexpandw zmm {k}, zmm
		__m512i a;
		__m512i src;
		__mmask32 k = 0x80000006;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 0x0331 + i;
			src.m512i_i16[i] = 0x1111;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&src);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_mask_expand_epi16(src, k, a);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 23) || (selector == -1))
	{	// vpexpandw zmm {z}, zmm
		__m512i a;
		__mmask32 k = 0x80000006;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 0x0331 + i;
		}
		hexPrint512x16(&a);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_maskz_expand_epi16(k, a);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 24) || (selector == -1))
	{	// vpexpandb xmm {k}, xmm
		__m128i a;
		__m128i src;
		__mmask16 k = 0xC007;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = 0x0D1 + i;
			src.m128i_i8[i] = 0x11;
		}
		hexPrint128x8(&a);
		hexPrint128x8(&src);
		hexPrintWord(&k);
		__m128i dst = _mm_mask_expand_epi8(src, k, a);
		hexPrint128x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 25) || (selector == -1))
	{	// vpexpandb xmm {z}, xmm
		__m128i a;
		__mmask16 k = 0xC007;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = 0x0D1 + i;
		}
		hexPrint128x8(&a);
		hexPrintWord(&k);
		__m128i dst = _mm_maskz_expand_epi8(k, a);
		hexPrint128x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 26) || (selector == -1))
	{	// vpexpandb ymm {k}, ymm
		__m256i a;
		__m256i src;
		__mmask32 k = 0xE0000003;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = 0x0D1 + i;
			src.m256i_i8[i] = 0x11;
		}
		hexPrint256x8(&a);
		hexPrint256x8(&src);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_mask_expand_epi8(src, k, a);
		hexPrint256x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 27) || (selector == -1))
	{	// vpexpandb ymm {z}, ymm
		__m256i a;
		__mmask32 k = 0xE0000003;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = 0x0D1 + i;
		}
		hexPrint256x8(&a);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_maskz_expand_epi8(k, a);
		hexPrint256x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 28) || (selector == -1))
	{	// vpexpandb zmm {k}, zmm
		__m512i a;
		__m512i src;
		__mmask64 k = 0xE000000000000003;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = 0x0C1 + i;
			src.m512i_i8[i] = 0x11;
		}
		hexPrint512x8(&a);
		hexPrint512x8(&src);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_mask_expand_epi8(src, k, a);
		hexPrint512x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 29) || (selector == -1))
	{	// vpexpandb zmm {z}, zmm
		__m512i a;
		__mmask64 k = 0xE000000000000003;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = 0x0C1 + i;
		}
		hexPrint512x8(&a);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_maskz_expand_epi8(k, a);
		hexPrint512x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 30) || (selector == -1))
	{	// vpexpandw xmm {k}, m128
		__m128i mem;
		__m128i src;
		__mmask8 k = 0x81;
		for (int i = 0; i < 8; i++)
		{
			mem.m128i_i16[i] = 0x0331 + i;
			src.m128i_i16[i] = 0x1111;
		}
		hexPrint128x16(&src);
		hexPrint128x16(&mem);
		hexPrintByte(&k);
		__m128i dst = _mm_mask_expandloadu_epi16(src, k, &mem);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 31) || (selector == -1))
	{	// vpexpandw xmm {z}, m128
		__m128i mem;
		__mmask8 k = 0x81;
		for (int i = 0; i < 8; i++)
		{
			mem.m128i_i16[i] = 0x0331 + i;
		}
		hexPrint128x16(&mem);
		hexPrintByte(&k);
		__m128i dst = _mm_maskz_expandloadu_epi16(k, &mem);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 32) || (selector == -1))
	{	// vpexpandw ymm {k}, m256
		__m256i mem;
		__m256i src;
		__mmask16 k = 0x0801;
		for (int i = 0; i < 16; i++)
		{
			mem.m256i_i16[i] = 0x0331 + i;
			src.m256i_i16[i] = 0x1111;
		}
		hexPrint256x16(&src);
		hexPrint256x16(&mem);
		hexPrintWord(&k);
		__m256i dst = _mm256_mask_expandloadu_epi16(src, k, &mem);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 33) || (selector == -1))
	{	// vpexpandw ymm {z}, m256
		__m256i mem;
		__mmask16 k = 0x0801;
		for (int i = 0; i < 16; i++)
		{
			mem.m256i_i16[i] = 0x0331 + i;
		}
		hexPrint256x16(&mem);
		hexPrintWord(&k);
		__m256i dst = _mm256_maskz_expandloadu_epi16(k, &mem);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 34) || (selector == -1))
	{	// vpexpandw zmm {k}, m512
		__m512i mem;
		__m512i src;
		__mmask32 k = 0xF0000001;
		for (int i = 0; i < 32; i++)
		{
			mem.m512i_i16[i] = 0x0331 + i;
			src.m512i_i16[i] = 0x1111;
		}
		hexPrint512x16(&src);
		hexPrint512x16(&mem);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_mask_expandloadu_epi16(src, k, &mem);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 35) || (selector == -1))
	{	// vpexpandw zmm {z}, m512
		__m512i mem;
		__mmask32 k = 0xF0000001;
		for (int i = 0; i < 32; i++)
		{
			mem.m512i_i16[i] = 0x0331 + i;
		}
		hexPrint512x16(&mem);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_maskz_expandloadu_epi16(k, &mem);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 36) || (selector == -1))
	{	// vpexpandb xmm {k}, m128
		__m128i mem;
		__m128i src;
		__mmask16 k = 0xA001;
		for (int i = 0; i < 16; i++)
		{
			mem.m128i_i8[i] = 0x31 + i;
			src.m128i_i8[i] = 0x11;
		}
		hexPrint128x8(&src);
		hexPrint128x8(&mem);
		hexPrintWord(&k);
		__m128i dst = _mm_mask_expandloadu_epi8(src, k, &mem);
		hexPrint128x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 37) || (selector == -1))
	{	// vpexpandb xmm {z}, m128
		__m128i mem;
		__mmask16 k = 0xA001;
		for (int i = 0; i < 16; i++)
		{
			mem.m128i_i8[i] = 0x31 + i;
		}
		hexPrint128x8(&mem);
		hexPrintWord(&k);
		__m128i dst = _mm_maskz_expandloadu_epi8(k, &mem);
		hexPrint128x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 38) || (selector == -1))
	{	// vpexpandb ymm {k}, m256
		__m256i mem;
		__m256i src;
		__mmask32 k = 0xA0000007;
		for (int i = 0; i < 32; i++)
		{
			mem.m256i_i8[i] = 0x31 + i;
			src.m256i_i8[i] = 0x11;
		}
		hexPrint256x8(&src);
		hexPrint256x8(&mem);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_mask_expandloadu_epi8(src, k, &mem);
		hexPrint256x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 39) || (selector == -1))
	{	// vpexpandb ymm {z}, m256
		__m256i mem;
		__mmask32 k = 0xA0000007;
		for (int i = 0; i < 32; i++)
		{
			mem.m256i_i8[i] = 0x31 + i;
		}
		hexPrint256x8(&mem);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_maskz_expandloadu_epi8(k, &mem);
		hexPrint256x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 40) || (selector == -1))
	{	// vpexpandb zmm {k}, m512
		__m512i mem;
		__m512i src;
		__mmask64 k = 0xA000000000000007;
		for (int i = 0; i < 64; i++)
		{
			mem.m512i_i8[i] = 0x31 + i;
			src.m512i_i8[i] = 0x11;
		}
		hexPrint512x8(&src);
		hexPrint512x8(&mem);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_mask_expandloadu_epi8(src, k, &mem);
		hexPrint512x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 41) || (selector == -1))
	{	// vpexpandb zmm {z}, m512
		__m512i mem;
		__mmask64 k = 0xA000000000000007;
		for (int i = 0; i < 64; i++)
		{
			mem.m512i_i8[i] = 0x31 + i;
		}
		hexPrint512x8(&mem);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_maskz_expandloadu_epi8(k, &mem);
		hexPrint512x8(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 42) || (selector == -1))
	{	// vpshldw xmm {k}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__m128i src;
		__mmask8 k = 0x3F;
		const int imm8 = 4;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 2 * i;
			b.m128i_i16[i] = static_cast<unsigned short>(0xFFFF);
			src.m128i_i16[i] = 0x1111;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrint128x16(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_mask_shldi_epi16(src, k, a, b, imm8);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 43) || (selector == -1))
	{	// vpshldw xmm {z}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__mmask8 k = 0x3F;
		const int imm8 = 4;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 2 * i;
			b.m128i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_maskz_shldi_epi16(k, a, b, imm8);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 44) || (selector == -1))
	{	// vpshldw xmm, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		const int imm8 = 4;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 2 * i;
			b.m128i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_shldi_epi16(a, b, imm8);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 45) || (selector == -1))
	{	// vpshldw ymm {k}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__m256i src;
		__mmask16 k = 0xFF1F;
		const int imm8 = 4;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 2 * i;
			b.m256i_i16[i] = static_cast<unsigned short>(0xFFFF);
			src.m256i_i16[i] = 0x1111;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrint256x16(&src);
		hexPrintWord(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_mask_shldi_epi16(src, k, a, b, imm8);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 46) || (selector == -1))
	{	// vpshldw ymm {z}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__mmask16 k = 0xFF1F;
		const int imm8 = 4;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 2 * i;
			b.m256i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrintWord(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_maskz_shldi_epi16(k, a, b, imm8);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 47) || (selector == -1))
	{	// vpshldw ymm, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		const int imm8 = 4;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 2 * i;
			b.m256i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_shldi_epi16(a, b, imm8);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 48) || (selector == -1))
	{	// vpshldw zmm {k}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__m512i src;
		__mmask32 k = 0xFF1FFFFF;
		const int imm8 = 4;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 2 * i;
			b.m512i_i16[i] = static_cast<unsigned short>(0xFFFF);
			src.m512i_i16[i] = 0x1111;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrint512x16(&src);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_mask_shldi_epi16(src, k, a, b, imm8);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 49) || (selector == -1))
	{	// vpshldw zmm {z}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__mmask32 k = 0xFF1FFFFF;
		const int imm8 = 4;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 2 * i;
			b.m512i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_maskz_shldi_epi16(k, a, b, imm8);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 50) || (selector == -1))
	{	// vpshldw zmm, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		const int imm8 = 4;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 2 * i;
			b.m512i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_shldi_epi16(a, b, imm8);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 51) || (selector == -1))
	{	// vpshldd xmm {k}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__m128i src;
		__mmask8 k = 0x7;
		const int imm8 = 12;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 2 * i;
			b.m128i_i32[i] = static_cast<unsigned int>(0xFFFFFFFF);
			src.m128i_i32[i] = 0x11111111;
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrint128x32(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_mask_shldi_epi32(src, k, a, b, imm8);
		hexPrint128x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 52) || (selector == -1))
	{	// vpshldd xmm {z}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__mmask8 k = 0x7;
		const int imm8 = 12;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 2 * i;
			b.m128i_i32[i] = static_cast<unsigned int>(0xFFFFFFFF);
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_maskz_shldi_epi32(k, a, b, imm8);
		hexPrint128x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 53) || (selector == -1))
	{	// vpshldd xmm, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		const int imm8 = 12;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 2 * i;
			b.m128i_i32[i] = static_cast<unsigned int>(0xFFFFFFFF);
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_shldi_epi32(a, b, imm8);
		hexPrint128x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 54) || (selector == -1))
	{	// vpshldd ymm {k}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__m256i src;
		__mmask8 k = 0xF3;
		const int imm8 = 12;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 2 * i;
			b.m256i_i32[i] = static_cast<unsigned int>(0xFFFFFFFF);
			src.m256i_i32[i] = 0x11111111;
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrint256x32(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_mask_shldi_epi32(src, k, a, b, imm8);
		hexPrint256x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 55) || (selector == -1))
	{	// vpshldd ymm {z}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__mmask8 k = 0xF3;
		const int imm8 = 12;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 2 * i;
			b.m256i_i32[i] = static_cast<unsigned int>(0xFFFFFFFF);
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_maskz_shldi_epi32(k, a, b, imm8);
		hexPrint256x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 56) || (selector == -1))
	{	// vpshldd ymm, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		const int imm8 = 12;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 2 * i;
			b.m256i_i32[i] = static_cast<unsigned int>(0xFFFFFFFF);
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_shldi_epi32(a, b, imm8);
		hexPrint256x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 57) || (selector == -1))
	{	// vpshldd zmm {k}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__m512i src;
		__mmask16 k = 0xA5FF;
		const int imm8 = 12;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 2 * i;
			b.m512i_i32[i] = static_cast<unsigned int>(0xFFFFFFFF);
			src.m512i_i32[i] = 0x11111111;
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrint512x32(&src);
		hexPrintWord(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_mask_shldi_epi32(src, k, a, b, imm8);
		hexPrint512x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 58) || (selector == -1))
	{	// vpshldd zmm {z}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__mmask16 k = 0xA5FF;
		const int imm8 = 12;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 2 * i;
			b.m512i_i32[i] = static_cast<unsigned int>(0xFFFFFFFF);
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrintWord(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_maskz_shldi_epi32(k, a, b, imm8);
		hexPrint512x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 59) || (selector == -1))
	{	// vpshldd zmm, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		const int imm8 = 12;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 2 * i;
			b.m512i_i32[i] = static_cast<unsigned int>(0xFFFFFFFF);
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_shldi_epi32(a, b, imm8);
		hexPrint512x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 60) || (selector == -1))
	{	// vpshldq xmm {k}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__m128i src;
		__mmask8 k = 0x3;
		const int imm8 = 12;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 2 * i + 0x101;
			b.m128i_i64[i] = static_cast<DWORD64>(-1);
			src.m128i_i64[i] = 0x1111111111111111;
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&src);
		AppLib::write("\r\n");
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_mask_shldi_epi64(src, k, a, b, imm8);
		hexPrint128x64(&dst);
		k = 0x2;
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		dst = _mm_mask_shldi_epi64(src, k, a, b, imm8);
		hexPrint128x64(&dst);
		k = 0x1;
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		dst = _mm_mask_shldi_epi64(src, k, a, b, imm8);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 61) || (selector == -1))
	{	// vpshldq xmm {z}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__mmask8 k = 0x3;
		const int imm8 = 12;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 2 * i + 0x101;
			b.m128i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		AppLib::write("\r\n");
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_maskz_shldi_epi64(k, a, b, imm8);
		hexPrint128x64(&dst);
		k = 0x2;
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		dst = _mm_maskz_shldi_epi64(k, a, b, imm8);
		hexPrint128x64(&dst);
		k = 0x1;
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		dst = _mm_maskz_shldi_epi64(k, a, b, imm8);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 62) || (selector == -1))
	{	// vpshldq xmm, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		const int imm8 = 13;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 2 * i + 0x101;
			b.m128i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_shldi_epi64(a, b, imm8);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 63) || (selector == -1))
	{	// vpshldq ymm {k}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__m256i src;
		__mmask8 k = 0xB;
		const int imm8 = 12;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 2 * i + 0x101;
			b.m256i_i64[i] = static_cast<DWORD64>(-1);
			src.m256i_i64[i] = 0x1111111111111111;
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_mask_shldi_epi64(src, k, a, b, imm8);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 64) || (selector == -1))
	{	// vpshldq ymm {z}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__mmask8 k = 0xB;
		const int imm8 = 12;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 2 * i + 0x101;
			b.m256i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_maskz_shldi_epi64(k, a, b, imm8);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 65) || (selector == -1))
	{	// vpshldq ymm, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		const int imm8 = 12;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 2 * i + 0x101;
			b.m256i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_shldi_epi64(a, b, imm8);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 66) || (selector == -1))
	{	// vpshldq zmm {k}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__m512i src;
		__mmask8 k = 0x5F;
		const int imm8 = 12;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 2 * i + 0x101;
			b.m512i_i64[i] = static_cast<DWORD64>(-1);
			src.m512i_i64[i] = 0x1111111111111111;
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_mask_shldi_epi64(src, k, a, b, imm8);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 67) || (selector == -1))
	{	// vpshldq zmm {z}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__mmask8 k = 0xAF;
		const int imm8 = 12;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 2 * i + 0x101;
			b.m512i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_maskz_shldi_epi64(k, a, b, imm8);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 68) || (selector == -1))
	{	// vpshldq zmm, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		const int imm8 = 12;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 2 * i + 0x101;
			b.m512i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_shldi_epi64(a, b, imm8);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 69) || (selector == -1))
	{	// vpshldvw xmm {k}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0xDF;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0001;
			b.m128i_i16[i] = static_cast<short>(-1);
			c.m128i_i16[i] = i;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrint128x16(&c);
		__m128i dst = _mm_mask_shldv_epi16(a, k, b, c);
		hexPrintByte(&k);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 70) || (selector == -1))
	{	// vpshldvw xmm {z}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0xDF;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0001;
			b.m128i_i16[i] = static_cast<short>(-1);
			c.m128i_i16[i] = i;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrint128x16(&c);
		__m128i dst = _mm_maskz_shldv_epi16(k, a, b, c);
		hexPrintByte(&k);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 71) || (selector == -1))
	{	// vpshldvw xmm, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0001;
			b.m128i_i16[i] = static_cast<short>(-1);
			c.m128i_i16[i] = i * 2;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrint128x16(&c);
		__m128i dst = _mm_shldv_epi16(a, b, c);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 72) || (selector == -1))
	{	// vpshldvw ymm {k}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask16 k = 0xA0FF;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0001;
			b.m256i_i16[i] = static_cast<short>(-1);
			c.m256i_i16[i] = i;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrint256x16(&c);
		__m256i dst = _mm256_mask_shldv_epi16(a, k, b, c);
		hexPrintWord(&k);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 73) || (selector == -1))
	{	// vpshldvw ymm {z}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask16 k = 0x5FFF;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0001;
			b.m256i_i16[i] = static_cast<short>(-1);
			c.m256i_i16[i] = i;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrint256x16(&c);
		__m256i dst = _mm256_maskz_shldv_epi16(k, a, b, c);
		hexPrintWord(&k);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 74) || (selector == -1))
	{	// vpshldvw ymm, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0001;
			b.m256i_i16[i] = static_cast<short>(-1);
			c.m256i_i16[i] = i;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrint256x16(&c);
		__m256i dst = _mm256_shldv_epi16(a, b, c);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 75) || (selector == -1))
	{	// vpshldvw zmm {k}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask32 k = 0xFDFFFFFF;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = i / 16;
			b.m512i_i16[i] = -1;
			c.m512i_i16[i] = i;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrint512x16(&c);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_mask_shldv_epi16(a, k, b, c);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 76) || (selector == -1))
	{	// vpshldvw zmm {z}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask32 k = 0xFDFFFFFF;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = i / 16;
			b.m512i_i16[i] = -1;
			c.m512i_i16[i] = i;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrint512x16(&c);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_maskz_shldv_epi16(k, a, b, c);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 77) || (selector == -1))
	{	// vpshldvw zmm, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = i / 16;
			b.m512i_i16[i] = -1;
			c.m512i_i16[i] = i;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrint512x16(&c);
		__m512i dst = _mm512_shldv_epi16(a, b, c);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 78) || (selector == -1))
	{	// vpshldvd xmm {k}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0x7;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 0x00000001;
			b.m128i_i32[i] = static_cast<short>(-1);
			c.m128i_i32[i] = i * 5;
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrint128x32(&c);
		hexPrintByte(&k);
		__m128i dst = _mm_mask_shldv_epi32(a, k, b, c);
		hexPrint128x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 79) || (selector == -1))
	{	// vpshldvd xmm {z}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0x7;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 0x00000001;
			b.m128i_i32[i] = static_cast<short>(-1);
			c.m128i_i32[i] = i * 5;
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrint128x32(&c);
		hexPrintByte(&k);
		__m128i dst = _mm_maskz_shldv_epi32(k, a, b, c);
		hexPrint128x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 80) || (selector == -1))
	{	// vpshldvd xmm, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 0x00000001;
			b.m128i_i32[i] = static_cast<short>(-1);
			c.m128i_i32[i] = i * 5;
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrint128x32(&c);
		__m128i dst = _mm_shldv_epi32(a, b, c);
		hexPrint128x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 81) || (selector == -1))
	{	// vpshldvd ymm {k}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask8 k = 0xF3;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 0x00000001;
			b.m256i_i32[i] = static_cast<short>(-1);
			c.m256i_i32[i] = i * 4;
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrint256x32(&c);
		hexPrintByte(&k);
		__m256i dst = _mm256_mask_shldv_epi32(a, k, b, c);
		hexPrint256x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 82) || (selector == -1))
	{	// vpshldvd ymm {z}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask8 k = 0x7F;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 0x00000001;
			b.m256i_i32[i] = static_cast<short>(-1);
			c.m256i_i32[i] = i * 4;
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrint256x32(&c);
		hexPrintByte(&k);
		__m256i dst = _mm256_maskz_shldv_epi32(k, a, b, c);
		hexPrint256x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 83) || (selector == -1))
	{	// vpshldvd ymm, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 0x00000001;
			b.m256i_i32[i] = static_cast<short>(-1);
			c.m256i_i32[i] = i * 4;
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrint256x32(&c);
		__m256i dst = _mm256_shldv_epi32(a, b, c);
		hexPrint256x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 84) || (selector == -1))
	{	// vpshldvd zmm {k}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask16 k = 0x5FFF;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 0x00000001;
			b.m512i_i32[i] = static_cast<short>(-1);
			c.m512i_i32[i] = i;
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrint512x32(&c);
		hexPrintWord(&k);
		__m512i dst = _mm512_mask_shldv_epi32(a, k, b, c);
		hexPrint512x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 85) || (selector == -1))
	{	// vpshldvd zmm {z}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask16 k = 0x5FFF;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 0x00000001;
			b.m512i_i32[i] = static_cast<short>(-1);
			c.m512i_i32[i] = i;
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrint512x32(&c);
		hexPrintWord(&k);
		__m512i dst = _mm512_maskz_shldv_epi32(k, a, b, c);
		hexPrint512x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 86) || (selector == -1))
	{	// vpshldvd zmm, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 0x00000001;
			b.m512i_i32[i] = static_cast<short>(-1);
			c.m512i_i32[i] = i;
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrint512x32(&c);
		__m512i dst = _mm512_shldv_epi32(a, b, c);
		hexPrint512x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 87) || (selector == -1))
	{	// vpshldvq xmm {k}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0x3;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 0x00000001;
			b.m128i_i64[i] = static_cast<short>(-1);
			c.m128i_i64[i] = i * 5;
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&c);
		hexPrintByte(&k);
		__m128i dst = _mm_mask_shldv_epi64(a, k, b, c);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 88) || (selector == -1))
	{	// vpshldvq xmm {z}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0x3;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 0x00000001;
			b.m128i_i64[i] = static_cast<short>(-1);
			c.m128i_i64[i] = i * 5;
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&c);
		hexPrintByte(&k);
		__m128i dst = _mm_maskz_shldv_epi64(k, a, b, c);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 89) || (selector == -1))
	{	// vpshldvq xmm, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 0x00000001;
			b.m128i_i64[i] = static_cast<short>(-1);
			c.m128i_i64[i] = i * 5;
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&c);
		__m128i dst = _mm_shldv_epi64(a, b, c);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 90) || (selector == -1))
	{	// vpshldvq ymm {k}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask8 k = 0x7;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 0x00000003;
			b.m256i_i64[i] = static_cast<short>(-1);
			c.m256i_i64[i] = i * 5;
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&c);
		hexPrintByte(&k);
		__m256i dst = _mm256_mask_shldv_epi64(a, k, b, c);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 91) || (selector == -1))
	{	// vpshldvq ymm {z}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask8 k = 0x7;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 0x00000003;
			b.m256i_i64[i] = static_cast<short>(-1);
			c.m256i_i64[i] = i * 5;
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&c);
		hexPrintByte(&k);
		__m256i dst = _mm256_maskz_shldv_epi64(k, a, b, c);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 92) || (selector == -1))
	{	// vpshldvq ymm, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 0x00000003;
			b.m256i_i64[i] = static_cast<short>(-1);
			c.m256i_i64[i] = i * 5;
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&c);
		__m256i dst = _mm256_shldv_epi64(a, b, c);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 93) || (selector == -1))
	{	// vpshldvq zmm {k}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask8 k = 0xC7;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 0x00000003;
			b.m512i_i64[i] = static_cast<short>(-1);
			c.m512i_i64[i] = i * 5;
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&c);
		hexPrintByte(&k);
		__m512i dst = _mm512_mask_shldv_epi64(a, k, b, c);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 94) || (selector == -1))
	{	// vpshldvq zmm {z}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask8 k = 0xC7;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 0x00000003;
			b.m512i_i64[i] = static_cast<short>(-1);
			c.m512i_i64[i] = i * 5;
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&c);
		hexPrintByte(&k);
		__m512i dst = _mm512_maskz_shldv_epi64(k, a, b, c);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 95) || (selector == -1))
	{	// vpshldvq zmm, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 0x00000003;
			b.m512i_i64[i] = static_cast<short>(-1);
			c.m512i_i64[i] = i * 5;
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&c);
		__m512i dst = _mm512_shldv_epi64(a, b, c);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 96) || (selector == -1))
	{	// vpshrdw xmm {k}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__m128i src;
		__mmask8 k = 0x3F;
		const int imm8 = 4;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 16 * i;
			b.m128i_i16[i] = static_cast<unsigned short>(0xFFFF);
			src.m128i_i16[i] = 0x1111;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrint128x16(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_mask_shrdi_epi16(src, k, a, b, imm8);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 97) || (selector == -1))
	{	// vpshrdw xmm {z}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__mmask8 k = 0x3F;
		const int imm8 = 4;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 16 * i;
			b.m128i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_maskz_shrdi_epi16(k, a, b, imm8);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 98) || (selector == -1))
	{	// vpshrdw xmm, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		const int imm8 = 4;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 16 * i;
			b.m128i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_shrdi_epi16(a, b, imm8);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 99) || (selector == -1))
	{	// vpshrdw ymm {k}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__m256i src;
		__mmask16 k = 0x3FFF;
		const int imm8 = 4;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 16 * i;
			b.m256i_i16[i] = static_cast<unsigned short>(0xFFFF);
			src.m256i_i16[i] = 0x1111;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrint256x16(&src);
		hexPrintWord(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_mask_shrdi_epi16(src, k, a, b, imm8);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 100) || (selector == -1))
	{	// vpshrdw ymm {z}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__mmask16 k = 0x3FFF;
		const int imm8 = 4;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 16 * i;
			b.m256i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrintWord(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_maskz_shrdi_epi16(k, a, b, imm8);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 101) || (selector == -1))
	{	// vpshrdw ymm, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		const int imm8 = 4;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 16 * i;
			b.m256i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_shrdi_epi16(a, b, imm8);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 102) || (selector == -1))
	{	// vpshrdw zmm {k}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__m512i src;
		__mmask32 k = 0x3FFFFFFF;
		const int imm8 = 4;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 16 * i;
			b.m512i_i16[i] = static_cast<unsigned short>(0xFFFF);
			src.m512i_i16[i] = 0x1111;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrint512x16(&src);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_mask_shrdi_epi16(src, k, a, b, imm8);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 103) || (selector == -1))
	{	// vpshrdw zmm {z}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__mmask32 k = 0x3FFFFFFF;
		const int imm8 = 4;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 16 * i;
			b.m512i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_maskz_shrdi_epi16(k, a, b, imm8);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 104) || (selector == -1))
	{	// vpshrdw zmm, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		const int imm8 = 4;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 16 * i;
			b.m512i_i16[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_shrdi_epi16(a, b, imm8);
		hexPrint512x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 105) || (selector == -1))
	{	// vpshrdd xmm {k}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__m128i src;
		__mmask8 k = 0xD;
		const int imm8 = 4;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 16 * i;
			b.m128i_i32[i] = static_cast<unsigned short>(0xFFFF);
			src.m128i_i32[i] = 0x11111111;
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrint128x32(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_mask_shrdi_epi32(src, k, a, b, imm8);
		hexPrint128x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 106) || (selector == -1))
	{	// vpshrdd xmm {z}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__mmask8 k = 0xD;
		const int imm8 = 4;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 16 * i;
			b.m128i_i32[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_maskz_shrdi_epi32(k, a, b, imm8);
		hexPrint128x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 107) || (selector == -1))
	{	// vpshrdd xmm, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		const int imm8 = 4;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 16 * i;
			b.m128i_i32[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_shrdi_epi32(a, b, imm8);
		hexPrint128x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 108) || (selector == -1))
	{	// vpshrdd ymm {k}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__m256i src;
		__mmask8 k = 0xDF;
		const int imm8 = 4;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 16 * i;
			b.m256i_i32[i] = static_cast<unsigned short>(0xFFFF);
			src.m256i_i32[i] = 0x11111111;
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrint256x32(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_mask_shrdi_epi32(src, k, a, b, imm8);
		hexPrint256x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 109) || (selector == -1))
	{	// vpshrdd ymm {z}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__mmask8 k = 0xDF;
		const int imm8 = 4;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 16 * i;
			b.m256i_i32[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_maskz_shrdi_epi32(k, a, b, imm8);
		hexPrint256x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 110) || (selector == -1))
	{	// vpshrdd ymm, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		const int imm8 = 4;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 16 * i;
			b.m256i_i32[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_shrdi_epi32(a, b, imm8);
		hexPrint256x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 111) || (selector == -1))
	{	// vpshrdd zmm {k}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__m512i src;
		__mmask16 k = 0xDFFF;
		const int imm8 = 4;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 16 * i;
			b.m512i_i32[i] = static_cast<unsigned short>(0xFFFF);
			src.m512i_i32[i] = 0x11111111;
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrint512x32(&src);
		hexPrintWord(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_mask_shrdi_epi32(src, k, a, b, imm8);
		hexPrint512x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 112) || (selector == -1))
	{	// vpshrdd zmm {z}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__mmask16 k = 0xDFFF;
		const int imm8 = 4;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 16 * i;
			b.m512i_i32[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrintWord(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_maskz_shrdi_epi32(k, a, b, imm8);
		hexPrint512x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 113) || (selector == -1))
	{	// vpshrdd zmm, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		const int imm8 = 4;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 16 * i;
			b.m512i_i32[i] = static_cast<unsigned short>(0xFFFF);
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_shrdi_epi32(a, b, imm8);
		hexPrint512x32(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 114) || (selector == -1))
	{	// vpshrdq xmm {k}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__m128i src;
		__mmask8 k = 0x3;
		const int imm8 = 12;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 4096 * (i + 3);
			b.m128i_i64[i] = static_cast<DWORD64>(-1);
			src.m128i_i64[i] = 0x1111111111111111;
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_mask_shrdi_epi64(src, k, a, b, imm8);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 115) || (selector == -1))
	{	// vpshrdq xmm {z}, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		__mmask8 k = 0x3;
		const int imm8 = 12;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 4096 * (i + 3);
			b.m128i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_maskz_shrdi_epi64(k, a, b, imm8);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 116) || (selector == -1))
	{	// vpshrdq xmm, xmm, xmm, imm8
		__m128i a;
		__m128i b;
		const int imm8 = 12;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 4096 * (i + 3);
			b.m128i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m128i dst = _mm_shrdi_epi64(a, b, imm8);
		hexPrint128x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 117) || (selector == -1))
	{	// vpshrdq ymm {k}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__m256i src;
		__mmask8 k = 0xB;
		const int imm8 = 12;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 4096 * (i + 3);
			b.m256i_i64[i] = static_cast<DWORD64>(-1);
			src.m256i_i64[i] = 0x1111111111111111;
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_mask_shrdi_epi64(src, k, a, b, imm8);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 118) || (selector == -1))
	{	// vpshrdq ymm {z}, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		__mmask8 k = 0xB;
		const int imm8 = 12;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 4096 * (i + 3);
			b.m256i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_maskz_shrdi_epi64(k, a, b, imm8);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 119) || (selector == -1))
	{	// vpshrdq ymm, ymm, ymm, imm8
		__m256i a;
		__m256i b;
		const int imm8 = 12;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 4096 * (i + 3);
			b.m256i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m256i dst = _mm256_shrdi_epi64(a, b, imm8);
		hexPrint256x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 120) || (selector == -1))
	{	// vpshrdq zmm {k}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__m512i src;
		__mmask8 k = 0xFB;
		const int imm8 = 12;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 4096 * (i + 3);
			b.m512i_i64[i] = static_cast<DWORD64>(-1);
			src.m512i_i64[i] = 0x1111111111111111;
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&src);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_mask_shrdi_epi64(src, k, a, b, imm8);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 121) || (selector == -1))
	{	// vpshrdq zmm {z}, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		__mmask8 k = 0xFB;
		const int imm8 = 12;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 4096 * (i + 3);
			b.m512i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrintByte(&k);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_maskz_shrdi_epi64(k, a, b, imm8);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 122) || (selector == -1))
	{	// vpshrdq zmm, zmm, zmm, imm8
		__m512i a;
		__m512i b;
		const int imm8 = 12;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 4096 * (i + 3);
			b.m512i_i64[i] = static_cast<DWORD64>(-1);
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrintDword(reinterpret_cast<const DWORD*>(&imm8));
		__m512i dst = _mm512_shrdi_epi64(a, b, imm8);
		hexPrint512x64(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 123) || (selector == -1))
	{	// vpshrdvw xmm {k}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0xDF;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0007;
			b.m128i_i16[i] = static_cast<short>(-1);
			c.m128i_i16[i] = i;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrint128x16(&c);
		__m128i dst = _mm_mask_shrdv_epi16(a, k, b, c);
		hexPrintByte(&k);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 124) || (selector == -1))
	{	// vpshrdvw xmm {z}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0xDF;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0007;
			b.m128i_i16[i] = static_cast<short>(-1);
			c.m128i_i16[i] = i;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrint128x16(&c);
		__m128i dst = _mm_maskz_shrdv_epi16(k, a, b, c);
		hexPrintByte(&k);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 125) || (selector == -1))
	{	// vpshrdvw xmm, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		for (int i = 0; i < 8; i++)
		{
			a.m128i_i16[i] = 0x0007;
			b.m128i_i16[i] = static_cast<short>(-1);
			c.m128i_i16[i] = i;
		}
		hexPrint128x16(&a);
		hexPrint128x16(&b);
		hexPrint128x16(&c);
		__m128i dst = _mm_shrdv_epi16(a, b, c);
		hexPrint128x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 126) || (selector == -1))
	{	// vpshrdvw ymm {k}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask16 k = 0xDFFF;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0007;
			b.m256i_i16[i] = static_cast<short>(-1);
			c.m256i_i16[i] = i;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrint256x16(&c);
		hexPrintWord(&k);
		__m256i dst = _mm256_mask_shrdv_epi16(a, k, b, c);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 127) || (selector == -1))
	{	// vpshrdvw ymm {z}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask16 k = 0xDFFF;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0007;
			b.m256i_i16[i] = static_cast<short>(-1);
			c.m256i_i16[i] = i;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrint256x16(&c);
		hexPrintWord(&k);
		__m256i dst = _mm256_maskz_shrdv_epi16(k, a, b, c);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 128) || (selector == -1))
	{	// vpshrdvw ymm, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		for (int i = 0; i < 16; i++)
		{
			a.m256i_i16[i] = 0x0007;
			b.m256i_i16[i] = static_cast<short>(-1);
			c.m256i_i16[i] = i;
		}
		hexPrint256x16(&a);
		hexPrint256x16(&b);
		hexPrint256x16(&c);
		__m256i dst = _mm256_shrdv_epi16(a, b, c);
		hexPrint256x16(&dst);
		AppLib::write("\r\n");
	}
	if ((selector == 129) || (selector == -1))
	{	// vpshrdvw zmm {k}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask32 k = 0xDFFFFFFF;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 0x0007;
			b.m512i_i16[i] = i + 0xFFFF;
			c.m512i_i16[i] = i;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrint512x16(&c);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_mask_shrdv_epi16(a, k, b, c);
		AppLib::write("\r\n");
		hexPrint512x16(&dst);
	}
	if ((selector == 130) || (selector == -1))
	{	// vpshrdvw zmm {z}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask32 k = 0xDFFFFFFF;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 0x0007;
			b.m512i_i16[i] = i + 0xFFFF;
			c.m512i_i16[i] = i;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrint512x16(&c);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m512i dst = _mm512_maskz_shrdv_epi16(k, a, b, c);
		AppLib::write("\r\n");
		hexPrint512x16(&dst);
	}
	if ((selector == 131) || (selector == -1))
	{	// vpshrdvw zmm, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		for (int i = 0; i < 32; i++)
		{
			a.m512i_i16[i] = 0x0007;
			b.m512i_i16[i] = i + 0xFFFF;
			c.m512i_i16[i] = i;
		}
		hexPrint512x16(&a);
		hexPrint512x16(&b);
		hexPrint512x16(&c);
		__m512i dst = _mm512_shrdv_epi16(a, b, c);
		AppLib::write("\r\n");
		hexPrint512x16(&dst);
	}
	if ((selector == 132) || (selector == -1))
	{	// vpshrdvd xmm {k}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0xD;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 0x0007;
			b.m128i_i32[i] = static_cast<short>(-1);
			c.m128i_i32[i] = i;
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrint128x32(&c);
		__m128i dst = _mm_mask_shrdv_epi32(a, k, b, c);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		hexPrint128x32(&dst);
	}
	if ((selector == 133) || (selector == -1))
	{	// vpshrdvd xmm {z}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0xD;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 0x0007;
			b.m128i_i32[i] = static_cast<short>(-1);
			c.m128i_i32[i] = i;
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrint128x32(&c);
		__m128i dst = _mm_maskz_shrdv_epi32(k, a, b, c);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		hexPrint128x32(&dst);
	}
	if ((selector == 134) || (selector == -1))
	{	// vpshrdvd xmm, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		for (int i = 0; i < 4; i++)
		{
			a.m128i_i32[i] = 0x0007;
			b.m128i_i32[i] = static_cast<short>(-1);
			c.m128i_i32[i] = i;
		}
		hexPrint128x32(&a);
		hexPrint128x32(&b);
		hexPrint128x32(&c);
		__m128i dst = _mm_shrdv_epi32(a, b, c);
		AppLib::write("\r\n");
		hexPrint128x32(&dst);
	}
	if ((selector == 135) || (selector == -1))
	{	// vpshrdvd ymm {k}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask8 k = 0xFD;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 0x0007;
			b.m256i_i32[i] = static_cast<short>(-1);
			c.m256i_i32[i] = i;
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrint256x32(&c);
		hexPrintByte(&k);
		__m256i dst = _mm256_mask_shrdv_epi32(a, k, b, c);
		AppLib::write("\r\n");
		hexPrint256x32(&dst);
	}
	if ((selector == 136) || (selector == -1))
	{	// vpshrdvd ymm {z}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask8 k = 0xFD;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 0x0007;
			b.m256i_i32[i] = static_cast<short>(-1);
			c.m256i_i32[i] = i;
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrint256x32(&c);
		hexPrintByte(&k);
		__m256i dst = _mm256_maskz_shrdv_epi32(k, a, b, c);
		AppLib::write("\r\n");
		hexPrint256x32(&dst);
	}
	if ((selector == 137) || (selector == -1))
	{	// vpshrdvd ymm, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		for (int i = 0; i < 8; i++)
		{
			a.m256i_i32[i] = 0x0007;
			b.m256i_i32[i] = static_cast<short>(-1);
			c.m256i_i32[i] = i;
		}
		hexPrint256x32(&a);
		hexPrint256x32(&b);
		hexPrint256x32(&c);
		__m256i dst = _mm256_shrdv_epi32(a, b, c);
		AppLib::write("\r\n");
		hexPrint256x32(&dst);
	}
	if ((selector == 138) || (selector == -1))
	{	// vpshrdvd zmm {k}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask16 k = 0xFDFF;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 0x0007;
			b.m512i_i32[i] = static_cast<short>(-1);
			c.m512i_i32[i] = i;
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrint512x32(&c);
		hexPrintWord(&k);
		__m512i dst = _mm512_mask_shrdv_epi32(a, k, b, c);
		AppLib::write("\r\n");
		hexPrint512x32(&dst);
	}
	if ((selector == 139) || (selector == -1))
	{	// vpshrdvd zmm {z}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask16 k = 0xFDFF;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 0x0007;
			b.m512i_i32[i] = static_cast<short>(-1);
			c.m512i_i32[i] = i;
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrint512x32(&c);
		hexPrintWord(&k);
		__m512i dst = _mm512_maskz_shrdv_epi32(k, a, b, c);
		AppLib::write("\r\n");
		hexPrint512x32(&dst);
	}
	if ((selector == 140) || (selector == -1))
	{	// vpshrdvd zmm, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		for (int i = 0; i < 16; i++)
		{
			a.m512i_i32[i] = 0x0007;
			b.m512i_i32[i] = static_cast<short>(-1);
			c.m512i_i32[i] = i;
		}
		hexPrint512x32(&a);
		hexPrint512x32(&b);
		hexPrint512x32(&c);
		__m512i dst = _mm512_shrdv_epi32(a, b, c);
		AppLib::write("\r\n");
		hexPrint512x32(&dst);
	}
	if ((selector == 141) || (selector == -1))
	{	// vpshrdvq xmm {k}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0x3;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 0x0007;
			b.m128i_i64[i] = static_cast<short>(-1);
			c.m128i_i64[i] = i + 2;
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&c);
		__m128i dst = _mm_mask_shrdv_epi64(a, k, b, c);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		hexPrint128x64(&dst);
	}
	if ((selector == 142) || (selector == -1))
	{	// vpshrdvq xmm {z}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		__mmask8 k = 0x3;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 0x0007;
			b.m128i_i64[i] = static_cast<short>(-1);
			c.m128i_i64[i] = i + 2;
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&c);
		__m128i dst = _mm_maskz_shrdv_epi64(k, a, b, c);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		hexPrint128x64(&dst);
	}
	if ((selector == 143) || (selector == -1))
	{	// vpshrdvq xmm, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i c;
		for (int i = 0; i < 2; i++)
		{
			a.m128i_i64[i] = 0x0007;
			b.m128i_i64[i] = static_cast<short>(-1);
			c.m128i_i64[i] = i + 2;
		}
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&c);
		__m128i dst = _mm_shrdv_epi64(a, b, c);
		AppLib::write("\r\n");
		hexPrint128x64(&dst);
	}
	if ((selector == 144) || (selector == -1))
	{	// vpshrdvq ymm {k}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask8 k = 0xB;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 0x0007;
			b.m256i_i64[i] = static_cast<short>(-1);
			c.m256i_i64[i] = i + 2;
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&c);
		__m256i dst = _mm256_mask_shrdv_epi64(a, k, b, c);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		hexPrint256x64(&dst);
	}
	if ((selector == 145) || (selector == -1))
	{	// vpshrdvq ymm {z}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		__mmask8 k = 0x9;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 0x0007;
			b.m256i_i64[i] = static_cast<short>(-1);
			c.m256i_i64[i] = i + 2;
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&c);
		__m256i dst = _mm256_maskz_shrdv_epi64(k, a, b, c);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		hexPrint256x64(&dst);
	}
	if ((selector == 146) || (selector == -1))
	{	// vpshrdvq ymm, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i c;
		for (int i = 0; i < 4; i++)
		{
			a.m256i_i64[i] = 0x0007;
			b.m256i_i64[i] = static_cast<short>(-1);
			c.m256i_i64[i] = i + 2;
		}
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&c);
		__m256i dst = _mm256_shrdv_epi64(a, b, c);
		AppLib::write("\r\n");
		hexPrint256x64(&dst);
	}
	if ((selector == 147) || (selector == -1))
	{	// vpshrdvq zmm {k}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask8 k = 0xFB;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 0x0007;
			b.m512i_i64[i] = static_cast<short>(-1);
			c.m512i_i64[i] = i + 2;
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&c);
		__m512i dst = _mm512_mask_shrdv_epi64(a, k, b, c);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		hexPrint512x64(&dst);
	}
	if ((selector == 148) || (selector == -1))
	{	// vpshrdvq zmm {z}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		__mmask8 k = 0xFB;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 0x0007;
			b.m512i_i64[i] = static_cast<short>(-1);
			c.m512i_i64[i] = i + 2;
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&c);
		__m512i dst = _mm512_maskz_shrdv_epi64(k, a, b, c);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		hexPrint512x64(&dst);
	}
	if ((selector == 149) || (selector == -1))
	{	// vpshrdvq zmm, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i c;
		for (int i = 0; i < 8; i++)
		{
			a.m512i_i64[i] = 0x0007;
			b.m512i_i64[i] = static_cast<short>(-1);
			c.m512i_i64[i] = i + 2;
		}
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&c);
		__m512i dst = _mm512_shrdv_epi64(a, b, c);
		AppLib::write("\r\n");
		hexPrint512x64(&dst);
	}

	return exitCode;
}
