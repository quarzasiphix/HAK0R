#include <common.hpp>

int main() {
	
	int myVariable = 42;
	int* pMyVariable = &myVariable;

	while (myVariable != 0) {
		std::cout << "myVariable: " << myVariable << ". addres: " << pMyVariable << std::endl;
		Sleep(300);
		system("cls");
	}

	printf("myvar = 0, bye\n");
}