#include "GeometryItem.h"
#include "VectorLayerData.h"
#include "MYGIS.h"
#include "MyGraphicsView.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

// GeometryItem 构造函数
GeometryItem::GeometryItem(int id, const GeometryData& geomData, QColor layerColor, int zIndex, const QString& layerName, MYGIS* pMyGIS, QGraphicsItem* parent)
	: QGraphicsItem(parent), mnId(id), mGeomData(geomData), mColor(layerColor), mOriginalColor(layerColor), mstrLayerName(layerName), mpMyGIS(pMyGIS)
{
	// 设置Z轴值以管理图层顺序
	setZValue(zIndex);

	// 初始化画笔和刷子
	mPen = QPen(mColor, 0.0001);  // 使用非常细的线宽避免图层重叠
	mBrush = QBrush(mColor);  // 使用图层的统一颜色进行填充
}

GeometryItem::~GeometryItem() {}

// 计算项的边界矩形，用于确定绘制区域
QRectF GeometryItem::boundingRect() const
{
	// 检查几何数据的类型并计算边界矩形
	if (std::holds_alternative<std::vector<std::vector<double>>>(mGeomData.getCoordinates()))
	{
		const auto& coords = std::get<std::vector<std::vector<double>>>(mGeomData.getCoordinates());
		if (coords.empty())
			return QRectF();

		double minX = std::numeric_limits<double>::max(), maxX = std::numeric_limits<double>::lowest();
		double minY = std::numeric_limits<double>::max(), maxY = std::numeric_limits<double>::lowest();

		// 遍历所有点以找到边界矩形
		for (const auto& point : coords)
		{
			minX = std::min(minX, point[0]);
			maxX = std::max(maxX, point[0]);
			minY = std::min(minY, point[1]);
			maxY = std::max(maxY, point[1]);
		}

		// 转换为视图坐标
		QPointF topLeft = mapToViewCoordinates(minX, maxY);
		QPointF bottomRight = mapToViewCoordinates(maxX, minY);

		return QRectF(topLeft, bottomRight).normalized();
	}
	else if (std::holds_alternative<std::vector<std::vector<std::vector<double>>>>(mGeomData.getCoordinates()))
	{
		const auto& polygons = std::get<std::vector<std::vector<std::vector<double>>>>(mGeomData.getCoordinates());
		if (polygons.empty())
			return QRectF();

		QRectF boundingRect;
		// 遍历每个多边形，计算联合的边界矩形
		for (const auto& polygon : polygons)
		{
			double minX = std::numeric_limits<double>::max(), maxX = std::numeric_limits<double>::lowest();
			double minY = std::numeric_limits<double>::max(), maxY = std::numeric_limits<double>::lowest();

			for (const auto& point : polygon)
			{
				minX = std::min(minX, point[0]);
				maxX = std::max(maxX, point[0]);
				minY = std::min(minY, point[1]);
				maxY = std::max(maxY, point[1]);
			}

			QPointF topLeft = mapToViewCoordinates(minX, maxY);
			QPointF bottomRight = mapToViewCoordinates(maxX, minY);

			boundingRect = boundingRect.united(QRectF(topLeft, bottomRight).normalized());
		}
		return boundingRect;
	}

	return QRectF();
}

