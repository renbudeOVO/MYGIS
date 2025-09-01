#include "RasterLayerData.h"
#include <cstring>  // 用于 std::memcpy

// 构造函数
RasterLayerData::RasterLayerData(const QString& layerName, int rows, int cols, int bands, std::vector<unsigned char> imgData, double geoTransform[6])
	: mLayerName(layerName), mRows(rows), mCols(cols), mBands(bands), mImgData(std::move(imgData)) {
	std::memcpy(mGeoTransform, geoTransform, sizeof(mGeoTransform));  // 复制地理变换参数
}

// 析构函数
RasterLayerData::~RasterLayerData() {
	// 确保释放波段数据的内存
	if (!mImgData.empty()) {
		mImgData.clear();
		std::vector<unsigned char>().swap(mImgData);  // 释放已分配的内存
	}
}

// 其他成员函数实现
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
