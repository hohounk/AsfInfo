#pragma once
#include <Rpc.h>
#include <cstdint>

/*
a. For all streams in the media file, the application extracts the Type Specific Data field of the **Stream Properties Object** into a file named #.dat, where # is the number of the stream.
b. For any stream that is encrypted (as defined by the **Encrypted Content Flag** of the **Stream Properties Object**), the application writes a message to the console output. E.g. “Stream 2 is encrypted”.
c. For any video stream (as defined by the **Stream Type** field of the **Stream Properties Object**), the application will write to the console the hex value of the Compression ID field in the 
BITMAPINFOHEADER structure that corresponds to this stream. E.g. “Compression ID for stream 1 is 0x12345678”.
*/

#include <string>
#include <fstream>
#include <vector>

/* Keeping the ASF object definitions in here for now.
 * In future if those are needed in more places they should be moved out to a separate header
 */

// XXX: make it into a class hierarchy to get rid of D.R.Y?
struct AsfBaseObject
{
	_GUID objectId;
	uint64_t objectSize;
};

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

struct StreamProperties
{
	_GUID objectId;
	uint64_t objectSize;
	_GUID streamType;	// If it's a video stream, print out compression ID
	_GUID errorCorrectionType;
	uint64_t timeOffset;
	uint32_t timeSpecificDataLength;
	uint32_t errorCorrectionDataLenght;
	StreamFlags flags; // print out "stream # is encrypted if flag tells it is
	uint32_t reserved;
	// uint8_t* typeSpecificData; // <- needs to be extracted to .dat
	// uint8_t* errorCorrectionData;
};

enum AsfObjectType {
	ASF_Header = 0,
	ASF_FileProperties,
	ASF_StreamProperties,
	ASF_Unknown
};

class AsfFile {
public:
	AsfFile(void) {}
	~AsfFile(void);

	void open(std::string filename);
	void process();
	void close();

private:
	AsfObjectType getNextObjectType();

	template<class Object>
	const Object* getAsfObject(int sizediff = 0);

	void processStreamProperties(const StreamProperties* prop);


	// Input file to be analyzed
	std::ifstream _input;

	// memory mapped input file and current position in the file
	char* _mappedFile;
	size_t _streamPos;

	// Handles needed for memory mapping
	HANDLE _hMapping;
	HANDLE _hFile;

	// Cached stream properties objects
	std::vector<const StreamProperties*> _streamProperties;
};

