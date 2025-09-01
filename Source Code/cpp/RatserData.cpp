#include "RasterData.h"
#include <cstring>

// ���캯������ʼ��դ�����ݼ����������
RasterData::RasterData(unsigned char* pImgData, size_t rows, size_t cols, size_t bands, size_t length, size_t perPixSize, double geoTransform[6])
	: mpImgData(pImgData), mnRows(rows), mnCols(cols), mnBands(bands), mnLength(length), mnPerPixSize(perPixSize)
{
	// ���Ƶ���任����
	std::memcpy(mGeoTransform, geoTransform, sizeof(mGeoTransform));
}

// �����������ͷ�դ�������ڴ�
RasterData::~RasterData()
{
	if (mpImgData)
	{
		delete[] mpImgData;
	}
}

// �������캯�������դ�����ݼ��������
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
	mvHistogram = other.mvHistogram; // ����ֱ��ͼ����
}

// �ͷ�դ�������ڴ�
void RasterData::closeRasterData()
{
	if (mpImgData)
	{
		delete[] mpImgData;
		mpImgData = nullptr;
	}
}

// ��ȡդ��ͼ������ָ��
unsigned char* RasterData::getImgData() const { return mpImgData; }
// ��ȡդ�����ݵ�����
size_t RasterData::getRows() const { return mnRows; }
// ��ȡդ�����ݵ�����
size_t RasterData::getCols() const { return mnCols; }
// ��ȡդ�����ݵĲ�������
size_t RasterData::getBandnum() const { return mnBands; }
// ��ȡդ�����ݵ����ֽ���
size_t RasterData::getLength() const { return mnLength; }
// ��ȡÿ�����ֽ���
size_t RasterData::getPerPixSize() const { return mnPerPixSize; }

// ��ȡ���ϽǾ���
double RasterData::getTopLeftLongitude() const
{
	return mGeoTransform[0];
}

// ��ȡ���Ͻ�γ��
double RasterData::getTopLeftLatitude() const
{
	return mGeoTransform[3];
}

// ����ֱ��ͼ����
void RasterData::setHistogram(int band, const std::vector<int>& histogram)
{
	mvHistogram[band] = histogram;
}

// ��ȡֱ��ͼ����
const std::vector<int>& RasterData::getHistogram(int band) const
{
	return mvHistogram.at(band);
}
