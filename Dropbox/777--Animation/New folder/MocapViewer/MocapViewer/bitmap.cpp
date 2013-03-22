
#include"bitmap.h"

void Bitmap_RGBA::writeInt(uint32 value, const int bytes){

	BYTE* bin = new BYTE[bytes];

	const int cByte(8); //byte constant
	uint32 off(0);
	
	uint32	temp = value;

	for(int i = 0; i<bytes; i++){
		temp >>= i*cByte;
		bin[i] = (BYTE)temp;
		
	}

	outstream.write(bin, bytes);
	delete[] bin;
	bin = NULL;
	

}
uint32 Bitmap_RGBA::readInt(const uint32 bytes){
	
	BYTE* bin = new BYTE[bytes];

	instream.read(bin, bytes);

	const int cByte(8); //byte constant
	uint32 numeral(0);
	for(unsigned i = 1; i<=bytes; i++){
		uint32 temp = (uint32)bin[bytes-i];
		temp &= 0x000000FF;//mask any filler bits
		temp <<=  (bytes-i)*cByte;//shift to proper location in int
		numeral |= temp;
	}

	delete[] bin;
	bin = NULL;
	return numeral;

}
void Bitmap_RGBA::load(const string& filename)
{
	instream.open(filename.c_str(), ios::in | ios::binary);
	
	instream.read(bmg.magic_num, 2);
	bh.size = readInt(4);
	bh.reserve1 = readInt(2);
	bh.reserve2 = readInt(2);
	bh.offset = readInt(4);
	//
	bhi.hSize = readInt(4);
	bhi.width = readInt(4);
	bhi.height = readInt (4);
	bhi.cPlanes = readInt(2);
	bhi.bpPixels = readInt(2);
	bhi.compression = readInt(4);
	bhi.imageSize = readInt(4);
	bhi.hRes = readInt(4);
	bhi.vRes = readInt(4);
	bhi.colors = readInt(4);
	bhi.iColors = readInt(4);
	//

	int padding(0);
	if(bhi.width % 4 != 0){
		padding = 4 - (bhi.width%4);
	}
	instream.seekg(bh.offset, ios_base::beg);

	uint32 bytes = bhi.imageSize;

	mapdata = new RGBA[bytes];
	uint32 pixel_bytes = bhi.bpPixels/8;
	uint32 line_bytes = (pixel_bytes)*bhi.width;//bytes per line
	for(unsigned row = 0; row < bhi.height; row++){
		
		BYTE* buffer = new BYTE[line_bytes];

		instream.read(buffer, line_bytes);

		for(unsigned pix_num = 0; pix_num < (bhi.width*pixel_bytes); pix_num += pixel_bytes){
			RGBA pixel;
			//parse raw data width/24bits
			pixel.blue = buffer[pix_num];
			pixel.green = buffer[pix_num+1];
			pixel.red = buffer[pix_num+2];
			
			mapdata[row*bhi.width+(pix_num/pixel_bytes)] = pixel;
		}

		instream.seekg(padding+instream.tellg(), ios_base::beg);//skip over padding

		delete[] buffer;
		buffer = NULL;
	}
	//cout<<instream.eof()<<endl;

	instream.close();

}
void Bitmap_RGBA::write(const string& filename){

	outstream.open(filename.c_str(), ios::out | ios::binary);

	outstream.write(bmg.magic_num, 2);
	writeInt(bh.size,4);
	writeInt(bh.reserve1, 2);
	writeInt(bh.reserve2, 2);
	writeInt(bh.offset, 4);
	//
	writeInt(bhi.hSize, 4);
	writeInt(bhi.width, 4);
	writeInt(bhi.height, 4);
	writeInt(bhi.cPlanes, 2);
	writeInt(bhi.bpPixels,2);
	writeInt(bhi.compression,4);
	writeInt(bhi.imageSize, 4);
	writeInt(bhi.hRes, 4);
	writeInt(bhi.vRes, 4);
	writeInt(bhi.colors,4);
	writeInt(bhi.iColors, 4);

	outstream.seekp(bh.offset, ios_base::beg);//move stream to bitmap data

	int padding(0);
	if(bhi.width % 4 != 0){
		padding = 4 - (bhi.width%4);
	}
	//padding = 3;
	//cout<<"Padding: "<<padding<<endl;
	BYTE pad[4] = {'\0','\0', '\0','\0'};//array of null strings for padding

	uint32 pixel_bytes = (bhi.bpPixels/8);
	uint32 line_bytes = pixel_bytes*bhi.width;//bytes per line, w/o padding
	for(unsigned i = 0; i < bhi.height; i++){
		//works if we write one more line
		
		BYTE* buffer = new BYTE[line_bytes];

		//pix num has to change
		for(uint32 pix_num = 0, buffer_i = 0; 
			pix_num < bhi.width; buffer_i += pixel_bytes, pix_num++)
		{
			//parse raw data width/24bits
			buffer[buffer_i]   =  mapdata[pix_num+(i*bhi.width)].blue;
			buffer[buffer_i+1] =  mapdata[pix_num+(i*bhi.width)].green;
			buffer[buffer_i+2] =  mapdata[pix_num+(i*bhi.width)].red;
			
		}

		outstream.write(buffer, line_bytes);
		outstream.write(pad, padding);//write padding

		delete[] buffer;
		buffer = NULL;
	}

	outstream.close();
}

