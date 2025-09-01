#include "LoggerConfig.h"

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>

#include <QDateTime>

// �����ⲿ��־��¼��
log4cpp::Category& logger = log4cpp::Category::getRoot();

void configureLogger()
{
	// �����ļ�Appender
	log4cpp::FileAppender* fileAppender = new log4cpp::FileAppender("fileAppender", "MYGIS_Log.log");

	// ʹ��PatternLayout�Զ����ʽ
	log4cpp::PatternLayout* patternLayout = new log4cpp::PatternLayout();
	patternLayout->setConversionPattern("%d [%p] %c %m%n"); // ʱ���, ��־����, ����, ��Ϣ

	fileAppender->setLayout(patternLayout);

	// ��������־��
	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.setAdditivity(false);
	root.setAppender(fileAppender);
	root.setPriority(log4cpp::Priority::DEBUG);

	// д���������
	root.info("=== Program Started at %s ===", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString().c_str());
}
