#include "matrix.h"
#include <iostream>

int main(int argc, char* argv[])
{
	CustomMatrix::Matrix<double> matrix({1, 2, 3, 4}, 2, 2);

	auto matrix2 = matrix * 2.0;

	std::cout << matrix2 << std::endl;

	return 0;
}