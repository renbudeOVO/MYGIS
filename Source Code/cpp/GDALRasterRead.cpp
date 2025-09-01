// դ��Ķ�ȡ�ο���ҦҢ��ʦ����Ⱥ��Ĺ���դ���ȡ�ķ���
// ������һ������ɾ�����罫rasterIO�ĳ�bip��ʽ�Ķ�ȡ�����������˾�γ�Ȼ�ȡ��ֱ��ͼ��ȡ�ȣ�
#include "GDALRasterRead.h"
#include <iostream>
#include <cstring>
#include <algorithm>

// ���캯������ʼ����Ա������ȷ����ʼ״̬��û�д��κ����ݼ�
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

// �����������ͷŷ�����ڴ沢�رմ򿪵�դ�����ݼ�
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

// �ر�դ�����ݼ��������ڴ�
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

// ���դ�����ݼ��Ƿ���Ч
bool GDALRasterRead::isRasterValid()
{
	return (mpoDataset != nullptr && mpData != nullptr);
}

// ��ָ���ļ�����դ�����ݲ���ʼ�������Ϣ
bool GDALRasterRead::loadFromGDAL(const char* filename, const QString& layerName)
{
	closeRaster();

	// ��� GDAL �����Ƿ���ע��
	if (GDALGetDriverCount() == 0)
	{
		GDALAllRegister();
		OGRRegisterAll();
		CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	}

	// ��ָ���ļ���դ�����ݼ�
	mpoDataset = (GDALDataset*)GDALOpenShared(filename, GA_ReadOnly);
	if (mpoDataset == nullptr)
	{
		std::cout << "GDALRasterRead::loadFromGDAL : read file error!" << std::endl;
		return false;
	}

	strcpy(msFilename, filename);

	// ��ȡդ�����ݵ�����������������������������
	mnRows = mpoDataset->GetRasterYSize();
	mnCols = mpoDataset->GetRasterXSize();
	mnBands = mpoDataset->GetRasterCount();
	mgDataType = mpoDataset->GetRasterBand(1)->GetRasterDataType();
	mdInvalidValue = mpoDataset->GetRasterBand(1)->GetNoDataValue();

	bool bRlt = false;
	// �����������͵��ö�Ӧ�Ķ�ȡ����
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

	// �����ȡʧ�ܣ��ر�դ�����ݼ������ش���
	if (!bRlt)
	{
		std::cout << "GDALRasterRead::loadFromGDAL : read data error!" << std::endl;
		closeRaster();
		return false;
	}

	// Ϊͼ�����ݷ����ڴ沢���ƶ�ȡ������
	unsigned char* pImgData = new unsigned char[(size_t)mnDatalength];
	if (pImgData == nullptr)
	{
		std::cout << "Memory allocation failed" << std::endl;
		return false;
	}
	std::memcpy(pImgData, mpData, (size_t)mnDatalength);

	// ��ȡ����任����
	double geoTransform[6];
	mpoDataset->GetGeoTransform(geoTransform);

	// �洢դ�����ݵ� mvRasterData
	mvRasterData[layerName] = new RasterData(pImgData, mnRows, mnCols, mnBands, mnDatalength, mnPerPixSize, geoTransform);

	// ��ȡÿ�����ε�ֱ��ͼ����
	for (int bandIndex = 1; bandIndex <= mnBands; ++bandIndex)
	{
		GDALRasterBand* band = mpoDataset->GetRasterBand(bandIndex);
		int nBuckets = 256; // ֱ��ͼͰ��������ͨ��Ϊ256��
		std::vector<GUIntBig> histogram(nBuckets); // ʹ�� GUIntBig ����

		double minMaxValues[2];
		band->ComputeRasterMinMax(TRUE, minMaxValues);
		double minValue = minMaxValues[0];
		double maxValue = minMaxValues[1];

		band->GetHistogram(minValue, maxValue, nBuckets, histogram.data(), TRUE, FALSE, nullptr, nullptr);

		// �� GUIntBig ת��Ϊ int ���Ͳ��洢ֱ��ͼ����
		std::vector<int> histogramInt(histogram.begin(), histogram.end());
		mvRasterData[layerName]->setHistogram(bandIndex, histogramInt);
	}

	return true;
}

// ģ�庯���������������Ͷ�ȡդ������
template<class TT> bool GDALRasterRead::readData()
{
	if (mpoDataset == nullptr)
		return false;

	// ����դ�����ݵ����ֽ����������ڴ�
	mnDatalength = mnRows * mnCols * mnBands * sizeof(TT);
	mpData = new unsigned char[(size_t)mnDatalength];

	// ʹ�� GDAL �� RasterIO ������ȡդ������
	CPLErr _err = mpoDataset->RasterIO(GF_Read, 0, 0, mnCols, mnRows, mpData, mnCols, mnRows, mgDataType, mnBands, nullptr,
		sizeof(unsigned char) * mnBands, sizeof(unsigned char) * mnBands * mnCols, sizeof(unsigned char));

	// ����Ƿ��ȡ�ɹ�
	if (_err != CE_None)
	{
		std::cout << "GDALRasterRead::readData : raster io error!" << std::endl;
		return false;
	}

	return true;
}

// ��ȡ GDAL ���ݼ�ָ��
GDALDataset* GDALRasterRead::getPoDataset() { return mpoDataset; }
// ��ȡդ�����ݵ�����
size_t GDALRasterRead::getRows() { return mnRows; }
// ��ȡդ�����ݵ�����
size_t GDALRasterRead::getCols() { return mnCols; }
// ��ȡդ�����ݵĲ�������
size_t GDALRasterRead::getBandnum() { return mnBands; }
// ��ȡդ��ͼ������ָ��
unsigned char* GDALRasterRead::getImgData() { return mpData; }
// ��ȡդ����������
GDALDataType GDALRasterRead::getDatatype() { return mgDataType; }
// ��ȡդ���������ֽ���
size_t GDALRasterRead::getDatalength() { return mnDatalength; }
// ��ȡ��Чֵ
double GDALRasterRead::getInvalidValue() { return mdInvalidValue; }
// ��ȡÿ�����ֽ���
size_t GDALRasterRead::getPerPixelSize() { return mnPerPixSize; }
