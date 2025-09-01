#ifndef MAPCONTROLS_H
#define MAPCONTROLS_H

#include <QObject>
#include <QGraphicsView>
#include <QPointF>

// MapControls 类用于管理地图视图中的交互控制，例如缩放和平移
class MapControls : public QObject
{
	Q_OBJECT

public:
	explicit MapControls(QGraphicsView* view, QObject* parent = nullptr);

protected:
	bool eventFilter(QObject* obj, QEvent* event) override;

private:
	QGraphicsView* mpGraphicsView;  // 指向 QGraphicsView 对象的指针，用于控制地图视图
	bool mbPanning;                 // 标志是否处于平移模式
	QPoint mLastMousePos;           // 存储上一次鼠标的位置，用于平移计算

	void handleWheelEvent(QWheelEvent* event);    // 处理鼠标滚轮事件（用于缩放）
	void handleMousePressEvent(QMouseEvent* event); // 处理鼠标按下事件（用于开始平移）
	void handleMouseMoveEvent(QMouseEvent* event);  // 处理鼠标移动事件（用于平移）
	void handleMouseReleaseEvent();// 处理鼠标释放事件（用于结束平移）
};

#endif