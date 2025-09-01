#ifndef GRAYSCALEHISTOGRAMWINDOW_H
#define GRAYSCALEHISTOGRAMWINDOW_H
#include <QDialog>
#include "ui_GrayscaleHistogramWindow.h"

class MYGIS;//前向声明

class GrayscaleHistogramWindow : public QDialog
{
	Q_OBJECT

public:
	GrayscaleHistogramWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~GrayscaleHistogramWindow();
	void plotHistogram(const std::vector<int>& histogram, const QString& bandName, const QString& outputFilePath);
public slots:
	void startGrayscaleHistogram();// 开始绘制灰度直方图
	void selectOutputFilePath();// 用于选择输出文件路径

private:
	Ui::GrayscaleHistogramWindowClass* ui;
	MYGIS* mpMYGIS;//主窗口指针
};
#endif // GRAYSCALEHISTOGRAMWINDOW_H