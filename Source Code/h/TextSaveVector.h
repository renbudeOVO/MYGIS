#ifndef TEXTSAVEVECTOR_H
#define TEXTSAVEVECTOR_H


#include <QDialog>
#include "ui_TextSaveVector.h"

class MYGIS;//前向声明

class TextSaveVector : public QDialog
{
	Q_OBJECT

public:
	TextSaveVector(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~TextSaveVector();

public slots:
	void selectSavePath();//选择路径
	void saveLayerToCSV();//保存函数

private:
	Ui::TextSaveVectorClass* ui;
	MYGIS* mpMYGIS;//主窗口指针
};

#endif