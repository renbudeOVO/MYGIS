#include "LoggerConfig.h"

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>

#include <QDateTime>

// 定义外部日志记录器
log4cpp::Category& logger = log4cpp::Category::getRoot();

void configureLogger()
{
	// 创建文件Appender
	log4cpp::FileAppender* fileAppender = new log4cpp::FileAppender("fileAppender", "MYGIS_Log.log");

	// 使用PatternLayout自定义格式
	log4cpp::PatternLayout* patternLayout = new log4cpp::PatternLayout();
	patternLayout->setConversionPattern("%d [%p] %c %m%n"); // 时间戳, 日志级别, 类名, 消息

	fileAppender->setLayout(patternLayout);

	// 创建根日志器
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.setAdditivity(false);
	root.setAppender(fileAppender);
	root.setPriority(log4cpp::Priority::DEBUG);

	// 写入启动标记
	root.info("=== Program Started at %s ===", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
}
