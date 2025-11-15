#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include<vector>

namespace Datastructs
{
	template <class T>
	concept MovAvgNumber =
		std::default_initializable<T> &&
		std::copyable<T> &&
		requires (T a, T b, T n, size_t s)
	{
		T{ 0 };
		{ a += b } -> std::same_as<T&>;
		{ a /= s } -> std::same_as<T&>;
	};

	template <MovAvgNumber T>
	class MovingAverage
	{
	public:
		MovingAverage(size_t windowSize) :
			m_movingAveHeadIdx(0),
			m_windowSizeCap(0),
			m_movingAveWindow(windowSize, T{ 0 })
		{
		}

		T getMovingAverage() const
		{
			if (m_windowSizeCap == 0)
			{
				return T{ 0 };
			}

			T ave{ 0 };
			for (size_t i = 0; i < m_windowSizeCap; i++)
			{
				ave += m_movingAveWindow[i];
			}
			ave /= m_windowSizeCap;
			return ave;
		}

		void pushItem(T item)
		{
			m_movingAveWindow[m_movingAveHeadIdx] = std::move(item);
			m_movingAveHeadIdx = m_movingAveHeadIdx == m_movingAveWindow.size() - 1 ? 0 : m_movingAveHeadIdx + 1;

			if (m_windowSizeCap < m_movingAveWindow.size())
			{
				m_windowSizeCap++;
			}
		}

	private:
		size_t m_movingAveHeadIdx;
		size_t m_windowSizeCap;
		std::vector<T> m_movingAveWindow;
	};
}

#endif //DATASTRUCTS_H
