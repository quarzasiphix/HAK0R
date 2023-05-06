#include <common.hpp>

int main() {
	
	int myVariable = 42;
	int* pMyVariable = &myVariable;


	while (myVariable != 0) {
		std::cout << ". addres: " << pMyVariable << std::endl;
		std::cout << "myVariable: " << myVariable << std::endl;
		std::cin.get();
		system("cls");
	}

	printf("myvar = 0, bye\n");
}