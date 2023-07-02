/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ig_expand=5121,5122,5103&avx512techs=AVX512_VBMI
*/

#include "Task_24.h"

int Task_24::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[Context for vector data intrinsics - AVX512 VBMI.]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

/*
	// __m128i _mm_mask_multishift_epi64_epi8 (__m128i src, __mmask16 k, __m128i a, __m128i b)
	// __m128i _mm_maskz_multishift_epi64_epi8 (__mmask16 k, __m128i a, __m128i b)
	// __m128i _mm_multishift_epi64_epi8 (__m128i a, __m128i b)
	// 256- and 512-bit forms also exists.
	// vpmultishiftqb xmm {k}, xmm, xmm
	// xa = 16 bytes, bit positions selector bytes for 64-bit shifts for each destination byte,
	// xb = 2 qwords, data, selected by xa-bytes,
	__m128i xsrc = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
	__m128i xa   = { 0x04, 0x03, 0x05, 0x37, 0x36, 0x35, 0x34, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x17 };
	__m128i xb   = { 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF };
	hexPrint128x8(&xsrc);
	hexPrint128x8(&xa);
	hexPrint128x8(&xb);
	__m128i y1 = _mm_mask_multishift_epi64_epi8(xsrc, 0xE7FF, xa, xb);
	hexPrint128x8(&y1, 16, APPCONST::TABLE_COLOR);
	__m128i y2 = _mm_maskz_multishift_epi64_epi8(0xE7FF, xa, xb);
	hexPrint128x8(&y2, 16, APPCONST::TABLE_COLOR);
	__m128i y3 = _mm_multishift_epi64_epi8(xa, xb);
	hexPrint128x8(&y3, 16, APPCONST::TABLE_COLOR);
*/
/*
	// __m128i _mm_mask_permutex2var_epi8 (__m128i a, __mmask16 k, __m128i idx, __m128i b)
	// vpermt2b xmm {k}, xmm, xmm
	__m128i xa = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };
	__m128i xb = { 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x40 };
	__m128i xi = { 0x1F, 0x1E, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x01, 0x10, 0x00 };
	hexPrint128x8(&xa);
	hexPrint128x8(&xb);
	hexPrint128x8(&xi);
	__m128i y1 = _mm_mask_permutex2var_epi8(xa, 0xFFEF, xi, xb);
	hexPrint128x8(&y1, 16, APPCONST::TABLE_COLOR);
*/

	// __m512i _mm512_permutexvar_epi8 (__m512i idx, __m512i a)
	// vpermb zmm, zmm, zmm
	// Note this is cross-lane operation.
	__m512i xa;
	__m512i xb;
	for (int i = 0; i < 64; i++)
	{
		xa.m512i_i8[i] = i + 0x40;
		xb.m512i_i8[i] = 0;
	}
	xb.m512i_i8[0] = 63;
	xb.m512i_i8[1] = 62;
	xb.m512i_i8[2] = 1;
	hexPrint512x8(&xa);
	hexPrint512x8(&xb);
	__m512i y = _mm512_permutexvar_epi8(xb, xa);
	hexPrint512x8(&y, 16, APPCONST::TABLE_COLOR);




	return exitCode;
}
