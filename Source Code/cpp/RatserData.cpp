#include "RasterData.h"
#include <cstring>

// 构造函数：初始化栅格数据及其相关属性
RasterData::RasterData(unsigned char* pImgData, size_t rows, size_t cols, size_t bands, size_t length, size_t perPixSize, double geoTransform[6])
	: mpImgData(pImgData), mnRows(rows), mnCols(cols), mnBands(bands), mnLength(length), mnPerPixSize(perPixSize)
{
	// 复制地理变换参数
	std::memcpy(mGeoTransform, geoTransform, sizeof(mGeoTransform));
}

// 析构函数：释放栅格数据内存
RasterData::~RasterData()
{
	if (mpImgData)
	{
		delete[] mpImgData;
	}
}

// 拷贝构造函数：深拷贝栅格数据及相关属性
RasterData::RasterData(const RasterData& other)
{
	if (other.mpImgData) {
		size_t dataSize = other.mnLength;
		mpImgData = new unsigned char[dataSize];
		std::memcpy(mpImgData, other.mpImgData, dataSize);
	}
	else {
		mpImgData = nullptr;
	}

	mnRows = other.mnRows;
	mnCols = other.mnCols;
	mnBands = other.mnBands;
	mnLength = other.mnLength;
	mnPerPixSize = other.mnPerPixSize;
	std::memcpy(mGeoTransform, other.mGeoTransform, sizeof(mGeoTransform));
	mvHistogram = other.mvHistogram; // 拷贝直方图数据
}

// 释放栅格数据内存
void RasterData::closeRasterData()
{
	if (mpImgData)
	{
		delete[] mpImgData;
		mpImgData = nullptr;
	}
}

// 获取栅格图像数据指针
unsigned char* RasterData::getImgData() const { return mpImgData; }
// 获取栅格数据的行数
size_t RasterData::getRows() const { return mnRows; }
// 获取栅格数据的列数
size_t RasterData::getCols() const { return mnCols; }
// 获取栅格数据的波段数量
size_t RasterData::getBandnum() const { return mnBands; }
// 获取栅格数据的总字节数
size_t RasterData::getLength() const { return mnLength; }
// 获取每像素字节数
size_t RasterData::getPerPixSize() const { return mnPerPixSize; }

// 获取左上角经度
double RasterData::getTopLeftLongitude() const
{
	return mGeoTransform[0];
}

// 获取左上角纬度
double RasterData::getTopLeftLatitude() const
{
	return mGeoTransform[3];
}

// 设置直方图数据
void RasterData::setHistogram(int band, const std::vector<int>& histogram)
{
	mvHistogram[band] = histogram;
}

// 获取直方图数据
const std::vector<int>& RasterData::getHistogram(int band) const
{
	return mvHistogram.at(band);
}
