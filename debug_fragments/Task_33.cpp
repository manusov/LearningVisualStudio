/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#othertechs=GFNI

Arbitrtry bit permutter example.
https://gist.github.com/IJzerbaard/c5c7bcfae2910e714605ba7bf2ca9b42
See document:
TECHNOLOGY GUIDE. Intel Corporation. Galois Field New Instructions (GFNI).
644497-002US.
galois-field-new-instructions-gfni-technology-guide-1-1639042826.PDF
*/

#include "Task_33.h"

__m512i Transpose8x64(__m512i x)
{
    x = _mm512_permutexvar_epi8(_mm512_setr_epi8(
        56, 48, 40, 32, 24, 16, 8, 0,
        57, 49, 41, 33, 25, 17, 9, 1,
        58, 50, 42, 34, 26, 18, 10, 2,
        59, 51, 43, 35, 27, 19, 11, 3,
        60, 52, 44, 36, 28, 20, 12, 4,
        61, 53, 45, 37, 29, 21, 13, 5,
        62, 54, 46, 38, 30, 22, 14, 6,
        63, 55, 47, 39, 31, 23, 15, 7), x);
    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x8040201008040201), x, 0);
    return x;
}

__m512i Transpose64x8(__m512i x)
{
    x = _mm512_shuffle_epi8(x, _mm512_setr_epi8(
        7, 6, 5, 4, 3, 2, 1, 0,
        15, 14, 13, 12, 11, 10, 9, 8,
        23, 22, 21, 20, 19, 18, 17, 16,
        31, 30, 29, 28, 27, 26, 25, 24,
        39, 38, 37, 36, 35, 34, 33, 32,
        47, 46, 45, 44, 43, 42, 41, 40,
        55, 54, 53, 52, 51, 50, 49, 48,
        63, 62, 61, 60, 59, 58, 57, 56));
    x = _mm512_gf2p8affine_epi64_epi8(_mm512_set1_epi64(0x0408201002014080), x, 0);
    x = _mm512_permutexvar_epi8(_mm512_setr_epi8(
        2, 10, 18, 26, 34, 42, 50, 58,
        3, 11, 19, 27, 35, 43, 51, 59,
        7, 15, 23, 31, 39, 47, 55, 63,
        6, 14, 22, 30, 38, 46, 54, 62,
        4, 12, 20, 28, 36, 44, 52, 60,
        5, 13, 21, 29, 37, 45, 53, 61,
        1, 9, 17, 25, 33, 41, 49, 57,
        0, 8, 16, 24, 32, 40, 48, 56), x);
    return x;
}

// bits = 8 x uint64
// permutation = array of bytes specifying source indices of bits
// result = each uint64 bit-permuted according to the permutation
__m512i Any64BitPermute(__m512i bits, __m512i permutation)
{
    bits = Transpose8x64(bits);
    bits = _mm512_permutexvar_epi8(permutation, bits);
    return Transpose64x8(bits);
}

