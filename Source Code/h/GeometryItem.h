#ifndef GEOMETRYITEM_H
#define GEOMETRYITEM_H

#include <QGraphicsItem>
#include <QColor>
#include <QPen>
#include <QBrush>
#include "VectorLayerData.h"  // 包含 VectorLayerData 和相关的数据结构定义

class MYGIS; // 前向声明

// GeometryItem 类用于表示和渲染矢量图层中的几何要素
class GeometryItem : public QGraphicsItem
{
public:
	// 构造函数，初始化几何要素的各项属性
	GeometryItem(int id, const GeometryData& geomData, QColor layerColor, int zIndex, const QString& layerName, MYGIS* pMyGIS, QGraphicsItem* parent = nullptr);
	~GeometryItem();

	// 返回几何要素的边界矩形，用于确定绘制区域
	QRectF boundingRect() const override;
	// 绘制函数，使用 QPainter 绘制几何要素
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	// 获取图层名称
	QString getLayerName() const { return mstrLayerName; }

	// 获取几何类型
	GeometryType getGeometryType() const {
		return mGeomData.getGeometryType();
	}

	// 获取几何数据
	GeometryData getGeometryData() const {
		return mGeomData;
	}

	// 设置颜色，并更新绘制
	void setColor(const QColor& color) {
		mColor = color;
		mBrush.setColor(color);
		update(); // 更新绘制
	}

	// 获取原始颜色
	QColor getOriginalColor() const {
		return mOriginalColor;
	}

	// 获取当前颜色
	QColor getColor() const {
		return mColor;
	}

	// 获取几何要素的 ID
	int getId() const {
		return mnId;
	}

	// 获取 Feature ID
	int getFeatureId() const {
		return mnId;
	}

	// 获取要素数据，包含 ID 和几何数据
	FeatureData getFeatureData() const {
		return FeatureData(mnId, mGeomData, AttributeData()); // 创建并返回 FeatureData 对象
	}

	// 判断当前要素是否被高亮（黄色表示高亮）
	bool isHighlighted() const {
		return mColor == Qt::yellow;
	}

	// 恢复到原始颜色
	void restoreOriginalColor() {
		setColor(mOriginalColor);
	}

private:
	MYGIS* mpMyGIS; // 指向 MYGIS 实例的指针
	int mnId; // 用于标识 GeometryItem 的 ID 或索引
	GeometryData mGeomData; // 存储几何数据
	QColor mColor; // 当前图层的颜色
	QPen mPen; // 用于绘制边界的画笔
	QBrush mBrush; // 用于填充的刷子
	QString mstrLayerName; // 存储图层名称

	QColor mOriginalColor; // 保存原始颜色

	// 处理选中状态的逻辑
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	// 将几何坐标映射到视图坐标系，翻转 Y 轴以适应视图
	QPointF mapToViewCoordinates(double x, double y) const;
};

// RasterItem 类用于表示和管理栅格图层
class RasterItem : public QGraphicsPixmapItem {
public:
	// 构造函数，初始化栅格项并设置图层名称
	RasterItem(const QPixmap& pixmap, const QString& layerName, QGraphicsItem* parent = nullptr)
		: QGraphicsPixmapItem(pixmap, parent), mstrLayerName(layerName)
	{
	}

	// 获取图层名称
	QString getLayerName() const {
		return mstrLayerName;
	}

private:
	QString mstrLayerName; // 存储栅格图层的名称
};

#endif
