#include "VectorLayerData.h"

// GeometryData 类的实现

// 默认构造函数，初始化为 Point 类型
GeometryData::GeometryData() : mGeometryType(GeometryType::Point) {
}

// 构造函数，传入几何类型和几何数据
GeometryData::GeometryData(GeometryType type, const GeometryCoordinates& coordinates)
	: mGeometryType(type), mCoordinates(coordinates) {
}

// 获取几何类型
GeometryType GeometryData::getGeometryType() const {
	return mGeometryType;
}

// 获取几何数据（坐标）
const GeometryCoordinates& GeometryData::getCoordinates() const {
	return mCoordinates;
}

// AttributeData 类的实现

// 添加属性，使用键值对存储
void AttributeData::addAttribute(const QString& key, const std::variant<int, double, QString>& value) {
	mAttributes[key] = value;
}

// 获取属性值，根据键查找对应的属性值
std::variant<int, double, QString> AttributeData::getAttribute(const QString& key) const {
	auto it = mAttributes.find(key);
	if (it != mAttributes.end()) {
		return it.value();
	}
	return {};
}

// 获取所有属性的键
QStringList AttributeData::getAttributeKeys() const {
	return mAttributes.keys();
}

// FeatureData 类的实现

// 构造函数，初始化几何数据和属性数据
FeatureData::FeatureData(int id, const GeometryData& geometry, const AttributeData& attributes)
	: mId(id), mGeometry(geometry), mAttributes(attributes) {
}

// 获取要素的 ID
int FeatureData::getId() const {
	return mId;
}

// 获取几何数据
const GeometryData& FeatureData::getGeometry() const {
	return mGeometry;
}

// 获取属性数据
const AttributeData& FeatureData::getAttributes() const {
	return mAttributes;
}

// 获取可修改的属性数据
AttributeData& FeatureData::getAttributesChangge() {
	return mAttributes;
}

// 获取属性键列表
QStringList FeatureData::getAttributeKeys() const {
	return mAttributes.getAttributeKeys();
}

// VectorLayerData 类的实现

// 添加一个要素到图层
void VectorLayerData::addFeature(const FeatureData& feature) {
	mvFeatures.push_back(feature);
}

// 获取图层中的所有要素（只读）
const std::vector<FeatureData>& VectorLayerData::getFeatures() const {
	return mvFeatures;
}

// 获取图层中的所有要素（可修改）
std::vector<FeatureData>& VectorLayerData::getFeatures() {
	return mvFeatures;
}

// 获取某一属性的所有值
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

// 设置图层的基础 Z 值
void VectorLayerData::setBaseZValue(int zValue) {
	mnBaseZValue = zValue;
}

// 获取图层的基础 Z 值
int VectorLayerData::getBaseZValue() const {
	return mnBaseZValue;
}
