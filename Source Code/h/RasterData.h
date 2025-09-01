#ifndef RASTER_DATA_H
#define RASTER_DATA_H

#include <QString>
#include <vector>
#include <map>

// RasterData �����ڴ洢�͹���դ��ͼ�����ݼ��������Ϣ
class RasterData
{
private:
	unsigned char* mpImgData; // �洢դ��ͼ������
	size_t mnRows;            // դ�����ݵ�����
	size_t mnCols;            // դ�����ݵ�����
	size_t mnBands;           // դ�����ݵĲ�����
	size_t mnLength;          // դ�����ݵ����ֽ���
	size_t mnPerPixSize;      // ÿ�����ֽ���
	double mGeoTransform[6];  // �洢����任����
	std::map<int, std::vector<int>> mvHistogram; // �洢ÿ�����ε�ֱ��ͼ

public:
	// ���캯������ʼ��դ�����ݺ���ز���
	RasterData(unsigned char* pImgData, size_t rows, size_t cols, size_t bands, size_t length, size_t perPixSize, double geoTransform[6]);
	~RasterData(); // �����������ͷ�դ�������ڴ�
	RasterData(const RasterData& other); // �������캯��

	void closeRasterData(); // �ͷ�դ�����ݵ��ڴ�
	unsigned char* getImgData() const; // ��ȡդ��ͼ������ָ��
	size_t getRows() const; // ��ȡդ�����ݵ�����
	size_t getCols() const; // ��ȡդ�����ݵ�����
	size_t getBandnum() const; // ��ȡդ�����ݵĲ�����
	size_t getLength() const; // ��ȡդ�����ݵ����ֽ���
	size_t getPerPixSize() const; // ��ȡÿ���ص��ֽ���
	double getTopLeftLongitude() const; // ��ȡ���ϽǾ���
	double getTopLeftLatitude() const;  // ��ȡ���Ͻ�γ��

	// ���úͻ�ȡֱ��ͼ����
	void setHistogram(int band, const std::vector<int>& histogram);
	const std::vector<int>& getHistogram(int band) const;
};

#endif
