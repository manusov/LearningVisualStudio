/*
Learn some intrinsics.
Learn some intrinsics. Some AVX512F instructions (context by Task_15).
*/

#include "Task_18.h"

int Task_18::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[ AVX512F demo. ]", APPCONST::DUMP_ADDRESS_COLOR);

	AppLib::writeColor("\r\n[ VPABSD, no masks ]\r\n", APPCONST::DUMP_ADDRESS_COLOR);
	DWORD64 alignas(64) x10[8] = { 0xAAAAAAAAAAAAAAAAULL, 0xFFFFFFFFFFFFFFFFULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL };
	DWORD64 alignas(64) y10[8] = { 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
		                           0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL };
	__m512i* px10 = reinterpret_cast<__m512i*>(&x10);
	__m512i* py10 = reinterpret_cast<__m512i*>(&y10);
	hexPrint512x64(px10, 8);
	hexPrint512x64(py10, 8);
	*py10 = _mm512_abs_epi32(*px10);
	hexPrint512x64(py10, 8, APPCONST::TABLE_COLOR);

	AppLib::writeColor("\r\n[ VPABSD, with mask ]\r\n", APPCONST::DUMP_ADDRESS_COLOR);
	DWORD64 alignas(64) x20[8] = { 0xAAAAAAAAAAAAAAAAULL, 0xFFFFFFFFFFFFFFFFULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL };
	DWORD64 alignas(64) x21[8] = { 0x111ULL, 0x2222ULL, 0x3333ULL, 0x4444ULL, 0x5555ULL, 0x6666ULL, 0x7777ULL, 0x8888ULL };
	DWORD64 alignas(64) y20[8] = { 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
								   0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL };
	__m512i* px20 = reinterpret_cast<__m512i*>(&x20);
	__m512i* px21 = reinterpret_cast<__m512i*>(&x21);
	__m512i* py20 = reinterpret_cast<__m512i*>(&y20);
	hexPrint512x64(px20, 8);
	hexPrint512x64(px21, 8);
	hexPrint512x64(py20, 8);
	*py20 = _mm512_mask_abs_epi32(*px21, 0x0F0F, *px20);
	hexPrint512x64(py20, 8, APPCONST::TABLE_COLOR);

	AppLib::writeColor("\r\n[ VPABSD, with zero-mask ]\r\n", APPCONST::DUMP_ADDRESS_COLOR);
	DWORD64 alignas(64) x30[8] = { 0xAAAAAAAAAAAAAAAAULL, 0xFFFFFFFFFFFFFFFFULL, 1ULL, 2ULL, 3ULL, 4ULL, 5ULL, 6ULL };
	DWORD64 alignas(64) y30[8] = { 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL,
								   0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL };
	__m512i* px30 = reinterpret_cast<__m512i*>(&x30);
	__m512i* py30 = reinterpret_cast<__m512i*>(&y30);
	hexPrint512x64(px30, 8);
	hexPrint512x64(py30, 8);
	*py30 = _mm512_maskz_abs_epi32(0x0F0F, *px30);
	hexPrint512x64(py30, 8, APPCONST::TABLE_COLOR);

	return exitCode;
}
