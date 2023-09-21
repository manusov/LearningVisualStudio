/*
Learn some intrinsics,
use lambdas for create scenarios useable for many instructions.
Special thanks to:
https://ravesli.com/lyambda-vyrazheniya-anonimnye-funktsii-v-s/
https://ravesli.com/lyambda-zahvaty-v-s/
*/

#include "Task_19.h"

// Named lambdas.
auto inst1{ [](__m512i* px1, __m512i* px2, __m512i* px3, __m512i* py) {*py = _mm512_ternarylogic_epi64(*px1, *px2, *px3, 0x40); } };
auto inst2{ [](__m512i* px1, __m512i* px2, __m512i* px3, __m512i* py) {*py = _mm512_ternarylogic_epi64(*px1, *px2, *px3, 0x80); } };

int Task_19::execute(int argc, char** argv)
{
	int exitCode = 0;
	AppLib::writeColor("\r\n[ AVX512F demo with lambdas. ]\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	// Use functional literal lambdas.
	exitCode += context([](__m512i* px1, __m512i* px2, __m512i* px3, __m512i* py) { *py = _mm512_ternarylogic_epi64(*px1, *px2, *px3, 0x01); });
	exitCode += context([](__m512i* px1, __m512i* px2, __m512i* px3, __m512i* py) { *py = _mm512_mask_ternarylogic_epi64(*px1, 0xA0, *px2, *px3, 0x01); });
	exitCode += context([](__m512i* px1, __m512i* px2, __m512i* px3, __m512i* py) { *py = _mm512_maskz_ternarylogic_epi64(0xF, *px1, *px2, *px3, 0x01); });

	// Use named lambdas.
	exitCode += context(inst1);
	exitCode += context(inst2);

	// Use lambdas with catch.
	__m512i a1{ 1,1,1,1, 1,1,1,1,  1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 3,1,1,1, 5,1,1,1 };
	__m512i b1{ 1,1,1,1, 1,1,1,1, -1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 7,1,1,1, 1,1,1,1 };
	__m512i c1{ 1,0,0,0, 2,0,0,0,  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
	__m512i* pa1 = &a1;
	__m512i* pb1 = &b1;
	__m512i* pc1 = &c1;
	([pa1, pb1, pc1]() { *pc1 = _mm512_dpbusd_epi32(*pc1, *pa1, *pb1); });
	decimalPrint512x32(&c1, 8, APPCONST::TABLE_COLOR);
	([pa1, pb1, pc1]() { *pc1 = _mm512_dpwssds_epi32(*pc1, *pa1, *pb1); });
	decimalPrint512x32(&c1, 8, APPCONST::TABLE_COLOR);

	// Use lambdas with catch, more practically.
	const char* show = "[ Data for show. ]";
	exitCode += context([show](__m512i* px1, __m512i* px2, __m512i* px3, __m512i* py) 
	{ 
		*py = _mm512_ternarylogic_epi64(*px1, *px2, *px3, 0x01); 
		std::cout << show << std::endl; 
	});

	return exitCode;
}

int Task_19::context(const std::function<void(__m512i* px1, __m512i* px2, __m512i* px3, __m512i* py)>& instruction)
{
	DWORD64 alignas(64) x1[8] = { 0xAAAAAAAAAAAAAAAAL, 0,0,0,0,0,0,0 };
	DWORD64 alignas(64) x2[8] = { 0xCCCCCCCCCCCCCCCCL, 0,0,0,0,0,0,0 };
	DWORD64 alignas(64) x3[8] = { 0xF0F0F0F0F0F0F0F0L, 0,0,0,0,0,0,0 };
	DWORD64 alignas(64) y[8] = { 0,0,0,0,0,0,0,0 };
	__m512i* px1 = reinterpret_cast<__m512i*>(&x1);
	__m512i* px2 = reinterpret_cast<__m512i*>(&x2);
	__m512i* px3 = reinterpret_cast<__m512i*>(&x3);
	__m512i* py = reinterpret_cast<__m512i*>(&y);
	hexPrint512x64(px1, 8);
	hexPrint512x64(px2, 8);
	hexPrint512x64(px3, 8);
	instruction(px1, px2, px3, py);
	hexPrint512x64(py, 8, APPCONST::TABLE_COLOR);
	AppLib::write("\r\n");
	return 0;
}

