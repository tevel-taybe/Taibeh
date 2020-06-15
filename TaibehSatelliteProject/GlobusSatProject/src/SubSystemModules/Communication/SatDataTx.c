#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Storage/FRAM.h>
#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include "SubSystemModules/PowerManagment/EPSOperationModes.h"
#include "GlobalStandards.h"
#include "SatDataTx.h"
#include "Logger.h"

Boolean 		g_mute_flag = MUTE_OFF;				// mute flag - is the mute enabled
time_unix 		g_mute_end_time = 0;				// time at which the mute will end
time_unix		g_idle_end_time = 0;				// indicates the time when idle will end

xSemaphoreHandle xIsTransmitting = NULL; // mutex on transmission.

void InitTxModule()
{
	if(NULL == xIsTransmitting)
		vSemaphoreCreateBinary(xIsTransmitting);
}

int muteTRXVU(time_unix duration) {
	if (duration > MAX_MUTE_TIME) {
		wlog(CNAME_TRXVU, LOG_ERROR, -1, "Required Mute Time is greater than permitted\n");
		return -1;
	}
	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);

	g_mute_end_time = curr_time + duration;
	g_mute_flag = MUTE_ON;

	return 0;
}

int SetIdleState(ISIStrxvuIdleState state, time_unix duration){

	if (duration > MAX_IDLE_TIME)
	{
		//return TRXVU_IDLE_RERQUIRED_TIME_TOO_LONG; //TODO:
		return -1;
	}

	int err = (IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX, state));

	if (err == E_NO_SS_ERR && state == trxvu_idle_state_on){
		// get current unix time
		time_unix curr_time = 0;
		Time_getUnixEpoch(&curr_time);
		// set mute end time
		g_idle_end_time = curr_time + duration;
	}
	return err;
}
void UnMuteTRXVU() {
	g_mute_end_time = 0;
	g_mute_flag = MUTE_OFF;
}

Boolean GetMuteFlag() {
	return g_mute_flag;
}

Boolean CheckForMuteEnd() {
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);
	return (curr_tick_time > g_mute_end_time);
}

int GetNumberOfFramesInBuffer() {
	unsigned short frameCounter = 0;
	int err = IsisTrxvu_rcGetFrameCount(0, &frameCounter);
	if (0 != err) {
		return err;
	}
	return frameCounter;
}

Boolean CheckTransmitionAllowed() {
	Boolean low_voltage_flag = TRUE;
	if(g_mute_flag == MUTE_ON)
	{
	time_unix current;
	Time_getUnixEpoch(&current);
	if(current >= g_mute_end_time )
	{
	UnMuteTRXVU();
	}
	else
	{
		return FALSE;
	}
	}
	low_voltage_flag = EpsGetLowVoltageFlag();
	//TODO: the below line ahould be deleted
	low_voltage_flag = FALSE;
	if(low_voltage_flag)
	{
		return FALSE;
	}
	/*if(pdTRUE == xSemaphoreTake(xIsTransmitting,0)){
		xSemaphoreGive(xIsTransmitting);
		return TRUE;
	}*/
	return TRUE;
}

int GetTrxvuBitrate(ISIStrxvuBitrateStatus *bitrate) {
	if (NULL == bitrate) {
		return E_NOT_INITIALIZED;
	}
	ISIStrxvuTransmitterState trxvu_state;
	int err = IsisTrxvu_tcGetState(ISIS_TRXVU_I2C_BUS_INDEX, &trxvu_state);

	if (E_NO_SS_ERR == err) {
		*bitrate = trxvu_state.fields.transmitter_bitrate;
	}
	return err;

}

int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data,unsigned int length)
{
	int err = 0;
	sat_packet_t packet = { 0 };
	if (NULL != cmd) {
		err = AssembleCommand(data, length, cmd->cmd_type, cmd->cmd_subtype,
				cmd->ID, &packet);
}  else
	{
	err = AssembleCommand(data, length, 0xFF, 0xFF, 0xFFFFFF03, &packet); //TODO: figure out what should be the 'FF'
	}
	if (err != 0)
	{
	return err;
	}
	err = TransmitSplPacket(&packet, NULL);
	return err;
}

int TransmitSplPacket(sat_packet_t *packet, int *avalFrames) {
	if (!CheckTransmitionAllowed()) {
		return -1;
	}

	if (NULL == packet) {
		return E_NOT_INITIALIZED;
	}

	int err = 0;
	unsigned int data_length = packet->length +
			sizeof(packet->length)+
			sizeof(packet->cmd_subtype) +
			sizeof(packet->cmd_type)+
			sizeof(packet->ID);

	if (xSemaphoreTake(xIsTransmitting,SECONDS_TO_TICKS(1)) != pdTRUE) {
		return E_GET_SEMAPHORE_FAILED;
	}
	err = IsisTrxvu_tcSendAX25DefClSign(ISIS_TRXVU_I2C_BUS_INDEX,(unsigned char*) packet, data_length, (unsigned char*) &avalFrames);

	xSemaphoreGive(xIsTransmitting);

	return err;
}
