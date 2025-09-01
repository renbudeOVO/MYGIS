#ifndef GRAYSCALEHISTOGRAMWINDOW_H
#define GRAYSCALEHISTOGRAMWINDOW_H
#include <QDialog>
#include "ui_GrayscaleHistogramWindow.h"

class MYGIS;//ǰ������

class GrayscaleHistogramWindow : public QDialog
{
	Q_OBJECT

public:
	GrayscaleHistogramWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~GrayscaleHistogramWindow();
	void plotHistogram(const std::vector<int>& histogram, const QString& bandName, const QString& outputFilePath);
public slots:
	void startGrayscaleHistogram();// ��ʼ���ƻҶ�ֱ��ͼ
	void selectOutputFilePath();// ����ѡ������ļ�·��

private:
	Ui::GrayscaleHistogramWindowClass* ui;
	MYGIS* mpMYGIS;//������ָ��
};
#endif // GRAYSCALEHISTOGRAMWINDOW_H