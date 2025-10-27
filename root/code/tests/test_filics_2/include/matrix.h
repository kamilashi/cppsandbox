#ifndef MATRIX_H
#define MATRIX_H

#include<vector>
#include <iostream>

namespace CustomMatrix
{
	template <typename T> 
	class Matrix
	{
	public:
	Matrix(std::vector<T> elements, size_t rCount, size_t cCount) :
		m_elements(std::move(elements)), 
		m_rowCount(rCount), 
		m_columnCount(cCount) 
	{}
	~Matrix() = default;

	Matrix operator*(const T s) const
	{
		Matrix out(m_elements, m_rowCount, m_columnCount);

		for (T& element: out.m_elements)
		{
			element = element * s;
		}

		return out;
	}

	friend std::ostream& operator<<(std::ostream& os, const Matrix& m)
	{ 
		for (size_t rI = 0; rI < m.m_rowCount; rI++)
		{
			for (size_t cI = 0; cI < m.m_columnCount; cI++)
			{
				os << m.m_elements[rI * m.m_columnCount + cI];
			}
			os << std::endl;
		}

		return os;
	};

	private:
		std::vector<T> m_elements;
		size_t m_rowCount;
		size_t m_columnCount;
	};
}

#endif