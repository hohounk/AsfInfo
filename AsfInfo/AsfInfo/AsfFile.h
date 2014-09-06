#pragma once
#include <Rpc.h>
#include <cstdint>
#include <string>


// Structs need to be packed to be able to use sizeof().
#define PACK( Object ) __pragma( pack(push, 1) ) Object __pragma( pack(pop) )


/* Keeping the ASF object definitions in here for now.
 * In future if those are needed in more places they should be moved out to a separate header
 */

// XXX: make it into a class hierarchy to get rid of D.R.Y?
PACK(
struct AsfBaseObject
{
	GUID objectId;
	uint64_t objectSize;
});

PACK(
struct AsfHeader
{
	GUID objectId;
	uint64_t objectSize;
	uint32_t numHeaderObjects;
	uint8_t reserved1; // can be ignored
	uint8_t reserved2; // != 0x02 -> "fail to source the content" -> exit
});


// Need to split the 8-bit reserved data into two as C++ doesn't allow having part of a byte crossing a byte boundary.
PACK(
struct StreamFlags
{
	unsigned char streamNumber : 7;
	unsigned char reserved1 : 1;
	unsigned char reserved2 : 7;
	unsigned char encrypted : 1;
});


PACK(
struct StreamPropertiesHeader
{
	GUID objectId;
	uint64_t objectSize;
	GUID streamType;
	GUID errorCorrectionType;
	uint64_t timeOffset;
	uint32_t typeSpecificDataLength;
	uint32_t errorCorrectionDataLenght;
	StreamFlags flags;
	uint32_t reserved;
});


PACK(
struct StreamProperties
{
	const StreamPropertiesHeader* header;
	const uint8_t* typeSpecificData;
	const uint8_t* errorCorrectionData;
});


PACK(
struct VideoStreamData
{
	uint32_t encodedImageWidth;
	uint32_t encodedImageHeight;
	uint8_t reservedFlags;
	uint16_t formatDataSize;
	BITMAPINFOHEADER formatData;
});


enum AsfObjectType {
	ASF_Header = 0,
	ASF_StreamProperties,
	ASF_Video_Media,
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
	const AsfObjectType getNextObjectType() const;

	template<class Object>
	const Object* getAsfObject(const int sizediff = 0);

	void processStreamProperties(const StreamProperties& prop);
	const uint32_t extractCompressionId(const StreamProperties& prop) const;

	// memory mapped input file and current position in the file
	uint8_t* _mappedFile;
	size_t _streamPos;

	// Handles needed for memory mapping
	HANDLE _hMapping;
	HANDLE _hFile;
};

