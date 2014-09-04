#include "AsfFile.h"
#include <fstream>
#include <iostream>
#include <array>

/*
typedef struct _GUID {
  DWORD Data1;
  WORD  Data2;
  WORD  Data3;
  BYTE  Data4[8];
} GUID;
*/

namespace {

// Taken from http://blog.tomaka17.com/2012/09/c-converting-guid-to-string-and-vice-versa/ with slight modifications
///
std::string guidToString(GUID guid) {
	std::array<char,40> output;
	_snprintf_s(output.data(), output.size(), 40, "{%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return std::string(output.data());
}

GUID stringToGUID(const std::string& guid) {
	GUID output;
	// Due to sscanf not being able to properly read stuff into 8-bit integers without trashing the stack I need to read in the last four bytes manually
	int p4, p5, p6, p7;
	const auto ret = sscanf_s(guid.c_str(), "{%8X-%4hX-%4hX-%2hX%2hX-%2hX%2hX%2hX%2hX%2hX%2hX}", &output.Data1, &output.Data2, &output.Data3, &output.Data4[0], &output.Data4[1], &output.Data4[2], &output.Data4[3], &p4, &p5, &p6, &p7);
	if (ret != 11)
		throw std::logic_error("Unvalid GUID, format should be {00000000-0000-0000-0000-000000000000}");
	output.Data4[4] = p4;
	output.Data4[5] = p5;
	output.Data4[6] = p6;
	output.Data4[7] = p7;
	return output;
}
///

//ASF_Index_Object	D6E229D3-35DA-11D1-9034-00A0C90349BE
//ASF_Media_Object_Index_Object	FEB103F8-12AD-4C64-840F-2A1D2F7AD48C
//ASF_Timecode_Index_Object	3CB73FD0-0C4A-4803-953D-EDF7B6228F0C


};

static const _GUID ASF_Header_Object = stringToGUID("{75B22630-668E-11CF-A6D9-00AA0062CE6C}");
static const _GUID ASF_Simple_Index_Object = stringToGUID("{33000890-E5B1-11CF-89F4-00A0C90349CB}");

static const _GUID ASF_File_Properties_Object = stringToGUID("{8CABDCA1-A947-11CF-8EE4-00C00C205365}");
static const _GUID ASF_Stream_Properties_Object	 = stringToGUID("{B7DC0791-A9B7-11CF-8EE6-00C00C205365}");
//ASF_Header_Extension_Object	5FBF03B5-A92E-11CF-8EE3-00C00C205365



AsfObjectType getNextfObjectType(std::ifstream& in)
{
	_GUID nextGUID;
	auto pos = in.tellg();
	in.read(reinterpret_cast<char*>(&nextGUID), sizeof(nextGUID));
	in.seekg(pos);

	if (nextGUID == ASF_Header_Object)
		return ASF_Header;
	if (nextGUID == ASF_File_Properties_Object)
		return ASF_FileProperties;
	if (nextGUID == ASF_Stream_Properties_Object)
		return ASF_StreamProperties;

	return ASF_Unknown;
}


// Sizediff allows to read in StreamProperties that has variable-lenght fields inside it
template<class Object>
void getAsfObject(std::ifstream& file, Object& object, int sizediff = 0)
{
	file.read(reinterpret_cast<char*>(&object), sizeof(object) + sizediff);
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
	getAsfObject<AsfHeader>(_input, header);

	std::cout << "Num header objects: " /*<< std::cout.hex*/ << header.numHeaderObjects << std::endl;
	if (header.objectId != ASF_Header_Object)
		throw std::exception("File not in ASF format");
	if (header.reserved2 != 0x02)
		throw std::exception("Failed to source the content");


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
