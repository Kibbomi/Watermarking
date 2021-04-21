#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
using namespace std;

typedef unsigned char BYTE;	//1byte
typedef unsigned short WORD;	//2byte
typedef unsigned long DWORD;	//4byte
bool extract = false, embed = false;
const int K = 2;	//strength factor

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
}pixelData[512][512], watermarkedPixelData[512][512];

BYTE watermarkPixelData[64][8];	//8*8 = 64bit 1bit = 1 pixel
BYTE extractedWatermarkPixelData[64][8];
BYTE preprocessedWatermark[64][8];	//mixed 

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

void loadFile(string &imageName, string &watermarkedImageName) {
	ifstream File(imageName, ios::in | ios::binary);

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
	else
	{
		cout << "error : such file does not exist!\n";
		exit(0);
	}
	File.close();

	if (extract)
	{
		ifstream File(watermarkedImageName, ios::in | ios::binary);

		if (File.is_open()) {
			//Header is same...
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
					watermarkedPixelData[i][j].blue = File.get();
					watermarkedPixelData[i][j].green = File.get();
					watermarkedPixelData[i][j].red = File.get();
				}
			}
		}
		else
		{
			cout << "error : such file is not exist!\n";
			exit(0);
		}
		File.close();
	}
	return;
}
void loadWatermark(string &watermarkName) {
	ifstream File(watermarkName, ios::in | ios::binary);

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
	ofstream File("output.bmp", ios::out | ios::binary | ios::trunc);
	
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

	//original data
	/*for (int i = 0; i < 512; ++i) {
		for (int j = 0; j < 512; ++j) {
			File.put(pixelData[i][j].blue);
			File.put(pixelData[i][j].green);
			File.put(pixelData[i][j].red);
		}
	}*/
	for (int i = 0; i < 512; ++i) {
		for (int j = 0; j < 512; ++j) {
			File.put(watermarkedPixelData[i][j].blue);
			File.put(watermarkedPixelData[i][j].green);
			File.put(watermarkedPixelData[i][j].red);
		}
	}


	File.close();
	return;
}
void saveWatermark()
{
	ofstream File("outputWatermark.bmp", ios::out | ios::binary | ios::trunc);
	
	//write header
	File.put(66);	//0
	File.put(77);	//1
	File.put(62);	//2
	File.put(2);	//3
	File.put(0);	//4
	File.put(0);	//5
	File.put(0);	//6
	File.put(0);	//7
	File.put(0);	//8
	File.put(0);	//9
	File.put(62);	//10
	File.put(0);	//11
	File.put(0);	//12
	File.put(0);	//13
	File.put(40);	//14
	File.put(0);	//15
	File.put(0);	//16
	File.put(0);	//17
	File.put(64);	//18
	File.put(0);	//19
	File.put(0);	//20
	File.put(0);	//21
	File.put(64);	//22
	File.put(0);	//23
	File.put(0);	//24
	File.put(0);	//25
	File.put(1);	//26
	File.put(0);	//27
	File.put(1);	//28
	File.put(0);	//29
	File.put(0);	//30
	File.put(0);	//31
	File.put(0);	//32
	File.put(0);	//33
	File.put(0);	//34
	File.put(2);	//35
	File.put(0);	//36
	File.put(0);	//37
	File.put(0);	//38
	File.put(0);	//39
	File.put(0);	//40
	File.put(0);	//41
	File.put(0);	//42
	File.put(0);	//43
	File.put(0);	//44
	File.put(0);	//45
	File.put(0);	//46
	File.put(0);	//47
	File.put(0);	//48
	File.put(0);	//49
	File.put(0);	//50
	File.put(0);	//51
	File.put(0);	//52
	File.put(0);	//53
	File.put(0);	//54
	File.put(0);	//55
	File.put(0);	//56
	File.put(0);	//57
	File.put(-1);	//58
	File.put(-1);	//59
	File.put(-1);	//60
	File.put(0);	//61

	//Header end
	
	
	//Data begin
	for (int i = 0; i < 64; ++i) 
		for (int j = 0; j < 8; ++j)
			File.put(extractedWatermarkPixelData[i][j]);
	
	File.close();
	return;
}

