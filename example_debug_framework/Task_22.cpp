/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=vmovaps&ig_expand=4035,4048
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#text=vmovapd&ig_expand=4035
*/

#include "Task_22.h"

int Task_22::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[Context for vector data intrinsics - load.]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	__m128i x1{ 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	__m128i x2{ 0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22 };

	__m128*  pfx1 = reinterpret_cast<__m128*>(&x1);
	__m128d* pdx1 = reinterpret_cast<__m128d*>(&x1);

	// __debugbreak();
	__m128  temp1 = _mm_mask_load_ps(*pfx1, 0x1, &x2);
	__m128d temp2 = _mm_mask_load_pd(*pdx1, 0x1, &x2);

	__m128i* pt1 = reinterpret_cast<__m128i*>(&temp1);
	__m128i* pt2 = reinterpret_cast<__m128i*>(&temp2);

	hexPrint128x8(pt1, 16, APPCONST::TABLE_COLOR);
	hexPrint128x8(pt2, 16, APPCONST::TABLE_COLOR);

	return exitCode;
}