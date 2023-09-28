#include <iostream>
#include <fstream>

typedef unsigned long ulong;
typedef unsigned long long ullong;

int main(int argc, char *argv[]) {
	unsigned int a = 0;
	unsigned long b = 0;
	unsigned long long c = 0;
	std::cout << "Max uint:   " << a-1 << "\nMax ulong:  " << b-1 << "\nMax ullong: " << c-1 << "\n";
}