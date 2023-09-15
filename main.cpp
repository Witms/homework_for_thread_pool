#include "Threadqueue.h"
#include <iostream>
#include <array>

#define LENTGH 20



int main()
{
	ThreadPool t{};
	std::array<int, LENTGH> arr{ 5, 4, 3, 2, 1, 6, 8, 11, 15, 9, 30, 25, 27, 40, 50, 52, 44, 64, 76, 89,  };

	std::promise<void> p;

	t.push_task(quicksort, arr.data(), 0L, LENTGH - 1, t, &p);
	t.start();
	t.stop();


	std::cout << "Array: ";
	for (auto m : arr)
	{
		std::cout << m << " ";
	}
	std::cout << std::endl;

	int n = 10;

	std::shared_ptr<int> z;
	z = std::make_shared<int>(n);

	std::cout << "z = " << *z << std::endl;
	
	return 0;
}