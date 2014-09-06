#include <iostream>

#include "AsfFile.h"


using namespace std;

void usage()
{
	std::cout << "Analyses media files in Advanced Systems Format and prints out some parameters for those files." << std::endl;
	std::cout << "Usage: AsfInfo.exe filename [filename]..." << std::endl;
}

int main(int argc, char* argv[])
{
	// At least one parameter must be given
	if (argc < 2) {
		usage();
		return -1;
	}

	for (auto i = 1; i < argc; i++)
	{
		try {
			AsfFile file;
			file.open(argv[i]);
			file.process();
		} catch (const std::exception& e) {
			std::cout << "Error processing '"<< argv[i] << "': " << e.what() << std::endl;
		}
	}
	return 0;
}

