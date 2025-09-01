#ifndef LOGGERCONFIG_H
#define LOGGERCONFIG_H

#include <log4cpp/Category.hh>

void configureLogger();
extern log4cpp::Category& logger;

#endif // LOGGERCONFIG_H