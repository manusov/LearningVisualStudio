/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ssetechs=SSE
*/

#include "Task_23.h"
#include <xmmintrin.h>

int Task_23::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[Context for vector data intrinsics - SSE.]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

/*
	// __m128 _mm_add_ps (__m128 a, __m128 b)
	// __m128 _mm_add_ss (__m128 a, __m128 b)
	__m128 x1{ 1.1f, 2.2f, 3.3f, 4.4f };
	__m128 x2{ 5.0f, 4.3f, 1.0f, -1.0f };
	float* px1 = reinterpret_cast<float*>(&x1);
	float* px2 = reinterpret_cast<float*>(&x2);
	__m128 y1 = _mm_load_ps(px1);
	__m128 y2 = _mm_load_ps(px2);
	// __debugbreak();
	__m128 z1 = _mm_add_ps(y1, y2);
	__m128 z2 = _mm_add_ss(y1, y2);
	floatPrint128x32(&x1);
	floatPrint128x32(&x2);
	floatPrint128x32(&z1, 16, APPCONST::VALUE_COLOR);
	floatPrint128x32(&z2, 16, APPCONST::VALUE_COLOR);
*/
/*
	// __m128 _mm_and_ps (__m128 a, __m128 b)
	// __m128 _mm_andnot_ps (__m128 a, __m128 b)
	BYTE x1[]{ 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	BYTE x2[]{ 0xFF,0xFF,0x00,0xF0,0xFF,0x0F,0x33,0x00,0x00,0x00,0xFF,0x00,0x70,0x07,0x0F,0xF0 };
	__m128* px1 = reinterpret_cast<__m128*>(&x1);
	__m128* px2 = reinterpret_cast<__m128*>(&x2);
	__m128i* px3 = reinterpret_cast<__m128i*>(&x1);
	__m128i* px4 = reinterpret_cast<__m128i*>(&x2);
	__m128 y1 = _mm_and_ps(*px1, *px2);
	__m128 y2 = _mm_andnot_ps(*px1, *px2);
	__m128i* py1 = reinterpret_cast<__m128i*>(&y1);
	__m128i* py2 = reinterpret_cast<__m128i*>(&y2);
	hexPrint128x8(px3);
	hexPrint128x8(px4);
	hexPrint128x8(py1, 16, APPCONST::TABLE_COLOR);
	hexPrint128x8(py2, 16, APPCONST::TABLE_COLOR);
*/
/*
	// __m64 _mm_avg_pu16 (__m64 a, __m64 b) , this available in x86, not available in x64. Compiler (not CPU) restriction.
	WORD x1[]{ 10, 20, 30, 30 };
	WORD x2[]{ 15, 44, 90, 20 };
	__m64* px1 = reinterpret_cast<__m64*>(&x1);
	__m64* px2 = reinterpret_cast<__m64*>(&x2);
	__m64 y = _mm_avg_pu16(*px1, *px2);
	decimalPrint64x16(px1);
	decimalPrint64x16(px2);
	decimalPrint64x16(&y, 8, APPCONST::TABLE_COLOR);
*/	
/*
	// __m64 _mm_avg_pu8 (__m64 a, __m64 b) , this available in x86, not available in x64. Compiler (not CPU) restriction.
	BYTE x1[]{ 10, 20, 30, 30, 1, 1, 1, 1 };
	WORD x2[]{ 15, 44, 90, 20, 1, 2, 3, 4 };
	__m64* px1 = reinterpret_cast<__m64*>(&x1);
	__m64* px2 = reinterpret_cast<__m64*>(&x2);
	__m64 y = _mm_avg_pu8(*px1, *px2);
	decimalPrint64x8(px1);
	decimalPrint64x8(px2);
	decimalPrint64x8(&y, 8, APPCONST::TABLE_COLOR);
*/
/*
	// __m128 _mm_cmpeq_ps (__m128 a, __m128 b)
	__m128 x1{ 1.1f, 2.2f, -1.0f, -4.4f };
	__m128 x2{ 5.0f, 2.2f,  1.0f,  4.4f };
	__m128 y1 = _mm_cmpeq_ps(x1, x2);
	__m128i* py1 = reinterpret_cast<__m128i*>(&y1);
	floatPrint128x32(&x1);
	floatPrint128x32(&x2);
	hexPrint128x32(py1, 4, APPCONST::TABLE_COLOR);
	__m128 x3 = _mm_xor_ps(x1, x1);       // Clear for prevent NaN, INF, ...
	__m128 y2 = _mm_cmpeq_ps(x3, x3);     // This for set all "1". But note about NaN, INF, ...
	__m128i* py2 = reinterpret_cast<__m128i*>(&y2);
	hexPrint128x32(py2, 4, APPCONST::TABLE_COLOR);
*/
/*
	// __m128 _mm_unpackhi_ps (__m128 a, __m128 b)
	// __m128 _mm_unpacklo_ps (__m128 a, __m128 b)
	__m128 x1{ 1.1f, 2.2f, 3.3f, 4.4f };
	__m128 x2{ 5.5f, 6.6f, 7.7f, 8.8f };
	__m128 y1 = _mm_unpackhi_ps(x1, x2);
	__m128 y2 = _mm_unpacklo_ps(x1, x2);
	floatPrint128x32(&x1);
	floatPrint128x32(&x2);
	floatPrint128x32(&y1, 16, APPCONST::TABLE_COLOR);
	floatPrint128x32(&y2, 16, APPCONST::TABLE_COLOR);
*/
	// void _MM_TRANSPOSE4_PS (__m128 row0, __m128 row1, __m128 row2, __m128 row3)

	__m128 x1{  1.0f,  2.0f,  3.0f,  4.0f };
	__m128 x2{  5.0f,  6.0f,  7.0f,  8.0f };
	__m128 x3{  9.0f, 10.0f, 11.0f, 12.0f };
	__m128 x4{ 13.0f, 14.0f, 15.0f, 16.0f };
	floatPrint128x32(&x1);
	floatPrint128x32(&x2);
	floatPrint128x32(&x3);
	floatPrint128x32(&x4);
	_MM_TRANSPOSE4_PS(x1, x2, x3, x4);
	floatPrint128x32(&x1, 16, APPCONST::TABLE_COLOR);
	floatPrint128x32(&x2, 16, APPCONST::TABLE_COLOR);
	floatPrint128x32(&x3, 16, APPCONST::TABLE_COLOR);
	floatPrint128x32(&x4, 16, APPCONST::TABLE_COLOR);



	return exitCode;
}