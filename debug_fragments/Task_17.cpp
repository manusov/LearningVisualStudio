/*
Learn some intrinsics.
AVX512 VNNI instructions (context by Task_15).
*/

#include "Task_17.h"

int Task_17::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[ AVX512 VNNI demo. ]\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	AppLib::writeColor("[ VPDPBUSD ]\r\n", APPCONST::DUMP_ADDRESS_COLOR);
	//         0        1         2         3        4        5        6        7        8        9        10       11       12       13       14       15
	__m512i a1{ 1,1,1,1, 1,1,1,1,  1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 3,1,1,1, 5,1,1,1 };
	__m512i b1{ 1,1,1,1, 1,1,1,1, -1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 7,1,1,1, 1,1,1,1 };
	__m512i c1{ 1,0,0,0, 2,0,0,0,  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
	AppLib::write("\r\n");
	decimalPrint512x8(&a1);
	AppLib::write("\r\n");
	decimalPrint512x8(&b1);
	AppLib::write("\r\n");
	decimalPrint512x32(&c1, 8);
	// __debugbreak();
	c1 = _mm512_dpbusd_epi32(c1, a1, b1);
	AppLib::write("\r\n");
	decimalPrint512x32(&c1, 8, APPCONST::TABLE_COLOR);

	AppLib::writeColor("\r\n[ VPDPWSSDS ]\r\n", APPCONST::DUMP_ADDRESS_COLOR);
	//          0        1         2          3        4        5        6        7        8        9        10       11       12       13       14       15
	__m512i a2{ 1,0,1,0, 1,0,1,0,  1, 0, 1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 5,0,1,0 };
	__m512i b2{ 1,0,1,0, 1,0,1,0, -1,-1, 1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0 };
	__m512i c2{ 1,0,0,0, 2,0,0,0,  0, 0, 0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
	AppLib::write("\r\n");
	decimalPrint512x16(&a2);
	AppLib::write("\r\n");
	decimalPrint512x16(&b2);
	AppLib::write("\r\n");
	decimalPrint512x32(&c2, 8);
	__debugbreak();
	c2 = _mm512_dpwssds_epi32(c2, a2, b2);
	AppLib::write("\r\n");
	decimalPrint512x32(&c2, 8, APPCONST::TABLE_COLOR);

	return exitCode;
}

