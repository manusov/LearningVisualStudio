/*
 
Context,
data types, initialization, visualization
for vector data and intrinsics.

m512i = Integer data, 8/16/32/64 bits.
        Can be initialized as constant array of up to 64 BYTES.

m512  = Single-precision floating point data, 16 x 32-bit numbers per 512-bit vector.
        Can be initialized as constant array of up to 16 FLOATS.

m512d = Double-precision floating point data, 8 x 64-bit numbers per 512-bit vector.
        Can be initialized as constant array of up to 8 DOUBLES.

*/

#include "Task_15.h"

int Task_15::execute(int argc, char** argv)
{
	int exitCode = 0;
	// char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("\r\n[Context for vector data intrinsics.]\r\n\r\n", APPCONST::DUMP_ADDRESS_COLOR);

	// Integer data, 8/16/32/64 bits.
	constexpr __m512i v8a{ 1, 2, 3, 4, 5, -6, 7 };
	__m512i v8b;
	__m512i v16;
	__m512i v32;
	__m512i v64;
	for (int i = 0; i < 64; i++) v8b.m512i_i8[i] = -i;
	for (int i = 0; i < 32; i++) v16.m512i_i16[i] = -i;
	for (int i = 0; i < 16; i++) v32.m512i_i32[i] = -i;
	for (int i = 0; i < 8; i++) v64.m512i_i64[i] = -i;

	// Single-precision floating point data, 16 x 32-bit numbers per 512-bit vector.
	constexpr __m512 vf1{ 1.5, -2.5, 3.5, 4.5, 5.5, -6.5, 7.5, -8.5, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0 };
	__m512 vf2;
	for (int i = 0; i < 16; i++) vf2.m512_f32[i] = static_cast<float>(100.0 * i * sin(i));

	// Double-precision floating point data, 8 x 64-bit numbers per 512-bit vector.
	constexpr __m512d vd1{ 10.7, -11.7, 12.7, -13.7, 1.1, 2.2, 3.3, 4.4 };
	__m512d vd2;
	for (int i = 0; i < 8; i++) vd2.m512d_f64[i] = 100.0 * i * cos(i);

	// Integer unsigned hex prints.
	hexPrint512x8(&v8a);
	AppLib::write("\r\n");
	hexPrint512x8(&v8b);
	AppLib::write("\r\n");
	hexPrint512x16(&v16);
	AppLib::write("\r\n");
	hexPrint512x32(&v32);
	AppLib::write("\r\n");
	hexPrint512x64(&v64);
	AppLib::write("\r\n");
	hexPrint512x64(&v64);
	AppLib::write("\r\n");
	hexPrint512x64(&v64, 8);                                     // second parameter non-default
	for (int i = 0; i < 8; i++) v64.m512i_i64[i] = i * 2 + 15;
	hexPrint512x64(&v64, 8, APPCONST::TABLE_COLOR);              // all parameters non-default
	AppLib::write("\r\n");

	// Integer signed decimal prints.
	decimalPrint512x8(&v8a);
	AppLib::write("\r\n");
	decimalPrint512x8(&v8b);
	AppLib::write("\r\n");
	decimalPrint512x16(&v16);
	AppLib::write("\r\n");
	decimalPrint512x32(&v32);
	AppLib::write("\r\n");
	decimalPrint512x64(&v64);
	AppLib::write("\r\n");
	v64.m512i_i64[0] = LLONG_MIN;
	v64.m512i_i64[1] = LLONG_MAX;
	v64.m512i_i64[2] = -1;
	v64.m512i_i64[3] = 1;
	v64.m512i_i64[4] = 0;
	v64.m512i_i64[5] = 0;
	v64.m512i_i64[6] = LLONG_MAX;
	v64.m512i_i64[7] = LLONG_MIN;
	decimalPrint512x64(&v64, 2, APPCONST::TABLE_COLOR);
	AppLib::write("\r\n");

	// Float (32-bit single precision floating point) prints.
	floatPrint512x32(&vf1);
	floatPrint512x32(&vf2);
	AppLib::write("\r\n");

	// Double (64-bit double precision floating point) prints.
	doublePrint512x64(&vd1);
	doublePrint512x64(&vd2);

	return exitCode;
}
void Task_15::hexPrint512x8(const __m512i* data, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 63; i >= 0; i--)
	{
		BYTE a = data->m512i_i8[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%02X", a);
		AppLib::writeColor(msg, color);
		if (i%16)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_15::hexPrint512x16(const __m512i* data, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 31; i >= 0; i--)
	{
		WORD a = data->m512i_i16[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%04X", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_15::hexPrint512x32(const __m512i* data, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 15; i >= 0; i--)
	{
		DWORD a = data->m512i_i32[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%08X", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_15::hexPrint512x64(const __m512i* data, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 7; i >= 0; i--)
	{
		DWORD64 a = data->m512i_i64[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%016llX", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_15::decimalPrint512x8(const __m512i* data, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 63; i >= 0; i--)
	{
		int a = data->m512i_i8[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%4d", a);
		AppLib::writeColor(msg, color);
		if (i % 16)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_15::decimalPrint512x16(const __m512i* data, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 31; i >= 0; i--)
	{
		int a = data->m512i_i16[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%6d", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_15::decimalPrint512x32(const __m512i* data, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 15; i >= 0; i--)
	{
		int a = data->m512i_i32[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%11d", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_15::decimalPrint512x64(const __m512i* data, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 7; i >= 0; i--)
	{
		long long a = data->m512i_i64[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%21lld", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_15::floatPrint512x32(const __m512* data, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 15; i >= 0; i--)
	{
		float a = data->m512_f32[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%7.1f", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
void Task_15::doublePrint512x64(const __m512d* data, int columns, WORD color)
{
	char msg[APPCONST::MAX_TEXT_STRING];
	AppLib::writeColor("[ ", color);
	for (int i = 7; i >= 0; i--)
	{
		double a = data->m512d_f64[i];
		snprintf(msg, APPCONST::MAX_TEXT_STRING, "%7.1f", a);
		AppLib::writeColor(msg, color);
		if (i % columns)
		{
			AppLib::writeColor(" ", color);
		}
		else if (i)
		{
			AppLib::writeColor("\r\n  ", color);
		}
		else
		{
			AppLib::writeColor(" ]\r\n", color);
		}
	}
}
