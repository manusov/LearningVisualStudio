/*
Realization of processor performance functions class.
Benchmarking code with intrinsics for low-level coding.
*/

#include "Performance.h"

Performance::Performance()
{
}

void Performance::addAvx512(void* ptr, size_t length, DWORD64& dTSC, double& dummyData)
{
	__m512d zmm0  = _mm512_setzero_pd();
	__m512d zmm1  = _mm512_setzero_pd();
	__m512d zmm2  = _mm512_setzero_pd();
	__m512d zmm3  = _mm512_setzero_pd();
	__m512d zmm4  = _mm512_setzero_pd();
	__m512d zmm5  = _mm512_setzero_pd();
	__m512d zmm6  = _mm512_setzero_pd();
	__m512d zmm7  = _mm512_setzero_pd();
	__m512d zmm8  = _mm512_setzero_pd();
	__m512d zmm9  = _mm512_setzero_pd();
	__m512d zmm10 = _mm512_setzero_pd();
	__m512d zmm11 = _mm512_setzero_pd();
	__m512d zmm12 = _mm512_setzero_pd();
	__m512d zmm13 = _mm512_setzero_pd();
	__m512d zmm14 = _mm512_setzero_pd();
	__m512d zmm15 = _mm512_setzero_pd();

	__m512d* p = reinterpret_cast<__m512d*>(ptr);
	size_t count = length >> 10;

	// __debugbreak();
	// _mm_mfence();
	// int a[4];
	// __cpuid(a, 0);
	dTSC = __rdtsc();

	for (size_t i = 0; i < count; i++)
	{
		zmm0  = _mm512_add_pd(zmm0,  *p);
		zmm1  = _mm512_add_pd(zmm1,  *(p + 1));
		zmm2  = _mm512_add_pd(zmm2,  *(p + 2));
		zmm3  = _mm512_add_pd(zmm3,  *(p + 3));
		zmm4  = _mm512_add_pd(zmm4,  *(p + 4));
		zmm5  = _mm512_add_pd(zmm5,  *(p + 5));
		zmm6  = _mm512_add_pd(zmm6,  *(p + 6));
		zmm7  = _mm512_add_pd(zmm7,  *(p + 7));
		zmm8  = _mm512_add_pd(zmm8,  *(p + 8));
		zmm9  = _mm512_add_pd(zmm9,  *(p + 9));
		zmm10 = _mm512_add_pd(zmm10, *(p + 10));
		zmm11 = _mm512_add_pd(zmm11, *(p + 11));
		zmm12 = _mm512_add_pd(zmm12, *(p + 12));
		zmm13 = _mm512_add_pd(zmm13, *(p + 13));
		zmm14 = _mm512_add_pd(zmm14, *(p + 14));
		zmm15 = _mm512_add_pd(zmm15, *(p + 15));
		p += 16;
	}

	// _mm_mfence();
	// __cpuid(a, 0);
	dTSC = __rdtsc() - dTSC;

	zmm0  = _mm512_add_pd(zmm0, zmm1);
	zmm2  = _mm512_add_pd(zmm2, zmm3);
	zmm4  = _mm512_add_pd(zmm4, zmm5);
	zmm6  = _mm512_add_pd(zmm6, zmm7);
	zmm8  = _mm512_add_pd(zmm8, zmm9);
	zmm10 = _mm512_add_pd(zmm10, zmm11);
	zmm12 = _mm512_add_pd(zmm12, zmm13);
	zmm13 = _mm512_add_pd(zmm14, zmm15);

	zmm0  = _mm512_add_pd(zmm0, zmm2);
	zmm4  = _mm512_add_pd(zmm4, zmm6);
	zmm8  = _mm512_add_pd(zmm8, zmm10);
	zmm12 = _mm512_add_pd(zmm12, zmm13);

	zmm0  = _mm512_add_pd(zmm0, zmm4);
	zmm8  = _mm512_add_pd(zmm8, zmm12);

	zmm0  = _mm512_add_pd(zmm0, zmm8);

	dummyData = zmm0.m512d_f64[0] + zmm0.m512d_f64[1] + zmm0.m512d_f64[2] + zmm0.m512d_f64[3] +
		        zmm0.m512d_f64[4] + zmm0.m512d_f64[5] + zmm0.m512d_f64[6] + zmm0.m512d_f64[7];
}
