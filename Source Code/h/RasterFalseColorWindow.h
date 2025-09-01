#ifndef RASTERFALSECOLORWINDOW_H
#define RASTERFALSECOLORWINDOW_H


#include <QDialog>
#include "ui_RasterFalseColorWindow.h"


class RasterData;
class MYGIS;//ǰ������

class RasterFalseColorWindow : public QDialog
{
	Q_OBJECT

public:
	RasterFalseColorWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~RasterFalseColorWindow();

	QImage createImageFromBands(RasterData* rasterData, int bandR, int bandG, int bandB);
public slots:
	void startAnalysis();
		
private:
	Ui::RasterFalseColorWindowClass* ui;
	MYGIS* mpMYGIS;//������ָ��
};

#endif