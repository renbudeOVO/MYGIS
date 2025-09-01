#ifndef VECTORLAYERDATA_H
#define VECTORLAYERDATA_H

#include <vector>
#include <QHash>
#include <QString>
#include <variant>
#include <QStringList>
#include <ogrsf_frmts.h>  // 用于 OGRGeometry 和其他 GDAL 几何类
#include <gdal_priv.h>    // 用于 GDAL 一般操作

// 枚举几何类型，用于表示不同的几何数据类型
enum class GeometryType {
	Point,           // 点类型
	Line,            // 线类型
	Polygon,         // 多边形类型
	MultiPoint,      // 多点类型
	MultiLineString, // 多线类型
	MultiPolygon,    // 多多边形类型
	GeometryCollection // 几何集合类型
};

// 几何数据类型，可以是点/线/多边形的坐标，或者它们的集合
using GeometryCoordinates = std::variant<
	std::vector<std::vector<double>>,              // 点、线、多边形的坐标
	std::vector<std::vector<std::vector<double>>>  // 多多边形、多线的坐标
>;

// 1. 存储几何数据的类
class GeometryData {
public:
	GeometryData(); // 默认构造函数，初始化为 Point 类型

	// 构造函数，传入几何类型和几何数据
	GeometryData(GeometryType type, const GeometryCoordinates& coordinates);

	// 获取几何类型
	GeometryType getGeometryType() const;

	// 获取几何数据（坐标）
	const GeometryCoordinates& getCoordinates() const;

private:
	GeometryType mGeometryType;      // 存储几何类型
	GeometryCoordinates mCoordinates; // 存储几何数据
};

// 2. 存储属性数据的类
class AttributeData {
public:
	// 添加属性，使用键值对存储
	void addAttribute(const QString& key, const std::variant<int, double, QString>& value);

	// 获取属性值，根据键查找对应的属性值
	std::variant<int, double, QString> getAttribute(const QString& key) const;

	// 获取所有属性的键
	QStringList getAttributeKeys() const;

private:
	QHash<QString, std::variant<int, double, QString>> mAttributes;  // 属性名与值的映射
};

// 3. 将几何数据和属性数据组合的类（表示一个要素）
class FeatureData {
public:
	// 构造函数，初始化几何数据和属性数据
	FeatureData(int id, const GeometryData& geometry, const AttributeData& attributes);

	// 获取id
	int getId() const;

	// 获取几何数据
	const GeometryData& getGeometry() const;

	// 获取属性数据
	const AttributeData& getAttributes() const;

	AttributeData& getAttributesChangge();

	// 获取属性键列表
	QStringList getAttributeKeys() const;

private:
	int mId; // 用于标识的ID
	GeometryData mGeometry;     // 存储几何数据
	AttributeData mAttributes;  // 存储属性数据
};

// 4. 存储整个矢量图层数据的类
class VectorLayerData {
public:
	// 添加一个要素到图层
	void addFeature(const FeatureData& feature);

	// 获取图层中的所有要素（只读）
	const std::vector<FeatureData>& getFeatures() const;

	// 获取图层中的所有要素（可修改）
	std::vector<FeatureData>& getFeatures();

	// 获取某一属性的所有值
	std::vector<std::variant<int, double, QString>> getAttributeValues(const QString& attributeName) const;

	void setBaseZValue(int zValue);

	int getBaseZValue() const;

private:
	std::vector<FeatureData> mvFeatures;  // 存储图层中的所有要素
	int mnBaseZValue;  // 每个图层的基础 Z 值
};

#endif // VECTORLAYERDATA_H
