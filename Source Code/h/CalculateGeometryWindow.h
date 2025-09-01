#ifndef CALCULATEGEOMETRYWINDOW_H
#define CALCULATEGEOMETRYWINDOW_H


#include <QDialog>
#include "ui_CalculateGeometryWindow.h"

class MYGIS;//前向声明

class CalculateGeometryWindow : public QDialog
{
	Q_OBJECT

public:
	CalculateGeometryWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~CalculateGeometryWindow();

public slots:
	//开始分析的槽函数
	void startcalculateGeometry();

private:
	Ui::CalculateGeometryWindowClass* ui;
	MYGIS* mpMYGIS;//主窗口指针
};

#endif