void Bitmap_RGBA::printDataFile(){

	ofstream infostream;
	infostream.open("Info.txt", ios::out);

	//infostream.write(bmg.magic_num, 2);
	infostream<<"Bitmap size: "<<bh.size<<endl;
	infostream<<"Reserve1: "<<bh.reserve1<<endl;
	infostream<<"Reserve2: "<<bh.reserve2<<endl;
	infostream<<"Offset: "<<bh.offset<<endl;
	//
	infostream<<"Header size: "<<bhi.hSize<<endl;
	infostream<<"Width: "<<bhi.width<<endl;
	infostream<<"Height: "<<bhi.height<<endl;
	infostream<<"Color planes: "<<bhi.cPlanes<<endl;
	infostream<<"Bits/Pixel: "<<bhi.bpPixels<<endl;
	infostream<<"Compression: "<<bhi.compression<<endl;
	infostream<<"ImageSize: "<<bhi.imageSize<<endl;
	infostream<<"Horizontal Resolution: "<<bhi.hRes<<endl;
	infostream<<"Vertical Resolution: "<<bhi.vRes<<endl;
	infostream<<"Color Palette (0 = default): "<<bhi.colors<<endl;
	infostream<<"Important Colors: "<<bhi.iColors<<endl;

	infostream.close();

}

uint32 Bitmap_RGBA::getPixel(uint32 x, uint32 y){
		
	if((x > bhi.height && x < 0) || (y > bhi.width && y < 0))
		return - 1;
	else{
		uint32 mask = 0x000000FF;
		uint32 color_value = 0x00000000;
		uint32 blue = (uint32)mapdata[y*bhi.width+x].blue & mask;
		uint32 green = (uint32)mapdata[y*bhi.width+x].green & mask;
		uint32 red   = (uint32)mapdata[y*bhi.width+x].red & mask;

		color_value |= (red <<= 24) | (green <<= 16) | (blue <<= 8);
		return color_value;


	}
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


void Bitmap_Loader::writeInt(uint32 value, const int bytes, ofstream& outstream){

	BYTE* bin = new BYTE[bytes];

	const int cByte(8); //byte constant
	uint32 off(0);
	
	uint32	temp = value;

	for(int i = 0; i<bytes; i++){
		temp >>= i*cByte;
		bin[i] = (BYTE)temp;
		
	}

	outstream.write(bin, bytes);
	delete[] bin;
	bin = NULL;
	

}
uint32 Bitmap_Loader::readInt(const uint32 bytes, ifstream& instream){
	
	BYTE* bin = new BYTE[bytes];

	instream.read(bin, bytes);

	const int cByte(8); //byte constant
	uint32 numeral(0);
	for(unsigned i = 1; i<=bytes; i++){
		uint32 temp = (uint32)bin[bytes-i];
		temp &= 0x000000FF;//mask any filler bits
		temp <<=  (bytes-i)*cByte;//shift to proper location in int
		numeral |= temp;
	}

	delete[] bin;
	bin = NULL;
	return numeral;

}
void Bitmap_Loader::BGR_TO_RGB(BYTE data[], uint32 size, uint32 tuple_size){

	for(unsigned i(0); i < (size - 2); i += tuple_size){
		
		BYTE temp	= data[i];
		data[  i]   = data[i+2];
		data[i+2]	= temp;
	
	}

}
BYTE* Bitmap_Loader::loadRAW(const string& filename,BYTE* &data, long &width,long &height){

	ifstream instream(filename.c_str(), ios::in | ios::binary);

	if( !instream )
	{
		cerr << "File not found " << filename.c_str() <<endl;
		exit(1);
	}


	instream.seekg(10, ios::cur);
	long offset = readInt(4,instream);
	
	//
	instream.seekg(4, ios::cur);
	width = readInt(4,instream);
	height = readInt (4,instream);
	uint32 planes = readInt(2,instream);

	if( planes != 1 )
	{
		cout << "Planes from " <<  filename.c_str() << " is not 1: " << planes <<endl;
		exit(1);
	}

	uint32 bpp = readInt(2,instream);//bytes per pixel

	if( bpp < 24 )
	{
		cout << "Bpp from " <<  filename.c_str()  << " is too small: " << bpp <<endl;
		exit(1);
	}

	instream.seekg(4, ios::cur);
	long size = readInt(4,instream);

	instream.seekg(offset, ios_base::beg);

	data = new BYTE[size];
	instream.read(data, size);

	BGR_TO_RGB(data,size, 4);

	return data;


}

//////////////////////////////////////////////////////////////////////////////////


BYTE raw_bmp::pixel_elem(int x, int y, int component)
{
	int pos = (y*width+x) * 3 + component;
	return data[pos];
}

BYTE *raw_bmp::pixel_pos(int x, int y)
{
	int pos = (y * width + x) * 3;
	return &data[pos];
}