#include "AsfFile.h"
#include <fstream>
#include <iostream>

/*
typedef struct _GUID {
  DWORD Data1;
  WORD  Data2;
  WORD  Data3;
  BYTE  Data4[8];
} GUID;
*/

/* Keeping the definitions in here for now.
 * In future if those are needed in more places they should be moved out to a separate header
 */

struct AsfHeader
{
	_GUID objectId;
	uint64_t objectSize;
	uint32_t numHeaderObjects;
	uint8_t reserved1; // can be ignored
	uint8_t reserved2; // != 0x02 -> "fail to source the content" -> exit
};

struct StreamFlags 
{
	unsigned char streamNumber : 7;
	uint8_t reserved;
	unsigned char encrypted : 1;
};

struct StreamPropertiesObject
{
	_GUID objectId;
	uint64_t objectSize;
	_GUID streamType;
	_GUID errorCorrectionType;
	uint64_t timeOffset;
	uint32_t timeSpecificDataLength;
	uint32_t errorCorrectionDataLenght;
	StreamFlags flags;
	uint32_t reserved;
	// uint8_t* typeSpecificData;
	// uint8_t* errorCorrectionData;
};


void getHeader(std::ifstream& file, AsfHeader& header)
{
	file.read(reinterpret_cast<char*>(&header), sizeof(AsfHeader));
}

AsfFile::~AsfFile(void)
{
	close();
}

void AsfFile::open(std::string filename)
{
	// Non-mmapped file for now. Get it working first, then optimize
	_input.open(filename, std::ios::in);
	if (!_input.good()) {
		throw(std::exception("error opening file"));
	}
}

void AsfFile::process()
{
	// get header
	AsfHeader header;
	getHeader(_input, header);

	std::cout << "Header object size: " /*<< std::cout.hex*/ << header.objectSize << std::endl;

	// count streams
	// loop over streams
	//	output Type Specific Data to #.dat
	//	if Encrypted Content Flag
	//		print "stream encrypted
	//	if Stream Type == video
	//		print Compression ID
}

void AsfFile::close()
{
	// Assume any errors in the file stream have been detected earlier already
	// so that closing would always succeed. If there were an error we couldn't 
	// throw an exception anyway as this would be called from a destructor and
	// throwing an exception from it would mess things up really bad
	_input.close();
}
