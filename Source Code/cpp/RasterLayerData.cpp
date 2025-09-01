#include "RasterLayerData.h"
#include <cstring>  // ���� std::memcpy

// ���캯��
RasterLayerData::RasterLayerData(const QString& layerName, int rows, int cols, int bands, std::vector<unsigned char> imgData, double geoTransform[6])
	: mLayerName(layerName), mRows(rows), mCols(cols), mBands(bands), mImgData(std::move(imgData)) {
	std::memcpy(mGeoTransform, geoTransform, sizeof(mGeoTransform));  // ���Ƶ���任����
}

// ��������
RasterLayerData::~RasterLayerData() {
	// ȷ���ͷŲ������ݵ��ڴ�
	if (!mImgData.empty()) {
		mImgData.clear();
		std::vector<unsigned char>().swap(mImgData);  // �ͷ��ѷ�����ڴ�
	}
}

// ������Ա����ʵ��
int RasterLayerData::getRows() const {
	return mRows;
}

int RasterLayerData::getCols() const {
	return mCols;
}

int RasterLayerData::getBands() const {
	return mBands;
}

const unsigned char* RasterLayerData::getImgData() const {
	return mImgData.data();
}

const double* RasterLayerData::getGeoTransform() const {
	return mGeoTransform;
}

QString RasterLayerData::getLayerName() const {
	return mLayerName;
}
