#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "TrxvuTestingDemo.h"

#include <string.h>
#include <stdlib.h>

#include <hal/Utility/util.h>
#include <hal/Timing/Time.h>

#ifdef ISISEPS
	#include <satellite-subsystems/isis_eps_driver.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

#include "SubSystemModules/PowerManagment/EPS.h"	// for EPS_Conditioning

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/ActUponCommand.h"
#include "SubSystemModules/Communication/CommandDictionary.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/SPL.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/Beacon.h"
#include "SubSystemModules/Communication/SatDataTx.h"

Boolean TestInitTrxvu()
{
	int err = InitTrxvu();
	if(0 != err){
		printf("Error in 'InitTrxvy' = %d\n",err);
		return TRUE;
	}
	printf("TRXVU initialized  successful\n");
	return TRUE;
}

Boolean TestInitTrxvuWithDifferentFrameLengths()
{

	ISIStrxvuI2CAddress myTRXVUAddress;
	ISIStrxvuFrameLengths myTRXVUBuffers;
	ISIStrxvuBitrate myTRXVUBitrates;

	int retValInt = 0;
	unsigned int bytes = 0;
	printf("please enter size of Tx packet in bytes(0 for default):\n");
	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&bytes,0,1000) == 0);
	if(bytes == 0){
		myTRXVUBuffers.maxAX25frameLengthTX = 235;
	}else
		myTRXVUBuffers.maxAX25frameLengthTX = bytes;

	printf("please enter size of Rx packet in bytes(0 for default):\n");
	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&bytes,0,1000) == 0);
	if(bytes == 0){
		myTRXVUBuffers.maxAX25frameLengthRX = 200;
	}else
		myTRXVUBuffers.maxAX25frameLengthRX = bytes;


	myTRXVUAddress.addressVu_rc = I2C_TRXVU_RC_ADDR;
	myTRXVUAddress.addressVu_tc = I2C_TRXVU_TC_ADDR;

	myTRXVUBitrates = trxvu_bitrate_9600;

	retValInt = IsisTrxvu_initialize(&myTRXVUAddress, &myTRXVUBuffers,
			&myTRXVUBitrates, 1);
	if (retValInt != 0) {
		printf("error in Trxvi Init = %d\n",retValInt);
		return retValInt;
	}
	vTaskDelay(100);

	return TRUE;
}

Boolean TestTrxvuLogic()
{
	printf("\nPlease insert number of minutes to test(0 to 10)\n");
	int minutes = 0;
	int err = 0;
	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&minutes,0,10) == 0);

	portTickType curr_time = xTaskGetTickCount();
	long end_time = MINUTES_TO_TICKS(minutes) + curr_time;

	while(end_time - (long)curr_time > 0)
	{
		curr_time = xTaskGetTickCount();

		err = TRX_Logic();
		if(0 != err){
			printf("error in TRX_Logic = %d\n exiting\n",err);
			return TRUE;
		}
		vTaskDelay(500);
	}
	return TRUE;
}

Boolean TestCheckTransmitionAllowed()
{
	char msg[10]  = {0};

	if(CheckTransmitionAllowed())
		strcpy(msg,"TRUE");
	else
		strcpy(msg,"FALSE");
	printf("Transmition is allowed: %s\n",msg);
	return TRUE;
}

Boolean TestSendDumpAbortRequest()
{
	printf("Sending Dump Abort Request\n");
	SendDumpAbortRequest();
	return TRUE;
}

Boolean TestTransmitDummySplPacket()
{
	sat_packet_t packet = {0};
	packet.ID = 0x12345678;
	packet.cmd_type = 0x42;
	packet.cmd_subtype = 0x43;

	char data[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10};
	memcpy(packet.data,data,sizeof(data));

	packet.length = sizeof(data);

	int err = TransmitSplPacket(&packet,NULL);
	if(err != 0 ){
		printf("error in 'TransmitSplPacket' = %d\n",err);
	}
	return TRUE;
}