int Mask1[3][3] = { {4,2,4},{2,1,2},{4,2,4} };
int Mask2[3][3] = { {8,4,8},{4,2,4},{8,4,8} };

int SIRD(colorData block[8][8]) 
{
	int subBlockIndex = -1;
	int SD = 0, maxSD = 0;

	//sub-block 1
	for (int i = 0; i < 3; ++i) 
		for (int j = 0; j < 3; ++j) 
			if (i == j) SD += abs(block[i][j].blue - block[i + 1][j + 1].blue);
	if (SD > maxSD) {
		maxSD = SD;
		subBlockIndex = 1;
	}

	//sub-block 2
	SD = 0;
	for (int i = 0; i < 3; ++i) 
		for (int j = 4; j < 7; ++j) 
			if (i + 4 == j) SD += abs(block[i][j].blue - block[i + 1][j + 1].blue);
	if (SD > maxSD){
		maxSD = SD;
		subBlockIndex = 2;
	}
	
	//sub-block 3
	SD = 0;
	for (int i = 4; i < 7; ++i) 
		for (int j = 4; j < 7; ++j) 
			if (i == j) SD += abs(block[i][j].blue - block[i + 1][j + 1].blue);
	if (SD > maxSD) {
		maxSD = SD;
		subBlockIndex = 3;
	}
	//sub-block 4
	SD = 0;
	for (int i = 4; i < 7; ++i) 
		for (int j = 0; j < 3; ++j) 
			if (i == j + 4) SD += abs(block[i][j].blue - block[i + 1][j + 1].blue);
	if (SD > maxSD) {
		maxSD = SD;
		subBlockIndex = 4;
	}
		
	return subBlockIndex;
}

void preprocessing() {
	
	vector<int> w[5];

	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 8; ++j) {
			BYTE sampleByte = watermarkPixelData[i][j];

			if (i % 2 == 0) {
				w[1].push_back(sampleByte >> 7 & 1);
				w[3].push_back(sampleByte >> 6 & 1);
				w[1].push_back(sampleByte >> 5 & 1);
				w[3].push_back(sampleByte >> 4 & 1);
				w[1].push_back(sampleByte >> 3 & 1);
				w[3].push_back(sampleByte >> 2 & 1);
				w[1].push_back(sampleByte >> 1 & 1);
				w[3].push_back(sampleByte >> 0 & 1);
			}
			else
			{
				w[2].push_back(sampleByte >> 7 & 1);
				w[4].push_back(sampleByte >> 6 & 1);
				w[2].push_back(sampleByte >> 5 & 1);
				w[4].push_back(sampleByte >> 4 & 1);
				w[2].push_back(sampleByte >> 3 & 1);
				w[4].push_back(sampleByte >> 2 & 1);
				w[2].push_back(sampleByte >> 1 & 1);
				w[4].push_back(sampleByte >> 0 & 1);
			}
		}
	}
	
	
	for (int i = 0; i < 32; ++i) {
		for (int j = 0; j < 4; ++j) {
			BYTE sampleByte[5] = { 0 };
			for (int k = 0; k < 8; ++k) {
				BYTE sampleBit[5] = { 0 };
				
				for (int l = 1; l < 5; ++l) {
					sampleBit[l] = w[l][i * 32 + j * 8 + k];
					sampleByte[l] = sampleByte[l] << 1;
					sampleByte[l] |= sampleBit[l];
				}
				
			}
			preprocessedWatermark[i][j] = sampleByte[1];
			preprocessedWatermark[i][j + 4] = sampleByte[2];
			preprocessedWatermark[i + 32][j + 4] = sampleByte[3];
			preprocessedWatermark[i + 32][j] = sampleByte[4];
		}
	}
	

	return;
}

