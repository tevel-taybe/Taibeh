#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>

#include "GlobalStandards.h"

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>
#include <satellite-subsystems/isis_eps_driver.h>

#include <hcc/api_fat.h>

#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SubsystemCommands/TRXVU_Commands.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "TLM_management.h"
#include "Maintenance.h"
#include "Logger.h"

Boolean CheckExecutionTime(time_unix prev_time, time_unix period)
{
	time_unix curr = 0;
	int err = Time_getUnixEpoch(&curr);
	if(0 != err){
		wlog(CNAME_GENERAL,LOG_ERROR,err,"Error Reading Unix-time\n");
		return FALSE;
	}
	printf("curr = %d\n",curr);
	printf("prev_time = %d\n",prev_time);
	printf("period = %d\n",period);
	//period=0;
	if(curr - prev_time >= period)
	{
		printf("the current time is :  %d\n",curr);
		return TRUE;

	}
	return FALSE;

}

Boolean CheckExecTimeFromFRAM(unsigned int fram_time_addr, time_unix period)
{
	int err = 0;
	time_unix prev_exec_time = 0;
	err = FRAM_read((unsigned char*)&prev_exec_time,fram_time_addr,sizeof(prev_exec_time));
	if(0 != err){
		return FALSE;
	}
	return CheckExecutionTime(prev_exec_time,period);
}

void SaveSatTimeInFRAM(unsigned int time_addr,unsigned int time_size)
{
	time_unix current_time = 0;

	Time_getUnixEpoch(&current_time);

	FRAM_write((unsigned char*) &current_time, time_addr, time_size);
}
Boolean IsFS_Corrupted()
{
	FN_SPACE space;
	int drivenum = f_getdrive();

	f_getfreespace(drivenum, &space);

	if (space.bad > 0) {
		return TRUE;
	}
	return FALSE;
}