Boolean TestTransmitSplPacket()
{
	sat_packet_t packet = {0};
	packet.ID = 0x12345678;
	packet.cmd_type = 0x42;
	packet.cmd_subtype = 0x43;

	char data[MAX_COMMAND_DATA_LENGTH] = {0};

	for(unsigned int i = 0; i < MAX_COMMAND_DATA_LENGTH; i++)
	{
		data[i] = i;
	}
	packet.length = MAX_COMMAND_DATA_LENGTH;
	memcpy(packet.data,data,MAX_COMMAND_DATA_LENGTH);
	int minutes = 0;
		while(UTIL_DbguGetIntegerMinMax((unsigned int*)&minutes,0,20) == 0);

		time_unix curr_time = 0;
		Time_getUnixEpoch(&curr_time);

		time_unix end_time = MINUTES_TO_SECONDS(minutes) + curr_time;
		ISIStrxvuTxTelemetry tlm;
		IsisTrxvu_tcGetLastTxTelemetry(ISIS_TRXVU_I2C_BUS_INDEX,&tlm);
		while(end_time > curr_time)
		{
			IsisTrxvu_tcGetLastTxTelemetry(ISIS_TRXVU_I2C_BUS_INDEX,&tlm);
			if(tlm.fields.board_temp >=60)
				break;
			printf("board temperature: %d\n",tlm.fields.board_temp);
			Time_getUnixEpoch(&curr_time);

			TransmitSplPacket(&packet,NULL);

			EPS_Conditioning();

			printf("seconds t'ill end: %u\n",end_time - curr_time);
			vTaskDelay(1000);
		}

	return TRUE;
}

Boolean TestExitDump()
{
	AbortDump();
	return TRUE;
}

Boolean TestDumpTelemetry()
{
	sat_packet_t cmd = {0};
	unsigned int temp = 0;
	printf("Starting Dump. Please Insert Dump Parameter:\n");

	printf("Please Insert Command Type:\n");
	while(UTIL_DbguGetIntegerMinMax(&temp,0,255));
	cmd.cmd_type = temp;

	printf("Please Insert Command Subtype:\n");
	while(UTIL_DbguGetIntegerMinMax(&temp,0,255));
	cmd.cmd_subtype = temp;

	printf("Please Insert Command ID:\n");
	while(UTIL_DbguGetIntegerMinMax(&temp,0,0xFFFFFFFF));
	cmd.ID = temp;

	DumpTelemetry(&cmd);
	return TRUE;
}

Boolean TestRestoreDefaultBeaconParameters()
{
	printf("Restoring to default beacon parameters\n");

	time_unix beacon_interval_time = 0;
	FRAM_read((unsigned char*) &beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);
	printf("Value of interval before: %u\n",beacon_interval_time);

	UpdateBeaconInterval(DEFAULT_BEACON_INTERVAL_TIME);

	FRAM_read((unsigned char*)&beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);
	printf("Value of interval after: %u\n",beacon_interval_time);

	unsigned char cycle = 0;
	FRAM_read((unsigned char*) &cycle, BEACON_BITRATE_CYCLE_ADDR,
	BEACON_BITRATE_CYCLE_SIZE);
	printf("Value of beacon cycle before: %d\n",cycle);

	UpdateBeaconBaudCycle(DEFALUT_BEACON_BITRATE_CYCLE);

	FRAM_read((unsigned char*) &cycle, BEACON_BITRATE_CYCLE_ADDR,
	BEACON_BITRATE_CYCLE_SIZE);
	printf("Value of beacon cycle after: %d\n",cycle);

	return TRUE;
}

Boolean TestChooseDefaultBeaconCycle()
{
	printf("Please state new beacon interval in seconds(0 to cancel; min 5, max 60):\n");

	unsigned int seconds = 0;
	while(UTIL_DbguGetIntegerMinMax(&seconds,0,60) == 0);
	if(0 == seconds){
		return TRUE;
	}
	time_unix beacon_interval_time = 0;
	FRAM_read((unsigned char*) &beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);
	printf("Value before: %u\n",beacon_interval_time);

	beacon_interval_time = seconds;
	FRAM_write((unsigned char*) &beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);

	FRAM_read((unsigned char*) &beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);
	printf("Value after: %u\n",beacon_interval_time);

	return TRUE;
}

Boolean TestBeaconLogic()
{
	printf("Please state number of minutes to perform the test(max 20, 0 to cancel):\n");

	int minutes = 0;
	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&minutes,0,20) == 0);

	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);

	time_unix end_time = MINUTES_TO_SECONDS(minutes) + curr_time;

	while(end_time > curr_time)
	{
		Time_getUnixEpoch(&curr_time);
		printf("Hello there entering beacon test\n");
		BeaconLogic();

		EPS_Conditioning();

		printf("seconds t'ill end: %u\n",end_time - curr_time);
		vTaskDelay(1000);
	}
	return TRUE;
}

