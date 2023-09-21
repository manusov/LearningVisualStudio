/*
Learn some intrinsics. VPTERNLOG.
*/

#include "Task_14.h"

int Task_14::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];

	// vpternlog: multiplexor with 3 inverted address bits from 3 source operands, 8 data bits from immediate data.
	AppLib::writeColor("\r\n[ VPTERNLOG demo. ]\r\n", APPCONST::DUMP_ADDRESS_COLOR);
	DWORD64 alignas(64) x1[8] = { 0xAAAAAAAAAAAAAAAAL, 0,0,0,0,0,0,0 };
	DWORD64 alignas(64) x2[8] = { 0xCCCCCCCCCCCCCCCCL, 0,0,0,0,0,0,0 };
	DWORD64 alignas(64) x3[8] = { 0xF0F0F0F0F0F0F0F0L, 0,0,0,0,0,0,0 };
	DWORD64 alignas(64) y[8]  = { 0,0,0,0,0,0,0,0 };
	hexPrint64(x1, APPCONST::TABLE_COLOR);
	hexPrint64(x2, APPCONST::TABLE_COLOR);
	hexPrint64(x3, APPCONST::TABLE_COLOR);
	__m512i* px1 = reinterpret_cast<__m512i*>(&x1);
	__m512i* px2 = reinterpret_cast<__m512i*>(&x2);
	__m512i* px3 = reinterpret_cast<__m512i*>(&x3);
	__m512i* py  = reinterpret_cast<__m512i*>(&y);
	// __debugbreak();
	*py = _mm512_ternarylogic_epi64(*px1, *px2, *px3, 0x01);
	hexPrint64(y, APPCONST::VALUE_COLOR);
	*py = _mm512_ternarylogic_epi64(*px1, *px2, *px3, 0x80);
	hexPrint64(y, APPCONST::VALUE_COLOR);
	*py = _mm512_ternarylogic_epi64(*px1, *px2, *px3, 0x40);
	hexPrint64(y, APPCONST::VALUE_COLOR);
	*py = _mm512_maskz_ternarylogic_epi64(0xF, *px1, *px2, *px3, 0x01);
	hexPrint64(y, APPCONST::VALUE_COLOR);
	*py = _mm512_mask_ternarylogic_epi64(*px1, 0xA0, *px2, *px3, 0x01);
	hexPrint64(y, APPCONST::VALUE_COLOR);

	return exitCode;
}
void Task_14::hexPrint32(DWORD32* data, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 15; i >= 0; i--)
	{
		DWORD32 a = data[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%08X", a);
		AppLib::writeColor(msg, color);
		if (i)
		{
			AppLib::writeColor(":", color);
		}
	}
	AppLib::writeColor(" ]\r\n", color);
}
void Task_14::hexPrint64(DWORD64* data, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 7; i >= 0; i--)
	{
		DWORD64 a = data[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%016llX", a);
		AppLib::writeColor(msg, color);
		if (i)
		{
			AppLib::writeColor(":", color);
		}
	}
	AppLib::writeColor(" ]\r\n", color);
}