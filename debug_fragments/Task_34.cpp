/*
Learn some intrinsics, selected from:
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ig_expand=7206,512,4940,5862,5876,5920,7018,7230,7199,937,5910&techs=AVX_512&cats=Set
Macro usage examples for set SSE/AVX/AVX512 variables and registers.
*/

#include "Task_34.h"

int Task_34::execute(int argc, char** argv)
{
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[Macro usage examples for set SSE/AVX/AVX512 variables and registers]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	int exitCode = 0;
	int selector = 1;

	if (selector == 0)
	{
		__m512i a = _mm512_set_epi16(2, 4, 4, 2, 4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		hexPrint512x16(&a);
		AppLib::write("\r\n");
		__m512i b = _mm512_set_epi32(2, 4, 4, 2, 4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		hexPrint512x32(&b);
		AppLib::write("\r\n");
		__m512i c = _mm512_set_epi64(2, 4, 4, 2, 4, 0, 3, 0);
		hexPrint512x64(&c);
		AppLib::write("\r\n");
		__m512i d = _mm512_set_epi8(2, 4, 4, 2, 4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4);
		hexPrint512x8(&d);
	}
	else if (selector == 1)
	{
		__m512d a = _mm512_set_pd(1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7);
		doublePrint512x64(&a);
		AppLib::write("\r\n");


		// ...
	}
	else if (selector == 2)
	{
		// ...
	}


	return exitCode;
}

