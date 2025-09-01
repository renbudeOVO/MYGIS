#ifndef RASTERLAYERDATA_H
#define RASTERLAYERDATA_H

#include <QString>
#include <vector>

class RasterLayerData {
public:
	RasterLayerData() = default;
	RasterLayerData(const QString& layerName, int rows, int cols, int bands, std::vector<unsigned char> imgData, double geoTransform[6]);
	~RasterLayerData();

	// 获取栅格的行数
	int getRows() const;

	// 获取栅格的列数
	int getCols() const;

	// 获取栅格的波段数
	int getBands() const;

	// 获取栅格图像数据的指针
	const unsigned char* getImgData() const;

	// 获取栅格的地理变换参数数组
	const double* getGeoTransform() const;

	// 获取图层名称
	QString getLayerName() const;

private:
	QString mLayerName;  // 图层名称
	int mRows;           // 栅格的行数
	int mCols;           // 栅格的列数
	int mBands;          // 栅格的波段数
	std::vector<unsigned char> mImgData;  // 栅格图像数据
	double mGeoTransform[6];  // 地理变换参数数组
};

#endif // RASTERLAYERDATA_H
