#include "MainTest.h"
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


#include "SubSystemModules/HouseKepping/TelemetryCollector.h"

#include "InitSystem.h"

#include "Logger.h"

Boolean selectAndExecuteTest()
{
	wlog(CNAME_EPS,LOG_INFO,4,"testing the log\n");
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

#define RESTART_INDEX		0
#define TLM_TEST_INDEX		1
#define EPS_TEST_INDEX		2
#define TRXVU_TEST_INDEX	3
#define CMD_TEST_INDEX		4
#define MNGMNT_TEST_INDEX	5
#define FS_TEST_INDEX 		6


	unsigned int number_of_tests = 7;

	printf("\n\r Select the device to be tested to perform: \n\r");
	printf("\t 0) Restart\n\r");
	printf("\t 1) Telemetry Testing\n\r");
	printf("\t 2) EPS Testing\n\r");
	printf("\t 3) TRXVU Testing\n\r");
	printf("\t 4) Commands Handeling Testing\n\r");
	printf("\t 5) Managment Testing\n\r");
	printf("\t 6) File System Testing\n\r");
	printf("\t 7) Hardware word size\n\r");



	while (UTIL_DbguGetIntegerMinMax(&selection, 0, number_of_tests) == 0);

	switch (selection) {
	case RESTART_INDEX:
		restart();
		vTaskDelay(10000);
		printf("what?? \n\n\nwhere am I???");
		break;

	case TLM_TEST_INDEX:
		offerMoreTests = MainTelemetryTestBench();
		break;

	case EPS_TEST_INDEX:
		offerMoreTests = MainEpsTestBench();
		break;

	case TRXVU_TEST_INDEX:
		offerMoreTests = MainTrxvuTestBench();
		break;

	case CMD_TEST_INDEX:
		offerMoreTests = MainCommandsTestBench();
		break;

	case MNGMNT_TEST_INDEX:
		offerMoreTests = MainMaintenanceTestBench();
		break;

	case FS_TEST_INDEX:
		offerMoreTests = MainFileSystemTestBench();
		break;

	case 7:
	{
#define HWI(x) printf(#x " : %d \n", (int)sizeof(x))
		HWI(int);
		HWI(long);
		HWI(short);
		HWI(float);
		int num = 1;
		printf("is little endian? %d\n", (int)*(char*)&num );
#undef HWI
	}
		break;
	default:
		printf("Undefined Test\n\r");
		offerMoreTests = TRUE;
		break;
	}

	return offerMoreTests;
}


void taskTesting()
{


	InitSubsystems();
	TelemetryCollectorLogic();
		//printf("Error collecting telemetry <<<<<<!\n");

	while (1) {
		selectAndExecuteTest();
		vTaskDelay(100);
	}
}
