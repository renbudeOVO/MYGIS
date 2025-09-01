#include "MYGIS.h"
#include "LoggerConfig.h"


#include <QtWidgets/QApplication>


#include <gdal_priv.h>
#include <ogrsf_frmts.h>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// 配置日志记录器
	configureLogger();

	// 注册GDAL
	GDALAllRegister();

	CPLSetConfigOption("PROJ_LIB", "./x64/Release");

	MYGIS w;

	w.show();
	int result = a.exec();

	// 程序退出时清理GDAL

	GDALDestroyDriverManager();

	// 关闭日志记录器

	log4cpp::Category::shutdown();

	return result;

}
