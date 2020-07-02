#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>
#include <at91/utility/exithandler.h>
#include <string.h>
#include "GlobalStandards.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include <satellite-subsystems/IsisAntS.h>
#include "InitSystem.h"
#include "TLM_management.h"
#include "Logger.h"
#include <hal/Storage/FRAM.h>
#ifdef GOMEPS
#include <satellite-subsystems/GomEPS.h>
#endif
#ifdef ISISEPS
#include <satellite-subsystems/isis_eps_driver.h>
#endif

#define I2c_SPEED_Hz 100000
#define I2c_Timeout 10

Boolean isFirstActivation()
{
	Boolean flag = FALSE;
	FRAM_read((unsigned char*) &flag, FIRST_ACTIVATION_FLAG_ADDR,FIRST_ACTIVATION_FLAG_SIZE);
	return flag;
}

int firstActivationProcedure()
{
//#ifdef ISISEPS
//	ieps_statcmd_t eps_cmd;
//#endif
	Boolean FirstActivation = isFirstActivation();
	int err = 0;
	time_unix seconds_since_deploy = 0;
	err = FRAM_read((unsigned char*) seconds_since_deploy,SECONDS_SINCE_DEPLOY_ADDR,SECONDS_SINCE_DEPLOY_SIZE);
	if (0 != err)
	{
		seconds_since_deploy = MINUTES_TO_SECONDS(30);	// deploy immediately. No mercy
	}

	while (seconds_since_deploy < MINUTES_TO_SECONDS(30))
	{
		vTaskDelay(SECONDS_TO_TICKS(10));
		FRAM_write((unsigned char*)&seconds_since_deploy, SECONDS_SINCE_DEPLOY_ADDR,SECONDS_SINCE_DEPLOY_SIZE);
		if (0 != err)
		{
			break;
		}
		TelemetryCollectorLogic();

		seconds_since_deploy += 10;
		// Resets the watchdog time keeping the system from performing a reset. TODO need to check if more need to be done
		isis_eps__watchdog__from_t eps_response;
		isis_eps__watchdog__tm(EPS_I2C_BUS_INDEX, &eps_response);
		//TODO: add more to this...
//#ifdef ISISEPS
	//	IsisEPS_resetWDT(EPS_I2C_BUS_INDEX, &eps_cmd);
//#endif
#ifdef GOMEPS
		GomEpsResetWDT(EPS_I2C_BUS_INDEX);

#endif
	}

//#ifndef TESTING // need to be removed and once we got the 30 mins to deploy Ants
	//IsisAntS_autoDeployment(0, isisants_sideA, 10);
	//IsisAntS_autoDeployment(0, isisants_sideB, 10);
//#endif
	//updating deploy time and Activation flag
	time_unix deploy_time = 0;
	Time_getUnixEpoch(&deploy_time);
	FRAM_write((unsigned char*) deploy_time, DEPLOYMENT_TIME_ADDR,DEPLOYMENT_TIME_SIZE);

	// updating first activation flag false in FRAM
	   FirstActivation = FALSE;
	   if ((FRAM_write((unsigned char*) &FirstActivation,FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE))!=0)
		{
			//TODO: return Error and write to the log file
		}
		return 0;
}

void WriteDefaultValuesToFRAM()
{
	//TODO: write to FRAM all default values (like threshold voltages...)

	time_unix default_no_comm_thresh = DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_write((unsigned char*) &default_no_comm_thresh,NO_COMM_WDT_KICK_TIME_ADDR,NO_COMM_WDT_KICK_TIME_SIZE);

	EpsThreshVolt_t def_thresh_volt = { .raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
	FRAM_write((unsigned char*)def_thresh_volt.raw, EPS_THRESH_VOLTAGES_ADDR,EPS_THRESH_VOLTAGES_SIZE);

	float def_alpha = DEFAULT_ALPHA_VALUE;
	FRAM_write((unsigned char*) &def_alpha, EPS_ALPHA_FILTER_VALUE_ADDR,EPS_ALPHA_FILTER_VALUE_SIZE);

	time_unix tlm_save_period = 0;
	tlm_save_period = DEFAULT_EPS_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, EPS_SAVE_TLM_PERIOD_ADDR,sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_TRXVU_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, TRXVU_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_ANT_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, ANT_SAVE_TLM_PERIOD_ADDR,sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_SOLAR_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, SOLAR_SAVE_TLM_PERIOD_ADDR,sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_WOD_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, WOD_SAVE_TLM_PERIOD_ADDR,sizeof(tlm_save_period));

	time_unix beacon_interval = 0;
	beacon_interval = DEFAULT_BEACON_INTERVAL_TIME;
	FRAM_write((unsigned char*) &beacon_interval, BEACON_INTERVAL_TIME_ADDR,BEACON_INTERVAL_TIME_SIZE);

}

