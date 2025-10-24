#include <vector>

namespace ProductTest
{
	std::vector<int> getProducts(const std::vector<int>& input)
	{
		std::vector<int> output(input.size(), 0);

		int product = 1;
		size_t zeroCounter = 0;
		size_t zeroElementIdx = 0;

		for (size_t i = 0; i < input.size(); i++)
		{
			if (input[i] != 0)
			{
				product *= input[i];
			}
			else
			{
				zeroElementIdx = i;
				zeroCounter++;
				if (zeroCounter > 1)
				{
					return output;
				}
			}
		}

		if (zeroCounter > 0)
		{
			output[zeroElementIdx] = product;
			return output;
		}

		for (size_t i = 0; i < input.size(); i++)
		{
			output[i] = product / input[i];
		}

		return output;
	}

	void runTest()
	{
		std::vector<int> input = { 1, 0, 3, 4 }; // output = [0, 12, 0, 0]

		std::vector<int> input2 = { 1, 0, 3, 0 }; // output = [0, 0, 0, 0]

		std::vector<int> output = getProducts(input);
		std::vector<int> output2 = getProducts(input2);
	}
}

int main(int argc, char* argv[])
{
	ProductTest::runTest();
}
