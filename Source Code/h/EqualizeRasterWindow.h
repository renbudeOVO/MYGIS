#ifndef EQUALIZERASTERWINDOW_H
#define EQUALIZERASTERWINDOW_H


#include <QDialog>
#include "ui_EqualizeRasterWindow.h"

class MYGIS;//前向声明

class EqualizeRasterWindow : public QDialog
{
	Q_OBJECT

public:
	EqualizeRasterWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~EqualizeRasterWindow();

public slots:
	void startEqualizeRaster();// 开始绘制均衡化图形

private:
	Ui::EqualizeRasterWindowClass* ui;
	MYGIS* mpMYGIS;//主窗口指针
};

#endif