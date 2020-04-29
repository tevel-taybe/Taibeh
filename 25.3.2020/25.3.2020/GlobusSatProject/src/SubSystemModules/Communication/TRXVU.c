#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

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

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/Beacon.h"



xQueueHandle xDumpQueue = NULL;
xSemaphoreHandle xDumpLock = NULL;
xTaskHandle xDumpHandle = NULL;			 //task handle for dump task


// defining  buffer for Dump
#define dump_buffer_size  50000
static unsigned char dump_buffer[dump_buffer_size];
#define TAIBEH   3


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
			unsigned int satID =  cmd.ID >> (sizeof(cmd.ID) - 1 ) * 8;

			if (satID == TAIBEH ||  satID == 0 )
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

	BeaconLogic();

	if (cmd_command_found != err)
		return err;

	return cmd_command_succsess;
}

void FinishDump(dump_arguments_t *task_args,unsigned char *buffer, ack_subtype_t acktype,
		unsigned char *err, unsigned int size) {

	SendAckPacket(acktype, task_args->cmd, err, size);
	if (NULL != task_args) {
		free(task_args);
	}
	if (NULL != xDumpLock) {
		xSemaphoreGive(xDumpLock);
	}
	if (xDumpHandle != NULL) {
		vTaskDelete(xDumpHandle);
	}
	if(NULL != buffer){
		free(buffer);
	}
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
	err = xQueueReceive(xDumpQueue, &queue_msg, SECONDS_TO_TICKS(1));
	if (err == pdPASS)
	{
	return queue_msg;
	}
	return FALSE;
}

void DumpTask(void *args)
{

	  if (NULL == args)
	   {
		FinishDump(NULL, NULL, ACK_DUMP_ABORT, NULL, 0);
		return;
	   }
	dump_arguments_t *task_args = (dump_arguments_t *) args;
	sat_packet_t dump_tlm = { 0 };
	int err = 0;
	int availFrames = 0;
	unsigned int num_of_packets = 0;
	unsigned int size_of_element = 0;
	time_unix last_time_read = 0;
	int num_of_tlm_elements_read = 0;
	char filename[MAX_F_FILE_NAME_SIZE] = { 0 };

	err = GetTelemetryFilenameByType((tlm_type) task_args->dump_type,filename);
		if (err)
		{
		FinishDump(task_args, dump_buffer, ACK_DUMP_ABORT, (unsigned char*) &err,sizeof(err));
		return;
		}

	 //c_fileRead(filename, task_args->t_start, task_args->t_end, &num_of_tlm_elements_read, &last_time_read, &size_of_element);

	//FileSystemResult c_fileGetNumOfElements(char* c_file_name, time_unix from_time, time_unix to_time,
		//	int* read, time_unix* last_read_time, unsigned int* size_of_element)

	//  c_fileGetNumOfElements(filename, task_args->t_start, task_args->t_end);

	c_fileRead(filename, dump_buffer, dump_buffer_size, task_args->t_start, task_args->t_end, &num_of_tlm_elements_read, &last_time_read);
	num_of_packets = dump_buffer_size / MAX_COMMAND_DATA_LENGTH; // each packet will be max 200 bytes ( 00c8)
		if (dump_buffer_size % MAX_COMMAND_DATA_LENGTH)
		num_of_packets++;
	SendAckPacket(ACK_DUMP_START, task_args->cmd,(unsigned char*) &num_of_packets, sizeof(num_of_packets));

	unsigned int currPacketSize;
	unsigned int totalDataLeft = dump_buffer_size;
	unsigned int i = 0;
	while (i < num_of_packets)
	{

		if (CheckDumpAbort() || !CheckTransmitionAllowed())
		return FinishDump(task_args, dump_buffer, ACK_DUMP_ABORT, NULL, 0);

		currPacketSize = totalDataLeft < MAX_COMMAND_DATA_LENGTH ? totalDataLeft : MAX_COMMAND_DATA_LENGTH;
		AssembleCommand(dump_buffer + i*size_of_element, currPacketSize,(char) DUMP_SUBTYPE, (char) (task_args->dump_type),task_args->cmd->ID,   &dump_tlm);

		err = TransmitSplPacket(&dump_tlm, &availFrames);
			if (err)
			return FinishDump(task_args, dump_buffer, ACK_DUMP_ABORT, NULL, 0);
			if (availFrames != NUM_OF_Available_Frames )
			{
			i++;
			totalDataLeft -= currPacketSize;
			}
			if (availFrames == 0 || availFrames == NUM_OF_Available_Frames)
			vTaskDelay(10);
	}
	FinishDump(task_args, dump_buffer, ACK_DUMP_FINISHED, NULL, 0);
}



int DumpTelemetry(sat_packet_t *cmd) {
	if (NULL == cmd) {
		return -1;
	}

	dump_arguments_t *dmp_pckt = malloc(sizeof(*dmp_pckt));
	unsigned int offset = 0;

	dmp_pckt->cmd = cmd;

	memcpy(&dmp_pckt->dump_type, cmd->data, sizeof(dmp_pckt->dump_type));
	offset += sizeof(dmp_pckt->dump_type);

	memcpy(&dmp_pckt->t_start, cmd->data + offset, sizeof(dmp_pckt->t_start));
	offset += sizeof(dmp_pckt->t_start);

	memcpy(&dmp_pckt->t_end, cmd->data + offset, sizeof(dmp_pckt->t_end));

	if (xSemaphoreTake(xDumpLock,SECONDS_TO_TICKS(1)) != pdTRUE) { /// TODO : need to be checked with Edan
		return E_GET_SEMAPHORE_FAILED;
	}
	xTaskCreate(DumpTask, (const signed char* const )"DumpTask", 2000,
			(void* )dmp_pckt, configMAX_PRIORITIES - 2, xDumpHandle);

	return 0;
}

