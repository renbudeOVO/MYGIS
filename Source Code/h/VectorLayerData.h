#ifndef VECTORLAYERDATA_H
#define VECTORLAYERDATA_H

#include <vector>
#include <QHash>
#include <QString>
#include <variant>
#include <QStringList>
#include <ogrsf_frmts.h>  // ���� OGRGeometry ������ GDAL ������
#include <gdal_priv.h>    // ���� GDAL һ�����

// ö�ټ������ͣ����ڱ�ʾ��ͬ�ļ�����������
enum class GeometryType {
	Point,           // ������
	Line,            // ������
	Polygon,         // ���������
	MultiPoint,      // �������
	MultiLineString, // ��������
	MultiPolygon,    // ����������
	GeometryCollection // ���μ�������
};

// �����������ͣ������ǵ�/��/����ε����꣬�������ǵļ���
using GeometryCoordinates = std::variant<
	std::vector<std::vector<double>>,              // �㡢�ߡ�����ε�����
	std::vector<std::vector<std::vector<double>>>  // �����Ρ����ߵ�����
>;

// 1. �洢�������ݵ���
class GeometryData {
public:
	GeometryData(); // Ĭ�Ϲ��캯������ʼ��Ϊ Point ����

	// ���캯�������뼸�����ͺͼ�������
	GeometryData(GeometryType type, const GeometryCoordinates& coordinates);

	// ��ȡ��������
	GeometryType getGeometryType() const;

	// ��ȡ�������ݣ����꣩
	const GeometryCoordinates& getCoordinates() const;

private:
	GeometryType mGeometryType;      // �洢��������
	GeometryCoordinates mCoordinates; // �洢��������
};

// 2. �洢�������ݵ���
class AttributeData {
public:
	// ������ԣ�ʹ�ü�ֵ�Դ洢
	void addAttribute(const QString& key, const std::variant<int, double, QString>& value);

	// ��ȡ����ֵ�����ݼ����Ҷ�Ӧ������ֵ
	std::variant<int, double, QString> getAttribute(const QString& key) const;

	// ��ȡ�������Եļ�
	QStringList getAttributeKeys() const;

private:
	QHash<QString, std::variant<int, double, QString>> mAttributes;  // ��������ֵ��ӳ��
};

// 3. ���������ݺ�����������ϵ��ࣨ��ʾһ��Ҫ�أ�
class FeatureData {
public:
	// ���캯������ʼ���������ݺ���������
	FeatureData(int id, const GeometryData& geometry, const AttributeData& attributes);

	// ��ȡid
	int getId() const;

	// ��ȡ��������
	const GeometryData& getGeometry() const;

	// ��ȡ��������
	const AttributeData& getAttributes() const;

	AttributeData& getAttributesChangge();

	// ��ȡ���Լ��б�
	QStringList getAttributeKeys() const;

private:
	int mId; // ���ڱ�ʶ��ID
	GeometryData mGeometry;     // �洢��������
	AttributeData mAttributes;  // �洢��������
};

// 4. �洢����ʸ��ͼ�����ݵ���
class VectorLayerData {
public:
	// ���һ��Ҫ�ص�ͼ��
	void addFeature(const FeatureData& feature);

	// ��ȡͼ���е�����Ҫ�أ�ֻ����
	const std::vector<FeatureData>& getFeatures() const;

	// ��ȡͼ���е�����Ҫ�أ����޸ģ�
	std::vector<FeatureData>& getFeatures();

	// ��ȡĳһ���Ե�����ֵ
	std::vector<std::variant<int, double, QString>> getAttributeValues(const QString& attributeName) const;

	void setBaseZValue(int zValue);

	int getBaseZValue() const;

private:
	std::vector<FeatureData> mvFeatures;  // �洢ͼ���е�����Ҫ��
	int mnBaseZValue;  // ÿ��ͼ��Ļ��� Z ֵ
};

#endif // VECTORLAYERDATA_H
