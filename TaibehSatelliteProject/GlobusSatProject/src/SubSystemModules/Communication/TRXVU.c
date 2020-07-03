#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hal/Storage/FRAM.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>
#include <hcc/api_fat.h>
#include <GlobalStandards.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <hcc/api_hcc_mem.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <stdlib.h>
#include <string.h>

#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"
#include "SatDataTx.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/Beacon.h"
#include "SubSystemModules/PowerManagment/EPSOperationModes.h"
#include "Logger.h"

extern time_unix		g_idle_end_time ;


xQueueHandle xDumpQueue = NULL;
xSemaphoreHandle xDumpLock = NULL;
xTaskHandle xDumpHandle = NULL;			 //task handle for dump task


// defining  buffer for Dump
#define dump_buffer_size  (4096)
char dump_buffer[dump_buffer_size];
#define TAIBEH   3


void idle_status_check()
{
		time_unix curr_time = 0;
		int err = Time_getUnixEpoch(&curr_time);
		//if (0!=err)
		//TODO: write to log
		if (curr_time >= g_idle_end_time && g_idle_end_time !=0 ) {
			g_idle_end_time = 0 ;
			SetIdleState(trxvu_idle_state_off,0);
		}
}




void InitSemaphores()
{
	if(NULL == xDumpLock)
		vSemaphoreCreateBinary(xDumpLock);
	if(NULL == xDumpQueue)
		xDumpQueue = xQueueCreate(1, sizeof(Boolean));
}

int InitTrxvu() {

	ISIStrxvuI2CAddress myTRXVUAddress;
	ISIStrxvuFrameLengths myTRXVUBuffers;

	int retValInt = 0;

	//Buffer definition
	myTRXVUBuffers.maxAX25frameLengthTX = SIZE_TXFRAME;//SIZE_TXFRAME;
	myTRXVUBuffers.maxAX25frameLengthRX = SIZE_RXFRAME;

	//I2C addresses defined
	myTRXVUAddress.addressVu_rc = I2C_TRXVU_RC_ADDR;
	myTRXVUAddress.addressVu_tc = I2C_TRXVU_TC_ADDR;


	//Bitrate definition
	ISIStrxvuBitrate myTRXVUBitrates;
	myTRXVUBitrates = trxvu_bitrate_9600;
	retValInt = IsisTrxvu_initialize(&myTRXVUAddress, &myTRXVUBuffers,
			&myTRXVUBitrates, 1);
	if (retValInt != 0) {
		return retValInt;
	}
	vTaskDelay(100);

	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,myTRXVUBitrates);
	vTaskDelay(100);

	ISISantsI2Caddress myAntennaAddress;
	myAntennaAddress.addressSideA = ANTS_I2C_SIDE_A_ADDR;
	myAntennaAddress.addressSideB = ANTS_I2C_SIDE_B_ADDR;

	//Initialize the AntS system
	retValInt = IsisAntS_initialize(&myAntennaAddress, 1);
	if (retValInt != 0) {
		return retValInt;
	}

	InitTxModule();
	InitBeaconParams();
	InitSemaphores();

	return 0;
}

CommandHandlerErr TRX_Logic() {
	int err = 0;
	int frame_count = GetNumberOfFramesInBuffer();
	sat_packet_t cmd = { 0 };

	if (frame_count > 0)
		{
			err = GetOnlineCommand(&cmd);
			if (cmd_command_found == err)
			{
				// MSB is the id of sat
				unsigned int satID =  cmd.ID >> (sizeof(cmd.ID) - 1 ) * 8;

				if (satID == TAIBEH ||  satID  == 0 )// 54 59 42 03
				{
					ResetGroundCommWDT();
					SendAckPacket(ACK_RECEIVE_COMM, &cmd, NULL, 0);
					err = ActUponCommand(&cmd);
				}
			}
		}
		else if (GetDelayedCommandBufferCount() > 0)
		{  // delayed command was removed from the scope
			err = GetDelayedCommand(&cmd);
		}
	    vTaskDelay(10);
	    // TODO: need to add check for mute
	    idle_status_check();
		BeaconLogic();
		vTaskDelay(5);

	  if (cmd_command_found != err)
		return err;

	return cmd_command_succsess;
}

void FinishDump(dump_arguments_t *task_args,unsigned char *buffer, ack_subtype_t acktype,
		unsigned char *err, unsigned int size) {

	SendAckPacket(acktype, &(task_args->cmd), err, size);
	if (NULL != task_args) {
		free(task_args);
	}
	if (NULL != xDumpLock) {
		xSemaphoreGive(xDumpLock);
	}
	if (xDumpHandle != NULL) {
		vTaskDelete(xDumpHandle);
	}
//	if(NULL != buffer){
//		free(buffer);
//	}
}


void AbortDump()
{
	FinishDump(NULL,NULL,ACK_DUMP_ABORT,NULL,0);
}

void SendDumpAbortRequest() {
	if (eTaskGetState(xDumpHandle) == eDeleted) {
		return;
	}
	Boolean queue_msg = TRUE;
	int err = xQueueSend(xDumpQueue, &queue_msg, SECONDS_TO_TICKS(1));
	if (0 != err) {
		if (NULL != xDumpLock) {
			xSemaphoreGive(xDumpLock);
		}
		if (xDumpHandle != NULL) {
			vTaskDelete(xDumpHandle);
		}
	}
}

Boolean CheckDumpAbort()
{
	portBASE_TYPE err;
	Boolean queue_msg;
	// err = xQueueReceive(xDumpQueue, &queue_msg, SECONDS_TO_TICKS(1));
	err = xQueueReceive(xDumpQueue, &queue_msg, 0);
	if (err == pdPASS)
	{
		return queue_msg;
	}
	return FALSE;
}


