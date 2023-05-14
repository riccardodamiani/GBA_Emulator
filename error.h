#ifndef ERROR_H
#define ERROR_H

#include <string>

enum ErrorType {
	WARNING,
	ERROR,
	CRITICAL_ERROR,
};

void printError(ErrorType type, std::string errorMessage);


#endif