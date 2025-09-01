#pragma once

#include <QDialog>
#include "ui_ProgramStatementWindow.h"

class MYGIS;

//程序说明窗口
class ProgramStatementWindow : public QDialog
{
	Q_OBJECT

public:
	ProgramStatementWindow(QWidget *parent = nullptr, MYGIS* myGIS = nullptr);
	~ProgramStatementWindow();

private:
	Ui::ProgramStatementWindowClass* ui;
	MYGIS* mpMYGIS;//主窗口指针
};