int WakeupFromResetCMD()
{
	int err = 0;
	unsigned char reset_flag = 0;
	unsigned int num_of_resets = 0;
	FRAM_read(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);

	if (reset_flag) {
		time_unix curr_time = 0;
		Time_getUnixEpoch(&curr_time);
		FRAM_write(&curr_time,MOST_UPDATED_SAT_TIME_ADDR,MOST_UPDATED_SAT_TIME_SIZE);

		err = SendAckPacket(ACK_RESET_WAKEUP, NULL, (unsigned char*) &curr_time,
				sizeof(time_unix));

		reset_flag = FALSE_8BIT;
		err = FRAM_write(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);

	err =	FRAM_read((unsigned char*) &num_of_resets,
		NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
		num_of_resets++;

	err =	FRAM_write((unsigned char*) &num_of_resets,
		NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
		if (0 != err) {
			return err;
		}
		int counter = 0;
		time_unix eps_save_time = DEFAULT_EPS_SAVE_TLM_TIME;
			 err = FRAM_write((unsigned char*) &eps_save_time,EPS_SAVE_TLM_PERIOD_ADDR,sizeof(eps_save_time));
			if(err!= 0)
			{

				wlog(CNAME_MTNCE,LOG_INFO,err,"Failed to return the default value of  EPS_SAVE_TLM_PERIOD!\n");
				counter++;
			}
			time_unix TRXVU_SAVE_TIME = DEFAULT_TRXVU_SAVE_TLM_TIME;
			err = FRAM_write((unsigned char*)&TRXVU_SAVE_TIME,TRXVU_SAVE_TLM_PERIOD_ADDR,sizeof(TRXVU_SAVE_TIME));

				  if(err!=0)
				  {
				  		wlog(CNAME_MTNCE,LOG_ERROR,err,"Failed to return the default value of TRXVU_SAVE_TLM_PERIOD!\n");
				  		counter++;
				  }
				  time_unix Ants_SAVE_PERIOD = DEFAULT_ANT_SAVE_TLM_TIME;
			err = FRAM_write((unsigned char*) &Ants_SAVE_PERIOD,ANT_SAVE_TLM_PERIOD_ADDR,sizeof(Ants_SAVE_PERIOD));
				  	  if(err!=0)
				  	  {
				  	  		wlog(CNAME_MTNCE,LOG_INFO,err,"Failed to return the default value of ANTS_SAVE_TLM_PERIOD!\n");
				  	  		counter++;
				  	  }
				  time_unix Solar_save_time = DEFAULT_SOLAR_SAVE_TLM_TIME;
			err = FRAM_write((unsigned char*) &Solar_save_time, SOLAR_SAVE_TLM_PERIOD_ADDR,sizeof(Solar_save_time));
					  if(err!=0)
				  		  {
				  		  		wlog(CNAME_MTNCE,LOG_INFO,err,"Failed to return the default value of SOLAR_SAVE_TLM_PERIOD!\n");
				  		  		counter++;
				  		  }
					  time_unix WOD_SAVE_TIME = DEFAULT_WOD_SAVE_TLM_TIME;
		    err = FRAM_write((unsigned char*) &WOD_SAVE_TIME, WOD_SAVE_TLM_PERIOD_ADDR,sizeof(WOD_SAVE_TIME));
					  		  if(err!=0)
					  		  {
					  		  		wlog(CNAME_MTNCE,LOG_INFO,err,"Failed to return the default value of WOD_SAVE_TLM_PERIOD!\n");
					  		  		counter++;
					  		  }
					  		  time_unix wdt_kick_time = DEFAULT_NO_COMM_WDT_KICK_TIME;
		    err = FRAM_write((unsigned char*) &wdt_kick_time,NO_COMM_WDT_KICK_TIME_ADDR,NO_COMM_WDT_KICK_TIME_SIZE);
		    				  if(err!=0)
		    				  {
		    					  wlog(CNAME_MTNCE,LOG_INFO,err,"Failed to return the default value of WDT kick time!\n");
		    					  counter++;
		    				  }
		    				  int beacon_bitrate=DEFALUT_BEACON_BITRATE_CYCLE;
		    err = FRAM_write((unsigned char*) &beacon_bitrate,BEACON_BITRATE_CYCLE_ADDR,BEACON_BITRATE_CYCLE_SIZE);
		    				  if(err!=0)
		    				  {
		    					  wlog(CNAME_MTNCE,LOG_INFO,err,"Failed to return the default value of Beacon Bitrate Cycle!\n");
		    					  counter++;
		    				  }
		    				time_unix BEACON_INTERVAL = DEFAULT_BEACON_INTERVAL_TIME;
		    err = FRAM_write((unsigned char*) &BEACON_INTERVAL,BEACON_INTERVAL_TIME_ADDR,BEACON_INTERVAL_TIME_SIZE);
		    					  if(err!=0)
		    						  {
		    						  		wlog(CNAME_MTNCE,LOG_INFO,err,"Failed to return the default value of Beacon Interval time!\n");
		    						  		counter++;
		    						  }
		    					  if(counter!=0){
		    						  wlog(CNAME_MTNCE,LOG_ERROR,counter,"Error in Writing default values to FRAM\n");
		    								  return -1;
		    					  }
	}

	return 0;
}

void ResetGroundCommWDT()
{
	SaveSatTimeInFRAM(LAST_COMM_TIME_ADDR,LAST_COMM_TIME_SIZE);
}

// check if last communication with the ground station has passed WDT kick time
// and return a boolean describing it.
Boolean IsGroundCommunicationWDTKick()
{
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	time_unix last_comm_time = 0;
	FRAM_read((unsigned char*) &last_comm_time, LAST_COMM_TIME_ADDR,
	LAST_COMM_TIME_SIZE);

	time_unix wdt_kick_thresh = GetGsWdtKickTime();

	//TODO: if current_time - last_comm_time < 0
	if (current_time - last_comm_time >= wdt_kick_thresh) {
		return TRUE;
	}
	return FALSE;
}

//TODO: add to command dictionary
int SetGsWdtKickTime(time_unix new_gs_wdt_kick_time)
{
	int err = FRAM_write((unsigned char*)&new_gs_wdt_kick_time, NO_COMM_WDT_KICK_TIME_ADDR,
		NO_COMM_WDT_KICK_TIME_SIZE);
	return err;
}

time_unix GetGsWdtKickTime()
{
	time_unix no_comm_thresh = 0;
	FRAM_read((unsigned char*)&no_comm_thresh, NO_COMM_WDT_KICK_TIME_ADDR,
	NO_COMM_WDT_KICK_TIME_SIZE);
	return no_comm_thresh;
}

static time_unix g_last_time_deplyCheck = 0;
#define DEPLOY_ANTS_CHECK_PERIOD 1800

void RedeployIfNeeded(time_unix curr_time)
{

	if ( curr_time - g_last_time_deplyCheck > DEPLOY_ANTS_CHECK_PERIOD)
	{
		CMD_Re_Deploy_Ants(NULL);
		g_last_time_deplyCheck = curr_time;
	}

}

void Maintenance()//
{
	isis_eps__watchdog__from_t response;//keeping EPS watchDog up
	(void)isis_eps__watchdog__tm( EPS_I2C_BUS_INDEX, &response );
	(void)response;

	if (CheckForMuteEnd())
	{
		UnMuteTRXVU();
	}
	time_unix curr_time = 0;
	int  err = Time_getUnixEpoch(&curr_time);
	if (0!=err)
	{
		wlog(CNAME_GENERAL,LOG_ERROR,err,"Couldn't read the Unix time in Maintenance procedure\n");
	}
	RedeployIfNeeded(curr_time);
	if (0!=err)
	{
	#ifdef TESTING
		printf(" there is an error getting the updated time . error= %d \n ",err);
	#endif
	}
	else
	{
		err= FRAM_write((unsigned char*)&curr_time,MOST_UPDATED_SAT_TIME_ADDR,MOST_UPDATED_SAT_TIME_SIZE);
		if (0!=err)
		{
			wlog(CNAME_GENERAL,LOG_INFO,err,"Couldn't write the Unix time to the FRAM in Maintenance procedure\n");
		# ifdef TESTING
		printf("an error occured updating the time in the FRAM , Error = %d \n",err);//TODO: do error log file
		#endif
		}
	}
	// SaveSatTimeInFRAM(MOST_UPDATED_SAT_TIME_ADDR,MOST_UPDATED_SAT_TIME_SIZE);
		///   Redeploys procedure 1- if ant armed ? 2- if Ants deployed

	if (IsFS_Corrupted()) //TODO: need to double check with hartzeleya if still neef to run this function in maintenance data is sent in beacon
	{
		wlog(CNAME_GENERAL,LOG_INFO,41,"Corrupted Sectors in FS\n");
#ifdef TESTING
		wlog(CNAME_GENERAL,LOG_INFO,41,"Corrupted Sectors in FS\n");
		//TODO: print out should be deleted
		printf("FS is corrupted\n");
#endif
	}

	if (IsGroundCommunicationWDTKick()) {
#ifdef TESTING
		printf("GS was kicked. now restarting...\n");
		vTaskDelay(5000);
#endif
		restart();
	}

	if(CheckForMuteEnd()){
		UnMuteTRXVU();
#ifdef TESTING
		printf("unmuted\n");
#endif
	}
	//TODO: if(current_time < FRAM_sat_time) maybe update 'sat_time' to be 'first_wakeup_'
}
