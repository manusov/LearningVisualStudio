
// AVX512 optimization enabled for this project.Properties \ C/C++ \ Create code \ ...

#include <windows.h>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <numeric>
#include <intrin.h>
#include <immintrin.h>

// About double compare (use debug build, otherwise constants optimization possible).
int main()
{
	double a = 1.0;
	double b = 1.5;
	if (a == b)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

// About memory allocation and release by std::vector in the cycle.
/*
int main()
{
	for (int i = 0; i < 100; i++)
	{
		std::vector<double> a = std::vector<double>(200000000);
		int vectorSize = static_cast<int>(a.size());
		printf("Index = %d, vector size = %d.\r\n", i, vectorSize);
	}
	printf("Done");
}
*/

// About std::accumulate usage bug, actual for statistics routines.
/*
int main()
{
	std::vector<double> a;
	a.push_back(1.5);
	a.push_back(5.4);
	a.push_back(10.9);
	a.push_back(4.1);

	double sum1 = std::accumulate(a.begin(), a.end(), 0);

	int n = a.size();
	double sum2 = 0.0;
	for (int i = 0; i < n; i++)
	{
		sum2 += a[i];
	}
	
	double sum3 = std::accumulate(a.begin(), a.end(), double(0));  // BUG FIX IS double(0), otherwise integer addition used.

	printf("Test value 1 = %.3f.\r\n", sum1);
	printf("Test value 2 = %.3f.\r\n", sum2);
	printf("Test value 3 = %.3f.\r\n", sum3);


	// size_t n1 = 5;
	// size_t m1 = n / 2;
	// printf("Test value 4 = %d.\r\n", m1);
}
*/

// About signed and unsigned integers conversion.
/*
int main()
{

	unsigned int a = 0xC0000001;
	double b = a;
	printf("A = %d, B = %.3f.\r\n", a, b);

	double* p = &b;
	printf("After cast = %d.\r\n", static_cast<int>(*p));

}
*/