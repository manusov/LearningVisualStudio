/*

Learn some intrinsics.
VPTERNLOG in the new context (context by Task_15).

VPTERNLOG instruction:
multiplexor with 3 inverted address bits from 3 source operands,
8 data bits from immediate data.

*/

#include "Task_16.h"

int Task_16::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[ VPTERNLOG demo. ]\r\n", APPCONST::DUMP_ADDRESS_COLOR);

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

	// __debugbreak();
	*py = _mm512_ternarylogic_epi64(*px1, *px2, *px3, 0x01);
	hexPrint512x64(py, 8, APPCONST::TABLE_COLOR);

	*py = _mm512_ternarylogic_epi64(*px1, *px2, *px3, 0x80);
	hexPrint512x64(py, 8, APPCONST::TABLE_COLOR);

	*py = _mm512_ternarylogic_epi64(*px1, *px2, *px3, 0x40);
	hexPrint512x64(py, 8, APPCONST::TABLE_COLOR);

	*py = _mm512_maskz_ternarylogic_epi64(0xF, *px1, *px2, *px3, 0x01);
	hexPrint512x64(py, 8, APPCONST::TABLE_COLOR);

	*py = _mm512_mask_ternarylogic_epi64(*px1, 0xA0, *px2, *px3, 0x01);
	hexPrint512x64(py, 8, APPCONST::TABLE_COLOR);

	return exitCode;
}
