#include <common.hpp>

void print_vars(int* i, float* f, double* d) {
	std::cout << ". addres: " << i << std::endl;
	std::cout << "myVariable: " << *i << std::endl;

	std::cout << ".float addres: " << f << std::endl;
	std::cout << "mFloat: " << *f << std::endl;

	std::cout << ".double addres: " << d << std::endl;
	std::cout << "myVariable: " << *d << std::endl;
}

int main() {
	/* attempt to set the variable memory address lol
		
	int* myVariable = reinterpret_cast<int*>(0x00000069);  // Replace with your desired memory address

	// Access and manipulate the variable through the pointer
		
	*myVariable = 42;
	*/

	int myVariable = 42;
	int* pmyVariable = &myVariable;

	float myFloat = 10;
	float* pmyFloat = &myFloat;

	double myDouble = 69;
	double* pmyDouble = &myDouble;

	while (myVariable != 0) {
		Sleep(200);
		system("cls");
		print_vars(&myVariable, &myFloat, &myDouble);
		while(myVariable == 42) Sleep(500);
		myVariable += 1;
	}

	printf("myvar = 0, bye\n");
}