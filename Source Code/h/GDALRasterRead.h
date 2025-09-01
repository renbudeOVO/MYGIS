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

// GDALRasterRead �����ڶ�ȡ�͹��� GDAL դ�����ݼ���
class GDALRasterRead
{
private:
	std::map<QString, RasterData*> mvRasterData; // ��ͼ������Ϊ�����洢դ������

	GDALDataset* mpoDataset;// GDAL ���ݼ�ָ��
	unsigned char* mpData;// դ������ָ��
	GDALDataType mgDataType;// դ����������
	size_t mnRows, mnCols, mnBands, mnDatalength, mnPerPixSize; // դ�����ݵ�ά����Ϣ
	char msFilename[2048];  // դ���ļ���
	double mdInvalidValue;  // ��Ч����ֵ

public:
	GDALRasterRead(void);       // ���캯��
	~GDALRasterRead(void);      // ��������

	bool loadFromGDAL(const char* filename, const QString& layerName);  // ���ļ�����դ������
	std::map<QString, RasterData*>& getRasterData() { return mvRasterData; } // ��ȡդ������
	GDALDataset* getGDALDataset() const { return mpoDataset; }  // ��ȡ GDAL ���ݼ�ָ��
	void closeRaster();  // �ر�դ�����ݼ�
	bool isRasterValid();  // ������ݼ��Ƿ���Ч

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
	template<class TT> bool readData(); // ģ�庯������ȡ��ͬ���͵�դ������
};

#endif
