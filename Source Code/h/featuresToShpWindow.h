#ifndef FEATURESTOSHPWINDOW_H
#define FEATURESTOSHPWINDOW_H


#include <QDialog>
#include "ui_featuresToShpWindow.h"

class MYGIS;//ǰ������

class featuresToShpWindow : public QDialog
{
	Q_OBJECT

public:
	featuresToShpWindow(QWidget *parent = nullptr,MYGIS * myGIS=nullptr);
	~featuresToShpWindow();
		
public slots:
	void selectSavePath();//ѡ��·��
	void saveLayerToShapefile();//���溯��

private:
	Ui::featuresToShpWindowClass* ui;
	MYGIS* mpMYGIS;//������ָ��
};

#endif