Boolean TestMuteTrxvu()
{
	time_unix duration = 3;
	printf("Please state number of minutes to perform the test(max 10):");

	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&duration,0,10) == 0);

	int err = muteTRXVU(duration);
	if(0 != err){
		printf("error in 'muteTRXVU' = %d\n",err);
		return TRUE;
	}

	time_unix curr = 0;
	Time_getUnixEpoch(&curr);

//#ifdef ISISEPS
//	ieps_statcmd_t cmd;
//#endif
	while(!CheckForMuteEnd()){

		printf("current tick = %d\n",(int)xTaskGetTickCount());

//#ifdef ISISEPS
	//	IsisEPS_resetWDT(EPS_I2C_BUS_INDEX,&cmd);
//#endif
#ifdef GOMEPS
		GomEpsResetWDT(EPS_I2C_BUS_INDEX);
#endif
		printf("sending ACK(if transmission was heard then error :/ )\n");
		SendAckPacket(ACK_MUTE,NULL,NULL,0);
		vTaskDelay(1000);
	}
	UnMuteTRXVU();
	Boolean mute_flag = GetMuteFlag();
	SendAckPacket(ACK_MUTE,NULL,(unsigned char*)&mute_flag,sizeof(mute_flag));
	return TRUE;
}

Boolean TestUnMuteTrxvu()
{
	printf("Stopping mute. tick = %d\n",(int)xTaskGetTickCount());
	UnMuteTRXVU();
	return TRUE;
}

Boolean TestGetNumberOfFramesInBuffer()
{
	int num_of_frames = GetNumberOfFramesInBuffer();
	if(-1 == num_of_frames){
		printf("error in 'GetNumberOfFramesInBuffer'\n");
		return TRUE;
	}
	printf("Number of frames in the online buffer = %d\n", num_of_frames);
	return TRUE;
}

Boolean TestSetTrxvuBitrate()
{
	int err = 0;
	ISIStrxvuBitrate bitrate = 0;
	unsigned int index = 0;
	printf("Choose bitrate:\n \t(0)Cancel\n\t(1) = 1200\n\t(2) = 2400\n\t(3) = 4800\n\t(4) = 9600\n");

	while (UTIL_DbguGetIntegerMinMax(&index, 0, 4) == 0);

	switch(index)
	{
	case 0:
		break;
	case 1:
		bitrate = trxvu_bitrate_1200;
		break;
	case 2:
		bitrate = trxvu_bitrate_2400;
			break;
	case 3:
		bitrate = trxvu_bitrate_4800;
			break;
	case 4:
		bitrate = trxvu_bitrate_9600;
			break;
	}

	err = IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,bitrate);
	if(0 != err){
		printf("error in 'IsisTrxvu_tcSetAx25Bitrate' = %d",err);
		return TRUE;
	}
	return TRUE;
}

Boolean TestGetTrxvuBitrate()
{
	int err = 0;
	ISIStrxvuBitrateStatus bitrate = 0;
	err = GetTrxvuBitrate(&bitrate);
	if(0 != err){
		printf("error in 'GetTrxvuBitrate' = %d\n",err);
		return TRUE;
	}
	switch(bitrate)
	{
	case trxvu_bitratestatus_1200:
		printf("bitrate is 'trxvu_bitratestatus_1200'\n");
		break;
	case trxvu_bitratestatus_2400:
		printf("bitrate is 'trxvu_bitratestatus_2400'\n");
			break;
	case trxvu_bitratestatus_4800:
		printf("bitrate is 'trxvu_bitratestatus_4800'\n");
			break;
	case trxvu_bitratestatus_9600:
		printf("bitrate is 'trxvu_bitratestatus_9600'\n");
			break;
	}
	return TRUE;
}

