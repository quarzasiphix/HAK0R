#include <common.hpp>

int main() {
	
	int myVariable = 42;
	int* pMyVariable = &myVariable;


	while (myVariable != 0) {
		system("cls");
		std::cout << ". addres: " << pMyVariable << std::endl;
		std::cout << "myVariable: " << myVariable << std::endl;
		while (myVariable == 42) Sleep(10);
		Sleep(200);
	}

	printf("myvar = 0, bye\n");
}