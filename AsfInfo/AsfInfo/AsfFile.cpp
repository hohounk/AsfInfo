#include "AsfFile.h"
#include <fstream>
#include <iostream>
#include <array>
#include <sstream>


namespace {

// Taken from http://blog.tomaka17.com/2012/09/c-converting-guid-to-string-and-vice-versa/ with
// slight modifications
const GUID stringToGUID(const std::string& guid) {
	GUID output;
	// Due to sscanf not being able to properly read stuff into 8-bit integers without trashing
	// the stack I need to read in the last four bytes manually
	// XXX: should it be last 8 bytes on 64bit? So far it looks like not.
	int p4, p5, p6, p7;
	const auto ret = sscanf_s(guid.c_str(), "{%8X-%4hX-%4hX-%2hX%2hX-%2hX%2hX%2hX%2hX%2hX%2hX}",
		&output.Data1, &output.Data2, &output.Data3, &output.Data4[0], &output.Data4[1],
		&output.Data4[2], &output.Data4[3], &p4, &p5, &p6, &p7);
	if (ret != 11)
		throw std::logic_error("Unvalid GUID, format should be {00000000-0000-0000-0000-000000000000}");
	output.Data4[4] = p4;
	output.Data4[5] = p5;
	output.Data4[6] = p6;
	output.Data4[7] = p7;
	return output;
}

};

// Values taken from http://drang.s4.xrea.com/program/tips/id3tag/wmp/10_asf_guids.html
static const GUID ASF_Header_Object            = stringToGUID("{75B22630-668E-11CF-A6D9-00AA0062CE6C}");
static const GUID ASF_Stream_Properties_Object = stringToGUID("{B7DC0791-A9B7-11CF-8EE6-00C00C205365}");
static const GUID ASF_Video_Media_Object       = stringToGUID("{BC19EFC0-5B4D-11CF-A8FD-00805F5C442B}");

const AsfObjectType AsfFile::getNextObjectType() const
{
	const GUID* nextGUID = reinterpret_cast<const _GUID*>(&_mappedFile[_streamPos]);

	if (*nextGUID == ASF_Header_Object)
		return ASF_Header;
	if (*nextGUID == ASF_Stream_Properties_Object)
		return ASF_StreamProperties;

	return ASF_Unknown;
}


// Sizediff allows to read in StreamProperties that has variable-lenght fields inside it
// XXX: not sure if moving stream pointer here is a good idea as it can make handling variable-lenght
// stuff a bit harder down the line
template<class Object>
const Object* AsfFile::getAsfObject(const int sizediff)
{
	const Object* o = reinterpret_cast<const Object*>(&_mappedFile[_streamPos]);
	_streamPos += sizeof(Object) + sizediff;
	return o;
}


AsfFile::~AsfFile(void)
{
	close();
}


void AsfFile::open(std::string filename)
{
	std::cout << "Opening file '" << filename << "'" << std::endl;
	_streamPos = 0;
	// CreateFileW requires wide strings
	std::wstring fnameW(filename.begin(), filename.end());
	_hFile = ::CreateFileW(fnameW.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	
	if (_hFile == INVALID_HANDLE_VALUE)
		throw std::exception("Error opening file");
	
	_hMapping = CreateFileMapping(_hFile, 0, PAGE_READONLY, 0, 0, 0);
	if (_hMapping == INVALID_HANDLE_VALUE)
		throw std::exception("Error creating file mapping");

	_mappedFile = reinterpret_cast<uint8_t*>(::MapViewOfFile(_hMapping, FILE_MAP_READ, 0, 0, 0));
	if (_mappedFile == NULL)
		throw std::exception("Error creating file mapping view");
}


const uint32_t AsfFile::extractCompressionId(const StreamProperties& prop) const
{
	const VideoStreamData* data = reinterpret_cast<const VideoStreamData*>(prop.typeSpecificData);
	return data->formatData.biCompression;
}


void AsfFile::processStreamProperties(const StreamProperties& prop)
{
	int streamNum = prop.header->flags.streamNumber;
	if (prop.header->flags.encrypted == 0x01)
		std::cout << "Stream #" << streamNum << " is encrypted" << std::endl;

	// Dump type specific data to file overwriting any existing.
	// Though it also means when more than one file is analysed then only the last files will remain.
	std::stringstream filename;
	filename << int(streamNum) << ".dat";

	std::ofstream out(filename.str(), std::ios::trunc | std::ios::out | std::ios::binary);
	if (!out.good()) {
		std::stringstream s;
		s << "Error opening file" << filename.str();
		throw std::exception(s.str().c_str());
	}
	out.write(reinterpret_cast<const char*>(&prop.typeSpecificData), prop.header->typeSpecificDataLength);

	if (prop.header->streamType == ASF_Video_Media_Object) {
		auto compressionId = extractCompressionId(prop);
		std::cout << "Compression ID for stream #" << streamNum << " is 0x" << std::hex << compressionId 
			<< std::dec << std::endl;
	}
}


void AsfFile::process()
{
	const AsfHeader* header = getAsfObject<AsfHeader>();

	// Elementary sanity check
	if (header->objectId != ASF_Header_Object)
		throw std::exception("File not in ASF format");
	if (header->reserved2 != 0x02)
		throw std::exception("Failed to source the content");

	for (uint32_t i = 0; i < header->numHeaderObjects; i++) {
		switch (getNextObjectType()) {
			case (ASF_StreamProperties): {
				StreamProperties prop;
				prop.header = getAsfObject<StreamPropertiesHeader>();

				// StreamProperties is variable-lenght, need to manually map the two fields to point to
				// proper places
				prop.typeSpecificData = &_mappedFile[_streamPos];
				_streamPos += prop.header->typeSpecificDataLength;
				prop.errorCorrectionData = &_mappedFile[_streamPos];
				_streamPos += prop.header->errorCorrectionDataLenght;

				processStreamProperties(prop);
				break;
			}
			default: {
				const AsfBaseObject* o = getAsfObject<AsfBaseObject>();
				// sizeof(AsfBaseObject) bytes is already seeked in getAsfObject
				_streamPos += o->objectSize - sizeof(AsfBaseObject);
			}
		}
	}
}


void AsfFile::close()
{
	// Assume any errors in the file stream have been detected earlier already
	// so that closing would always succeed. If there were an error we couldn't
	// throw an exception anyway as this would be called from a destructor and
	// throwing an exception from it would mess things up really bad
	UnmapViewOfFile(_mappedFile);
	CloseHandle(_hMapping);
	CloseHandle(_hFile);
}
