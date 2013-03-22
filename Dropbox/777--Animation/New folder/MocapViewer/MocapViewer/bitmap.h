//////////////////////////////////////////////////////////
//
//
//Last Modified: 03/02/2013
//
//
//
//
//////////////////////////////////////////////////////////
#ifndef _BITMAPCLASS_
#define _BITMAPCLASS_
#include<iostream>
#include<fstream>
using namespace std;

typedef char BYTE;
typedef unsigned int uint32;
typedef unsigned short uint16;

class Bitmap_Loader{

	public:

		Bitmap_Loader(){
		}
		~Bitmap_Loader(){
		}
//		RGBA* loadRGBA(const string& filename);
		void write(const string& filename);

		BYTE* loadRAW(const string& filename,BYTE* &data, long &width,long &height);

		//void writeRAW(const string& filename);
		//void writeRGBA(const string& filename);

	private:

		struct BITMAP_MagicNum{
			BYTE magic_num[2];//2
		}bmg;
		struct BITMAP_Header{
			uint32 size;//4 bytes in file
			uint16 reserve1;//2
			uint16 reserve2;//2
			uint32 offset;//4

		}bh;
		struct BITMAP_Header_Info{
			uint32 hSize;//header size, 4
			uint32 width;//image width, 4
			uint32 height;//image height, 4
			uint16 cPlanes; //color planes, 2
			uint16 bpPixels;//bits per pixel, 2
			uint32 compression;//4
			uint32 imageSize;//4
			uint32 hRes;//4
			uint32 vRes;//4
			uint32 colors;//4
			uint32 iColors;//4
		}bhi;
		struct RGBA{
			BYTE blue;
			BYTE green;
			BYTE red;
			BYTE alpha;

		}*mapdata;//actual bitmap data
		//
		ifstream instream;
		ofstream outstream;
		uint32 readInt(const uint32 bytes, ifstream& instream);
		void writeInt(uint32 value, const int bytes, ofstream& outstream);
		void BGR_TO_RGB(BYTE data[], uint32 size, uint32 tuple_size);
		void BGRA_TO_RGBA(BYTE data[], uint32 size);
		

};

class Bitmap_RGBA  {
	public:
		Bitmap_RGBA(const string& filename){
			load(filename);
			//Bitmap_Loader loader = new Bitmap_Loader();
			//mapdata = loader.load();
			//delete loader;

		}
		~Bitmap_RGBA(){
			delete[] mapdata;
		}
	
		void load(const string& filename);
		void write(const string& filename);
		void printDataFile();
		inline uint32 getSize(){
			return bhi.imageSize;
		}
		inline uint32 getWidth(){
			return bhi.width;
		}
		inline uint32 getHeight(){
			return bhi.height;
		}
		void deleteRow(uint32 row){
			int col(bhi.width*row);
			for(unsigned i(0); i<bhi.width; ++i){
				mapdata[i+col].red = '\0';
				mapdata[i+col].green = '\0';
				mapdata[i+col].blue = '\0';
			} 
		}
		void invert(){
			for(unsigned i(0); i< bhi.imageSize; ++i){
				mapdata[i].red = ~mapdata[i].red;
				mapdata[i].green = ~mapdata[i].green;
				mapdata[i].blue = ~mapdata[i].blue;
			}
		}
		void clearImage(){
			for(unsigned i(0); i< bhi.imageSize; ++i){
				mapdata[i].red = '\0';
				mapdata[i].green = '\0';
				mapdata[i].blue = '\0';
			}
		
		}
		inline void deleteColume(){
		}
		uint32 getPixel(uint32 x, uint32 y);
		uint32 setPixel(uint32 x, uint32 y, uint32 red = 0xFFFFFF00, uint32 green = 0xFFFFFF00, uint32 blue  = 0xFFFFFF00);

	private:

		struct BITMAP_MagicNum{
			BYTE magic_num[2];//2
		}bmg;
		struct BITMAP_Header{
			uint32 size;//4 bytes in file
			uint16 reserve1;//2
			uint16 reserve2;//2
			uint32 offset;//4

		}bh;
		struct BITMAP_Header_Info{
			uint32 hSize;//header size, 4
			uint32 width;//image width, 4
			uint32 height;//image height, 4
			uint16 cPlanes; //color planes, 2
			uint16 bpPixels;//bits per pixel, 2
			uint32 compression;//4
			uint32 imageSize;//4
			uint32 hRes;//4
			uint32 vRes;//4
			uint32 colors;//4
			uint32 iColors;//4
		}bhi;
		struct RGBA{
			BYTE blue;
			BYTE green;
			BYTE red;
			BYTE alpha;

		}*mapdata;//actual bitmap data
		//
		ifstream instream;
		ofstream outstream;
		uint32 readInt(uint32 bytes);
		void writeInt(uint32, int length);
		
};
struct raw_bmp {

	long width, height;

	BYTE* data;

	raw_bmp(const string& filename) : width(0), height(0), data(NULL){

		Bitmap_Loader().loadRAW(filename,data, width, height);

	}
	~raw_bmp(){
		if( data )	delete[] data;
	}

	BYTE pixel_elem(int x, int y, int elem);
	BYTE *pixel_pos(int x, int y);

};

#endif