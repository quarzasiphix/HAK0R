#include <common.hpp>

int main() {
	try {

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
			system("cls");
			std::cout << ". addres: " << pmyVariable << std::endl;
			std::cout << "myVariable: " << myVariable << std::endl;

			std::cout << ".float addres: " << pmyFloat << std::endl;
			std::cout << "mFloat: " << myFloat << std::endl;

			std::cout << ".double addres: " << pmyDouble << std::endl;
			std::cout << "myVariable: " << myDouble << std::endl;


			while(myVariable == 42) Sleep(500);
			myVariable += 1;
			Sleep(200);
		}
	}
	catch (...) {
		printf("caught an exception");
	}

	printf("myvar = 0, bye\n");
}