int Task_33::execute(int argc, char** argv)
{
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[Arbitrtry bit permutter example]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	int exitCode = 0;
    int selector = 7;

    if (selector == 0)
    {
        __m512i permutation;
        __m512i bits;
        /*
        for (int i = 0; i < 64; i++)
        {
            permutation.m512i_i8[i] = i % 8;
            bits.m512i_i8[i] = 7 - (i % 8);
        }
        */
        for (int i = 0; i < 8; i++)
        {
            permutation.m512i_i64[i] = i;
            bits.m512i_i64[i] = i;
        }
        hexPrint512x64(&permutation);
        hexPrint512x64(&bits);
        __m512i dst = Any64BitPermute(bits, permutation);
        AppLib::write("\r\n");
        hexPrint512x64(&dst);
    }
    else if (selector == 1)
    {
        __m512i a{ 0 };
        a.m512i_i8[0] = 0x7F;
        a.m512i_i8[1] = 0x1;
        a.m512i_i8[2] = 0x3;
        a.m512i_i8[4] = 0x7;
        a.m512i_i8[6] = 0xF;
        a.m512i_i8[7] = 0x1F;

        a.m512i_i8[8] = 0x1;

        hexPrint512x64(&a);
        AppLib::write("\r\n");
        __m512i dst = Transpose8x64(a);
        hexPrint512x64(&dst);
        AppLib::write("\r\n");
        dst = Transpose64x8(dst);
        hexPrint512x64(&dst);
    }
    else if (selector == 2)
    {
        __m512i a{ 0 };
        for (int i = 0; i < 8; i++)
        {
            a.m512i_i8[i] = 0xFF;
        }
        a.m512i_i8[8] = 0x1;

        hexPrint512x64(&a);
        AppLib::write("\r\n");
        __m512i dst = Transpose8x64(a);
        hexPrint512x64(&dst);
        AppLib::write("\r\n");
        dst = Transpose64x8(dst);
        hexPrint512x64(&dst);
    }
    else if (selector == 3)
    {
        __m512i a{ 0 };
        for (int i = 0; i < 8; i++)
        {
            a.m512i_i8[i] = 0xFF;
        }
        a.m512i_i8[8] = 0x1;

        hexPrint512x64(&a);
        AppLib::write("\r\n");
        __m512i dst = Transpose8x64(a);
        hexPrint512x64(&dst);
        AppLib::write("\r\n");

        __m512i permutation{ 0 };
        for (int i = 0; i < 64; i++)
        {
            permutation.m512i_i8[i] = i + 3;
        }
        hexPrint512x64(&permutation);
        AppLib::write("\r\n");
        
        dst = _mm512_permutexvar_epi8(permutation, dst);

        hexPrint512x64(&dst);
        AppLib::write("\r\n");
        dst = Transpose64x8(dst);
        hexPrint512x64(&dst);
    }
    else if (selector == 4)
    {   // Learn Transpose8x64, transpose bits array (bit step = 1) to bytes array (bit step = 8), for example: Y0=X0, Y8=X1 ...
        __m512i idx = _mm512_setr_epi8(
            56, 48, 40, 32, 24, 16, 8, 0,
            57, 49, 41, 33, 25, 17, 9, 1,
            58, 50, 42, 34, 26, 18, 10, 2,
            59, 51, 43, 35, 27, 19, 11, 3,
            60, 52, 44, 36, 28, 20, 12, 4,
            61, 53, 45, 37, 29, 21, 13, 5,
            62, 54, 46, 38, 30, 22, 14, 6,
            63, 55, 47, 39, 31, 23, 15, 7);
        decimalPrint512x8(&idx);
        AppLib::write("\r\n");

        __m512i x{ 0 };
        for (int i = 0; i < 5; i++)
        {
            x.m512i_i8[i] = 0xFF;
        }
        hexPrint512x64(&x);
        AppLib::write("\r\n");

        x = _mm512_permutexvar_epi8(idx, x);
        hexPrint512x8(&x);
        AppLib::write("\r\n");

        __m512i vector = _mm512_set1_epi64(0x8040201008040201);
        hexPrint512x8(&vector);
        AppLib::write("\r\n");

        x = _mm512_gf2p8affine_epi64_epi8(vector, x, 0);  // vector,  x=as matrix,  0 for XOR.
        hexPrint512x8(&x);
        AppLib::write("\r\n");
    }
    else if (selector == 5)
    {   // Learn Transpose64x8, transpose bytes array to bits array.
        __m512i x{ 0 };
        for (int i = 0; i < 40; i++)
        {
            x.m512i_i8[i] = 0x1;
        }
        hexPrint512x64(&x);
        AppLib::write("\r\n");

        __m512i idx1 =_mm512_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 0,
            15, 14, 13, 12, 11, 10, 9, 8,
            23, 22, 21, 20, 19, 18, 17, 16,
            31, 30, 29, 28, 27, 26, 25, 24,
            39, 38, 37, 36, 35, 34, 33, 32,
            47, 46, 45, 44, 43, 42, 41, 40,
            55, 54, 53, 52, 51, 50, 49, 48,
            63, 62, 61, 60, 59, 58, 57, 56);
        hexPrint512x8(&idx1);
        AppLib::write("\r\n");

        x = _mm512_shuffle_epi8(x, idx1);
        hexPrint512x64(&x);
        AppLib::write("\r\n");

        __m512i vector = _mm512_set1_epi64(0x0408201002014080);
        hexPrint512x8(&vector);
        AppLib::write("\r\n");

        x = _mm512_gf2p8affine_epi64_epi8(vector, x, 0);
        hexPrint512x64(&x);
        AppLib::write("\r\n");

        __m512i idx2 = _mm512_setr_epi8(
            2, 10, 18, 26, 34, 42, 50, 58,
            3, 11, 19, 27, 35, 43, 51, 59,
            7, 15, 23, 31, 39, 47, 55, 63,
            6, 14, 22, 30, 38, 46, 54, 62,
            4, 12, 20, 28, 36, 44, 52, 60,
            5, 13, 21, 29, 37, 45, 53, 61,
            1, 9, 17, 25, 33, 41, 49, 57,
            0, 8, 16, 24, 32, 40, 48, 56);
        hexPrint512x8(&idx2);
        AppLib::write("\r\n");

        x = _mm512_permutexvar_epi8(idx2, x);
        hexPrint512x64(&x);
        AppLib::write("\r\n");
    }
    else if (selector == 6)
    {   // Learn function:  bits = _mm512_permutexvar_epi8(permutation, bits);
        __m512i bits{ 0 };
        for (int i = 0; i < 40; i++)
        {
            bits.m512i_i8[i] = 0x1;
        }
        hexPrint512x64(&bits);
        AppLib::write("\r\n");

        __m512i permutation = _mm512_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 63,
            15, 14, 13, 12, 11, 10, 9, 8,
            23, 22, 21, 20, 19, 18, 17, 16,
            31, 30, 29, 28, 27, 26, 25, 24,
            39, 38, 37, 36, 35, 34, 33, 32,
            47, 46, 45, 44, 43, 42, 41, 40,
            55, 54, 53, 52, 51, 50, 49, 48,
            0, 62, 61, 60, 59, 58, 57, 56);
        hexPrint512x8(&permutation);
        AppLib::write("\r\n");

        bits = _mm512_permutexvar_epi8(permutation, bits);
        hexPrint512x64(&bits);
        AppLib::write("\r\n");
    }
    else if (selector == 7)
    {   // Verify arbitrary permutation at 64-bit QWORD.
        __m512i permutation = _mm512_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 56,
            15, 14, 13, 12, 11, 10, 9, 8,
            23, 22, 21, 20, 19, 18, 17, 16,
            31, 30, 29, 28, 27, 26, 25, 24,
            39, 38, 37, 36, 35, 34, 33, 32,
            47, 46, 45, 44, 43, 42, 41, 40,
            55, 54, 53, 52, 51, 50, 49, 48,
            63, 62, 61, 60, 59, 58, 57, 0);     // Swapped 0 (7) and 56 (63).
        hexPrint512x8(&permutation);
        AppLib::write("\r\n");

        __m512i bits{ 0 };
        for (int i = 0; i < 5; i++)
        {
            bits.m512i_i8[i] = 0xFF;
        }
        hexPrint512x64(&bits);
        AppLib::write("\r\n");

        __m512i result = Any64BitPermute(bits, permutation);
        hexPrint512x64(&result);
        AppLib::write("\r\n");
    }

	return exitCode;
}


