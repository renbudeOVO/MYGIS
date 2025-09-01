#include "GeometryItem.h"
#include "VectorLayerData.h"
#include "MYGIS.h"
#include "MyGraphicsView.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

// GeometryItem ���캯��
GeometryItem::GeometryItem(int id, const GeometryData& geomData, QColor layerColor, int zIndex, const QString& layerName, MYGIS* pMyGIS, QGraphicsItem* parent)
	: QGraphicsItem(parent), mnId(id), mGeomData(geomData), mColor(layerColor), mOriginalColor(layerColor), mstrLayerName(layerName), mpMyGIS(pMyGIS)
{
	// ����Z��ֵ�Թ���ͼ��˳��
	setZValue(zIndex);

	// ��ʼ�����ʺ�ˢ��
	mPen = QPen(mColor, 0.0001);  // ʹ�÷ǳ�ϸ���߿����ͼ���ص�
	mBrush = QBrush(mColor);  // ʹ��ͼ���ͳһ��ɫ�������
}

GeometryItem::~GeometryItem() {}

// ������ı߽���Σ�����ȷ����������
QRectF GeometryItem::boundingRect() const
{
	// ��鼸�����ݵ����Ͳ�����߽����
	if (std::holds_alternative<std::vector<std::vector<double>>>(mGeomData.getCoordinates()))
	{
		const auto& coords = std::get<std::vector<std::vector<double>>>(mGeomData.getCoordinates());
		if (coords.empty())
			return QRectF();

		double minX = std::numeric_limits<double>::max(), maxX = std::numeric_limits<double>::lowest();
		double minY = std::numeric_limits<double>::max(), maxY = std::numeric_limits<double>::lowest();

		// �������е����ҵ��߽����
		for (const auto& point : coords)
		{
			minX = std::min(minX, point[0]);
			maxX = std::max(maxX, point[0]);
			minY = std::min(minY, point[1]);
			maxY = std::max(maxY, point[1]);
		}

		// ת��Ϊ��ͼ����
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
		// ����ÿ������Σ��������ϵı߽����
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

// ���Ƽ�����
void GeometryItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option)
		Q_UNUSED(widget)

		// ��鼸�������Ƿ�Ϊ��
		if (mGeomData.getCoordinates().index() == std::variant_npos) {
			qWarning() << "Geometry data is empty, nothing to paint.";
			return;
		}

	double scaleFactor = painter->transform().m11();

	// ���ݼ������ͻ�����Ӧ����״
	switch (mGeomData.getGeometryType())
	{
	case GeometryType::Point:
	case GeometryType::MultiPoint:
	{
		// ���Ƶ����
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
		// �����߻����
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
		// ���ƶ���λ������
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
				path.closeSubpath();  // �ر�·�����γ������Ķ����
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

// mousePressEvent �д�����ɫ�л�
void GeometryItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (mpMyGIS->isEditingMode()) {
		if (mColor != Qt::yellow) {
			setColor(Qt::yellow);
			mpMyGIS->getSelectedItems().append(this);
		}
		else {
			// ��ȡ��ǰͼ�����ɫ������ԭʼ��ɫ
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

// ��תy����Ӧ��ͼ
QPointF GeometryItem::mapToViewCoordinates(double x, double y) const
{
	// ��ȡ�����ı߽����
	QRectF sceneRect = scene()->sceneRect();
	// ����������ת��Ϊ��ͼ���꣬Y ����Ҫ���з�ת
	double mappedX = x;
	double mappedY = sceneRect.bottom() - (y - sceneRect.top());
	return QPointF(mappedX, mappedY);
}
