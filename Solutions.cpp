#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

typedef unsigned char BYTE;	//1byte
typedef unsigned short WORD;	//2byte
typedef unsigned long DWORD;	//4byte

#define SHIFT(a, n) (a<<n)
int i = 0;

struct BITMAPFILEHEADER {
	BYTE bfType[2];	//2byte [0]='M', [1] = 'B' --> "BM"
	DWORD bfSize;	//4byte
	WORD bfReserved1;	//2byte
	WORD bfReserved2;	//2byte
	DWORD bfOffBits;	//4byte
}bmpHeader, watermarkHeader;

struct BITMAPINFOHEADER {
	DWORD biSize;
	DWORD biWidth;
	DWORD biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	DWORD biXPelsPerMeter;
	DWORD biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
}bmpInfo, watermarkInfo;

//B G R color data
struct colorData {
	BYTE blue;
	BYTE green;
	BYTE red;
}pixelData[512][512];

BYTE watermarkPixelData[64][8];	//8*8 = 64bit 1bit = 1 pixel
BYTE subWatermark[5][64][2];	//index 1~4

DWORD read4byte(ifstream &File) {
	DWORD ret = 0;

	BYTE data[4];
	for (int i = 0; i < 4; ++i) 
		data[i] = File.get();
	
	for (int i = 3; i >= 0; --i)
	{
		ret = ret << 8;
		ret |= data[i];
	}

	return ret;
}
void write4byte(ofstream &File, DWORD &original)
{
	for (int i = 0; i < 4; ++i) {
		BYTE writedata = 0;
		writedata |= original;
		File.put(writedata);
		original = original >> 8;
	}
	return;
}

WORD read2byte(ifstream &File) {
	WORD ret = 0;

	BYTE data[2];
	for (int i = 0; i < 2; ++i)
		data[i] = File.get();

	for (int i = 1; i >= 0; --i)
	{
		ret = ret << 8;
		ret |= data[i];
	}

	return ret;
}
void write2byte(ofstream &File, WORD &original)
{
	for (int i = 0; i < 2; ++i) {
		BYTE writedata = 0;
		writedata |= original;
		File.put(writedata);
		original = original >> 8;
	}
	return;
}

void loadFile() {
	ifstream File("Test.bmp", ios::in | ios::binary);

	if (File.is_open()) {
		bmpHeader.bfType[1] = File.get();
		bmpHeader.bfType[0] = File.get();
		bmpHeader.bfSize = read4byte(File);
		bmpHeader.bfReserved1 = read2byte(File);
		bmpHeader.bfReserved2 = read2byte(File);
		bmpHeader.bfOffBits = read4byte(File);

		bmpInfo.biSize = read4byte(File);
		bmpInfo.biWidth = read4byte(File);
		bmpInfo.biHeight = read4byte(File);
		bmpInfo.biPlanes = read2byte(File);
		bmpInfo.biBitCount = read2byte(File);
		bmpInfo.biCompression = read4byte(File);
		bmpInfo.biSizeImage = read4byte(File);
		bmpInfo.biXPelsPerMeter = read4byte(File);
		bmpInfo.biYPelsPerMeter = read4byte(File);
		bmpInfo.biClrUsed = read4byte(File);
		bmpInfo.biClrImportant = read4byte(File);

		for (int i = 0; i < 512; ++i) {
			for (int j = 0; j < 512; ++j) {
				pixelData[i][j].blue = File.get();
				pixelData[i][j].green = File.get();
				pixelData[i][j].red = File.get();
			}
		}
	}
	File.close();
	return;
}
void loadWatermark() {
	ifstream File("watermarkTest.bmp", ios::in | ios::binary);

	if (File.is_open()) {
		watermarkHeader.bfType[1] = File.get();
		watermarkHeader.bfType[0] = File.get();
		watermarkHeader.bfSize = read4byte(File);
		watermarkHeader.bfReserved1 = read2byte(File);
		watermarkHeader.bfReserved2 = read2byte(File);
		watermarkHeader.bfOffBits = read4byte(File);

		watermarkInfo.biSize = read4byte(File);
		watermarkInfo.biWidth = read4byte(File);
		watermarkInfo.biHeight = read4byte(File);
		watermarkInfo.biPlanes = read2byte(File);
		watermarkInfo.biBitCount = read2byte(File);
		watermarkInfo.biCompression = read4byte(File);
		watermarkInfo.biSizeImage = read4byte(File);
		watermarkInfo.biXPelsPerMeter = read4byte(File);
		watermarkInfo.biYPelsPerMeter = read4byte(File);
		watermarkInfo.biClrUsed = read4byte(File);
		watermarkInfo.biClrImportant = read4byte(File);

		//color table
		//File.get();
		//File.get();
		//File.get();
		//File.get();
		File.seekg(watermarkHeader.bfOffBits);

		for (int i = 0; i < 64; ++i) 
			for (int j = 0; j < 8; ++j) 
				watermarkPixelData[i][j] = File.get();

	}
	File.close();
	return;
}
void saveFile()
{
	ofstream File("out.bmp", ios::out | ios::binary | ios::trunc);
	
	File.put(bmpHeader.bfType[1]);
	File.put(bmpHeader.bfType[0]);
	write4byte(File, bmpHeader.bfSize);
	write2byte(File, bmpHeader.bfReserved1);
	write2byte(File, bmpHeader.bfReserved2);
	write4byte(File, bmpHeader.bfOffBits);

	write4byte(File, bmpInfo.biSize);
	write4byte(File, bmpInfo.biWidth);
	write4byte(File, bmpInfo.biHeight);
	write2byte(File, bmpInfo.biPlanes);
	write2byte(File, bmpInfo.biBitCount);
	write4byte(File, bmpInfo.biCompression);
	write4byte(File, bmpInfo.biSizeImage);
	write4byte(File, bmpInfo.biXPelsPerMeter);
	write4byte(File, bmpInfo.biYPelsPerMeter);
	write4byte(File, bmpInfo.biClrUsed);
	write4byte(File, bmpInfo.biClrImportant);

	for (int i = 0; i < 512; ++i) {
		for (int j = 0; j < 512; ++j) {
			File.put(pixelData[i][j].blue);
			File.put(pixelData[i][j].green);
			File.put(pixelData[i][j].red);
		}
	}
	
	File.close();
	return;
}
void saveWatermark()
{
	ofstream File("outWatermark.bmp", ios::out | ios::binary | ios::trunc);

	File.put(watermarkHeader.bfType[1]);
	File.put(watermarkHeader.bfType[0]);
	write4byte(File, watermarkHeader.bfSize);
	write2byte(File, watermarkHeader.bfReserved1);
	write2byte(File, watermarkHeader.bfReserved2);
	write4byte(File, watermarkHeader.bfOffBits);

	write4byte(File, watermarkInfo.biSize);
	write4byte(File, watermarkInfo.biWidth);
	write4byte(File, watermarkInfo.biHeight);
	write2byte(File, watermarkInfo.biPlanes);
	write2byte(File, watermarkInfo.biBitCount);
	write4byte(File, watermarkInfo.biCompression);
	write4byte(File, watermarkInfo.biSizeImage);
	write4byte(File, watermarkInfo.biXPelsPerMeter);
	write4byte(File, watermarkInfo.biYPelsPerMeter);
	write4byte(File, watermarkInfo.biClrUsed);
	write4byte(File, watermarkInfo.biClrImportant);

	//colortable.
	File.put(0);	//54
	File.put(0);	//55
	File.put(0);	//56
	File.put(0);	//57
	File.put(-1);	//58
	File.put(-1);	//59
	File.put(-1);	//60
	File.put(0);	//61
	//File.seekp(62);

	for (int i = 0; i < 64; ++i) 
		for (int j = 0; j < 8; ++j)
			File.put(watermarkPixelData[i][j]);

	File.close();
	return;
}

