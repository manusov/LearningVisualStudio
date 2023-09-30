/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#othertechs=GFNI
https://gist.github.com/IJzerbaard/c5c7bcfae2910e714605ba7bf2ca9b42
See document:
TECHNOLOGY GUIDE. Intel Corporation. Galois Field New Instructions (GFNI).
644497-002US.
galois-field-new-instructions-gfni-technology-guide-1-1639042826.PDF
*/

#include "Task_32.h"

int Task_32::execute(int argc, char** argv)
{
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[AVX512 GFNI]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	int exitCode = 0;
	int selector = 26;  // -1;

	if ((selector == 0) || (selector == -1))
	{	// vgf2p8affineqb xmm, xmm, xmm, imm8
		__m128i x;
		__m128i A;
		for (int i = 0; i < 16; i++)
		{
			x.m128i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m128i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m128i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		hexPrint128x8(&x);
		hexPrint128x8(&A);
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__m128i dst = _mm_gf2p8affine_epi64_epi8(x, A, b);
		AppLib::write("\r\n");
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrint128x8(&dst);
		const int b1 = 0xFF;                  // Set constant imm8 = byte for result bytes XOR. 0xFF means invert all bits.
		dst = _mm_gf2p8affine_epi64_epi8(x, A, b1);
		AppLib::write("\r\n");
		hexPrintDword(reinterpret_cast<const DWORD*>(&b1));
		hexPrint128x8(&dst);
	}
	if ((selector == 1) || (selector == -1))
	{	// vgf2p8affineqb xmm {k}, xmm, xmm, imm8
		__m128i x;
		__m128i A;
		__m128i src = _mm_set1_epi8(0x11);
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask16 k = 0xF0FF;
		for (int i = 0; i < 16; i++)
		{
			x.m128i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m128i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m128i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		hexPrint128x8(&src);
		hexPrint128x8(&x);
		hexPrint128x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintWord(&k);
		__m128i dst = _mm_mask_gf2p8affine_epi64_epi8(src, k, x, A, b);
		AppLib::write("\r\n");
		hexPrint128x8(&dst);
	}
	if ((selector == 2) || (selector == -1))
	{	// vgf2p8affineqb xmm {z}, xmm, xmm, imm8
		__m128i x;
		__m128i A;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask16 k = 0xF0FF;
		for (int i = 0; i < 16; i++)
		{
			x.m128i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m128i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m128i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		hexPrint128x8(&x);
		hexPrint128x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintWord(&k);
		__m128i dst = _mm_maskz_gf2p8affine_epi64_epi8(k, x, A, b);
		AppLib::write("\r\n");
		hexPrint128x8(&dst);
	}
	if ((selector == 3) || (selector == -1))
	{	// vgf2p8affineqb ymm, ymm, ymm, imm8
		__m256i x;
		__m256i A;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		for (int i = 0; i < 32; i++)
		{
			x.m256i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m256i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m256i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m256i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m256i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		hexPrint256x8(&x);
		hexPrint256x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		__m256i dst = _mm256_gf2p8affine_epi64_epi8(x, A, b);
		AppLib::write("\r\n");
		hexPrint256x8(&dst);
	}
	if ((selector == 4) || (selector == -1))
	{	// vgf2p8affineqb ymm {k}, ymm, ymm, imm8
		__m256i x;
		__m256i A;
		__m256i src;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask32 k = 0xF0FFFFFF;
		for (int i = 0; i < 32; i++)
		{
			x.m256i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
			src.m256i_i8[i] = 0x11;
		}
		A.m256i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m256i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m256i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m256i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		hexPrint256x8(&src);
		hexPrint256x8(&x);
		hexPrint256x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_mask_gf2p8affine_epi64_epi8(src, k, x, A, b);
		AppLib::write("\r\n");
		hexPrint256x8(&dst);
	}
	if ((selector == 5) || (selector == -1))
	{	// vgf2p8affineqb ymm {z}, ymm, ymm, imm8
		__m256i x;
		__m256i A;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask32 k = 0xF0FFFFFF;
		for (int i = 0; i < 32; i++)
		{
			x.m256i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m256i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m256i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m256i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m256i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		hexPrint256x8(&x);
		hexPrint256x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_maskz_gf2p8affine_epi64_epi8(k, x, A, b);
		AppLib::write("\r\n");
		hexPrint256x8(&dst);
	}
	if ((selector == 6) || (selector == -1))
	{	// vgf2p8affineqb zmm, zmm, zmm, imm8
		__m512i x;
		__m512i A;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		for (int i = 0; i < 64; i++)
		{
			x.m512i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m512i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m512i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m512i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m512i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		A.m512i_i64[4] = 0x0202020202020202;
		A.m512i_i64[5] = 0x0404040404040404;
		A.m512i_i64[6] = 0x0808080808080808;
		A.m512i_i64[7] = 0xFFFFFFFFFFFFFFFF;
		hexPrint512x8(&x);
		hexPrint512x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		__m512i dst = _mm512_gf2p8affine_epi64_epi8(x, A, b);
		AppLib::write("\r\n");
		hexPrint512x8(&dst);
	}
	if ((selector == 7) || (selector == -1))
	{	// vgf2p8affineqb zmm {k}, zmm, zmm, imm8
		__m512i x;
		__m512i A;
		__m512i src;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask64 k = 0x00000FFFFFFFFFFF;
		for (int i = 0; i < 64; i++)
		{
			x.m512i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
			src.m512i_i8[i] = 0x11;
		}
		A.m512i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m512i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m512i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m512i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		A.m512i_i64[4] = 0x0202020202020202;
		A.m512i_i64[5] = 0x0404040404040404;
		A.m512i_i64[6] = 0x0808080808080808;
		A.m512i_i64[7] = 0xFFFFFFFFFFFFFFFF;
		hexPrint512x8(&src);
		hexPrint512x8(&x);
		hexPrint512x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_mask_gf2p8affine_epi64_epi8(src, k, x, A, b);
		AppLib::write("\r\n");
		hexPrint512x8(&dst);
	}
	if ((selector == 8) || (selector == -1))
	{	// vgf2p8affineqb zmm {z}, zmm, zmm, imm8
		__m512i x;
		__m512i A;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask64 k = 0x00000FFFFFFFFFFF;
		for (int i = 0; i < 64; i++)
		{
			x.m512i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m512i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m512i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m512i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m512i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		A.m512i_i64[4] = 0x0202020202020202;
		A.m512i_i64[5] = 0x0404040404040404;
		A.m512i_i64[6] = 0x0808080808080808;
		A.m512i_i64[7] = 0xFFFFFFFFFFFFFFFF;
		hexPrint512x8(&x);
		hexPrint512x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_maskz_gf2p8affine_epi64_epi8(k, x, A, b);
		AppLib::write("\r\n");
		hexPrint512x8(&dst);
	}
	if ((selector == 9) || (selector == -1))
	{	// vgf2p8affineinvqb xmm, xmm, xmm, imm8
		__m128i x;
		__m128i A;
		for (int i = 0; i < 16; i++)
		{
			x.m128i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m128i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m128i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		hexPrint128x8(&x);
		hexPrint128x8(&A);
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__m128i dst = _mm_gf2p8affineinv_epi64_epi8(x, A, b);
		AppLib::write("\r\n");
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrint128x8(&dst);
		const int b1 = 0xFF;                  // Set constant imm8 = byte for result bytes XOR. 0xFF means invert all bits.
		dst = _mm_gf2p8affineinv_epi64_epi8(x, A, b1);
		AppLib::write("\r\n");
		hexPrintDword(reinterpret_cast<const DWORD*>(&b1));
		hexPrint128x8(&dst);
	}
	if ((selector == 10) || (selector == -1))
	{	// vgf2p8affineinvqb xmm {k}, xmm, xmm, imm8
		__m128i x;
		__m128i A;
		__m128i src = _mm_set1_epi8(0x11);
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask16 k = 0xF0FF;
		for (int i = 0; i < 16; i++)
		{
			x.m128i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m128i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m128i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		hexPrint128x8(&src);
		hexPrint128x8(&x);
		hexPrint128x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintWord(&k);
		__m128i dst = _mm_mask_gf2p8affineinv_epi64_epi8(src, k, x, A, b);
		AppLib::write("\r\n");
		hexPrint128x8(&dst);
	}
	if ((selector == 11) || (selector == -1))
	{	// vgf2p8affineinvqb xmm {z}, xmm, xmm, imm8
		__m128i x;
		__m128i A;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask16 k = 0xF0FF;
		for (int i = 0; i < 16; i++)
		{
			x.m128i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m128i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m128i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		hexPrint128x8(&x);
		hexPrint128x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintWord(&k);
		__m128i dst = _mm_maskz_gf2p8affineinv_epi64_epi8(k, x, A, b);
		AppLib::write("\r\n");
		hexPrint128x8(&dst);
	}
	if ((selector == 12) || (selector == -1))
	{	// vgf2p8affineinvqb ymm, ymm, ymm, imm8
		__m256i x;
		__m256i A;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		for (int i = 0; i < 32; i++)
		{
			x.m256i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m256i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m256i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m256i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m256i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		hexPrint256x8(&x);
		hexPrint256x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		__m256i dst = _mm256_gf2p8affineinv_epi64_epi8(x, A, b);
		AppLib::write("\r\n");
		hexPrint256x8(&dst);
	}
	if ((selector == 13) || (selector == -1))
	{	// vgf2p8affineinvqb ymm {k}, ymm, ymm, imm8
		__m256i x;
		__m256i A;
		__m256i src;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask32 k = 0xF0FFFFFF;
		for (int i = 0; i < 32; i++)
		{
			x.m256i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
			src.m256i_i8[i] = 0x11;
		}
		A.m256i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m256i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m256i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m256i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		hexPrint256x8(&src);
		hexPrint256x8(&x);
		hexPrint256x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_mask_gf2p8affineinv_epi64_epi8(src, k, x, A, b);
		AppLib::write("\r\n");
		hexPrint256x8(&dst);
	}
	if ((selector == 14) || (selector == -1))
	{	// vgf2p8affineinvqb ymm {z}, ymm, ymm, imm8
		__m256i x;
		__m256i A;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask32 k = 0xF0FFFFFF;
		for (int i = 0; i < 32; i++)
		{
			x.m256i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m256i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m256i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m256i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m256i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		hexPrint256x8(&x);
		hexPrint256x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_maskz_gf2p8affineinv_epi64_epi8(k, x, A, b);
		AppLib::write("\r\n");
		hexPrint256x8(&dst);
	}
	if ((selector == 15) || (selector == -1))
	{	// vgf2p8affineinvqb zmm, zmm, zmm, imm8
		__m512i x;
		__m512i A;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		for (int i = 0; i < 64; i++)
		{
			x.m512i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m512i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m512i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m512i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m512i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		A.m512i_i64[4] = 0x0202020202020202;
		A.m512i_i64[5] = 0x0404040404040404;
		A.m512i_i64[6] = 0x0808080808080808;
		A.m512i_i64[7] = 0xFFFFFFFFFFFFFFFF;
		hexPrint512x8(&x);
		hexPrint512x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		__m512i dst = _mm512_gf2p8affineinv_epi64_epi8(x, A, b);
		AppLib::write("\r\n");
		hexPrint512x8(&dst);
	}
	if ((selector == 16) || (selector == -1))
	{	// vgf2p8affineinvqb zmm {k}, zmm, zmm, imm8
		__m512i x;
		__m512i A;
		__m512i src;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask64 k = 0x00000FFFFFFFFFFF;
		for (int i = 0; i < 64; i++)
		{
			x.m512i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
			src.m512i_i8[i] = 0x11;
		}
		A.m512i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m512i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m512i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m512i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		A.m512i_i64[4] = 0x0202020202020202;
		A.m512i_i64[5] = 0x0404040404040404;
		A.m512i_i64[6] = 0x0808080808080808;
		A.m512i_i64[7] = 0xFFFFFFFFFFFFFFFF;
		hexPrint512x8(&src);
		hexPrint512x8(&x);
		hexPrint512x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_mask_gf2p8affineinv_epi64_epi8(src, k, x, A, b);
		AppLib::write("\r\n");
		hexPrint512x8(&dst);
	}
	if ((selector == 17) || (selector == -1))
	{	// vgf2p8affineinvqb zmm {z}, zmm, zmm, imm8
		__m512i x;
		__m512i A;
		const int b = 0;                      // Set constant imm8 = byte for result bytes XOR. 0 means non invert bits.
		__mmask64 k = 0x00000FFFFFFFFFFF;
		for (int i = 0; i < 64; i++)
		{
			x.m512i_i8[i] = i % 8;            // Set x[0] = x[1] = vectors examples.
		}
		A.m512i_i64[0] = 0x0102040810204080;  // Set A[0] = identity matrix.
		A.m512i_i64[1] = 0x8040201008040201;  // Set A[1] = horizontally flipped identity matrix for bits reversal.
		A.m512i_i64[2] = 0x0102040801020408;  // Set A[2] = this matrix for replicate low nibble.
		A.m512i_i64[3] = 0x0101010101010101;  // Set A[3] = this matrix for replicate bit 0.
		A.m512i_i64[4] = 0x0202020202020202;
		A.m512i_i64[5] = 0x0404040404040404;
		A.m512i_i64[6] = 0x0808080808080808;
		A.m512i_i64[7] = 0xFFFFFFFFFFFFFFFF;
		hexPrint512x8(&x);
		hexPrint512x8(&A);
		hexPrintDword(reinterpret_cast<const DWORD*>(&b));
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_maskz_gf2p8affineinv_epi64_epi8(k, x, A, b);
		AppLib::write("\r\n");
		hexPrint512x8(&dst);
	}
	if ((selector == 18) || (selector == -1))
	{	// vgf2p8mulb xmm, xmm, xmm
		__m128i a;
		__m128i b;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i % 8;
			b.m128i_i8[i] = 2;
		}
		hexPrint128x8(&a);
		hexPrint128x8(&b);
		__m128i dst = _mm_gf2p8mul_epi8(a, b);
		AppLib::write("\r\n");
		hexPrint128x8(&dst);
	}
	if ((selector == 19) || (selector == -1))
	{	// vgf2p8mulb xmm {k}, xmm, xmm
		__m128i a;
		__m128i b;
		__m128i src = _mm_set1_epi8(0x11);
		__mmask16 k = 0xAAFF;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i % 8;
			b.m128i_i8[i] = 2;
		}
		hexPrint128x8(&src);
		hexPrint128x8(&a);
		hexPrint128x8(&b);
		hexPrintWord(&k);
		__m128i dst = _mm_mask_gf2p8mul_epi8(src, k, a, b);
		AppLib::write("\r\n");
		hexPrint128x8(&dst);
	}
	if ((selector == 20) || (selector == -1))
	{	// vgf2p8mulb xmm {z}, xmm, xmm
		__m128i a;
		__m128i b;
		__mmask16 k = 0xAAFF;
		for (int i = 0; i < 16; i++)
		{
			a.m128i_i8[i] = i % 8;
			b.m128i_i8[i] = 2;
		}
		hexPrint128x8(&a);
		hexPrint128x8(&b);
		hexPrintWord(&k);
		__m128i dst = _mm_maskz_gf2p8mul_epi8(k, a, b);
		AppLib::write("\r\n");
		hexPrint128x8(&dst);
	}
	if ((selector == 21) || (selector == -1))
	{	// vgf2p8mulb ymm, ymm, ymm
		__m256i a;
		__m256i b;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i;
			b.m256i_i8[i] = 2;
		}
		hexPrint256x8(&a);
		hexPrint256x8(&b);
		__m256i dst = _mm256_gf2p8mul_epi8(a, b);
		AppLib::write("\r\n");
		hexPrint256x8(&dst);
	}
	if ((selector == 22) || (selector == -1))
	{	// vgf2p8mulb ymm {k}, ymm, ymm
		__m256i a;
		__m256i b;
		__m256i src;
		__mmask32 k = 0xAAFFFFFF;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i;
			b.m256i_i8[i] = 2;
			src.m256i_i8[i] = 0x11;
		}
		hexPrint256x8(&src);
		hexPrint256x8(&a);
		hexPrint256x8(&b);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_mask_gf2p8mul_epi8(src, k, a, b);
		AppLib::write("\r\n");
		hexPrint256x8(&dst);
	}
	if ((selector == 23) || (selector == -1))
	{	// vgf2p8mulb ymm {z}, ymm, ymm
		__m256i a;
		__m256i b;
		__mmask32 k = 0xAAFFFFFF;
		for (int i = 0; i < 32; i++)
		{
			a.m256i_i8[i] = i;
			b.m256i_i8[i] = 2;
		}
		hexPrint256x8(&a);
		hexPrint256x8(&b);
		hexPrintDword(reinterpret_cast<DWORD*>(&k));
		__m256i dst = _mm256_maskz_gf2p8mul_epi8(k, a, b);
		AppLib::write("\r\n");
		hexPrint256x8(&dst);
	}
	if ((selector == 24) || (selector == -1))
	{	// vgf2p8mulb zmm, zmm, zmm
		__m512i a;
		__m512i b;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i;
			b.m512i_i8[i] = 2;
		}
		hexPrint512x8(&a);
		hexPrint512x8(&b);
		__m512i dst = _mm512_gf2p8mul_epi8(a, b);
		AppLib::write("\r\n");
		hexPrint512x8(&dst);
	}
	if ((selector == 25) || (selector == -1))
	{	// vgf2p8mulb zmm {k}, zmm, zmm
		__m512i a;
		__m512i b;
		__m512i src;
		__mmask64 k = 0xAAFFFFFFFFFFFFFF;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i;
			b.m512i_i8[i] = 2;
			src.m512i_i8[i] = 0x11;
		}
		hexPrint512x8(&src);
		hexPrint512x8(&a);
		hexPrint512x8(&b);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_mask_gf2p8mul_epi8(src, k, a, b);
		AppLib::write("\r\n");
		hexPrint512x8(&dst);
	}
	if ((selector == 26) || (selector == -1))
	{	// vgf2p8mulb zmm {z}, zmm, zmm
		__m512i a;
		__m512i b;
		__mmask64 k = 0xAAFFFFFFFFFFFFFF;
		for (int i = 0; i < 64; i++)
		{
			a.m512i_i8[i] = i;
			b.m512i_i8[i] = 2;
		}
		hexPrint512x8(&a);
		hexPrint512x8(&b);
		hexPrintQword(reinterpret_cast<DWORD64*>(&k));
		__m512i dst = _mm512_maskz_gf2p8mul_epi8(k, a, b);
		AppLib::write("\r\n");
		hexPrint512x8(&dst);
	}

	return exitCode;
}