Boolean TestTransmitDataAsSPL_Packet()
{
	sat_packet_t cmd = {0};
	cmd.ID = 0x42;
	cmd.cmd_type = 0x43;
	cmd.cmd_subtype = 0x44;

	unsigned char data[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10};
	printf("Transmitted data\n");
	for(unsigned int i =0; i < sizeof(data);i++){
		printf("%X\t",data[i]);
	}
	printf("\n\n");
	int err = TransmitDataAsSPL_Packet(&cmd,data,sizeof(data));
	if(0 != err){
		printf("error in 'TransmitDataAsSPL_Packet' = %d\n",err);
		return TRUE;
	}
	return TRUE;
}
Boolean send_TxUpTime()
{
	if(!CheckTransmitionAllowed()){
		return TRUE;
	}
		unsigned char upTime[4] = {0};
		int err = IsisTrxvu_tcGetUptime(0, (unsigned int*)upTime);

		if (err != 0)
		{
			printf("ERR: %d\n", err);
		}
		else
		{
			printf("seconds: %u\n", upTime[0]);
			printf("minutes: %u\n", upTime[1]);
			printf("hours: %u\n", upTime[2]);
			printf("days: %u\n", upTime[3]);

			sat_packet_t cmd = { 0 };
			err = AssembleCommand((unsigned char*) upTime, sizeof(upTime), trxvu_cmd_type,
					GET_TX_UPTIME, 0x54594245, &cmd);
			if (err != 0)
			{
				printf("error in AssembleCommand for Uptime: %d\n", err);
			}

			IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
			TransmitSplPacket(&cmd, NULL);

		}


	return TRUE;
}
Boolean SendRawHKDataMB() //siham
{
	if(!CheckTransmitionAllowed())
		return FALSE;
	//ieps_rawhk_data_mb_t rawhk_data_mb_t={0};
	isis_eps__gethousekeepingraw__from_t tlm_mb_raw; ////ieps_rawhk_data_mb_t tlm_mb_raw;

	if (isis_eps__gethousekeepingraw__tm(EPS_I2C_BUS_INDEX, &tlm_mb_raw) != 0)////int err = IsisEPS_getRawHKDataMB(0,&rawhk_data_mb_t,&p_rsp_code); //p_rsp_code???
	{
			return FALSE;  //if(err!=0) return FALSE;
	}
	//ieps_statcmd_t p_rsp_code;

	sat_packet_t cmd = {0};
	eps_subtypes_t eps_Raw_subtype ;
	int err = AssembleCommand((unsigned char*)&tlm_mb_raw,sizeof(tlm_mb_raw), eps_cmd_type, eps_Raw_subtype, 0x21344312,&cmd);
	if(err!=0)
		return FALSE;

	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
	TransmitSplPacket(&cmd, NULL);
	return TRUE;

}
 /*Boolean SendRAEngHKDataMB() //azal
 {
	 if(!CheckTransmitionAllowed())
		 return FALSE;
	 ieps_enghk_data_mb_t p_raenghk_data_mb = {0};
	 ieps_statcmd_t p_rsp_code;
	 int err = IsisEPS_getRAEngHKDataMB(0,&p_raenghk_data_mb,&p_rsp_code);
	 if(err!=0)
		 return FALSE;
	 sat_packet_t cmd = {0};
	 eps_subtypes_t eps_Eng_subtype;
	 err = AssembleCommand((unsigned char*)&p_raenghk_data_mb,sizeof(p_raenghk_data_mb),eps_cmd_type,eps_Eng_subtype,0x51561825,cmd);
	 if(err!=0)
		 return FALSE;
	 IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
	 	 			TransmitSplPacket(&cmd, NULL);
	 return TRUE;


 }

  Boolean Ants_GetAllTelemty() //nasreen
  {
     ISISantsSide ants_side;
     ISISantsTelemetry tlm_return ;

     if(!CheckTransmitionAllowed())
		 return FALSE;
      int err = IsisAntS_getAlltelemetry(0 ,ants_side , &tlm_return);
      if ( err != 0 )
    	  return FALSE ;
      sat_packet_t cmd = {0};
 	 err = AssembleCommand((unsigned char*)&tlm_return,0x51961825,cmd);
 	 if(err!=0)
 		 return FALSE;
 	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
 	      			TransmitSplPacket(&cmd, NULL);
      return TRUE ;


}

  Boolean AntsTemperaturesideA() //shahd
  {
  	sat_packet_t cmd = {0};
  	unsigned short temp=0;
  	if(!CheckTransmitionAllowed())
  	{
  		return FALSE;
  	}
  //	int err=IsisAntS_getTemperature(0,isisants_sideA,&temp);
  	if(0 != err){
  		printf("ERR %d:");
  	}
  	else
  	{
  		AssembleCommand(&temp,sizeof(temp),tlm_antenna,solar_panel_tlm,115104121,&cmd);
  	}
    IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,trxvu_bitrate_9600);
  	TransmitSplPacket(&cmd,NULL);
  	return TRUE;

  }

  Boolean AntsTemperaturesideB() //shahd
  {
  	sat_packet_t cmd = {0};
  	unsigned short temp=0;
  	if(!CheckTransmitionAllowed())
  	{
  		return FALSE;
  	}
  	int err=IsisAntS_getTemperature(0,isisants_sideB,&temp);
  	if(0 != err){
  		printf("ERR %d:\n");
  	}
  	else
  	{
  		AssembleCommand(&temp,sizeof(temp),tlm_antenna,solar_panel_tlm,115104121,&cmd);
  	}
    IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,trxvu_bitrate_9600);//ששמה אותו 9600 לשידור אפשר לשים אותה בציק טראנזמישין אלאוד בפעולה עצמה
  	TransmitSplPacket(&cmd,NULL);
  	return TRUE;

  }
  Boolean send_RAH()  //mhmd tllawi
  {

  	if(!CheckTransmitionAllowed()){
  		return TRUE;
  	}




  		//ieps_board_t board;
  		//ieps_statcmd_t StateCmd;
  		//ieps_enghk_data_cdb_t EngDataCDB;
  		isis_eps__gethousekeepingengincdb__from_t response;
  		int err = isis_eps__gethousekeepingengincdb__tm(0, &response);//IsisEPS_getRAengHKDataCDB(0,board,&EngDataCDB,&StateCmd);
  		//unsigned char raw[IEPS_HKDATACDB_SIZE];
  		//raw = EngDataCDB;
  		if (err != 0)
  		{
  			printf("ERR: %d\n",err);
  			return FALSE;
  		}
  		else
  		{
  			sat_packet_t cmd = { 0 };
  			eps_subtypes_t Get_EPS_RAverage_DBoard;
  			err = AssembleCommand( response.raw, sizeof(response.raw), eps_cmd_type,Get_EPS_RAverage_DBoard, 0x54594245, &cmd);
  			if (err != 0)
  			{
  				printf("error in AssembleCommand for the average of housekeeping: %d\n", err);
  				return FALSE;
  			}
  			IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
  			TransmitSplPacket(&cmd, NULL);
  		}

  	return TRUE;
  }*/

  Boolean TrxvuGetState()
  {
  	sat_packet_t cmd = {0};
  	unsigned short temp=0;
  	ISIStrxvuTransmitterState ts;
  	trxvu_subtypes_t GET_STATE = 0;
  	if(!CheckTransmitionAllowed())
  	{
  		return FALSE;
  	}
  	int err=IsisTrxvu_tcGetState(0,&ts);
  	if(0 != err){
  		printf("ERR %d:", err);
  	}
  	else
  	{
  		AssembleCommand((unsigned char*)&temp,sizeof(temp),trxvu_cmd_type,GET_STATE,115104121,&cmd);
  	}
    IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,trxvu_bitrate_9600);
  	TransmitSplPacket(&cmd,NULL);
  	return TRUE;
  }

 /* Boolean send_TxAllTelemetry()
  {
  	if(!CheckTransmitionAllowed())
  	{
  		return FALSE;
  	}
  	ISIStrxvuTxTelemetry tlm;
  	int err = IsisTrxvu_tcGetTelemetryAll(0,&tlm);
  	if(err != 0)
  	{
  		return FALSE;
  	}
  	else
  	{
  		sat_packet_t cmd = {0};
  		trxvu_subtypes_t Get_Telemetry_All;
  		err = AssembleCommand((unsigned char*) tlm, sizeof(tlm), trxvu_cmd_type,Get_Telemetry_All, 0x54594245, &cmd);
  		if (err != 0)
  		{
  			return FALSE;
  		}
  		IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
  		TransmitSplPacket(&cmd, NULL);
  	}
  	return TRUE;
  }

  Boolean send_EpsEngHKDataCDB()
  {
  	if(!CheckTransmitionAllowed())
  	{
  		return FALSE;
  	}
  	ieps_board_t board;
  	ieps_enghk_data_cdb_t p_enghk_data_cdb = {0};
  	ieps_statcmd_t p_rsp_code;
  	int err = IsisEPS_getEngHKDataCDB(0,&board,&p_enghk_data_cdb,&p_rsp_code);
  	if (err != 0)
  	{
  		return FALSE;
  	}
  	sat_packet_t cmd = {0};
  	eps_subtypes_t Get_EPS_EngHK_CDBoard;
  	err = AssembleCommand((unsigned char*)&p_enghk_data_cdb, sizeof(p_enghk_data_cdb), eps_cmd_type,
  			Get_EPS_EngHK_CDBoard, 0x54594245, &cmd);
  	if (err != 0)
  	{
  		return FALSE;
  	}
  	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
  	TransmitSplPacket(&cmd, NULL);
  	return TRUE;
  }*/

  Boolean SendRawaHKDataCDB() //jana
  {
  	if(!CheckTransmitionAllowed())
  			return FALSE;
  	//ieps_board_t board;
  	//ieps_enghk_data_cdb_t p_rawhk_data_cdb;
  	//ieps_statcmd_t p_rsp_code;
  	isis_eps__gethousekeepingrawincdb__from_t response;

  	/*!
  	 *	Prepare the response buffer with housekeeping data. The housekeeping data is returned in raw form, as received from the hardware, unaltered by the main controller.
  	 *
  	 *	@param [out] Response received from subsystem.
  	 * 	@return ISIS_EPS_ERR_t
  	 */
  	//int isis_eps__gethousekeepingrawincdb__tm( uint8_t index, isis_eps__gethousekeepingrawincdb__from_t *response );
  	int err= isis_eps__gethousekeepingrawincdb__tm(0,&response);//IsisEPS_getRawHKDataCDB(0,board,&p_rawhk_data_cdb,&p_rsp_code);
  	if(err!=0)
  			return FALSE;
  	sat_packet_t cmd ={0};
  	eps_subtypes_t Get_EPS_RAW_DBoard ;
  	err = AssembleCommand(response.raw ,sizeof(response) ,eps_cmd_type ,Get_EPS_RAW_DBoard , 0x2568,&cmd);
  	if(err!=0)
  		return FALSE;
  	TransmitSplPacket(&cmd,NULL);
  	return TRUE;
  }

