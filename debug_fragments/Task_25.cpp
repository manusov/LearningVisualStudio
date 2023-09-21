/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avx512techs=AVX512_VBMI2
*/

#include "Task_25.h"

int Task_25::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[Context for vector data intrinsics - AVX512 VBMI2.]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);
/*
	// __m512i _mm512_maskz_compress_epi8 (__mmask64 k, __m512i a)
	// vpcompressb zmm {z}, zmm
	// Note this is cross-lane operation.
	__m512i xa;
	for (int i = 0; i < 64; i++)
	{
		xa.m512i_i8[i] = i + 0x80;
	}
	__mmask64 k = 0xC000000000000055ULL;
	hexPrint512x8(&xa);
	__m512i y = _mm512_maskz_compress_epi8(k, xa);
	hexPrint512x8(&y, 16, APPCONST::TABLE_COLOR);
*/
/*
	// __m512i _mm512_mask_compress_epi8 (__m512i src, __mmask64 k, __m512i a)
	// vpcompressb zmm {k}, zmm
	// Note this is cross-lane operation.
	__m512i xa;
	__m512i xsrc;
	for (int i = 0; i < 64; i++)
	{
		xa.m512i_i8[i] = i + 0x80;
		xsrc.m512i_i8[i] = 0x11;
	}
	__mmask64 k = 0xC000000000000055ULL;
	hexPrint512x8(&xa);
	hexPrint512x8(&xsrc);
	__m512i y = _mm512_mask_compress_epi8(xsrc, k, xa);
	hexPrint512x8(&y, 16, APPCONST::TABLE_COLOR);
*/
/*
	// __m512i _mm512_mask_expand_epi8 (__m512i src, __mmask64 k, __m512i a)
	// vpexpandb zmm {k}, zmm
	// Note this is cross-lane operation.
	__m512i xa;
	__m512i xsrc;
	for (int i = 0; i < 64; i++)
	{
		xa.m512i_i8[i] = i + 0x80;
		xsrc.m512i_i8[i] = 0x11;
	}
	__mmask64 k = 0xC000000000000055ULL;
	hexPrint512x8(&xa);
	hexPrint512x8(&xsrc);
	__m512i y = _mm512_mask_expand_epi8(xsrc, k, xa);
	hexPrint512x8(&y, 16, APPCONST::TABLE_COLOR);
*/
	// __m512i _mm512_mask_shldi_epi16 (__m512i src, __mmask32 k, __m512i a, __m512i b, int imm8)
	// vpshldw zmm {k}, zmm, zmm, imm8
	__m512i xa;
	__m512i xb;
	__m512i xsrc;
	for (int i = 0; i < 64; i++)
	{
		xa.m512i_i8[i] = 0x00;
		xb.m512i_i8[i] = 0xFF;
		xsrc.m512i_i8[i] = 0x11;
	}
	hexPrint512x8(&xa);
	hexPrint512x8(&xb);
	hexPrint512x8(&xsrc);
	__mmask32 k = 0x0FFFFFFE;
	constexpr int imm8 = 3;
	__m512i y = _mm512_mask_shldi_epi16(xsrc, k, xa, xb, imm8);
	hexPrint512x8(&y, 16, APPCONST::TABLE_COLOR);




	return exitCode;
}
