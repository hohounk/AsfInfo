#include <iostream>

#include "AsfFile.h"


using namespace std;

int main(int argc, char* argv[])
{
	cout << argv[0] << endl;

	AsfFile file;
	// XXX: temporary, get from command line later
	file.open("SimpleSampleVideo.wmv");
	file.process();

	/*
	read input from param
	memmap the file
	check for file type
	printInfo()
	close everything down

	throw exceptions on errors

	*/
	return 0;
}