Boolean getEPS_device()
{
	if(!CheckTransmitionAllowed())
	  	{
		return FALSE;
	  	}
	else {
	//ieps_systemstate_t  EPS_system_state_information;
	//ieps_statcmd_t default_response ;
	eps_subtypes_t Get_EPS_StateData ;
	isis_eps__getsystemstatus__from_t  response;


	/*!
	 *	Return system status information
	 *
	 *	@param [out] Response received from subsystem.
	 * 	@return ISIS_EPS_ERR_t
	 */
	//int isis_eps__getsystemstatus__tm( uint8_t index, isis_eps__getsystemstatus__from_t *response );

	int err=isis_eps__getsystemstatus__tm( 0,&response); // IsisEPS_getSystemState(0, &EPS_system_state_information, &default_response);
	if(0!=err)
		return FALSE ;

	sat_packet_t cmd = { 0 };
	err = AssembleCommand(response.raw, sizeof(response),eps_cmd_type,Get_EPS_StateData, 0x54594245, &cmd);
	//err= AssembleCommand(EPS_system_state_information, sizeof(EPS_system_state_information), eps_cmd_type,Get_EPS_StateData, 0x54594245, &cmd);
	if(0!=err)
			return FALSE ;

	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
	 TransmitSplPacket(&cmd, NULL);
	}
	  return TRUE ;
}

