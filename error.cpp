#include "error.h"

#include <string>
#include <iostream>

void printError(ErrorType type, std::string errorMessage) {
	switch (type) {
	case WARNING:
#ifdef _DEBUG
		std::cout << "WARNING: " << errorMessage << std::endl;
#else
		return;
#endif
		break;
	case ERROR:
		std::cout << "ERROR: " << errorMessage << std::endl;
		break;

	case CRITICAL_ERROR:
		std::cout << "CRITICAL ERROR: " << errorMessage << std::endl;
		std::cout << "Press a key to close the program" << std::endl;
		std::cin.get();
		exit(EXIT_SUCCESS);
		break;
	}
}
