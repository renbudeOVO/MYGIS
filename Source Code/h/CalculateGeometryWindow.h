#ifndef CALCULATEGEOMETRYWINDOW_H
#define CALCULATEGEOMETRYWINDOW_H


#include <QDialog>
#include "ui_CalculateGeometryWindow.h"

class MYGIS;//ǰ������

class CalculateGeometryWindow : public QDialog
{
	Q_OBJECT

public:
	CalculateGeometryWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~CalculateGeometryWindow();

public slots:
	//��ʼ�����Ĳۺ���
	void startcalculateGeometry();

private:
	Ui::CalculateGeometryWindowClass* ui;
	MYGIS* mpMYGIS;//������ָ��
};

#endif