Boolean SendEngHKDataCDB()  //maisan
{
//	if(!CheckTransmitionAllowed())
//		return FALSE;
//	//ieps_board_t board;
//	//ieps_enghk_data_cdb_t p_enghk_data_cdb;
//	//ieps_statcmd_t p_rsp_code;
//	isis_eps__gethousekeepingengincdb__from_t response;
//	int err =isis_eps__gethousekeepingengincdb__tm(0, &response);//err=IsisEPS_getEngHKDataCDB(0,board,&p_enghk_data_cdb,&p_rsp_code);
//	if(err!=0)
//		return FALSE;
//	sat_packet_t cmd ={0};
//	eps_subtypes_t Get_EPS_Eng_DBoard = a;
//	err = AssembleCommand((unsigned char*)response.raw,sizeof(response),eps_cmd_type, Get_EPS_Eng_DBoard ,0x15847,&cmd);
//	if(err!=0)
//		return FALSE;
//	TransmitSplPacket(&cmd,NULL);
	return TRUE;
}


Boolean TxTlm_revc()
{
	sat_packet_t cmd = {0};
		unsigned short temp=0;
		ISIStrxvuRxTelemetry_revC tlm;
		trxvu_subtypes_t GET_TLM_ALL_REVC = 90;
		if(!CheckTransmitionAllowed())
		{
			return FALSE;
		}
		int err=IsisTrxvu_rcGetTelemetryAll_revC(0, &tlm);
		if(0 != err)
		{
			printf("ERR %d:", err);
		}
		else
		{
			AssembleCommand((unsigned char*)&temp ,sizeof(temp),trxvu_cmd_type, GET_TLM_ALL_REVC, 0x15104121, &cmd);
		}
		IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,trxvu_bitrate_9600);
		TransmitSplPacket(&cmd,NULL);
		return TRUE;
}