void embedSolution()
{
	//begin
	int y = 0, x = 0;

	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 8; ++j) {
			BYTE sampledByte = preprocessedWatermark[i][j];
			for (int k = 0; k < 8; ++k)
			{
				colorData subBlock[8][8] = { 0 };

				//8x8
				for (int dy = 0; dy < 8; ++dy)
					for (int dx = 0; dx < 8; ++dx) {
						subBlock[dy][dx].blue = watermarkedPixelData[y + dy][x + dx].blue = pixelData[y + dy][x + dx].blue;
						subBlock[dy][dx].green = watermarkedPixelData[y + dy][x + dx].green = pixelData[y + dy][x + dx].green;
						subBlock[dy][dx].red = watermarkedPixelData[y + dy][x + dx].red = pixelData[y + dy][x + dx].red;
					}

				int embedded_subBlock = SIRD(subBlock);

				//4x4 sub block is selected
				int sub_y, sub_x;

				if (embedded_subBlock == 1) {
					sub_y = y; sub_x = x;
				}
				else if (embedded_subBlock == 2) {
					sub_y = y; sub_x = x + 4;
				}
				else if (embedded_subBlock == 3) {
					sub_y = y + 4; sub_x = x + 4;
				}
				else if (embedded_subBlock == 4) {
					sub_y = y + 4; sub_x = x;
				}

				BYTE sampledBit = sampledByte >> k & 1;

				for (int yy = 0; yy < 3; ++yy) {
					for (int xx = 0; xx < 3; ++xx) {
						//Blue Channel
						if (sampledBit == 1) {
							watermarkedPixelData[sub_y + yy][sub_x + xx].blue = pixelData[sub_y + yy][sub_x + xx].blue + (BYTE)ceil((K / Mask1[yy][xx]));
						}
						else
						{
							watermarkedPixelData[sub_y + yy][sub_x + xx].blue = pixelData[sub_y + yy][sub_x + xx].blue - (BYTE)ceil((K / Mask1[yy][xx]));
						}

						//Green, Red Channel
						if (sampledBit == 1) {
							watermarkedPixelData[sub_y + yy][sub_x + xx].green = pixelData[sub_y + yy][sub_x + xx].green + (BYTE)ceil((K / Mask2[yy][xx]));
							watermarkedPixelData[sub_y + yy][sub_x + xx].red = pixelData[sub_y + yy][sub_x + xx].red + (BYTE)ceil((K / Mask2[yy][xx]));
						}
						else
						{
							watermarkedPixelData[sub_y + yy][sub_x + xx].green = pixelData[sub_y + yy][sub_x + xx].green - (BYTE)ceil((K / Mask2[yy][xx]));
							watermarkedPixelData[sub_y + yy][sub_x + xx].red = pixelData[sub_y + yy][sub_x + xx].red - (BYTE)ceil((K / Mask2[yy][xx]));
						}
					}
				}

				x += 8;
				if (x == 512)
				{
					x = 0;
					y += 8;
				}

			}
		}
	}
	return;
}

void extractSolution()
{
	//begin
	int y = 0, x = 0;

	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 8; ++j) {

			for (int k = 0; k < 8; ++k)
			{
				colorData subBlock[8][8] = { 0 };

				//8x8
				for (int dy = 0; dy < 8; ++dy)
					for (int dx = 0; dx < 8; ++dx) {
						subBlock[dy][dx].blue = pixelData[y + dy][x + dx].blue;
						subBlock[dy][dx].green = pixelData[y + dy][x + dx].green;
						subBlock[dy][dx].red = pixelData[y + dy][x + dx].red;
					}

				int embedded_subBlock = SIRD(subBlock);

				//4x4 sub block is selected
				int sub_y, sub_x;

				if (embedded_subBlock == 1) {
					sub_y = y; sub_x = x;
				}
				else if (embedded_subBlock == 2) {
					sub_y = y; sub_x = x + 4;
				}
				else if (embedded_subBlock == 3) {
					sub_y = y + 4; sub_x = x + 4;
				}
				else if (embedded_subBlock == 4) {
					sub_y = y + 4; sub_x = x;
				}

				int Ps = 0, PsW = 0;

				for (int yy = 0; yy < 3; ++yy) 
					for (int xx = 0; xx < 3; ++xx) {
						Ps += pixelData[sub_y + yy][sub_x + xx].blue;
						PsW += watermarkedPixelData[sub_y + yy][sub_x + xx].blue;
						Ps += pixelData[sub_y + yy][sub_x + xx].green;
						PsW += watermarkedPixelData[sub_y + yy][sub_x + xx].green;
						Ps += pixelData[sub_y + yy][sub_x + xx].red;
						PsW += watermarkedPixelData[sub_y + yy][sub_x + xx].red;
					}
				BYTE wbit = 0;

				if (PsW > Ps)
					wbit = 1;
				else
					wbit = 0;

				extractedWatermarkPixelData[i][j] |= wbit << k;

				x += 8;
				if (x == 512)
				{
					x = 0;
					y += 8;
				}
			}
		}
	}
	return;
}

