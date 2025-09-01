// 栅格的读取参考了姚尧老师发在群里的关于栅格读取的方法
// 进行了一部分增删，比如将rasterIO改成bip格式的读取，比如增加了经纬度获取，直方图获取等！
#include "GDALRasterRead.h"
#include <iostream>
#include <cstring>
#include <algorithm>

// 构造函数：初始化成员变量，确保初始状态下没有打开任何数据集
GDALRasterRead::GDALRasterRead(void)
{
	mpoDataset = nullptr;
	mpData = nullptr;
	mgDataType = GDT_Byte;
	mnRows = mnCols = mnBands = -1;
	mnDatalength = -1;
	strcpy(msFilename, "");
	mdInvalidValue = 0.0f;
	mnPerPixSize = 1;
}

// 析构函数：释放分配的内存并关闭打开的栅格数据集
GDALRasterRead::~GDALRasterRead(void)
{
	for (auto& raster : mvRasterData)
	{
		raster.second->closeRasterData();
		delete raster.second;
	}
	mvRasterData.clear();
	std::cout << "Cleared mvRasterData" << std::endl;
}

// 关闭栅格数据集并清理内存
void GDALRasterRead::closeRaster()
{
	if (mpoDataset != nullptr)
	{
		GDALClose(mpoDataset);
		mpoDataset = nullptr;
	}

	if (mpData != nullptr)
	{
		delete[] mpData;
		mpData = nullptr;
	}

	mgDataType = GDT_Byte;
	mnDatalength = -1;
	mnRows = mnCols = mnBands = -1;
	strcpy(msFilename, "");
	mdInvalidValue = 0.0f;
	mnPerPixSize = 1;
}

// 检查栅格数据集是否有效
bool GDALRasterRead::isRasterValid()
{
	return (mpoDataset != nullptr && mpData != nullptr);
}

// 从指定文件加载栅格数据并初始化相关信息
bool GDALRasterRead::loadFromGDAL(const char* filename, const QString& layerName)
{
	closeRaster();

	// 检查 GDAL 驱动是否已注册
	if (GDALGetDriverCount() == 0)
	{
		GDALAllRegister();
		OGRRegisterAll();
		CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	}

	// 打开指定文件的栅格数据集
	mpoDataset = (GDALDataset*)GDALOpenShared(filename, GA_ReadOnly);
	if (mpoDataset == nullptr)
	{
		std::cout << "GDALRasterRead::loadFromGDAL : read file error!" << std::endl;
		return false;
	}

	strcpy(msFilename, filename);

	// 获取栅格数据的行数、列数、波段数和数据类型
	mnRows = mpoDataset->GetRasterYSize();
	mnCols = mpoDataset->GetRasterXSize();
	mnBands = mpoDataset->GetRasterCount();
	mgDataType = mpoDataset->GetRasterBand(1)->GetRasterDataType();
	mdInvalidValue = mpoDataset->GetRasterBand(1)->GetNoDataValue();

	bool bRlt = false;
	// 根据数据类型调用对应的读取函数
	switch (mgDataType)
	{
	case GDT_Byte:
		mnPerPixSize = sizeof(unsigned char);
		bRlt = readData<unsigned char>();
		break;
	case GDT_UInt16:
		mnPerPixSize = sizeof(unsigned short);
		bRlt = readData<unsigned short>();
		break;
	case GDT_Int16:
		mnPerPixSize = sizeof(short);
		bRlt = readData<short>();
		break;
	case GDT_UInt32:
		mnPerPixSize = sizeof(unsigned int);
		bRlt = readData<unsigned int>();
		break;
	case GDT_Int32:
		mnPerPixSize = sizeof(int);
		bRlt = readData<int>();
		break;
	case GDT_Float32:
		mnPerPixSize = sizeof(float);
		bRlt = readData<float>();
		break;
	case GDT_Float64:
		mnPerPixSize = sizeof(double);
		bRlt = readData<double>();
		break;
	default:
		std::cout << "GDALRasterRead::loadFromGDAL : unknown data type!" << std::endl;
		closeRaster();
		return false;
	}

	// 如果读取失败，关闭栅格数据集并返回错误
	if (!bRlt)
	{
		std::cout << "GDALRasterRead::loadFromGDAL : read data error!" << std::endl;
		closeRaster();
		return false;
	}

	// 为图像数据分配内存并复制读取的数据
	unsigned char* pImgData = new unsigned char[(size_t)mnDatalength];
	if (pImgData == nullptr)
	{
		std::cout << "Memory allocation failed" << std::endl;
		return false;
	}
	std::memcpy(pImgData, mpData, (size_t)mnDatalength);

	// 获取地理变换参数
	double geoTransform[6];
	mpoDataset->GetGeoTransform(geoTransform);

	// 存储栅格数据到 mvRasterData
	mvRasterData[layerName] = new RasterData(pImgData, mnRows, mnCols, mnBands, mnDatalength, mnPerPixSize, geoTransform);

	// 读取每个波段的直方图数据
	for (int bandIndex = 1; bandIndex <= mnBands; ++bandIndex)
	{
		GDALRasterBand* band = mpoDataset->GetRasterBand(bandIndex);
		int nBuckets = 256; // 直方图桶的数量（通常为256）
		std::vector<GUIntBig> histogram(nBuckets); // 使用 GUIntBig 类型

		double minMaxValues[2];
		band->ComputeRasterMinMax(TRUE, minMaxValues);
		double minValue = minMaxValues[0];
		double maxValue = minMaxValues[1];

		band->GetHistogram(minValue, maxValue, nBuckets, histogram.data(), TRUE, FALSE, nullptr, nullptr);

		// 将 GUIntBig 转换为 int 类型并存储直方图数据
		std::vector<int> histogramInt(histogram.begin(), histogram.end());
		mvRasterData[layerName]->setHistogram(bandIndex, histogramInt);
	}

	return true;
}

