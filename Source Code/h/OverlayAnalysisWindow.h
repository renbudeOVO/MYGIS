#ifndef OVERLAYANALYSISWINDOW_H
#define OVERLAYANALYSISWINDOW_H


#include <QDialog>
#include "ui_OverlayAnalysisWindow.h"

class MYGIS;//前向声明


class OverlayAnalysisWindow : public QDialog
{
	Q_OBJECT

public:
	OverlayAnalysisWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~OverlayAnalysisWindow();

public slots:
	void startOverlayAnalysisCalculate();//开始分析
	void selectOutputFilePath();//选择文件路径

private:
	Ui::OverlayAnalysisWindowClass* ui;
	MYGIS* mpMYGIS;//主窗口指针
};

#endif