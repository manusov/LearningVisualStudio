/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avx512techs=AVX512_VBMI
*/

#include "Task_29.h"

int Task_29::execute(int argc, char** argv)
{
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[AVX512 VBMI]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	int exitCode = 0;
	int selector = 29;  // -1;

	if ((selector == 0) || (selector == -1))
	{	// vpmultishiftqb xmm {k}, xmm, xmm
		__m128i xa;             // Shift counts as 16 bytes, 2 x 64-bit qwords x 8 counts.
		__m128i xb;             // Shift data as 2 x 64-bit qwords.
		__m128i xsrc;           // Source operand as bytes used if predicate mask bit = 0.
		__mmask16 k = 0x1FFF;   // k = mask.
		for (int i = 0; i < 16; i++)
		{
			xa.m128i_i8[i] = i % 8;
			xsrc.m128i_i8[i] = 0x11;
		}
		xb.m128i_i64[0] = 0xFFFFFFFFFFFFFF00LL;
		xb.m128i_i64[1] = 0x80;
		hexPrint128x64(&xa);
		hexPrint128x64(&xb);
		hexPrint128x64(&xsrc);
		hexPrintWord(&k);
		__m128i ydst = _mm_mask_multishift_epi64_epi8(xsrc, k, xa, xb);
		hexPrint128x64(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 1) || (selector == -1))
	{	// vpmultishiftqb xmm {z}, xmm, xmm
		__m128i xa;             // Shift counts as 16 bytes, 2 x 64-bit qwords x 8 counts. 
		__m128i xb;             // Shift data as 2 x 64-bit qwords.
		__mmask16 k = 0x1FFF;   // k = mask.
		for (int i = 0; i < 16; i++)
		{
			xa.m128i_i8[i] = i % 8;
		}
		xb.m128i_i64[0] = 0xFFFFFFFFFFFFFF00LL;
		xb.m128i_i64[1] = 0x80;
		hexPrint128x64(&xa);
		hexPrint128x64(&xb);
		hexPrintWord(&k);
		__m128i ydst = _mm_maskz_multishift_epi64_epi8(k, xa, xb);
		hexPrint128x64(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 2) || (selector == -1))
	{	// vpmultishiftqb xmm, xmm, xmm
		__m128i xa;             // Shift counts as 16 bytes, 2 x 64-bit qwords x 8 counts. 
		__m128i xb;             // Shift data as 2 x 64-bit qwords.
		for (int i = 0; i < 16; i++)
		{
			xa.m128i_i8[i] = i % 8;
		}
		xb.m128i_i64[0] = 0xFFFFFFFFFFFFFF00LL;
		xb.m128i_i64[1] = 0x80;
		hexPrint128x64(&xa);
		hexPrint128x64(&xb);
		__m128i ydst = _mm_multishift_epi64_epi8(xa, xb);
		hexPrint128x64(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 3) || (selector == -1))
	{	// vpmultishiftqb ymm {k}, ymm, ymm
		__m256i xa;                  // Shift counts as 32 bytes, 4 x 64-bit qwords x 8 counts.
		__m256i xb;                  // Shift data as 4 x 64-bit qwords.
		__m256i xsrc;                // Source operand as bytes used if predicate mask bit = 0.
		__mmask32 k = 0x1FFFFFFF;    // k = mask.
		for (int i = 0; i < 32; i++)
		{
			xa.m256i_i8[i] = i % 8;
			xsrc.m256i_i8[i] = 0x11;
		}
		xb.m256i_i64[0] = 0xFFFFFFFFFFFFFF00LL;
		xb.m256i_i64[1] = 0x80;
		xb.m256i_i64[2] = 0xFFFFFFFFFFFFF000LL;
		xb.m256i_i64[3] = 0x180;
		hexPrint256x64(&xa);
		hexPrint256x64(&xb);
		hexPrint256x64(&xsrc);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i ydst = _mm256_mask_multishift_epi64_epi8(xsrc, k, xa, xb);
		hexPrint256x64(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 4) || (selector == -1))
	{	// vpmultishiftqb ymm {z}, ymm, ymm
		__m256i xa;                  // Shift counts as 32 bytes, 4 x 64-bit qwords x 8 counts.
		__m256i xb;                  // Shift data as 4 x 64-bit qwords.
		__mmask32 k = 0xFFFFFF00;    // k = mask.
		for (int i = 0; i < 32; i++)
		{
			xa.m256i_i8[i] = i % 8;
		}
		xb.m256i_i64[0] = 0xFFFFFFFFFFFFFF00LL;
		xb.m256i_i64[1] = 0x80;
		xb.m256i_i64[2] = 0xFFFFFFFFFFFFF000LL;
		xb.m256i_i64[3] = 0x180;
		hexPrint256x64(&xa);
		hexPrint256x64(&xb);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i ydst = _mm256_maskz_multishift_epi64_epi8(k, xa, xb);
		hexPrint256x64(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 5) || (selector == -1))
	{	// vpmultishiftqb ymm, ymm, ymm
		__m256i xa;                  // Shift counts as 32 bytes, 4 x 64-bit qwords x 8 counts.
		__m256i xb;                  // Shift data as 4 x 64-bit qwords.
		for (int i = 0; i < 32; i++)
		{
			xa.m256i_i8[i] = i % 8;
		}
		xb.m256i_i64[0] = 0xFFFFFFFFFFFFFF00LL;
		xb.m256i_i64[1] = 0x80;
		xb.m256i_i64[2] = 0xFFFFFFFFFFFFF000LL;
		xb.m256i_i64[3] = 0x180;
		hexPrint256x64(&xa);
		hexPrint256x64(&xb);
		__m256i ydst = _mm256_multishift_epi64_epi8(xa, xb);
		hexPrint256x64(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 6) || (selector == -1))
	{	// vpmultishiftqb zmm {k}, zmm, zmm
		__m512i xa;                         // Shift counts as 64 bytes, 8 x 64-bit qwords x 8 counts.
		__m512i xb;                         // Shift data as 8 x 64-bit qwords.
		__m512i xsrc;                       // Source operand as bytes used if predicate mask bit = 0.
		__mmask64 k = 0xFFFFFFFF0000FFFF;   // k = mask.
		for (int i = 0; i < 64; i++)
		{
			xa.m512i_i8[i] = i % 8;
			xsrc.m512i_i8[i] = 0x11;
		}
		xb.m512i_i64[0] = 0xFFFFFFFFFFFFFF00LL;
		xb.m512i_i64[1] = 0x80;
		xb.m512i_i64[2] = 0xFFFFFFFFFFFFF000LL;
		xb.m512i_i64[3] = 0x180;
		xb.m512i_i64[4] = 0x1;
		xb.m512i_i64[5] = 0x2;
		xb.m512i_i64[6] = 0x4;
		xb.m512i_i64[7] = 0x8;
		hexPrint512x64(&xa);
		hexPrint512x64(&xb);
		hexPrint512x64(&xsrc);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i ydst = _mm512_mask_multishift_epi64_epi8(xsrc, k, xa, xb);
		hexPrint512x64(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 7) || (selector == -1))
	{	// vpmultishiftqb zmm {z}, zmm, zmm
		__m512i xa;                         // Shift counts as 64 bytes, 8 x 64-bit qwords x 8 counts.
		__m512i xb;                         // Shift data as 8 x 64-bit qwords.
		__mmask64 k = 0xFFFFFFFF0000FFFF;   // k = mask.
		for (int i = 0; i < 64; i++)
		{
			xa.m512i_i8[i] = i % 8;
		}
		xb.m512i_i64[0] = 0xFFFFFFFFFFFFFF00LL;
		xb.m512i_i64[1] = 0x80;
		xb.m512i_i64[2] = 0xFFFFFFFFFFFFF000LL;
		xb.m512i_i64[3] = 0x180;
		xb.m512i_i64[4] = 0x1;
		xb.m512i_i64[5] = 0x2;
		xb.m512i_i64[6] = 0x4;
		xb.m512i_i64[7] = 0x8;
		hexPrint512x64(&xa);
		hexPrint512x64(&xb);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i ydst = _mm512_maskz_multishift_epi64_epi8(k, xa, xb);
		hexPrint512x64(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 8) || (selector == -1))
	{	// vpmultishiftqb zmm, zmm, zmm
		__m512i xa;                     // Shift counts as 64 bytes, 8 x 64-bit qwords x 8 counts.
		__m512i xb;                     // Shift data as 8 x 64-bit qwords.
		for (int i = 0; i < 64; i++)
		{
			xa.m512i_i8[i] = i % 8;
		}
		xb.m512i_i64[0] = 0xFFFFFFFFFFFFFF00LL;
		xb.m512i_i64[1] = 0x80;
		xb.m512i_i64[2] = 0xFFFFFFFFFFFFF000LL;
		xb.m512i_i64[3] = 0x180;
		xb.m512i_i64[4] = 0x1;
		xb.m512i_i64[5] = 0x2;
		xb.m512i_i64[6] = 0x4;
		xb.m512i_i64[7] = 0x8;
		hexPrint512x64(&xa);
		hexPrint512x64(&xb);
		__m512i ydst = _mm512_multishift_epi64_epi8(xa, xb);
		hexPrint512x64(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 9) || (selector == -1))
	{	// vpermt2b xmm {k}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i idx;
		__mmask16 k = 0x9FFF;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i + 0x10;
			b.m128i_i8[i] = i + 0x20;
			idx.m128i_i8[i] = 0;
		}
		idx.m128i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m128i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&idx);
		hexPrintWord(&k);
		__m128i y = _mm_mask_permutex2var_epi8(a, k, idx, b);
		hexPrint128x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 10) || (selector == -1))
	{	// vpermi2b xmm {k}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i idx;
		__mmask16 k = 0x9FFF;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i + 0x10;
			b.m128i_i8[i] = i + 0x20;
			idx.m128i_i8[i] = 0;
		}
		idx.m128i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m128i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m128i_i8[14] = 0x11;
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&idx);
		hexPrintWord(&k);
		__m128i y = _mm_mask2_permutex2var_epi8(a, idx, k, b);
		hexPrint128x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 11) || (selector == -1))
	{	// vpermi2b xmm {z}, xmm, xmm  // vpermt2b xmm {z}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i idx;
		__mmask16 k = 0x9FFF;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i + 0x10;
			b.m128i_i8[i] = i + 0x20;
			idx.m128i_i8[i] = 0;
		}
		idx.m128i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m128i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m128i_i8[14] = 0x11;
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&idx);
		hexPrintWord(&k);
		__m128i y = _mm_maskz_permutex2var_epi8(k, a, idx, b);
		hexPrint128x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 12) || (selector == -1))
	{	// vpermi2b xmm, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i idx;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i + 0x10;
			b.m128i_i8[i] = i + 0x20;
			idx.m128i_i8[i] = 0;
		}
		idx.m128i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m128i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m128i_i8[14] = 0x11;
		hexPrint128x64(&a);
		hexPrint128x64(&b);
		hexPrint128x64(&idx);
		__m128i y = _mm_permutex2var_epi8(a, idx, b);
		hexPrint128x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 13) || (selector == -1))
	{	// vpermt2b ymm {k}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i idx;
		__mmask32 k = 0x9FFF;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i + 0x10;
			b.m256i_i8[i] = i + 0x20;
			idx.m256i_i8[i] = 0;
		}
		idx.m256i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m256i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m256i_i8[14] = 0x11;
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&idx);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i y = _mm256_mask_permutex2var_epi8(a, k, idx, b);
		hexPrint256x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 14) || (selector == -1))
	{	// vpermi2b ymm {k}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i idx;
		__mmask32 k = 0x9FFF;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i + 0x10;
			b.m256i_i8[i] = i + 0x20;
			idx.m256i_i8[i] = 0;
		}
		idx.m256i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m256i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m256i_i8[14] = 0x11;
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&idx);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i y = _mm256_mask2_permutex2var_epi8(a, idx, k, b);
		hexPrint256x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 15) || (selector == -1))
	{	// vpermi2b ymm {z}, ymm, ymm  // vpermt2b ymm {z}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i idx;
		__mmask32 k = 0x9FFF;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i + 0x10;
			b.m256i_i8[i] = i + 0x20;
			idx.m256i_i8[i] = 0;
		}
		idx.m256i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m256i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m256i_i8[14] = 0x11;
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&idx);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i y = _mm256_maskz_permutex2var_epi8(k, a, idx, b);
		hexPrint256x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 16) || (selector == -1))
	{	// vpermi2b ymm, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i idx;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i + 0x10;
			b.m256i_i8[i] = i + 0x20;
			idx.m256i_i8[i] = 0;
		}
		idx.m256i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m256i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m256i_i8[14] = 0x11;
		hexPrint256x64(&a);
		hexPrint256x64(&b);
		hexPrint256x64(&idx);
		__m256i y = _mm256_permutex2var_epi8(a, idx, b);
		hexPrint256x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 17) || (selector == -1))
	{	// vpermt2b zmm {k}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i idx;
		__mmask64 k = 0xFFFF00FF;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i + 0x10;
			b.m512i_i8[i] = i + 0x20;
			idx.m512i_i8[i] = 0;
		}
		idx.m512i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m512i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m512i_i8[14] = 0x11;
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&idx);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i y = _mm512_mask_permutex2var_epi8(a, k, idx, b);
		hexPrint512x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 18) || (selector == -1))
	{	// vpermi2b zmm {k}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i idx;
		__mmask64 k = 0xFFFF00FF;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i + 0x10;
			b.m512i_i8[i] = i + 0x20;
			idx.m512i_i8[i] = 0;
		}
		idx.m512i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m512i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m512i_i8[14] = 0x11;
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&idx);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i y = _mm512_mask2_permutex2var_epi8(a, idx, k, b);
		hexPrint512x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 19) || (selector == -1))
	{	// vpermi2b zmm {z}, zmm, zmm  // vpermt2b zmm {z}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i idx;
		__mmask64 k = 0xFFFF00FF;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i + 0x10;
			b.m512i_i8[i] = i + 0x20;
			idx.m512i_i8[i] = 0;
		}
		idx.m512i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m512i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m512i_i8[14] = 0x11;
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&idx);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i y = _mm512_maskz_permutex2var_epi8(k, a, idx, b);
		hexPrint512x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 20) || (selector == -1))
	{	// vpermi2b zmm, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i idx;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i + 0x10;
			b.m512i_i8[i] = i + 0x20;
			idx.m512i_i8[i] = 0;
		}
		idx.m512i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m512i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		idx.m512i_i8[14] = 0x11;
		hexPrint512x64(&a);
		hexPrint512x64(&b);
		hexPrint512x64(&idx);
		__m512i y = _mm512_permutex2var_epi8(a, idx, b);
		hexPrint512x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 21) || (selector == -1))
	{	// vpermb xmm {k}, xmm, xmm
		__m128i a;
		__m128i src;
		__m128i idx;
		__mmask16 k = 0x9FFF;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i + 0x10;
			src.m128i_i8[i] = 0x20;
			idx.m128i_i8[i] = 0;
		}
		idx.m128i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m128i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		hexPrint128x64(&a);
		hexPrint128x64(&src);
		hexPrint128x64(&idx);
		hexPrintWord(&k);
		__m128i y = _mm_mask_permutexvar_epi8(src, k, idx, a);
		hexPrint128x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 22) || (selector == -1))
	{	// vpermb xmm {z}, xmm, xmm
		__m128i a;
		__m128i idx;
		__mmask16 k = 0x9FFF;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i + 0x10;
			idx.m128i_i8[i] = 0;
		}
		idx.m128i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m128i_i8[2] = 0x05;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		hexPrint128x64(&a);
		hexPrint128x64(&idx);
		hexPrintWord(&k);
		__m128i y = _mm_maskz_permutexvar_epi8(k, idx, a);
		hexPrint128x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 23) || (selector == -1))
	{	// vpermb xmm, xmm, xmm
		__m128i a;
		__m128i idx;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i + 0x70;
			idx.m128i_i8[i] = 0;
		}
		idx.m128i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m128i_i8[2] = 0x05;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		hexPrint128x64(&a);
		hexPrint128x64(&idx);
		__m128i y = _mm_permutexvar_epi8(idx, a);
		hexPrint128x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 24) || (selector == -1))
	{	// vpermb ymm {k}, ymm, ymm
		__m256i a;
		__m256i src;
		__m256i idx;
		__mmask32 k = 0xFFFF0FFF;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i + 0x10;
			src.m256i_i8[i] = 0x20;
			idx.m256i_i8[i] = 0;
		}
		idx.m256i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m256i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		hexPrint256x64(&a);
		hexPrint256x64(&src);
		hexPrint256x64(&idx);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i y = _mm256_mask_permutexvar_epi8(src, k, idx, a);
		hexPrint256x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 25) || (selector == -1))
	{	// vpermb ymm {z}, ymm, ymm
		__m256i a;
		__m256i idx;
		__mmask32 k = 0xFFFF0FFF;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i + 0x10;
			idx.m256i_i8[i] = 0;
		}
		idx.m256i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m256i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		hexPrint256x64(&a);
		hexPrint256x64(&idx);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i y = _mm256_maskz_permutexvar_epi8(k, idx, a);
		hexPrint256x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 26) || (selector == -1))
	{	// vpermb ymm, ymm, ymm
		__m256i a;
		__m256i idx;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i + 0x10;
			idx.m256i_i8[i] = 0;
		}
		idx.m256i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m256i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		hexPrint256x64(&a);
		hexPrint256x64(&idx);
		__m256i y = _mm256_permutexvar_epi8(idx, a);
		hexPrint256x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 27) || (selector == -1))
	{	// vpermb zmm {k}, zmm, zmm
		__m512i a;
		__m512i src;
		__m512i idx;
		__mmask64 k = 0xFFFFFFFFFFFF0FFF;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i + 0x10;
			src.m512i_i8[i] = 0x20;
			idx.m512i_i8[i] = 0;
		}
		idx.m512i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m512i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		hexPrint512x64(&a);
		hexPrint512x64(&src);
		hexPrint512x64(&idx);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i y = _mm512_mask_permutexvar_epi8(src, k, idx, a);
		hexPrint512x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 28) || (selector == -1))
	{	// vpermb zmm {z}, zmm, zmm
		__m512i a;
		__m512i src;
		__m512i idx;
		__mmask64 k = 0xFFFFFFFFFFFF0FFF;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i + 0x10;
			src.m512i_i8[i] = 0x20;
			idx.m512i_i8[i] = 0;
		}
		idx.m512i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m512i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		hexPrint512x64(&a);
		hexPrint512x64(&src);
		hexPrint512x64(&idx);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i y = _mm512_maskz_permutexvar_epi8(k, idx, a);
		hexPrint512x64(&y);
		AppLib::write("\r\n");
	}
	if ((selector == 29) || (selector == -1))
	{	// vpermb zmm, zmm, zmm
		__m512i a;
		__m512i src;
		__m512i idx;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i + 0x10;
			src.m512i_i8[i] = 0x20;
			idx.m512i_i8[i] = 0;
		}
		idx.m512i_i8[1] = 0xF;    // Bit [4] = selector = 0, bits [3-0] = index = 15.
		idx.m512i_i8[2] = 0x15;   // Bit [4] = selector = 1, bits [3-0] = index = 5.
		hexPrint512x64(&a);
		hexPrint512x64(&src);
		hexPrint512x64(&idx);
		__m512i y = _mm512_permutexvar_epi8(idx, a);
		hexPrint512x64(&y);
		AppLib::write("\r\n");
	}

	return exitCode;
}
