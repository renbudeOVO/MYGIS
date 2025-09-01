#include "ProgramStatementWindow.h"
#include "MYGIS.h"

ProgramStatementWindow::ProgramStatementWindow(QWidget *parent, MYGIS* myGIS)
	: QDialog(parent), mpMYGIS(myGIS), ui(new Ui::ProgramStatementWindowClass)
{
	ui->setupUi(this);
}

ProgramStatementWindow::~ProgramStatementWindow()
{
	delete ui;
}
