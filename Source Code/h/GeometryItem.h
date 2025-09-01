#ifndef GEOMETRYITEM_H
#define GEOMETRYITEM_H

#include <QGraphicsItem>
#include <QColor>
#include <QPen>
#include <QBrush>
#include "VectorLayerData.h"  // ���� VectorLayerData ����ص����ݽṹ����

class MYGIS; // ǰ������

// GeometryItem �����ڱ�ʾ����Ⱦʸ��ͼ���еļ���Ҫ��
class GeometryItem : public QGraphicsItem
{
public:
	// ���캯������ʼ������Ҫ�صĸ�������
	GeometryItem(int id, const GeometryData& geomData, QColor layerColor, int zIndex, const QString& layerName, MYGIS* pMyGIS, QGraphicsItem* parent = nullptr);
	~GeometryItem();

	// ���ؼ���Ҫ�صı߽���Σ�����ȷ����������
	QRectF boundingRect() const override;
	// ���ƺ�����ʹ�� QPainter ���Ƽ���Ҫ��
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	// ��ȡͼ������
	QString getLayerName() const { return mstrLayerName; }

	// ��ȡ��������
	GeometryType getGeometryType() const {
		return mGeomData.getGeometryType();
	}

	// ��ȡ��������
	GeometryData getGeometryData() const {
		return mGeomData;
	}

	// ������ɫ�������»���
	void setColor(const QColor& color) {
		mColor = color;
		mBrush.setColor(color);
		update(); // ���»���
	}

	// ��ȡԭʼ��ɫ
	QColor getOriginalColor() const {
		return mOriginalColor;
	}

	// ��ȡ��ǰ��ɫ
	QColor getColor() const {
		return mColor;
	}

	// ��ȡ����Ҫ�ص� ID
	int getId() const {
		return mnId;
	}

	// ��ȡ Feature ID
	int getFeatureId() const {
		return mnId;
	}

	// ��ȡҪ�����ݣ����� ID �ͼ�������
	FeatureData getFeatureData() const {
		return FeatureData(mnId, mGeomData, AttributeData()); // ���������� FeatureData ����
	}

	// �жϵ�ǰҪ���Ƿ񱻸�������ɫ��ʾ������
	bool isHighlighted() const {
		return mColor == Qt::yellow;
	}

	// �ָ���ԭʼ��ɫ
	void restoreOriginalColor() {
		setColor(mOriginalColor);
	}

private:
	MYGIS* mpMyGIS; // ָ�� MYGIS ʵ����ָ��
	int mnId; // ���ڱ�ʶ GeometryItem �� ID ������
	GeometryData mGeomData; // �洢��������
	QColor mColor; // ��ǰͼ�����ɫ
	QPen mPen; // ���ڻ��Ʊ߽�Ļ���
	QBrush mBrush; // ��������ˢ��
	QString mstrLayerName; // �洢ͼ������

	QColor mOriginalColor; // ����ԭʼ��ɫ

	// ����ѡ��״̬���߼�
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	// ����������ӳ�䵽��ͼ����ϵ����ת Y ������Ӧ��ͼ
	QPointF mapToViewCoordinates(double x, double y) const;
};

// RasterItem �����ڱ�ʾ�͹���դ��ͼ��
class RasterItem : public QGraphicsPixmapItem {
public:
	// ���캯������ʼ��դ�������ͼ������
	RasterItem(const QPixmap& pixmap, const QString& layerName, QGraphicsItem* parent = nullptr)
		: QGraphicsPixmapItem(pixmap, parent), mstrLayerName(layerName)
	{
	}

	// ��ȡͼ������
	QString getLayerName() const {
		return mstrLayerName;
	}

private:
	QString mstrLayerName; // �洢դ��ͼ�������
};

#endif
