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

class AsfFile {
public:
	AsfFile(void);
	void open(std::string filename);
	void process();
	void close();
	~AsfFile(void);
private:
	std::ifstream _input;
};

