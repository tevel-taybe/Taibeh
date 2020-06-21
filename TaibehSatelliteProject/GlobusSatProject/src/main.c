#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/WatchDogTimer.h>
#include <hal/boolean.h>
#include <hal/Utility/util.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>

#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <stdlib.h>



#include <hcc/api_fat.h>

#include "GlobalStandards.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SubsystemCommands/TRXVU_Commands.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "TLM_management.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include <satellite-subsystems/isis_eps_driver.h>
#include "InitSystem.h"
#include "main.h"
#include "Logger.h"

#ifdef TESTING //configrutions  at src
	#include "TestingDemos/MainTest.h"
#else
void taskMain()
{
	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);
	printf("entering to the init system \n");

	InitSubsystems();
	printf("   ++++++++++++++++++++++++++++++++++++++++++++++++after the init system \n ");


	while(TRUE)
	{
		printf("looping..................................................................................................................\n");
		wlog(CNAME_GENERAL, LOG_INFO, 0, "Looping.....\n");
		int err = EPS_Conditioning();
		printf( "8888888888888888888888888888888888888888888   returned EPS result id %d\n",err );
		if (0 != err)
		{
			printf("7777777777777777777777777777777777777       eps error returned code, %d",err);
			wlog(CNAME_EPS,LOG_INFO,err,"Error in EPS conditioning procedure.\n");
			wlog(CNAME_GENERAL,LOG_ERROR,err,"Couldn't read the Unix time in Maintenance procedure\n");
		}
		else
		//TODO: printf("///////////////////////        EPS conditioning  .......   OK ...... /////////////////////////// \n");
		printf("no erro in eps99999999999999999999999999999999999999999999999999999999999999999");
		err = TRX_Logic();
		printf(" ************* %%%%%%%%%%  @@@@@@@@@     ###### returned TRX result id %d\n",err);
		if (0!=err)
		{
			wlog(CNAME_TRXVU,LOG_ERROR,err,"Error in TRX Logic procedure.\n");

		}
		printf(" TRX LOgic OK /////////////////////////////////////*****************/////////////////////////////////////////////////////////\n");
		TelemetryCollectorLogic();

		Maintenance();


		// Payload_Logic();
	}
}
#endif //! TESTING

// main operation function. will be called upon software boot.
int main()
{
	//gracefulReset();//TODO: to be deleted
	xTaskHandle taskMainHandle;
	TRACE_CONFIGURE_ISP(DBGU_STANDARD, 2000000, BOARD_MCK);

	// Enable the Instruction cache of the ARM9 core. Keep the MMU and Data Cache disabled.
	CP15_Enable_I_Cache();

	// The actual watchdog is already started, this only initializes the watchdog-kick interface.
	WDT_start();

	// create the main operation task of the satellite
#ifdef TESTING
	xTaskGenericCreate(taskTesting, (const signed char*) "taskTesting", 4096,NULL,
			configMAX_PRIORITIES - 2, &taskMainHandle, NULL, NULL);
#else
	xTaskGenericCreate(taskMain, (const signed char*) "taskMain", 4096, NULL,
			configMAX_PRIORITIES - 2, &taskMainHandle, NULL, NULL);
#endif
	vTaskStartScheduler();
	exit(0);
}