int Mask1[3][3] = { {4,2,4},{2,1,2},{4,2,4} };
int Mask2[3][3] = { {8,4,8},{4,2,4},{8,4,8} };

int SIRD(BYTE block[8][8]) 
{
	int subBlockIndex = -1;
	int SD = 0, maxSD = 0;

	//sub-block 1
	for (int i = 0; i < 3; ++i) 
		for (int j = 0; j < 3; ++j) 
			if (i == j) SD += abs(block[i][j] - block[i + 1][j + 1]);
	if (SD > maxSD) {
		maxSD = SD;
		subBlockIndex = 1;
	}

	//sub-block 2
	SD = 0;
	for (int i = 0; i < 3; ++i) 
		for (int j = 4; j < 7; ++j) 
			if (i + 4 == j) SD += abs(block[i][j] - block[i + 1][j + 1]);
	if (SD > maxSD){
		maxSD = SD;
		subBlockIndex = 2;
	}
	
	//sub-block 3
	SD = 0;
	for (int i = 4; i < 7; ++i) 
		for (int j = 4; j < 7; ++j) 
			if (i == j) SD += abs(block[i][j] - block[i + 1][j + 1]);
	if (SD > maxSD) {
		maxSD = SD;
		subBlockIndex = 3;
	}
	//sub-block 4
	SD = 0;
	for (int i = 4; i < 7; ++i) 
		for (int j = 0; j < 3; ++j) 
			if (i == j + 4) SD += abs(block[i][j] - block[i + 1][j + 1]);
	if (SD > maxSD) {
		maxSD = SD;
		subBlockIndex = 4;
	}
		
	return subBlockIndex;
}

void preprocessing() {
	//일단 십자가 모양으로 해보자.
	//Attack을 하지 않으면 추출에는 문제 없을 듯.
	
	//sub watermark 1
	for (int i = 0; i < 32; ++i) 
		for (int j = 0; j < 4; ++j) 
			subWatermark[1][i][j] = watermarkPixelData[i][j];
		
	

	//sub watermark 2
	for (int i = 0; i < 32; ++i) 
		for (int j = 4; j < 8; ++j) 
			subWatermark[2][i][j - 4] = watermarkPixelData[i][j];
		
	

	//sub watermark3
	for (int i = 32; i < 64; ++i)
		for (int j = 4; j < 8; ++j)
			subWatermark[3][i - 32][j - 4] = watermarkPixelData[i][j];
		
	

	for (int i = 32; i < 64; ++i) 
		for (int j = 0; j < 4; ++j)
			subWatermark[4][i - 32][j] = watermarkPixelData[i][j];
		
	return;
}

int main()
{
	//loadFile();
	loadWatermark();
	
	
	//algorithm
	








	//saveFile();
	saveWatermark();
	return 0;
}