#ifndef RASTER_DATA_H
#define RASTER_DATA_H

#include <QString>
#include <vector>
#include <map>

// RasterData 类用于存储和管理栅格图像数据及其相关信息
class RasterData
{
private:
	unsigned char* mpImgData; // 存储栅格图像数据
	size_t mnRows;            // 栅格数据的行数
	size_t mnCols;            // 栅格数据的列数
	size_t mnBands;           // 栅格数据的波段数
	size_t mnLength;          // 栅格数据的总字节数
	size_t mnPerPixSize;      // 每像素字节数
	double mGeoTransform[6];  // 存储地理变换参数
	std::map<int, std::vector<int>> mvHistogram; // 存储每个波段的直方图

public:
	// 构造函数：初始化栅格数据和相关参数
	RasterData(unsigned char* pImgData, size_t rows, size_t cols, size_t bands, size_t length, size_t perPixSize, double geoTransform[6]);
	~RasterData(); // 析构函数：释放栅格数据内存
	RasterData(const RasterData& other); // 拷贝构造函数

	void closeRasterData(); // 释放栅格数据的内存
	unsigned char* getImgData() const; // 获取栅格图像数据指针
	size_t getRows() const; // 获取栅格数据的行数
	size_t getCols() const; // 获取栅格数据的列数
	size_t getBandnum() const; // 获取栅格数据的波段数
	size_t getLength() const; // 获取栅格数据的总字节数
	size_t getPerPixSize() const; // 获取每像素的字节数
	double getTopLeftLongitude() const; // 获取左上角经度
	double getTopLeftLatitude() const;  // 获取左上角纬度

	// 设置和获取直方图数据
	void setHistogram(int band, const std::vector<int>& histogram);
	const std::vector<int>& getHistogram(int band) const;
};

#endif