/*Boolean AntsStatusTLMSideA()
{
	if(!CheckTransmitionAllowed())
	{
		return FALSE;
	}
	ISISantsStatus Status = {0};
	sat_packet_t cmd = {0};
	ISISantsSide isisants_sideA;
	int err = IsisAntS_getStatusData(0,isisants_sideA,&Status);
	if(err != 0)
	{
		printf("ERR %d\n");
		return FALSE;
	}
	else
	{
		AssembleCommand(&Status,sizeof(Status),tlm_antenna,solar_panel_tlm,656566,&cmd);
	}
	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,trxvu_bitrate_9600);
		TransmitSplPacket(&cmd,NULL);
	return TRUE;


}
Boolean AntsStatusTLMSideB()
{
	if(!CheckTransmitionAllowed())
	{
		return FALSE;
	}
	ISISantsStatus Status = {0};
	sat_packet_t cmd = {0};
	int err = IsisAntS_getStatusData(0,isisants_sideB,&Status);
	if(err != 0)
	{
		printf("ERR %d\n");
		return FALSE;
	}
	else
	{
		AssembleCommand(&Status,sizeof(Status),tlm_antenna,solar_panel_tlm,656566,&cmd);
	}
	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,trxvu_bitrate_9600);
	TransmitSplPacket(&cmd,NULL);
	return TRUE;


}

Boolean send_RCAllTelemetry()
{
	if(!CheckTransmitionAllowed()){
		return TRUE;
	}
	telemetry_subtypes_t Get_Telemetry_All;
	ISIStrxvuRxTelemetry* tlm;
			int err = IsisTrxvu_rcGetTelemetryAll(0, &tlm);

		if (err != 0)
		{
			printf("ERR: %d\n",err);
			return FALSE;
		}
		else
		{
			sat_packet_t cmd = { 0 };
			err = AssembleCommand(tlm, sizeof(tlm), telemetry_cmd_type,Get_Telemetry_All, 0x54594245, &cmd);
			if (err != 0)
			{
				printf("error in AssembleCommand for all telemetry: %d\n", err);
				return FALSE;
			}

			IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
			TransmitSplPacket(&cmd, NULL);
		}


	return TRUE;
}
Boolean send_RCAllTelemetryREVC()
{
	if(!CheckTransmitionAllowed()){
		return TRUE;
	}
	telemetry_subtypes_t Get_Telemetry_AllREVC;
	ISIStrxvuRxTelemetry_revC* tlm;
			int err = IsisTrxvu_rcGetTelemetryAll_revC(0, &tlm);

		if (err != 0)
		{
			printf("ERR: %d\n",err);
			return FALSE;
		}
		else
		{
			sat_packet_t cmd = { 0 };
			err = AssembleCommand(tlm, sizeof(tlm), telemetry_cmd_type,Get_Telemetry_AllREVC, 0x54594245, &cmd);
			if (err != 0)
			{
				printf("error in AssembleCommand for all telemetryREVC: %d\n", err);
				return FALSE;
			}

			IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
			TransmitSplPacket(&cmd, NULL);
		}


	return TRUE;
}
Boolean send_GetUpTimeAnt()
{
	if(!CheckTransmitionAllowed()){
		return TRUE;
	}
	time_unix uptime;
	ISISantsSide side;
			int err = IsisAntS_getUptime(0, side,&uptime);

		if (err != 0)
		{
			printf("ERR: %d\n",err);
			return FALSE;
		}
		else
		{
			sat_packet_t cmd = { 0 };
			err = AssembleCommand(uptime, sizeof(uptime), trxvu_cmd_type,ANT_GET_UPTIME, 0x54594245, &cmd);
			if (err != 0)
			{
				printf("error in AssembleCommand for all telemetryREVC: %d\n", err);
				return FALSE;
			}

			IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
			TransmitSplPacket(&cmd, NULL);
		}


	return TRUE;
}*/
Boolean Test_dummySend()
{

	return TRUE;
}
Boolean selectAndExecuteTrxvuDemoTest()
{
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Init Trxvu \n\r");
	printf("\t 2) Init Trxvu With different Fram Lengths\n\r");
	printf("\t 3) Test Trxvu Logic \n\r");
	printf("\t 4) Send Dump Abort Request using Queue\n\r");
	printf("\t 5) Transmit Dummy SPL Packet\n\r");
	printf("\t 6) Transmit SPL Packet\n\r");
	printf("\t 7) Exit Dump\n\r");
	printf("\t 8) Test Dump\n\r");
	printf("\t 9) Test Beacon Logic\n\r");
	printf("\t 10) Mute Trxvu\n\r");
	printf("\t 11) Unmute Trxvu\n\r");
	printf("\t 12) Get Number Of Frames In Buffer\n\r");
	printf("\t 13) Set Trxvu Bitrate\n\r");
	printf("\t 14) Get Trxvu Bitrate\n\r");
	printf("\t 15) Transmit Data As SPL Packet\n\r");
	printf("\t 16) Choose to default beacon inervals\n\r");
	printf("\t 17) Restore to default beacon inervals\n\r");
	printf("\t 18) Check Transmition Allowed\n\r");

	unsigned int number_of_tests = 18;
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, number_of_tests) == 0);

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = TestInitTrxvu();
		break;
	case 2:
		offerMoreTests = TestInitTrxvuWithDifferentFrameLengths();
		break;
	case 3:
		offerMoreTests = TestTrxvuLogic();
		break;
	case 4:
		offerMoreTests = TestSendDumpAbortRequest();
		break;
	case 5:
		offerMoreTests = TestTransmitDummySplPacket();
		break;
	case 6:
		offerMoreTests = TestTransmitSplPacket();
		break;
	case 7:
		offerMoreTests = TestExitDump();
		break;
	case 8:
		offerMoreTests = TestDumpTelemetry();
		break;
	case 9:
		offerMoreTests = TestBeaconLogic();
		break;
	case 10:
		offerMoreTests = TestMuteTrxvu();
		break;
	case 11:
		offerMoreTests = TestUnMuteTrxvu();
		break;
	case 12:
		offerMoreTests = TestGetNumberOfFramesInBuffer();
		break;
	case 13:
		offerMoreTests = TestSetTrxvuBitrate();
		break;
	case 14:
		offerMoreTests = TestGetTrxvuBitrate();
		break;
	case 15:
		offerMoreTests = TestTransmitDataAsSPL_Packet();
		break;
	case 16:
		offerMoreTests = TestChooseDefaultBeaconCycle();
		break;
	case 17:
		offerMoreTests = TestRestoreDefaultBeaconParameters();
		break;
	case 18:
		offerMoreTests = TestCheckTransmitionAllowed();
		break;
	case 19:
	    offerMoreTests = send_TxUpTime();
			break;
	case 20:
		    offerMoreTests = SendRawHKDataMB();
				break;
	case 21:
		//offerMoreTests = SendRAEngHKDataMB();
	            break ;
	case 22 :
		//offerMoreTests = Ants_GetAllTelemty();
	            break ;
	case 23 :
			//offerMoreTests = AntsTemperaturesideA();
			//offerMoreTests = AntsTemperaturesideB();
		            break ;
	case 24 :
			//offerMoreTests = send_RAH();
		            break ;
	case 25 :
				offerMoreTests = TrxvuGetState();
			    break ;
	case 26 :
				//offerMoreTests = send_TxAllTelemetry();
			            break ;
	case 27 :
					//offerMoreTests = send_EpsEngHKDataCDB();
				            break ;
	case 28 :
					offerMoreTests = SendRawaHKDataCDB();
				            break ;
	case 29 :
					offerMoreTests = getEPS_device();
				            break ;
	case 30 :
					offerMoreTests = SendEngHKDataCDB();
				            break ;
	case 31 :
					offerMoreTests = TxTlm_revc();
				            break ;
	case 32 :
					//offerMoreTests = AntsStatusTLMSideA();
					//offerMoreTests = AntsStatusTLMSideA();
				            break ;
	case 33 :
						//offerMoreTests = send_RCAllTelemetry();
					            break ;
	case 34 :
						//offerMoreTests = send_RCAllTelemetry();
					            break ;
	case 35 :
						//offerMoreTests = send_GetUpTimeAnt();
					            break ;
	default:
	    offerMoreTests = Test_dummySend();
		break;
	}
	return offerMoreTests;
}

Boolean MainTrxvuTestBench()
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteTrxvuDemoTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
	return FALSE;
}
