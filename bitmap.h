//////////////////////////////////////////////////////////
//														//
//														//
// Creation: October 2009								//
// Modified: Dec 2020									//
//														//
//														//
//////////////////////////////////////////////////////////
#pragma once

#include <iostream>
#include <fstream>
#include <assert.h>

#include "color.h"

enum BitmapFormat : uint32_t
{
	BITMAP_ABGR = 0,
	BITMAP_ARGB = 1,
	BITMAP_BGRA = 2,
	BITMAP_RGBA = 3,
};


class Bitmap
{
public:
	Bitmap() = delete;

	Bitmap( const std::string& filename );
	Bitmap( const Bitmap& bitmap );
	Bitmap( const uint32_t width, const uint32_t height, const uint32_t color = ~0x00 );
	~Bitmap();

	Bitmap& operator=( const Bitmap& bitmap );
	static void CopyToPixel( const RGBA& rgba, Pixel& pixel, BitmapFormat format );

	void Load( const std::string& filename );
	void Write( const std::string& filename );

	uint32_t GetSize();
	uint32_t GetWidth();
	uint32_t GetHeight();

	void ClearImage();

	void GetBuffer( uint32_t buffer[] );

	uint32_t GetPixel( const int32_t x, const int32_t y );
	bool SetPixel( const int32_t x, const int32_t y, const uint32_t color );

private:

	struct headerInfo_t
	{
		uint8_t magicNum[ 2 ];
		uint32_t size;
		uint16_t reserve1;
		uint16_t reserve2;
		uint32_t offset;
		uint32_t hSize;
		uint32_t width;
		uint32_t height;
		uint16_t cPlanes;
		uint16_t bpPixels;
		uint32_t compression;
		uint32_t imageSize;
		uint32_t hRes;
		uint32_t vRes;
		uint32_t colors;
		uint32_t iColors;
	} h;

	Pixel* mapdata;
	uint32_t pixelsNum;

	std::ifstream instream;
	std::ofstream outstream;

	template<int ByteCnt>
	uint32_t ReadInt();

	template<int ByteCnt>
	void WriteInt( const uint32_t value );
};