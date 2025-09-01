#ifndef EQUALIZERASTERWINDOW_H
#define EQUALIZERASTERWINDOW_H


#include <QDialog>
#include "ui_EqualizeRasterWindow.h"

class MYGIS;//ǰ������

class EqualizeRasterWindow : public QDialog
{
	Q_OBJECT

public:
	EqualizeRasterWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~EqualizeRasterWindow();

public slots:
	void startEqualizeRaster();// ��ʼ���ƾ��⻯ͼ��

private:
	Ui::EqualizeRasterWindowClass* ui;
	MYGIS* mpMYGIS;//������ָ��
};

#endif