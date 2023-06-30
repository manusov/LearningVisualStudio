/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avx512techs=AVX512F&cats=Miscellaneous&ig_expand=262
*/

#include "Task_21.h"

int Task_21::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[Context for vector data intrinsics - miscellaneous.]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);
/*
// valignd xmm, xmm, xmm, imm8 (valignd dst, src1, src2).
// concatenate src1:src2 as 256-bit vector, shift right by imm8 32-bit dwords, store low 128 bits to dst.
	__m128i x1{ 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	__m128i x2{ 0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22 };
	// __debugbreak();
	__m128i y = _mm_alignr_epi32(x1, x2, 1);
	hexPrint128x32(&x1);
	hexPrint128x32(&x2);
	hexPrint128x32(&y, 4, APPCONST::TABLE_COLOR);
*/
/*
// valignd xmm, xmm, xmm, imm8
// valignd xmm {k}, xmm, xmm, imm8
// valignd xmm {z}, xmm, xmm, imm8
	__m128i x1{ 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	__m128i x2{ 0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22 };
	__m128i x3{ 0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33 };
	__mmask8 m = 0xE;
	// __debugbreak();
	__m128i y1 = _mm_alignr_epi32(x2, x3, 1);
	__m128i y2 = _mm_mask_alignr_epi32(x1, m, x2, x3, 1);
	__m128i y3 = _mm_maskz_alignr_epi32(m, x2, x3, 1);
	hexPrint128x32(&x1);
	hexPrint128x32(&x2);
	hexPrint128x32(&x3);
	hexPrint128x32(&y1, 4, APPCONST::TABLE_COLOR);
	hexPrint128x32(&y2, 4, APPCONST::TABLE_COLOR);
	hexPrint128x32(&y3, 4, APPCONST::TABLE_COLOR);
// Same functionality supported for 256- and 512-bit operands at YMM and ZMM registers.
// Same functionality supported with 64-bit granularity.
*/
/*
// vpblendmd xmm{k},xmm,xmm
	__m128i x1{ 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	__m128i x2{ 0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22 };
	__m128i x3{ 0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33 };
	__mmask8 m = 0x5;
	__debugbreak();
	__m128i y1 = _mm_mask_blend_epi32(m, x2, x3);
	hexPrint128x32(&x2);
	hexPrint128x32(&x3);
	hexPrint128x32(&y1, 4, APPCONST::TABLE_COLOR);
// Same functionality supported for 256-bit operands at YMM registers.
// Same functionality supported with 64-bit granularity, integer 64-bit, integer 32-bit, float 32-bit, double 64-bit.
// Note can blend 512-bit operands by predicate masks for example by vmovapd zmm{k},zmm.
*/
/*
// vbroadcastf32x4 ymm, m128
// Broadcast 128-bit XMM at 256-bit YMM.
	__m128 x1{ 1.55, 1.76, 1.0, -1.0 };
	__m256 x2{ 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1 };
	// __debugbreak();
	__m256 y1 = _mm256_broadcast_f32x4(x1);
	__m256 y2 = _mm256_mask_broadcast_f32x4(x2, 0xC3, x1);
	__m256 y3 = _mm256_maskz_broadcast_f32x4(0xC3, x1);
	floatPrint128x32(&x1);
	floatPrint256x32(&y1, 8, APPCONST::TABLE_COLOR);
	floatPrint256x32(&y2, 8, APPCONST::TABLE_COLOR);
	floatPrint256x32(&y3, 8, APPCONST::TABLE_COLOR);
*/
/*
// vpbroadcastd xmm {k}, xmm
	__m128i x1{ 0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0xFF,0xFF,0xEE,0xDD,0x77,0x78,0x34,0x86 };
	__m128i x2{ 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	// __debugbreak();
	__m128i y1 = _mm_mask_broadcastd_epi32(x2, 0x7, x1);
	hexPrint128x32(&x1);
	hexPrint128x32(&x2);
	hexPrint128x32(&y1, 4, APPCONST::TABLE_COLOR);
*/
/*
// vpbroadcastd ymm {k}, xmm
	__m128i x1{ 0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0xFF,0xFF,0xEE,0xDD,0x77,0x78,0x34,0x86 };
	__m256i x2{ 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
		        0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	// __debugbreak();
	__m256i y1 = _mm256_mask_broadcastd_epi32(x2, 0xE3, x1);
	__m256i y2 = _mm256_maskz_broadcastd_epi32(0xE3, x1);
	hexPrint128x32(&x1);
	hexPrint256x32(&x2, 8);
	hexPrint256x32(&y1, 8, APPCONST::TABLE_COLOR);
	hexPrint256x32(&y2, 8, APPCONST::TABLE_COLOR);
*/
/*
// vpcompressd xmm{k}, xmm
// vpcompressd xmm{z}, xmm
	__m128i x1{ 0x22,0x22,0x22,0x22,0x33,0x33,0x33,0x33,0x44,0x44,0x44,0x44,0x55,0x55,0x55,0x55 };
	__m128i x2{ 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	// __debugbreak();
	__m128i y1 = _mm_mask_compress_epi32(x2, 0x9, x1);
	__m128i y2 = _mm_maskz_compress_epi32(0x9, x1);
	hexPrint128x32(&x1);
	hexPrint128x32(&x2);
	hexPrint128x32(&y1, 4, APPCONST::TABLE_COLOR);
	hexPrint128x32(&y2, 4, APPCONST::TABLE_COLOR);
*/
/*
// vpcompressd ymm{k}, ymm
// vpcompressd ymm{z}, ymm
	__m256i x1{ 0x22,0x22,0x22,0x22,0x33,0x33,0x33,0x33,0x44,0x44,0x44,0x44,0x55,0x55,0x55,0x55,
	            0x66,0x66,0x66,0x66,0x77,0x77,0x77,0x77,0x88,0x88,0x88,0x88,0x99,0x99,0x99,0x99 };
	__m256i x2{ 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
	            0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	// __debugbreak();
	__m256i y1 = _mm256_mask_compress_epi32(x2, 0x89, x1);
	__m256i y2 = _mm256_maskz_compress_epi32(0x89, x1);
	hexPrint256x32(&x1, 8);
	hexPrint256x32(&x2, 8);
	hexPrint256x32(&y1, 8, APPCONST::TABLE_COLOR);
	hexPrint256x32(&y2, 8, APPCONST::TABLE_COLOR);
*/
/*
// vpexpanddd xmm{k}, xmm
// vpexpanddd xmm{z}, xmm
	__m128i x1{ 0x22,0x22,0x22,0x22,0x33,0x33,0x33,0x33,0x44,0x44,0x44,0x44,0x55,0x55,0x55,0x55 };
	__m128i x2{ 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	// __debugbreak();
	__m128i y1 = _mm_mask_expand_epi32(x2, 0x9, x1);
	__m128i y2 = _mm_maskz_expand_epi32(0x9, x1);
	hexPrint128x32(&x1);
	hexPrint128x32(&x2);
	hexPrint128x32(&y1, 4, APPCONST::TABLE_COLOR);
	hexPrint128x32(&y2, 4, APPCONST::TABLE_COLOR);
*/
/*
// vextractf32x4 xmm, ymm, imm8
// vextractf32x4 xmm{k}, ymm, imm8
// vextractf32x4 xmm{z}, ymm, imm8
	__m256 x1{ 2.1, 3.1, 4.1, 5.1, 6.1, 7.1, 8.1, 9.1 };
	__m128 x2{ 1.1, 1.1, 1.1, 1.1 };
	__m128 y1 = _mm256_extractf32x4_ps(x1, 0);
	__m128 y2 = _mm256_extractf32x4_ps(x1, 1);
	__m128 y3 = _mm256_mask_extractf32x4_ps(x2, 0x5, x1, 1);
	__m128 y4 = _mm256_maskz_extractf32x4_ps(0x5, x1, 1);
	floatPrint256x32(&x1);
	floatPrint128x32(&y1, 4, APPCONST::TABLE_COLOR);
	floatPrint128x32(&y2, 4, APPCONST::TABLE_COLOR);
	floatPrint128x32(&y3, 4, APPCONST::TABLE_COLOR);
	floatPrint128x32(&y4, 4, APPCONST::TABLE_COLOR);
*/
/*
// https://www.officedaytime.com/simd512e/simdimg/fixup.php?f=vfixupimmpd
// vfixupimmpd xmm, xmm, xmm, imm8
	__m128d x1{ 3.4, 3.4 };
	__m128d x2{ 0.0, 7.3 };
	__m128i x3{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	__m128i x4{ 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	__m128i x5{ 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	__m128d y1 = _mm_fixupimm_pd(x1, x2, x3, 0);
	__m128d y2 = _mm_fixupimm_pd(x1, x2, x4, 0);
	__m128d y3 = _mm_fixupimm_pd(x1, x2, x5, 0);
	doublePrint128x64(&x1);
	doublePrint128x64(&x2);
	hexPrint128x32(&x3);
	hexPrint128x32(&x4);
	hexPrint128x32(&x5);
	doublePrint128x64(&y1, 2, APPCONST::TABLE_COLOR);
	doublePrint128x64(&y2, 2, APPCONST::TABLE_COLOR);
	doublePrint128x64(&y3, 2, APPCONST::TABLE_COLOR);
*/
/*
// vgetexppd xmm, xmm
	__m128d x1{ 3.4, 1E2 };
	__m128d y = _mm_getexp_pd(x1);
	doublePrint128x64(&x1);
	doublePrint128x64(&y, 2, APPCONST::TABLE_COLOR);
*/
/*
// vgetmantpd xmm, xmm
	__m128d x1{ 3.4, 1E2 };
	__m128d y = _mm_getmant_pd(x1, _MM_MANT_NORM_1_2, _MM_MANT_SIGN_src);
	doublePrint128x64(&x1);
	doublePrint128x64(&y, 2, APPCONST::TABLE_COLOR);
*/
/*
// vinsertf32x4 ymm, ymm, xmm, imm8
	__m256 x1{ 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1 };
	__m128 x2{ 1.2, 3.4, 5.6, 7.8 };
	__m256 y1 = _mm256_insertf32x4(x1, x2, 0);
	__m256 y2 = _mm256_insertf32x4(x1, x2, 1);
	floatPrint256x32(&x1);
	floatPrint128x32(&x2);
	floatPrint256x32(&y1, 8, APPCONST::TABLE_COLOR);
	floatPrint256x32(&y2, 8, APPCONST::TABLE_COLOR);
*/
/*
// vpermilpd xmm{ k }, xmm, imm8
// vpermilpd xmm{ z }, xmm, imm8
	__m128d x1{ 1.1, 1.1 };
	__m128d x2{ 1.2, 3.4 };
	__m128d y1 = _mm_mask_permute_pd(x1, 0x3, x2, 0x00);
	__m128d y2 = _mm_mask_permute_pd(x1, 0x3, x2, 0x01);
	__m128d y3 = _mm_mask_permute_pd(x1, 0x3, x2, 0x02);
	__m128d y4 = _mm_mask_permute_pd(x1, 0x3, x2, 0x03);
	__m128d y5 = _mm_maskz_permute_pd(0x0, x2, 0x00);
	__m128d y6 = _mm_maskz_permute_pd(0x1, x2, 0x01);
	__m128d y7 = _mm_maskz_permute_pd(0x2, x2, 0x02);
	__m128d y8 = _mm_maskz_permute_pd(0x3, x2, 0x03);
	doublePrint128x64(&x1);
	doublePrint128x64(&x2);
	doublePrint128x64(&y1, 2, APPCONST::TABLE_COLOR);
	doublePrint128x64(&y2, 2, APPCONST::TABLE_COLOR);
	doublePrint128x64(&y3, 2, APPCONST::TABLE_COLOR);
	doublePrint128x64(&y4, 2, APPCONST::TABLE_COLOR);
	doublePrint128x64(&y5, 2, APPCONST::TABLE_COLOR);
	doublePrint128x64(&y6, 2, APPCONST::TABLE_COLOR);
	doublePrint128x64(&y7, 2, APPCONST::TABLE_COLOR);
	doublePrint128x64(&y8, 2, APPCONST::TABLE_COLOR);
*/
/*
// vpermilps ymm{k},ymm,imm8
// vpermilps ymm{z},ymm,imm8
	__m256 x1{ 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9 };
	__m256 x2{ 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1 };
	__m256 y1 = _mm256_mask_permute_ps(x2, 0x7F, x1, 0x00);
	__m256 y2 = _mm256_maskz_permute_ps(0x7F, x1, 0x00);
	floatPrint256x32(&x1);
	floatPrint256x32(&x2);
	floatPrint256x32(&y1, 8, APPCONST::TABLE_COLOR);
	floatPrint256x32(&y2, 8, APPCONST::TABLE_COLOR);
*/
/*
// vpermilpd xmm {k}, xmm, xmm
// vpermilpd xmm {z}, xmm, xmm
	__m128d x1{ 2.2, 3.3 };
	__m128d x2{ 1.1, 1.1 };
	__m128i x3{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	__m128i x4{ 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	__m128d y1 = _mm_mask_permutevar_pd(x2, 0x3, x1, x3);
	__m128d y2 = _mm_mask_permutevar_pd(x2, 0x3, x1, x4);
	doublePrint128x64(&x1);
	doublePrint128x64(&x2);
	doublePrint128x64(&y1, 2, APPCONST::TABLE_COLOR);
	doublePrint128x64(&y2, 2, APPCONST::TABLE_COLOR);
*/
/*
// vpermq ymm {k}, ymm, imm8
// Cross-lane for 256 = 128 + 128. Note use AVX512DQ instructions for cross-lane 512 = 256 + 256.
	__m256i x1{ 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	            0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
	__m256i x2{ 1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0 };
	__m256i y1 = _mm256_mask_permutex_epi64(x2, 0x7, x1, 0x00);
	__m256i y2 = _mm256_mask_permutex_epi64(x2, 0x7, x1, 0xFF);
	decimalPrint256x64(&x1, 4);
	decimalPrint256x64(&x2, 4);
	decimalPrint256x64(&y1, 4, APPCONST::TABLE_COLOR);
	decimalPrint256x64(&y2, 4, APPCONST::TABLE_COLOR);
*/

// vprorvd zmm {z}, zmm, zmm
	__m512i x1{ 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0 };
	__m512i x2{ 1,0,0,0, 2,0,0,0, 3,0,0,0, 4,0,0,0, 5,0,0,0, 6,0,0,0, 7,0,0,0, 8,0,0,0, 20,0,0,0, 21,0,0,0, 22,0,0,0, 23,0,0,0, 24,0,0,0, 25,0,0,0, 26,0,0,0, 27,0,0,0 };
	__m512i y1 = _mm512_maskz_rorv_epi32(0xFFFF, x1, x2);
	__m512i y2 = _mm512_maskz_rorv_epi32(0x7FFF, x1, x2);
	hexPrint512x32(&x1, 8);
	hexPrint512x32(&x2, 8);
	hexPrint512x32(&y1, 8, APPCONST::TABLE_COLOR);
	hexPrint512x32(&y2, 8, APPCONST::TABLE_COLOR);



	return exitCode;
}

