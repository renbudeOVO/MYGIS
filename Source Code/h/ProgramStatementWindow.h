#pragma once

#include <QDialog>
#include "ui_ProgramStatementWindow.h"

class MYGIS;

//����˵������
class ProgramStatementWindow : public QDialog
{
	Q_OBJECT

public:
	ProgramStatementWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~ProgramStatementWindow();

private:
	Ui::ProgramStatementWindowClass* ui;
	MYGIS* mpMYGIS;//������ָ��
};
