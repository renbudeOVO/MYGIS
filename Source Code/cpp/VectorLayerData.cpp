#include "VectorLayerData.h"

// GeometryData ���ʵ��

// Ĭ�Ϲ��캯������ʼ��Ϊ Point ����
GeometryData::GeometryData() : mGeometryType(GeometryType::Point) {
}

// ���캯�������뼸�����ͺͼ�������
GeometryData::GeometryData(GeometryType type, const GeometryCoordinates& coordinates)
	: mGeometryType(type), mCoordinates(coordinates) {
}

// ��ȡ��������
GeometryType GeometryData::getGeometryType() const {
	return mGeometryType;
}

// ��ȡ�������ݣ����꣩
const GeometryCoordinates& GeometryData::getCoordinates() const {
	return mCoordinates;
}

// AttributeData ���ʵ��

// ������ԣ�ʹ�ü�ֵ�Դ洢
void AttributeData::addAttribute(const QString& key, const std::variant<int, double, QString>& value) {
	mAttributes[key] = value;
}

// ��ȡ����ֵ�����ݼ����Ҷ�Ӧ������ֵ
std::variant<int, double, QString> AttributeData::getAttribute(const QString& key) const {
	auto it = mAttributes.find(key);
	if (it != mAttributes.end()) {
		return it.value();
	}
	return {};
}

// ��ȡ�������Եļ�
QStringList AttributeData::getAttributeKeys() const {
	return mAttributes.keys();
}

// FeatureData ���ʵ��

// ���캯������ʼ���������ݺ���������
FeatureData::FeatureData(int id, const GeometryData& geometry, const AttributeData& attributes)
	: mId(id), mGeometry(geometry), mAttributes(attributes) {
}

// ��ȡҪ�ص� ID
int FeatureData::getId() const {
	return mId;
}

// ��ȡ��������
const GeometryData& FeatureData::getGeometry() const {
	return mGeometry;
}

// ��ȡ��������
const AttributeData& FeatureData::getAttributes() const {
	return mAttributes;
}

// ��ȡ���޸ĵ���������
AttributeData& FeatureData::getAttributesChangge() {
	return mAttributes;
}

// ��ȡ���Լ��б�
QStringList FeatureData::getAttributeKeys() const {
	return mAttributes.getAttributeKeys();
}

// VectorLayerData ���ʵ��

// ���һ��Ҫ�ص�ͼ��
void VectorLayerData::addFeature(const FeatureData& feature) {
	mvFeatures.push_back(feature);
}

// ��ȡͼ���е�����Ҫ�أ�ֻ����
const std::vector<FeatureData>& VectorLayerData::getFeatures() const {
	return mvFeatures;
}

// ��ȡͼ���е�����Ҫ�أ����޸ģ�
std::vector<FeatureData>& VectorLayerData::getFeatures() {
	return mvFeatures;
}

// ��ȡĳһ���Ե�����ֵ
std::vector<std::variant<int, double, QString>> VectorLayerData::getAttributeValues(const QString& attributeName) const {
	std::vector<std::variant<int, double, QString>> values;
	for (const auto& feature : mvFeatures) {
		auto value = feature.getAttributes().getAttribute(attributeName);
		if (value.index() != std::variant_npos) {
			values.push_back(value);
		}
	}
	return values;
}

// ����ͼ��Ļ��� Z ֵ
void VectorLayerData::setBaseZValue(int zValue) {
	mnBaseZValue = zValue;
}

// ��ȡͼ��Ļ��� Z ֵ
int VectorLayerData::getBaseZValue() const {
	return mnBaseZValue;
}
