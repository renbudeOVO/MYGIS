#include "MapControls.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>

// MapControls 构造函数，初始化视图和控制参数
MapControls::MapControls(QGraphicsView* view, QObject* parent)
	: QObject(parent), mpGraphicsView(view), mbPanning(false)
{
	// 安装事件过滤器到视图的视口，用于捕获事件
	mpGraphicsView->viewport()->installEventFilter(this);
	// 开启抗锯齿效果
	mpGraphicsView->setRenderHint(QPainter::Antialiasing, true);
	// 设置视图的拖拽模式为手动拖动
	mpGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
	// 启用鼠标跟踪
	mpGraphicsView->setMouseTracking(true);
	// 隐藏水平和垂直滚动条
	mpGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mpGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

// 事件过滤器，用于处理鼠标和滚轮事件
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
		return false; // 确保事件继续传播
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		handleMouseReleaseEvent();
		return true;
	}

	return QObject::eventFilter(obj, event);
}

// 处理滚轮事件，实现缩放功能
void MapControls::handleWheelEvent(QWheelEvent* event)
{
	const double scaleFactor = 1.2;

	// 获取当前视图的中心点
	QPointF currentCenter = mpGraphicsView->mapToScene(mpGraphicsView->viewport()->rect().center());

	// 获取鼠标位置相对于视图的坐标
	QPointF mousePosition = mpGraphicsView->mapToScene(event->pos());

	// 根据滚轮方向进行缩放
	if (event->angleDelta().y() > 0) {
		mpGraphicsView->scale(scaleFactor, scaleFactor);
	}
	else {
		mpGraphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
	}

	// 计算缩放后的新视图中心
	QPointF delta = mousePosition - currentCenter;
	QPointF newCenter = currentCenter + delta * (scaleFactor - 1);

	// 设置新的视图中心
	mpGraphicsView->centerOn(newCenter);
}

// 处理鼠标按下事件，开启平移模式
void MapControls::handleMousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		mLastMousePos = event->pos(); // 记录鼠标按下时的位置
		mbPanning = true;             // 开启平移模式
	}
}

// 处理鼠标移动事件，实现视图平移
void MapControls::handleMouseMoveEvent(QMouseEvent* event)
{
	if (mbPanning && (event->buttons() & Qt::LeftButton)) {
		int dx = event->pos().x() - mLastMousePos.x(); // 计算鼠标移动的水平距离
		int dy = event->pos().y() - mLastMousePos.y(); // 计算鼠标移动的垂直距离
		// 调整滚动条位置，实现视图平移
		mpGraphicsView->horizontalScrollBar()->setValue(mpGraphicsView->horizontalScrollBar()->value() - dx);
		mpGraphicsView->verticalScrollBar()->setValue(mpGraphicsView->verticalScrollBar()->value() - dy);
		mLastMousePos = event->pos(); // 更新最后的鼠标位置
	}
}

// 处理鼠标释放事件，结束平移模式
void MapControls::handleMouseReleaseEvent()
{
	mbPanning = false; // 关闭平移模式
}