// 模板函数：根据数据类型读取栅格数据
template<class TT> bool GDALRasterRead::readData()
{
	if (mpoDataset == nullptr)
		return false;

	// 计算栅格数据的总字节数并分配内存
	mnDatalength = mnRows * mnCols * mnBands * sizeof(TT);
	mpData = new unsigned char[(size_t)mnDatalength];

	// 使用 GDAL 的 RasterIO 方法读取栅格数据
	CPLErr _err = mpoDataset->RasterIO(GF_Read, 0, 0, mnCols, mnRows, mpData, mnCols, mnRows, mgDataType, mnBands, nullptr,
		sizeof(unsigned char) * mnBands, sizeof(unsigned char) * mnBands * mnCols, sizeof(unsigned char));

	// 检查是否读取成功
	if (_err != CE_None)
	{
		std::cout << "GDALRasterRead::readData : raster io error!" << std::endl;
		return false;
	}

	return true;
}

// 获取 GDAL 数据集指针
GDALDataset* GDALRasterRead::getPoDataset() { return mpoDataset; }
// 获取栅格数据的行数
size_t GDALRasterRead::getRows() { return mnRows; }
// 获取栅格数据的列数
size_t GDALRasterRead::getCols() { return mnCols; }
// 获取栅格数据的波段数量
size_t GDALRasterRead::getBandnum() { return mnBands; }
// 获取栅格图像数据指针
unsigned char* GDALRasterRead::getImgData() { return mpData; }
// 获取栅格数据类型
GDALDataType GDALRasterRead::getDatatype() { return mgDataType; }
// 获取栅格数据总字节数
size_t GDALRasterRead::getDatalength() { return mnDatalength; }
// 获取无效值
double GDALRasterRead::getInvalidValue() { return mdInvalidValue; }
// 获取每像素字节数
size_t GDALRasterRead::getPerPixelSize() { return mnPerPixSize; }
