#include "AsfFile.h"

/*
typedef struct _GUID {
  DWORD Data1;
  WORD  Data2;
  WORD  Data3;
  BYTE  Data4[8];
} GUID;
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


AsfFile::AsfFile(void)
{
}


AsfFile::~AsfFile(void)
{
}
