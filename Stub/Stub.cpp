// Stub.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		std::string tmp = "Stub is running ";
		tmp += argv[1]; 
		tmp += "!!";
		std::cout << tmp << std::endl;
	}		
	else
		std::cout << "Stub is running!!\n";

	return 0;
}
