#ifndef MAPCONTROLS_H
#define MAPCONTROLS_H

#include <QObject>
#include <QGraphicsView>
#include <QPointF>

// MapControls �����ڹ����ͼ��ͼ�еĽ������ƣ��������ź�ƽ��
class MapControls : public QObject
{
	Q_OBJECT

public:
	explicit MapControls(QGraphicsView* view, QObject* parent = nullptr);

protected:
	bool eventFilter(QObject* obj, QEvent* event) override;

private:
	QGraphicsView* mpGraphicsView;  // ָ�� QGraphicsView �����ָ�룬���ڿ��Ƶ�ͼ��ͼ
	bool mbPanning;                 // ��־�Ƿ���ƽ��ģʽ
	QPoint mLastMousePos;           // �洢��һ������λ�ã�����ƽ�Ƽ���

	void handleWheelEvent(QWheelEvent* event);    // �����������¼����������ţ�
	void handleMousePressEvent(QMouseEvent* event); // ������갴���¼������ڿ�ʼƽ�ƣ�
	void handleMouseMoveEvent(QMouseEvent* event);  // ��������ƶ��¼�������ƽ�ƣ�
	void handleMouseReleaseEvent();// ��������ͷ��¼������ڽ���ƽ�ƣ�
};

#endif