// 绘制几何项
void GeometryItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option)
		Q_UNUSED(widget)

		// 检查几何数据是否为空
		if (mGeomData.getCoordinates().index() == std::variant_npos) {
			qWarning() << "Geometry data is empty, nothing to paint.";
			return;
		}

	double scaleFactor = painter->transform().m11();

	// 根据几何类型绘制相应的形状
	switch (mGeomData.getGeometryType())
	{
	case GeometryType::Point:
	case GeometryType::MultiPoint:
	{
		// 绘制点或多点
		const auto& points = std::get<std::vector<std::vector<double>>>(mGeomData.getCoordinates());
		for (const auto& point : points)
		{
			QPointF viewPoint = mapToViewCoordinates(point[0], point[1]);
			painter->setPen(QPen(mColor, 1.0 / scaleFactor));
			painter->setBrush(QBrush(mColor));
			painter->drawEllipse(viewPoint, 2.0 / scaleFactor, 2.0 / scaleFactor);
		}
		break;
	}
	case GeometryType::Line:
	case GeometryType::MultiLineString:
	{
		// 绘制线或多线
		QPainterPath path;
		if (std::holds_alternative<std::vector<std::vector<double>>>(mGeomData.getCoordinates()))
		{
			const auto& line = std::get<std::vector<std::vector<double>>>(mGeomData.getCoordinates());
			for (const auto& point : line)
			{
				QPointF viewPoint = mapToViewCoordinates(point[0], point[1]);
				if (&point == &line.front())
					path.moveTo(viewPoint);
				else
					path.lineTo(viewPoint);
			}
		}
		else if (std::holds_alternative<std::vector<std::vector<std::vector<double>>>>(mGeomData.getCoordinates()))
		{
			const auto& lines = std::get<std::vector<std::vector<std::vector<double>>>>(mGeomData.getCoordinates());
			for (const auto& line : lines)
			{
				for (const auto& point : line)
				{
					QPointF viewPoint = mapToViewCoordinates(point[0], point[1]);
					if (&point == &line.front())
						path.moveTo(viewPoint);
					else
						path.lineTo(viewPoint);
				}
			}
		}
		painter->setPen(QPen(mColor, 1.0 / scaleFactor));
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(path);
		break;
	}
	case GeometryType::Polygon:
	case GeometryType::MultiPolygon:
	{
		// 绘制多边形或多多边形
		QPainterPath path;
		if (std::holds_alternative<std::vector<std::vector<double>>>(mGeomData.getCoordinates()))
		{
			const auto& polygon = std::get<std::vector<std::vector<double>>>(mGeomData.getCoordinates());
			for (const auto& point : polygon)
			{
				QPointF viewPoint = mapToViewCoordinates(point[0], point[1]);
				if (&point == &polygon.front())
					path.moveTo(viewPoint);
				else
					path.lineTo(viewPoint);
			}
		}
		else if (std::holds_alternative<std::vector<std::vector<std::vector<double>>>>(mGeomData.getCoordinates()))
		{
			const auto& polygons = std::get<std::vector<std::vector<std::vector<double>>>>(mGeomData.getCoordinates());
			for (const auto& polygon : polygons)
			{
				for (const auto& point : polygon)
				{
					QPointF viewPoint = mapToViewCoordinates(point[0], point[1]);
					if (&point == &polygon.front())
						path.moveTo(viewPoint);
					else
						path.lineTo(viewPoint);
				}
				path.closeSubpath();  // 关闭路径，形成完整的多边形
			}
		}
		painter->setPen(QPen(Qt::black, 1.0 / scaleFactor));
		painter->setBrush(QBrush(mColor));
		painter->drawPath(path);
		break;
	}
	default:
		qWarning() << "Unknown Geometry Type!";
		break;
	}
}

// mousePressEvent 中处理颜色切换
void GeometryItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (mpMyGIS->isEditingMode()) {
		if (mColor != Qt::yellow) {
			setColor(Qt::yellow);
			mpMyGIS->getSelectedItems().append(this);
		}
		else {
			// 获取当前图层的颜色而不是原始颜色
			QColor currentLayerColor = mpMyGIS->getLayerColor(getLayerName());
			setColor(currentLayerColor);
			mpMyGIS->getSelectedItems().removeOne(this);
		}
		event->accept();
	}
	else {
		QGraphicsItem::mousePressEvent(event);
	}
}

// 翻转y轴适应视图
QPointF GeometryItem::mapToViewCoordinates(double x, double y) const
{
	// 获取场景的边界矩形
	QRectF sceneRect = scene()->sceneRect();
	// 将地理坐标转换为视图坐标，Y 轴需要进行翻转
	double mappedX = x;
	double mappedY = sceneRect.bottom() - (y - sceneRect.top());
	return QPointF(mappedX, mappedY);
}
