#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define LOG_LEVEL_NONE		0
#define LOG_LEVEL_ERROR 	1
#define LOG_LEVEL_WARN		2
#define LOG_LEVEL_INFO		3
#define LOG_LEVEL_VERBOSE	4
#define LOG_LEVEL_TRACE		5

#ifndef LOG_LEVEL
	#ifdef DEBUG
		#define LOG_LEVEL LOG_LEVEL_TRACE
	#else
		#define LOG_LEVEL LOG_LEVEL_NONE
	#endif
#endif


#define LOG(level, fmt, ...) \
	do { if	(LOG_LEVEL >= level) printf("[%s:%d] " fmt "\n", __FILE__, __LINE__,  ##__VA_ARGS__); } while(0) 



#define LOG_ERROR(fmt, ...)		LOG(LOG_LEVEL_ERROR, "[ERROR] " fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) 		LOG(LOG_LEVEL_WARN, "[WARN] " fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)		LOG(LOG_LEVEL_INFO, "[INFO] " fmt, ##__VA_ARGS__)
#define LOG_VERBOSE(fmt, ...)   LOG(LOG_LEVEL_VERBOSE, "[VERBOSE] " fmt, ##__VA_ARGS__)
#define LOG_TRACE(fmt, ...)		LOG(LOG_LEVEL_TRACE, "[TRACE] " fmt, ##__VA_ARGS__)




#endif

