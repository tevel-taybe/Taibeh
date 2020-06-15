#ifndef LOGGER_H_
#define LOGGER_H_


#include <string.h>
#include <stdio.h>
#include <hal/errors.h>
#include "TLM_management.h"
#include <SubSystemModules/Housekepping/TelemetryFiles.h>
#include <Logger.h>

//int logError(int error);
//
//int logInfo(char *info);
//
//#define E_CANT_TRANSMIT    		-200
//#define E_TOO_EARLY_4_BEACON    -201
//#define TRXVU_MUTE_TOO_LOMG    -202
//#define TRXVU_IDLE_TOO_LOMG    -203
//#define MAX_LOG_STR				40

#define 		MAX_LOG_CONTENT			64
#define 		MAX_LOG_COMPONENT_NAME 	16



typedef enum LogSeverity {
	LOG_ERROR,
	LOG_INFO
} LogSeverity;

typedef struct LogFileRecord
{
	LogSeverity level;
	time_unix time;
	int error;
	char content[MAX_LOG_CONTENT];
	char comp_name[MAX_LOG_COMPONENT_NAME];

} LogFileRecord;

typedef enum ComponentName {
	CNAME_EPS,
	CNAME_TRXVU,
	CNAME_ANTS,
	CNAME_GENERAL,
	CNAME_MTNCE,
	CNAME_SOLAR
} ComponentName;

/*!
 * @brief writes to log file in case function returns an error
 * @param[in] the error code [int]
 * @param[in] pointer to the information which will be written into the log file [string]
 * @param[in] pointer to the component which returns the error [string]
 * @return	TRUE if success
 * 			FALSE otherwise
 */
int wlog(ComponentName component, LogSeverity severity, int err, const char *discription);

#endif /* LOGGER_H_ */
