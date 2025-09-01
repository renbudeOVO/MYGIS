#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QDebug>

class MyGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	MyGraphicsView(QWidget* parent = nullptr) : QGraphicsView(parent) {}

signals:
	void mouseMoved(QPointF scenePos);

protected:
	void mouseMoveEvent(QMouseEvent* event) override
	{
		QPointF scenePos = mapToScene(event->pos());
		//qDebug() << "Mouse moved at scene position:" << scenePos;
		emit mouseMoved(scenePos); // 发射 mouseMoved 信号
		QGraphicsView::mouseMoveEvent(event); // 确保基础处理被调用
	}

};

#endif