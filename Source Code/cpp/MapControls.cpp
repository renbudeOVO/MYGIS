#include "MapControls.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>

// MapControls ���캯������ʼ����ͼ�Ϳ��Ʋ���
MapControls::MapControls(QGraphicsView* view, QObject* parent)
	: QObject(parent), mpGraphicsView(view), mbPanning(false)
{
	// ��װ�¼�����������ͼ���ӿڣ����ڲ����¼�
	mpGraphicsView->viewport()->installEventFilter(this);
	// ���������Ч��
	mpGraphicsView->setRenderHint(QPainter::Antialiasing, true);
	// ������ͼ����קģʽΪ�ֶ��϶�
	mpGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
	// ����������
	mpGraphicsView->setMouseTracking(true);
	// ����ˮƽ�ʹ�ֱ������
	mpGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mpGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

// �¼������������ڴ������͹����¼�
bool MapControls::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::Wheel) {
		QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
		handleWheelEvent(wheelEvent);
		return true;
	}
	else if (event->type() == QEvent::MouseButtonPress) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		handleMousePressEvent(mouseEvent);
		return true;
	}
	else if (event->type() == QEvent::MouseMove) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		handleMouseMoveEvent(mouseEvent);
		return false; // ȷ���¼���������
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		handleMouseReleaseEvent();
		return true;
	}

	return QObject::eventFilter(obj, event);
}

// ��������¼���ʵ�����Ź���
void MapControls::handleWheelEvent(QWheelEvent* event)
{
	const double scaleFactor = 1.2;

	// ��ȡ��ǰ��ͼ�����ĵ�
	QPointF currentCenter = mpGraphicsView->mapToScene(mpGraphicsView->viewport()->rect().center());

	// ��ȡ���λ���������ͼ������
	QPointF mousePosition = mpGraphicsView->mapToScene(event->pos());

	// ���ݹ��ַ����������
	if (event->angleDelta().y() > 0) {
		mpGraphicsView->scale(scaleFactor, scaleFactor);
	}
	else {
		mpGraphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
	}

	// �������ź������ͼ����
	QPointF delta = mousePosition - currentCenter;
	QPointF newCenter = currentCenter + delta * (scaleFactor - 1);

	// �����µ���ͼ����
	mpGraphicsView->centerOn(newCenter);
}

// ������갴���¼�������ƽ��ģʽ
void MapControls::handleMousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		mLastMousePos = event->pos(); // ��¼��갴��ʱ��λ��
		mbPanning = true;             // ����ƽ��ģʽ
	}
}

// ��������ƶ��¼���ʵ����ͼƽ��
void MapControls::handleMouseMoveEvent(QMouseEvent* event)
{
	if (mbPanning && (event->buttons() & Qt::LeftButton)) {
		int dx = event->pos().x() - mLastMousePos.x(); // ��������ƶ���ˮƽ����
		int dy = event->pos().y() - mLastMousePos.y(); // ��������ƶ��Ĵ�ֱ����
		// ����������λ�ã�ʵ����ͼƽ��
		mpGraphicsView->horizontalScrollBar()->setValue(mpGraphicsView->horizontalScrollBar()->value() - dx);
		mpGraphicsView->verticalScrollBar()->setValue(mpGraphicsView->verticalScrollBar()->value() - dy);
		mLastMousePos = event->pos(); // �����������λ��
	}
}

// ��������ͷ��¼�������ƽ��ģʽ
void MapControls::handleMouseReleaseEvent()
{
	mbPanning = false; // �ر�ƽ��ģʽ
}