#ifndef CLASS_GDAL_RASTERREAD
#define CLASS_GDAL_RASTERREAD

#include "gdal_priv.h"
#include "ogr_core.h"
#include "ogr_spatialref.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "RasterData.h"

// GDALRasterRead 类用于读取和管理 GDAL 栅格数据集。
class GDALRasterRead
{
private:
	std::map<QString, RasterData*> mvRasterData; // 以图层名称为键，存储栅格数据

	GDALDataset* mpoDataset;// GDAL 数据集指针
	unsigned char* mpData;// 栅格数据指针
	GDALDataType mgDataType;// 栅格数据类型
	size_t mnRows, mnCols, mnBands, mnDatalength, mnPerPixSize; // 栅格数据的维度信息
	char msFilename[2048];  // 栅格文件名
	double mdInvalidValue;  // 无效数据值

public:
	GDALRasterRead(void);       // 构造函数
	~GDALRasterRead(void);      // 析构函数

	bool loadFromGDAL(const char* filename, const QString& layerName);  // 从文件加载栅格数据
	std::map<QString, RasterData*>& getRasterData() { return mvRasterData; } // 获取栅格数据
	GDALDataset* getGDALDataset() const { return mpoDataset; }  // 获取 GDAL 数据集指针
	void closeRaster();  // 关闭栅格数据集
	bool isRasterValid();  // 检查数据集是否有效

	GDALDataset* getPoDataset();
	size_t getRows();
	size_t getCols();
	size_t getBandnum();
	size_t getDatalength();
	double getInvalidValue();
	unsigned char* getImgData();
	GDALDataType getDatatype();
	size_t getPerPixelSize();

protected:
	template<class TT> bool readData(); // 模板函数，读取不同类型的栅格数据
};

#endif
