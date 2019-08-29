/*
Sample code to decrypt any user-created .sai file
*/

#include <cstdint>
#include <fstream>
#include <iostream>
#include <chrono>
#include <utility>
#include <sai.hpp>

#include "Benchmark.hpp"

const char* const Help =
"Decrypt user-created .sai files:\n"
"\tDecrypt.exe (filename) (output)\n"
"\tWunkolo - Wunkolo@gmail.com";

int main(int argc, char* argv[])
{
	if( argc < 3 )
	{
		puts(Help);
		return EXIT_FAILURE;
	}

	sai::ifstreambuf FileIn;
	FileIn.open(argv[1]);

	if( !FileIn.is_open() )
	{
		std::cout << "Error opening file for reading: " << argv[1] << std::endl;
		return EXIT_FAILURE;
	}

	std::ofstream FileOut;
	FileOut.open(argv[2], std::ios::binary);

	if( !FileOut.is_open() )
	{
		std::cout << "Error opening file for writing: " << argv[2] << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "File decrypted in:"
		<< Benchmark<std::chrono::nanoseconds>::Run(
		[&]() -> void
	{
		FileOut << &FileIn;
	}
	).count() << "ns" << std::endl;

	return EXIT_SUCCESS;
}
