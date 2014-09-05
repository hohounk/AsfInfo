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

// From http://www.codeproject.com/Tips/479880/GetLastError-as-std-string
std::string getLastErrorStdStr()
{
  DWORD error = GetLastError();
  if (error)
  {
    LPVOID lpMsgBuf;
    DWORD bufLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
    if (bufLen)
    {
      LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
      std::wstring result(lpMsgStr, lpMsgStr+bufLen*2);
      
      LocalFree(lpMsgBuf);

	  return std::string(result.begin(), result.end());
    }
  }
  return std::string();
}

};

// Values taken from http://drang.s4.xrea.com/program/tips/id3tag/wmp/10_asf_guids.html

static const _GUID ASF_Header_Object = stringToGUID("{75B22630-668E-11CF-A6D9-00AA0062CE6C}");
static const _GUID ASF_Simple_Index_Object = stringToGUID("{33000890-E5B1-11CF-89F4-00A0C90349CB}");

static const _GUID ASF_File_Properties_Object = stringToGUID("{8CABDCA1-A947-11CF-8EE4-00C00C205365}");
static const _GUID ASF_Stream_Properties_Object	 = stringToGUID("{B7DC0791-A9B7-11CF-8EE6-00C00C205365}");
//ASF_Header_Extension_Object	5FBF03B5-A92E-11CF-8EE3-00C00C205365



AsfObjectType AsfFile::getNextObjectType()
{
	const _GUID* nextGUID = reinterpret_cast<const _GUID*>(&_mappedFile[_streamPos]);

	if (*nextGUID == ASF_Header_Object)
		return ASF_Header;
	if (*nextGUID == ASF_File_Properties_Object)
		return ASF_FileProperties;
	if (*nextGUID == ASF_Stream_Properties_Object)
		return ASF_StreamProperties;

	return ASF_Unknown;
}

// Sizediff allows to read in StreamProperties that has variable-lenght fields inside it
// XXX: not sure if moving stream pointer here is a good idea as it can make handling variable-lenght stuff a bit harder down the line
template<class Object>
const Object* AsfFile::getAsfObject(int sizediff)
{
	const Object* o = reinterpret_cast<const Object*>(&_mappedFile[_streamPos]);
	_streamPos += sizeof(Object) + sizediff;
	return o;
}

AsfFile::~AsfFile(void)
{
	close();
}

std::wstring s2ws (const std::string& s) 
{
    std::wstring ws;
    ws.assign (s.begin (), s.end ());
    return ws;
}


void AsfFile::open(std::string filename)
{
	// CreateFileW requires wide strings
	std::wstring fnameW = s2ws(filename);
	_hFile = ::CreateFileW(fnameW.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	
	if (_hFile == INVALID_HANDLE_VALUE)
		throw std::exception("Error opening file");
	
	_hMapping = CreateFileMapping(_hFile, 0, PAGE_READONLY, 0, 0, 0);
	if (_hMapping == INVALID_HANDLE_VALUE)
		throw std::exception("Error creating file mapping");

	_mappedFile = reinterpret_cast<char*>(::MapViewOfFile(_hMapping, FILE_MAP_READ, 0, 0, 0));
	if (_mappedFile == NULL)
		throw std::exception("Error creating file mapping view");
}

void AsfFile::processStreamProperties(const StreamProperties* prop)
{
	//a. For all streams in the media file, the application extracts the Type Specific Data field of the **Stream Properties Object** into a file named #.dat, where # is the number of the stream.
	//b. For any stream that is encrypted (as defined by the **Encrypted Content Flag** of the **Stream Properties Object**), the application writes a message to the console output. E.g. “Stream 2 is encrypted”.
}

void AsfFile::process()
{
	// get header
	const AsfHeader* header = getAsfObject<AsfHeader>();

	std::cout << "Num header objects: " << header->numHeaderObjects << std::endl;
	if (header->objectId != ASF_Header_Object)
		throw std::exception("File not in ASF format");
	if (header->reserved2 != 0x02)
		throw std::exception("Failed to source the content");

	for (int i = 0; i < header->numHeaderObjects; i++) {
		switch (getNextObjectType()) {
			case (ASF_StreamProperties): {
				const StreamProperties* prop = getAsfObject<StreamProperties>();
				processStreamProperties(prop);
				break;
			}
			default: {
				const AsfBaseObject* o = getAsfObject<AsfBaseObject>();
				_streamPos += o->objectSize-sizeof(AsfBaseObject); // 24 bytes is already seeked in getAsfObject
			}
		}
	}
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
	UnmapViewOfFile(_mappedFile);
	CloseHandle(_hMapping);
	CloseHandle(_hFile);
}