int StartFRAM()

{
	int error =0;
			error = FRAM_start();
	return error;
}




int StartI2C()
{

	int error = I2C_start(I2c_SPEED_Hz, I2c_Timeout);
	return error;
}

int StartSPI()

{
	int error = SPI_start(bus1_spi, slave1_spi);
	return error;
}

int StartTIME()

{
	int error = 0;
	Time expected_deploy_time = UNIX_DEPLOY_DATE_JAN_D1_Y2020;
	error = Time_start(&expected_deploy_time, 0);
	if (0 != error) {
		return error;
	}
	time_unix time_before_wakeup = 0;
	Boolean isFirst_Activation=0;// TODO:need to be deleted
	if (!isFirst_Activation) { //TODO : original command if (!isFirst_Activation())
		FRAM_read((unsigned char*) &time_before_wakeup,MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);

		Time_setUnixEpoch(time_before_wakeup);

	}
	return 0;
}
Boolean8bit tlms_created[NUMBER_OF_TELEMETRIES];
int DeploySystem()
{
	//TelemetryCreateFiles(tlms_created);
	Boolean first_activation=1;

	//Boolean first_activation = isFirstActivation();


	if (first_activation!=0)// Activation was done !
	{
		TelemetryCreateFiles(tlms_created);//TODO: need to removed
		first_activation = 0 ;//TODO: need to removed
		return 0;
	}
	/*else
	{
		 firstActivationProcedure();
		time_unix deploy_time = 0;
		Time_getUnixEpoch(&deploy_time);
		FRAM_write((unsigned char*) deploy_time, DEPLOYMENT_TIME_ADDR,DEPLOYMENT_TIME_SIZE);

		first_activation = FALSE; //TODO: set 'first_activation' to TRUE before launch
		FRAM_write((unsigned char*) &first_activation,FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);

		WriteDefaultValuesToFRAM();
	} */
	return 0;
}

#define PRINT_IF_ERR(method) if(0 != err)printf("error in '" #method  "' err = %d\n",err);
int InitSubsystems()
{
	//TODO: check for return value errors
	int err = 0;

	err = StartI2C();
	wlog(CNAME_GENERAL,LOG_ERROR,err,"Error in calling start I2C function\n");

	err = StartSPI();
	wlog(CNAME_GENERAL,LOG_ERROR,err,"Error in calling start SPI function\n");

	err = StartFRAM();
	wlog(CNAME_GENERAL,LOG_ERROR,err,"Error in calling start FRAM function\n");

	err = StartTIME();
	wlog(CNAME_GENERAL,LOG_ERROR,err,"Error in calling Start time function\n");

    err = InitializeFS(isFirstActivation());
	//err = InitializeFS(1);
	wlog(CNAME_GENERAL,LOG_ERROR,err,"Error in calling initialize FS function\n");

	err = EPS_Init();
	wlog(CNAME_GENERAL,LOG_ERROR,err,"Error in calling EPS Init function\n");

	err = InitTrxvu();
	wlog(CNAME_GENERAL,LOG_ERROR,err,"Error in calling Init Trxvu function\n");


	err = DeploySystem();
	wlog(CNAME_GENERAL,LOG_ERROR,err,"Error in calling Deploying System function\n");

	err = WakeupFromResetCMD();
	wlog(CNAME_GENERAL,LOG_ERROR,err,"Error in calling wake up from reset function\n");
}

