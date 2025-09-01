#ifndef CONVEXHULLWINDOW_H
#define CONVEXHULLWINDOW_H

#include <QDialog>
#include "ui_ConvexHullWindow.h"

class MYGIS;//前向声明

class ConvexHullWindow : public QDialog
{
	Q_OBJECT

public:
	ConvexHullWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~ConvexHullWindow();

public slots:
	//开始分析
	void startConvexHullCalculate();
	//路径选择
	void selectOutputFilePath();
private:
	Ui::ConvexHullWindowClass* ui;
	MYGIS* mpMYGIS;//主窗口指针
};

#endif