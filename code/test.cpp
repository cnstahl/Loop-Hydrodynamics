#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>
 
volatile int sink;
const auto start = std::chrono::steady_clock::now();
 
class Bucket {
private:
	const static int cont;
	const static int fall;
	std::array<int, fall> underlying_state;

public:
	Bucket() : cont(0) {
		std::cout << "Empty Bucket\n";
	}

	Bucket(int cont) : cont(cont) {
		std::cout << "Full Bucket\n";
	}

	Bucket(int cont, int fall) : cont(cont), fall(fall) {
		std::cout << "Fall Bucket\n";
	}

	void print_cont() {
		std::cout << "Contents " << cont << "\n";
	}

	void print_fall() {
		std::cout << "Fallows  " << fall << "\n";
	}
};
 
int main()
{
    Bucket bucket(4);
    bucket.print_cont();
    bucket.print_fall();
    bucket = Bucket(3, 77);
    bucket.print_cont();
    bucket.print_fall();
}