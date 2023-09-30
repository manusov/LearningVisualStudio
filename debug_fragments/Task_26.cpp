/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avx512techs=AVX512_VNNI&ig_expand=2702
*/

#include "Task_26.h"

int Task_26::execute(int argc, char** argv)
{
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[AVX512 VNNI]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	int exitCode = 0;
	int selector = 35;  // -1;

	if ((selector == 0) || (selector == -1))
	{	// vpdpbusd xmm, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 4;
			xa.m128i_i8[i] = i;
			xb.m128i_i8[i] = j;
			xsrc.m128i_i32[j] = -1;
		}
		decimalPrint128x8(&xa);
		decimalPrint128x8(&xb);
		decimalPrint128x32(&xsrc);
		__m128i ydst = _mm_dpbusd_epi32(xsrc, xa, xb);
		decimalPrint128x32(&ydst);
		AppLib::write("\r\n");
		for (int i = 0; i < 16; i++)
		{
			int j = i / 4;
			xa.m128i_i8[i] = 0;
			xb.m128i_i8[i] = 0;
			xsrc.m128i_i32[j] = 0;
		}
		xa.m128i_i8[0] = -1;       // here -1 interpreted as 255
		xb.m128i_i8[0] = -1;       // here -1 interpreted as -1
		xsrc.m128i_i32[0] = -1;
		decimalPrint128x8(&xa);
		decimalPrint128x8(&xb);
		decimalPrint128x32(&xsrc);
		ydst = _mm_dpbusd_epi32(xsrc, xa, xb);
		decimalPrint128x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 1) || (selector == -1))
	{	// vpdpbusd xmm {k}, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		__mmask8 k = 8;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 4;
			xa.m128i_i8[i] = i;
			xb.m128i_i8[i] = j;
			xsrc.m128i_i32[j] = -1;
		}
		decimalPrint128x8(&xa);
		decimalPrint128x8(&xb);
		decimalPrint128x32(&xsrc);
		hexPrintByte(&k);
		__m128i ydst = _mm_mask_dpbusd_epi32(xsrc, k, xa, xb);
		decimalPrint128x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 2) || (selector == -1))
	{	// vpdpbusd xmm {z}, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		__mmask8 k = 8;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 4;
			xa.m128i_i8[i] = i;
			xb.m128i_i8[i] = j;
			xsrc.m128i_i32[j] = -1;
		}
		decimalPrint128x8(&xa);
		decimalPrint128x8(&xb);
		decimalPrint128x32(&xsrc);
		hexPrintByte(&k);
		__m128i ydst = _mm_maskz_dpbusd_epi32(k, xsrc, xa, xb);
		decimalPrint128x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 3) || (selector == -1))
	{	// vpdpbusd ymm, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 4;
			xa.m256i_i8[i] = i;
			xb.m256i_i8[i] = j;
			xsrc.m256i_i32[j] = -1;
		}
		decimalPrint256x8(&xa);
		decimalPrint256x8(&xb);
		decimalPrint256x32(&xsrc);
		__m256i ydst = _mm256_dpbusd_epi32(xsrc, xa, xb);
		decimalPrint256x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 4) || (selector == -1))
	{	// vpdpbusd ymm {k}, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		__mmask8 k = 0x55;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 4;
			xa.m256i_i8[i] = i;
			xb.m256i_i8[i] = j;
			xsrc.m256i_i32[j] = -1;
		}
		decimalPrint256x8(&xa);
		decimalPrint256x8(&xb);
		decimalPrint256x32(&xsrc);
		hexPrintByte(&k);
		__m256i ydst = _mm256_mask_dpbusd_epi32(xsrc, k, xa, xb);
		decimalPrint256x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 5) || (selector == -1))
	{	// vpdpbusd ymm {z}, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		__mmask8 k = 0xAA;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 4;
			xa.m256i_i8[i] = 1;
			xb.m256i_i8[i] = 1;
			xsrc.m256i_i32[j] = j * 100000;
		}
		decimalPrint256x8(&xa);
		decimalPrint256x8(&xb);
		decimalPrint256x32(&xsrc);
		hexPrintByte(&k);
		__m256i ydst = _mm256_maskz_dpbusd_epi32(k, xsrc, xa, xb);
		decimalPrint256x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 6) || (selector == -1))
	{	// vpdpbusd zmm, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		for (int i = 0; i < 64; i++)
		{
			int j = i / 4;
			xa.m512i_i8[i] = 2;
			xb.m512i_i8[i] = 3;
			xsrc.m512i_i32[j] = 1000;
		}
		decimalPrint512x8(&xa);
		decimalPrint512x8(&xb);
		decimalPrint512x32(&xsrc);
		__m512i ydst = _mm512_dpbusd_epi32(xsrc, xa, xb);
		decimalPrint512x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 7) || (selector == -1))
	{	// vpdpbusd zmm {k}, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		__mmask16 k = 0xE007;
		for (int i = 0; i < 64; i++)
		{
			int j = i / 4;
			xa.m512i_i8[i] = 2;
			xb.m512i_i8[i] = 3;
			xsrc.m512i_i32[j] = 100;
		}
		decimalPrint512x8(&xa);
		decimalPrint512x8(&xb);
		decimalPrint512x32(&xsrc);
		hexPrintWord(&k);
		__m512i ydst = _mm512_mask_dpbusd_epi32(xsrc, k, xa, xb);
		decimalPrint512x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 8) || (selector == -1))
	{	// vpdpbusd zmm {z}, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		__mmask16 k = 0x9AAF;
		for (int i = 0; i < 64; i++)
		{
			int j = i / 4;
			xa.m512i_i8[i] = 2;
			xb.m512i_i8[i] = 3;
			xsrc.m512i_i32[j] = 100;
		}
		decimalPrint512x8(&xa);
		decimalPrint512x8(&xb);
		decimalPrint512x32(&xsrc);
		hexPrintWord(&k);
		__m512i ydst = _mm512_maskz_dpbusd_epi32(k, xsrc, xa, xb);
		decimalPrint512x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 9) || (selector == -1))
	{	// vpdpbusds xmm, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 4;
			xa.m128i_i8[i] = i;
			xb.m128i_i8[i] = -1;
			xsrc.m128i_i32[j] = INT_MIN;
		}
		decimalPrint128x8(&xa);
		decimalPrint128x8(&xb);
		decimalPrint128x32(&xsrc);
		AppLib::write("\r\n");
		__m128i ydstSaturated = _mm_dpbusds_epi32(xsrc, xa, xb);
		__m128i ydstNonSaturated = _mm_dpbusd_epi32(xsrc, xa, xb);
		decimalPrint128x32(&ydstSaturated);
		hexPrint128x32(&ydstSaturated);
		decimalPrint128x32(&ydstNonSaturated);
		hexPrint128x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 10) || (selector == -1))
	{	// vpdpbusds xmm {k}, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		__mmask8 k = 9;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 4;
			xa.m128i_i8[i] = i;
			xb.m128i_i8[i] = 1;
			xsrc.m128i_i32[j] = INT_MAX;
		}
		decimalPrint128x8(&xa);
		decimalPrint128x8(&xb);
		decimalPrint128x32(&xsrc);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		__m128i ydstSaturated = _mm_mask_dpbusds_epi32(xsrc, k, xa, xb);
		__m128i ydstNonSaturated = _mm_mask_dpbusd_epi32(xsrc, k, xa, xb);
		decimalPrint128x32(&ydstSaturated);
		hexPrint128x32(&ydstSaturated);
		decimalPrint128x32(&ydstNonSaturated);
		hexPrint128x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 11) || (selector == -1))
	{	// vpdpbusds xmm {z}, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		__mmask8 k = 9;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 4;
			xa.m128i_i8[i] = i;
			xb.m128i_i8[i] = 1;
			xsrc.m128i_i32[j] = INT_MAX;
		}
		decimalPrint128x8(&xa);
		decimalPrint128x8(&xb);
		decimalPrint128x32(&xsrc);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		__m128i ydstSaturated = _mm_maskz_dpbusds_epi32(k, xsrc, xa, xb);
		__m128i ydstNonSaturated = _mm_maskz_dpbusd_epi32(k, xsrc, xa, xb);
		decimalPrint128x32(&ydstSaturated);
		hexPrint128x32(&ydstSaturated);
		decimalPrint128x32(&ydstNonSaturated);
		hexPrint128x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 12) || (selector == -1))
	{	// vpdpbusds ymm, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 4;
			xa.m256i_i8[i] = i;
			xb.m256i_i8[i] = -1;
			xsrc.m256i_i32[j] = INT_MIN;
		}
		decimalPrint256x8(&xa);
		decimalPrint256x8(&xb);
		decimalPrint256x32(&xsrc);
		AppLib::write("\r\n");
		__m256i ydstSaturated = _mm256_dpbusds_epi32(xsrc, xa, xb);
		__m256i ydstNonSaturated = _mm256_dpbusd_epi32(xsrc, xa, xb);
		decimalPrint256x32(&ydstSaturated);
		hexPrint256x32(&ydstSaturated);
		decimalPrint256x32(&ydstNonSaturated);
		hexPrint256x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 13) || (selector == -1))
	{	// vpdpbusds ymm {k}, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		__mmask8 k = 0xC3;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 4;
			xa.m256i_i8[i] = i;
			xb.m256i_i8[i] = -1;
			xsrc.m256i_i32[j] = INT_MIN;
		}
		decimalPrint256x8(&xa);
		decimalPrint256x8(&xb);
		decimalPrint256x32(&xsrc);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		__m256i ydstSaturated = _mm256_mask_dpbusds_epi32(xsrc, k, xa, xb);
		__m256i ydstNonSaturated = _mm256_mask_dpbusd_epi32(xsrc, k, xa, xb);
		decimalPrint256x32(&ydstSaturated);
		hexPrint256x32(&ydstSaturated);
		decimalPrint256x32(&ydstNonSaturated);
		hexPrint256x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 14) || (selector == -1))
	{	// vpdpbusds ymm {z}, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		__mmask8 k = 0xC3;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 4;
			xa.m256i_i8[i] = i;
			xb.m256i_i8[i] = -1;
			xsrc.m256i_i32[j] = INT_MIN;
		}
		decimalPrint256x8(&xa);
		decimalPrint256x8(&xb);
		decimalPrint256x32(&xsrc);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		__m256i ydstSaturated = _mm256_maskz_dpbusds_epi32(k, xsrc, xa, xb);
		__m256i ydstNonSaturated = _mm256_maskz_dpbusd_epi32(k, xsrc, xa, xb);
		decimalPrint256x32(&ydstSaturated);
		hexPrint256x32(&ydstSaturated);
		decimalPrint256x32(&ydstNonSaturated);
		hexPrint256x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 15) || (selector == -1))
	{	// vpdpbusds zmm, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		for (int i = 0; i < 64; i++)
		{
			int j = i / 4;
			xa.m512i_i8[i] = i;
			xb.m512i_i8[i] = -1;
			xsrc.m512i_i32[j] = INT_MIN;
		}
		decimalPrint512x8(&xa);
		decimalPrint512x8(&xb);
		decimalPrint512x32(&xsrc);
		AppLib::write("\r\n");
		__m512i ydstSaturated = _mm512_dpbusds_epi32(xsrc, xa, xb);
		__m512i ydstNonSaturated = _mm512_dpbusd_epi32(xsrc, xa, xb);
		decimalPrint512x32(&ydstSaturated);
		hexPrint512x32(&ydstSaturated);
		decimalPrint512x32(&ydstNonSaturated);
		hexPrint512x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 16) || (selector == -1))
	{	// vpdpbusds zmm {k}, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		__mmask16 k = 0xF5A1;
		for (int i = 0; i < 64; i++)
		{
			int j = i / 4;
			xa.m512i_i8[i] = i;
			xb.m512i_i8[i] = -1;
			xsrc.m512i_i32[j] = INT_MIN;
		}
		decimalPrint512x8(&xa);
		decimalPrint512x8(&xb);
		decimalPrint512x32(&xsrc);
		AppLib::write("\r\n");
		__m512i ydstSaturated = _mm512_mask_dpbusds_epi32(xsrc, k, xa, xb);
		__m512i ydstNonSaturated = _mm512_mask_dpbusd_epi32(xsrc, k, xa, xb);
		decimalPrint512x32(&ydstSaturated);
		hexPrint512x32(&ydstSaturated);
		decimalPrint512x32(&ydstNonSaturated);
		hexPrint512x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 17) || (selector == -1))
	{	// vpdpbusds zmm {z}, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		__mmask16 k = 0xF5A1;
		for (int i = 0; i < 64; i++)
		{
			int j = i / 4;
			xa.m512i_i8[i] = i;
			xb.m512i_i8[i] = -1;
			xsrc.m512i_i32[j] = INT_MIN;
		}
		decimalPrint512x8(&xa);
		decimalPrint512x8(&xb);
		decimalPrint512x32(&xsrc);
		AppLib::write("\r\n");
		__m512i ydstSaturated = _mm512_maskz_dpbusds_epi32(k, xsrc, xa, xb);
		__m512i ydstNonSaturated = _mm512_maskz_dpbusd_epi32(k, xsrc, xa, xb);
		decimalPrint512x32(&ydstSaturated);
		hexPrint512x32(&ydstSaturated);
		decimalPrint512x32(&ydstNonSaturated);
		hexPrint512x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 18) || (selector == -1))
	{	// vpdpwssd xmm, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		for (int i = 0; i < 8; i++)
		{
			int j = i / 2;
			xa.m128i_i16[i] = i;
			xb.m128i_i16[i] = 3;
			xsrc.m128i_i32[j] = -1;
		}
		decimalPrint128x16(&xa);
		decimalPrint128x16(&xb);
		decimalPrint128x32(&xsrc);
		__m128i ydst = _mm_dpwssd_epi32(xsrc, xa, xb);
		decimalPrint128x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 19) || (selector == -1))
	{	// vpdpwssd xmm {k}, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		__mmask8 k = 0x9;
		for (int i = 0; i < 8; i++)
		{
			int j = i / 2;
			xa.m128i_i16[i] = i;
			xb.m128i_i16[i] = 3;
			xsrc.m128i_i32[j] = -1;
		}
		decimalPrint128x16(&xa);
		decimalPrint128x16(&xb);
		decimalPrint128x32(&xsrc);
		hexPrintByte(&k);
		__m128i ydst = _mm_mask_dpwssd_epi32(xsrc, k, xa, xb);
		decimalPrint128x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 20) || (selector == -1))
	{	// vpdpwssd xmm {z}, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		__mmask8 k = 0x9;
		for (int i = 0; i < 8; i++)
		{
			int j = i / 2;
			xa.m128i_i16[i] = i;
			xb.m128i_i16[i] = 3;
			xsrc.m128i_i32[j] = -1;
		}
		decimalPrint128x16(&xa);
		decimalPrint128x16(&xb);
		decimalPrint128x32(&xsrc);
		hexPrintByte(&k);
		__m128i ydst = _mm_maskz_dpwssd_epi32(k, xsrc, xa, xb);
		decimalPrint128x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 21) || (selector == -1))
	{	// vpdpwssd ymm, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 2;
			xa.m256i_i16[i] = i;
			xb.m256i_i16[i] = 3;
			xsrc.m256i_i32[j] = -1;
		}
		decimalPrint256x16(&xa);
		decimalPrint256x16(&xb);
		decimalPrint256x32(&xsrc);
		__m256i ydst = _mm256_dpwssd_epi32(xsrc, xa, xb);
		decimalPrint256x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 22) || (selector == -1))
	{	// vpdpwssd ymm {k}, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		__mmask8 k = 0x96;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 2;
			xa.m256i_i16[i] = i;
			xb.m256i_i16[i] = 3;
			xsrc.m256i_i32[j] = -1;
		}
		decimalPrint256x16(&xa);
		decimalPrint256x16(&xb);
		decimalPrint256x32(&xsrc);
		__m256i ydst = _mm256_mask_dpwssd_epi32(xsrc, k, xa, xb);
		decimalPrint256x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 23) || (selector == -1))
	{	// vpdpwssd ymm {z}, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		__mmask8 k = 0x96;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 2;
			xa.m256i_i16[i] = i;
			xb.m256i_i16[i] = 3;
			xsrc.m256i_i32[j] = -1;
		}
		decimalPrint256x16(&xa);
		decimalPrint256x16(&xb);
		decimalPrint256x32(&xsrc);
		__m256i ydst = _mm256_maskz_dpwssd_epi32(k, xsrc, xa, xb);
		decimalPrint256x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 24) || (selector == -1))
	{	// vpdpwssd zmm, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 2;
			xa.m512i_i16[i] = i;
			xb.m512i_i16[i] = 3;
			xsrc.m512i_i32[j] = -1;
		}
		decimalPrint512x16(&xa);
		decimalPrint512x16(&xb);
		decimalPrint512x32(&xsrc);
		__m512i ydst = _mm512_dpwssd_epi32(xsrc, xa, xb);
		decimalPrint512x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 25) || (selector == -1))
	{	// vpdpwssd zmm {k}, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		__mmask16 k = 0x5555;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 2;
			xa.m512i_i16[i] = i;
			xb.m512i_i16[i] = 3;
			xsrc.m512i_i32[j] = -1;
		}
		decimalPrint512x16(&xa);
		decimalPrint512x16(&xb);
		decimalPrint512x32(&xsrc);
		hexPrintWord(&k);
		__m512i ydst = _mm512_mask_dpwssd_epi32(xsrc, k, xa, xb);
		decimalPrint512x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 26) || (selector == -1))
	{	// vpdpwssd zmm {z}, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		__mmask16 k = 0x5555;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 2;
			xa.m512i_i16[i] = i;
			xb.m512i_i16[i] = 3;
			xsrc.m512i_i32[j] = -1;
		}
		decimalPrint512x16(&xa);
		decimalPrint512x16(&xb);
		decimalPrint512x32(&xsrc);
		hexPrintWord(&k);
		__m512i ydst = _mm512_maskz_dpwssd_epi32(k, xsrc, xa, xb);
		decimalPrint512x32(&ydst);
		AppLib::write("\r\n");
	}
	if ((selector == 27) || (selector == -1))
	{	// vpdpwssds xmm, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 4;
			xa.m128i_i8[i] = i;
			xb.m128i_i8[i] = -1;
			xsrc.m128i_i32[j] = INT_MIN;
		}
		decimalPrint128x8(&xa);
		decimalPrint128x8(&xb);
		decimalPrint128x32(&xsrc);
		AppLib::write("\r\n");
		__m128i ydstSaturated = _mm_dpwssds_epi32(xsrc, xa, xb);
		__m128i ydstNonSaturated = _mm_dpwssd_epi32(xsrc, xa, xb);
		decimalPrint128x32(&ydstSaturated);
		hexPrint128x32(&ydstSaturated);
		decimalPrint128x32(&ydstNonSaturated);
		hexPrint128x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 28) || (selector == -1))
	{	// vpdpwssds xmm {k}, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		__mmask8 k = 0xA;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 4;
			xa.m128i_i8[i] = i;
			xb.m128i_i8[i] = -1;
			xsrc.m128i_i32[j] = INT_MIN;
		}
		decimalPrint128x8(&xa);
		decimalPrint128x8(&xb);
		decimalPrint128x32(&xsrc);
		AppLib::write("\r\n");
		__m128i ydstSaturated = _mm_mask_dpwssds_epi32(xsrc, k, xa, xb);
		__m128i ydstNonSaturated = _mm_mask_dpwssd_epi32(xsrc, k, xa, xb);
		decimalPrint128x32(&ydstSaturated);
		hexPrint128x32(&ydstSaturated);
		decimalPrint128x32(&ydstNonSaturated);
		hexPrint128x32(&ydstNonSaturated);
		AppLib::write("\r\n");

	}
	if ((selector == 29) || (selector == -1))
	{	// vpdpwssds xmm {z}, xmm, xmm
		__m128i xa;
		__m128i xb;
		__m128i xsrc;
		__mmask8 k = 0xA;
		for (int i = 0; i < 16; i++)
		{
			int j = i / 4;
			xa.m128i_i8[i] = i;
			xb.m128i_i8[i] = -1;
			xsrc.m128i_i32[j] = INT_MIN;
		}
		decimalPrint128x8(&xa);
		decimalPrint128x8(&xb);
		decimalPrint128x32(&xsrc);
		AppLib::write("\r\n");
		__m128i ydstSaturated = _mm_maskz_dpwssds_epi32(k, xsrc, xa, xb);
		__m128i ydstNonSaturated = _mm_maskz_dpwssd_epi32(k, xsrc, xa, xb);
		decimalPrint128x32(&ydstSaturated);
		hexPrint128x32(&ydstSaturated);
		decimalPrint128x32(&ydstNonSaturated);
		hexPrint128x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 30) || (selector == -1))
	{	// vpdpwssds ymm, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 4;
			xa.m256i_i8[i] = i;
			xb.m256i_i8[i] = -1;
			xsrc.m256i_i32[j] = INT_MIN;
		}
		decimalPrint256x8(&xa);
		decimalPrint256x8(&xb);
		decimalPrint256x32(&xsrc);
		AppLib::write("\r\n");
		__m256i ydstSaturated = _mm256_dpwssds_epi32(xsrc, xa, xb);
		__m256i ydstNonSaturated = _mm256_dpwssd_epi32(xsrc, xa, xb);
		decimalPrint256x32(&ydstSaturated);
		hexPrint256x32(&ydstSaturated);
		decimalPrint256x32(&ydstNonSaturated);
		hexPrint256x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 31) || (selector == -1))
	{	// vpdpwssds ymm {k}, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		__mmask8 k = 0xE3;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 4;
			xa.m256i_i8[i] = i;
			xb.m256i_i8[i] = -1;
			xsrc.m256i_i32[j] = INT_MIN;
		}
		decimalPrint256x8(&xa);
		decimalPrint256x8(&xb);
		decimalPrint256x32(&xsrc);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		__m256i ydstSaturated = _mm256_mask_dpwssds_epi32(xsrc, k, xa, xb);
		__m256i ydstNonSaturated = _mm256_mask_dpwssd_epi32(xsrc, k, xa, xb);
		decimalPrint256x32(&ydstSaturated);
		hexPrint256x32(&ydstSaturated);
		decimalPrint256x32(&ydstNonSaturated);
		hexPrint256x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 32) || (selector == -1))
	{	// vpdpwssds ymm {z}, ymm, ymm
		__m256i xa;
		__m256i xb;
		__m256i xsrc;
		__mmask8 k = 0xE3;
		for (int i = 0; i < 32; i++)
		{
			int j = i / 4;
			xa.m256i_i8[i] = i;
			xb.m256i_i8[i] = -1;
			xsrc.m256i_i32[j] = INT_MIN;
		}
		decimalPrint256x8(&xa);
		decimalPrint256x8(&xb);
		decimalPrint256x32(&xsrc);
		hexPrintByte(&k);
		AppLib::write("\r\n");
		__m256i ydstSaturated = _mm256_maskz_dpwssds_epi32(k, xsrc, xa, xb);
		__m256i ydstNonSaturated = _mm256_maskz_dpwssd_epi32(k, xsrc, xa, xb);
		decimalPrint256x32(&ydstSaturated);
		hexPrint256x32(&ydstSaturated);
		decimalPrint256x32(&ydstNonSaturated);
		hexPrint256x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 33) || (selector == -1))
	{	// vpdpwssds zmm, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		for (int i = 0; i < 64; i++)
		{
			int j = i / 4;
			xa.m512i_i8[i] = i;
			xb.m512i_i8[i] = -1;
			xsrc.m512i_i32[j] = INT_MIN;
		}
		decimalPrint512x8(&xa);
		decimalPrint512x8(&xb);
		decimalPrint512x32(&xsrc);
		AppLib::write("\r\n");
		__m512i ydstSaturated = _mm512_dpwssds_epi32(xsrc, xa, xb);
		__m512i ydstNonSaturated = _mm512_dpwssd_epi32(xsrc, xa, xb);
		decimalPrint512x32(&ydstSaturated);
		hexPrint512x32(&ydstSaturated);
		decimalPrint512x32(&ydstNonSaturated);
		hexPrint512x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 34) || (selector == -1))
	{	// vpdpwssds zmm {k}, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		__mmask16 k = 0xF003;
		for (int i = 0; i < 64; i++)
		{
			int j = i / 4;
			xa.m512i_i8[i] = i;
			xb.m512i_i8[i] = -1;
			xsrc.m512i_i32[j] = INT_MIN;
		}
		decimalPrint512x8(&xa);
		decimalPrint512x8(&xb);
		decimalPrint512x32(&xsrc);
		hexPrintWord(&k);
		AppLib::write("\r\n");
		__m512i ydstSaturated = _mm512_mask_dpwssds_epi32(xsrc, k, xa, xb);
		__m512i ydstNonSaturated = _mm512_mask_dpwssd_epi32(xsrc, k, xa, xb);
		decimalPrint512x32(&ydstSaturated);
		hexPrint512x32(&ydstSaturated);
		decimalPrint512x32(&ydstNonSaturated);
		hexPrint512x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}
	if ((selector == 35) || (selector == -1))
	{	// vpdpwssds zmm {z}, zmm, zmm
		__m512i xa;
		__m512i xb;
		__m512i xsrc;
		__mmask16 k = 0xF003;
		for (int i = 0; i < 64; i++)
		{
			int j = i / 4;
			xa.m512i_i8[i] = i;
			xb.m512i_i8[i] = -1;
			xsrc.m512i_i32[j] = INT_MIN;
		}
		decimalPrint512x8(&xa);
		decimalPrint512x8(&xb);
		decimalPrint512x32(&xsrc);
		hexPrintWord(&k);
		AppLib::write("\r\n");
		__m512i ydstSaturated = _mm512_maskz_dpwssds_epi32(k, xsrc, xa, xb);
		__m512i ydstNonSaturated = _mm512_maskz_dpwssd_epi32(k, xsrc, xa, xb);
		decimalPrint512x32(&ydstSaturated);
		hexPrint512x32(&ydstSaturated);
		decimalPrint512x32(&ydstNonSaturated);
		hexPrint512x32(&ydstNonSaturated);
		AppLib::write("\r\n");
	}

	return exitCode;
}