void DumpTask(void *args)
{

	if (NULL == args){
		FinishDump(NULL, NULL, ACK_DUMP_ABORT, NULL, 0);
		return;
	}
	dump_arguments_t *task_args = (dump_arguments_t *) args;
	sat_packet_t dump_tlm = { 0 };
	int err = 0;
	int ack_code = ACK_DUMP_FINISHED;
	int End_Of_Data = 0;
	FileSystemResult result = 0;
	int finish_read = 0;
	int availFrames = 1;
	unsigned int num_of_packets = 0;
	unsigned int num_packets_read = 0; //number of packets read from buffer in each time
	unsigned int size_of_elements_include_time_stamp = 0;
	unsigned int size_of_element=0;
	unsigned int num_of_elements =0;
	time_unix last_sent_time = task_args->t_start;// from this time we need to start the next search
	time_unix last_time_read = 0; //
	int num_of_tlm_elements_read = 0;
	unsigned int total_packets_read = 0; // the amount of all packets read from the buffer
	char filename[MAX_F_FILE_NAME_SIZE] = { 0 };
	unsigned char *buffer = NULL;
	buffer = dump_buffer;
// ============================================================================================

	err = GetTelemetryFilenameByType((tlm_type) task_args->dump_type,filename);
	if (err)
	{
		//TODO: check if we need to Write to Log file
		FinishDump(task_args, dump_buffer, ACK_DUMP_ABORT, (unsigned char*) &err,sizeof(err));
		return ;
	}
	f_managed_enterFS();
	if (c_fileGetSizeOfElement(filename,&size_of_element)!= FS_SUCCSESS)
	{
		//return ;
		//TODO: write to log
	}
	size_of_elements_include_time_stamp = size_of_element + sizeof(time_unix);
	//TODO:should add to the log ( filename, size_of_element, (start time)task_args->t_start, (ens time)task_args->t_end

	//f_managed_enterFS();// TODO: check if needed

	//=================================
	SendAckPacket(ACK_DUMP_START, &(task_args->cmd),(unsigned char*) &num_of_elements, sizeof(num_of_elements));


	while(0 == End_Of_Data)
	{
		num_packets_read = 0;
		unsigned int resolution =1;
		// TODO: need to check with other groups the issue of resolution
		result = c_fileRead(filename, buffer, SIZE_DUMP_BUFFER,last_sent_time, task_args->t_end, &num_packets_read, &last_time_read,resolution);
		if(result != FS_BUFFER_OVERFLOW)
		{
			// TODO: consider writting the error to Log
			End_Of_Data = 1;
		}
		last_sent_time = last_time_read;
		total_packets_read += num_packets_read;
		for(int i = 0; i < num_packets_read; i++)
		{
			if (CheckDumpAbort() || !CheckTransmitionAllowed())
			{
				wlog(CNAME_TRXVU, LOG_INFO, -1, "Dump Abort request \n");
				ack_code = ACK_DUMP_ABORT;
				goto All_Done;
			}
			if (0 == availFrames)
			{
				vTaskDelay(20);
			}
			AssembleCommand((unsigned char*)buffer + size_of_elements_include_time_stamp * i, size_of_elements_include_time_stamp,
					(char) DUMP_SUBTYPE, (char) (task_args->dump_type),task_args->cmd.ID, &dump_tlm);
			err = TransmitSplPacket(&dump_tlm, &availFrames);
			if(err != 0)
			{
				wlog(CNAME_TRXVU, LOG_INFO, err, "Transmitting Data Dump \n");
			}
			if((i+1)%10 == 0)
				vTaskDelay(50);
		}
	}
	All_Done:
		//f_managed_releaseFS();
		f_releaseFS();

		// send ack that we finished?
		SendAckPacket(ack_code, &(task_args->cmd), NULL, 0);
		if (NULL != task_args) {
			free(task_args);
		}
		if (NULL != xDumpLock) {
			xSemaphoreGive(xDumpLock);
		}

		vTaskDelete(NULL);
}// ***************************************************     DONE      *****************************

int DumpTelemetry(sat_packet_t *cmd) {
	if (NULL == cmd) {
		return -1;
	}

	dump_arguments_t *dmp_pckt = malloc(sizeof(*dmp_pckt));
	unsigned int offset = 0;
	//dmp_pckt->cmd = cmd;
	memcpy(&(dmp_pckt->dump_type), &cmd->data[offset], sizeof(dmp_pckt->dump_type));
	offset += sizeof(dmp_pckt->dump_type);
	memcpy(&(dmp_pckt->t_start), &cmd->data[offset], sizeof(dmp_pckt->t_start));
	offset += sizeof(dmp_pckt->t_start);
	memcpy(&(dmp_pckt->t_end), &cmd->data[offset], sizeof(dmp_pckt->t_end));
	offset += sizeof(dmp_pckt->t_end);
	memcpy(&(dmp_pckt->cmd),cmd,sizeof(*cmd));
	//memcpys(&dmp_pckt->delog, cmd->data + offset, sizeof(dmp_pckt->delog));
	//dmp_pckt->delog = 0;

	if (xSemaphoreTake(xDumpLock,SECONDS_TO_TICKS(1)) != pdTRUE) { /// TODO : need to be checked with Edan
		return E_GET_SEMAPHORE_FAILED;
	}

	xTaskCreate(DumpTask, (const signed char* const )"DumpTask", 2000, (void* )dmp_pckt, configMAX_PRIORITIES - 2, &xDumpHandle);
	//DumpTask((void* )dmp_pckt);

	return 0;
}

