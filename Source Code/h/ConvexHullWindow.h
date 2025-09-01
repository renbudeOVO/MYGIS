#ifndef CONVEXHULLWINDOW_H
#define CONVEXHULLWINDOW_H

#include <QDialog>
#include "ui_ConvexHullWindow.h"

class MYGIS;//ǰ������

class ConvexHullWindow : public QDialog
{
	Q_OBJECT

public:
	ConvexHullWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~ConvexHullWindow();

public slots:
	//��ʼ����
	void startConvexHullCalculate();
	//·��ѡ��
	void selectOutputFilePath();
private:
	Ui::ConvexHullWindowClass* ui;
	MYGIS* mpMYGIS;//������ָ��
};

#endif