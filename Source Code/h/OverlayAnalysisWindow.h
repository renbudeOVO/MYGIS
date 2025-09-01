#ifndef OVERLAYANALYSISWINDOW_H
#define OVERLAYANALYSISWINDOW_H


#include <QDialog>
#include "ui_OverlayAnalysisWindow.h"

class MYGIS;//ǰ������


class OverlayAnalysisWindow : public QDialog
{
	Q_OBJECT

public:
	OverlayAnalysisWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~OverlayAnalysisWindow();

public slots:
	void startOverlayAnalysisCalculate();//��ʼ����
	void selectOutputFilePath();//ѡ���ļ�·��

private:
	Ui::OverlayAnalysisWindowClass* ui;
	MYGIS* mpMYGIS;//������ָ��
};

#endif