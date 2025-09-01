#ifndef RASTERLAYERDATA_H
#define RASTERLAYERDATA_H

#include <QString>
#include <vector>

class RasterLayerData {
public:
	RasterLayerData() = default;
	RasterLayerData(const QString& layerName, int rows, int cols, int bands, std::vector<unsigned char> imgData, double geoTransform[6]);
	~RasterLayerData();

	// ��ȡդ�������
	int getRows() const;

	// ��ȡդ�������
	int getCols() const;

	// ��ȡդ��Ĳ�����
	int getBands() const;

	// ��ȡդ��ͼ�����ݵ�ָ��
	const unsigned char* getImgData() const;

	// ��ȡդ��ĵ���任��������
	const double* getGeoTransform() const;

	// ��ȡͼ������
	QString getLayerName() const;

private:
	QString mLayerName;  // ͼ������
	int mRows;           // դ�������
	int mCols;           // դ�������
	int mBands;          // դ��Ĳ�����
	std::vector<unsigned char> mImgData;  // դ��ͼ������
	double mGeoTransform[6];  // ����任��������
};

#endif // RASTERLAYERDATA_H