void combineWatermark()
{
	vector<int> w[5];

	for (int i = 0; i < 32; ++i) {
		for (int j = 0; j < 4; ++j) {

			BYTE sampleByte[5];
			sampleByte[1] = extractedWatermarkPixelData[i][j];
			sampleByte[2] = extractedWatermarkPixelData[i][j + 4];
			sampleByte[3] = extractedWatermarkPixelData[i + 32][j + 4];
			sampleByte[4] = extractedWatermarkPixelData[i + 32][j];

			for (int k = 7; k >= 0; --k) 
			{
				w[1].push_back(sampleByte[1] >> k & 1);
				w[2].push_back(sampleByte[2] >> k & 1);
				w[3].push_back(sampleByte[3] >> k & 1);
				w[4].push_back(sampleByte[4] >> k & 1);
			}			
		}
	}

	int index1 = 0, index2 = 0;

	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 8; ++j) {
			BYTE sampleByte = { 0 };

			if (i % 2 == 0) {
				sampleByte |= w[1][index1];
				sampleByte = sampleByte << 1;
				sampleByte |= w[3][index1++];
				sampleByte = sampleByte << 1;

				sampleByte |= w[1][index1];
				sampleByte = sampleByte << 1;
				sampleByte |= w[3][index1++];
				sampleByte = sampleByte << 1;

				sampleByte |= w[1][index1];
				sampleByte = sampleByte << 1;
				sampleByte |= w[3][index1++];
				sampleByte = sampleByte << 1;

				sampleByte |= w[1][index1];
				sampleByte = sampleByte << 1;
				sampleByte |= w[3][index1++];
				
			}
			else
			{
				sampleByte |= w[2][index2];
				sampleByte = sampleByte << 1;
				sampleByte |= w[4][index2++];
				sampleByte = sampleByte << 1;

				sampleByte |= w[2][index2];
				sampleByte = sampleByte << 1;
				sampleByte |= w[4][index2++];
				sampleByte = sampleByte << 1;

				sampleByte |= w[2][index2];
				sampleByte = sampleByte << 1;
				sampleByte |= w[4][index2++];
				sampleByte = sampleByte << 1;

				sampleByte |= w[2][index2];
				sampleByte = sampleByte << 1;
				sampleByte |= w[4][index2++];
			}
			extractedWatermarkPixelData[i][j] = sampleByte;
		}
	}



	
	return;

}
int main()
{
	cout << "What do you want to do?\n" << "1. embed watermark\n" << "2. extract watermark\n" <<  "3. exit \n"<<">> ";
	int input = 0;
	cin >> input;

	while (input <= 0 || 4 <= input) {
		cout << "error : enter the number between 1 to 3\n";
		cin >> input;
	}

	if (input == 1) 
		embed = true;
	else if(input == 2)
		extract = true;
	else
		return 0;
	

	if (embed) {
		string imageName, watermarkedImageName, watermarkName;
		cout << "enter the file(image) name: ";
		cin >> imageName;
		watermarkedImageName = "";

		cout << "enter the file(watermark) name: ";
		cin >> watermarkName;

		loadFile(imageName, watermarkedImageName);
		loadWatermark(watermarkName);
		preprocessing();
		embedSolution();
		saveFile();
	}
	else
	{
		string imageName, watermarkedImageName;
		cout << "enter the file(image) name: ";
		cin >> imageName;
		cout << "enter the file(watermarked image) name : ";
		cin >> watermarkedImageName;

		loadFile(imageName, watermarkedImageName);
		extractSolution();
		combineWatermark();
		saveWatermark();
	}

	return 0;
}
