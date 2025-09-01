#ifndef TEXTSAVEVECTOR_H
#define TEXTSAVEVECTOR_H


#include <QDialog>
#include "ui_TextSaveVector.h"

class MYGIS;//ǰ������

class TextSaveVector : public QDialog
{
	Q_OBJECT

public:
	TextSaveVector(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~TextSaveVector();

public slots:
	void selectSavePath();//ѡ��·��
	void saveLayerToCSV();//���溯��

private:
	Ui::TextSaveVectorClass* ui;
	MYGIS* mpMYGIS;//������ָ��
};

#endif