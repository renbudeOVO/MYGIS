#include "MYGIS.h"
#include "LoggerConfig.h"


#include <QtWidgets/QApplication>


#include <gdal_priv.h>
#include <ogrsf_frmts.h>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// ������־��¼��
	configureLogger();

	// ע��GDAL
	GDALAllRegister();

	CPLSetConfigOption("PROJ_LIB", "./x64/Release");

	MYGIS w;

	w.show();
	int result = a.exec();

	// �����˳�ʱ����GDAL

	GDALDestroyDriverManager();

	// �ر���־��¼��

	log4cpp::Category::shutdown();

	return result;

}
