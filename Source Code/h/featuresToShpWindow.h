#ifndef FEATURESTOSHPWINDOW_H
#define FEATURESTOSHPWINDOW_H


#include <QDialog>
#include "ui_featuresToShpWindow.h"

class MYGIS;//前向声明

class featuresToShpWindow : public QDialog
{
	Q_OBJECT

public:
	featuresToShpWindow(QWidget *parent = nullptr,MYGIS * myGIS=nullptr);
	~featuresToShpWindow();
		
public slots:
	void selectSavePath();//选择路径
	void saveLayerToShapefile();//保存函数

private:
	Ui::featuresToShpWindowClass* ui;
	MYGIS* mpMYGIS;//主窗口指针
};

#endif