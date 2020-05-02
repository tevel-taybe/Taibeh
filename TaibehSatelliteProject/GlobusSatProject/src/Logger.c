#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>

#include "GlobalStandards.h"

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <hcc/api_fat.h>

#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "TLM_management.h"
#include "SubSystemModules/maintenance/Maintenance.h"
#include "Logger.h"

int wlog(int err, const char *discription, const char *componenet_name)
{
	return 0;
}
