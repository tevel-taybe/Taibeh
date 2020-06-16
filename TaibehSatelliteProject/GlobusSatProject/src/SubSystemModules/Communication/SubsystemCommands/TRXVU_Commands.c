#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>
#include <hal/Timing/Time.h>

#include "GlobalStandards.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "TRXVU_Commands.h"
#include "SubSystemModules/Communication/Beacon.h"
#include "TLM_management.h"



int CMD_StartDump(sat_packet_t *cmd)
{
	printf("******************************************getting into startDump function******************************** ");
	int err = 0;
	err = DumpTelemetry(cmd);
	return err;
}

int CMD_StopDump(sat_packet_t *cmd)
{
	(void) cmd;
	AbortDump();
	return 0;
}

int CMD_SendDumpAbortRequest(sat_packet_t *cmd)
{
	(void)cmd;
	SendDumpAbortRequest();
	return 0;
}

int CMD_ForceDumpAbort(sat_packet_t *cmd)
{
	(void)cmd;
	AbortDump();
	return 0;
}

int CMD_SetIdleState(sat_packet_t *cmd)
{
	    ISIStrxvuIdleState state;
		memcpy(&state,cmd->data,sizeof(state));
		time_unix duaration = 0;
		if (state == trxvu_idle_state_on)
		{
			memcpy(&duaration,cmd->data+sizeof(state),sizeof(duaration));
		}
		else
			SetIdleState(trxvu_idle_state_off,0);
		int err = SetIdleState(state,duaration);
		if (err == 0)
		{
			SendAckPacket(ACK_CMD_DONE, cmd, NULL, 0);
		}

		return err;
}

int CMD_MuteTRXVU(sat_packet_t *cmd)
{

	int err = 0;
	time_unix mute_period = 0;
	memcpy(&mute_period, cmd->data, sizeof(mute_period));// (&data
	err = muteTRXVU(mute_period);
	return err;
}

int CMD_UnMuteTRXVU(sat_packet_t *cmd)
{
	(void)cmd;
	UnMuteTRXVU();
	return 0;
}

int CMD_GetBaudRate(sat_packet_t *cmd)
{
	int err = 0;
	ISIStrxvuBitrateStatus bitrate;
	err = GetTrxvuBitrate(&bitrate);
	TransmitDataAsSPL_Packet(cmd, &bitrate, sizeof(bitrate));

	return err;
}

int CMD_SetBeaconCycleTime(sat_packet_t *cmd)
{
	int err = 0;
	ISIStrxvuBitrate bitrate = 0;

	err =  FRAM_write(cmd->data, BEACON_BITRATE_CYCLE_ADDR, BEACON_BITRATE_CYCLE_SIZE);
	err += FRAM_read(&bitrate, BEACON_BITRATE_CYCLE_ADDR, BEACON_BITRATE_CYCLE_SIZE);
	TransmitDataAsSPL_Packet(cmd,(unsigned char*)&bitrate, sizeof(bitrate));

	return err;
}

int CMD_GetBeaconInterval(sat_packet_t *cmd)
{
	int err = 0;
	time_unix beacon_interval = 0;
	err = FRAM_read((unsigned char*) &beacon_interval,
			BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &beacon_interval,
			sizeof(beacon_interval));
	return err;
}

typedef struct SetBeaconReply{
	int error;
	time_unix interval;
} SetBeaconReply;

int CMD_SetBeaconInterval(sat_packet_t *cmd)
{
	SetBeaconReply reply = {0, 0};

	memcpy(&reply.interval,  &cmd->data, sizeof(reply.interval));
	printf("cmd setbeacon interval start >>>>>>>\n");
	reply.error = Beacon_SetInterval(reply.interval);
	vTaskDelay(100);
	int err = TransmitDataAsSPL_Packet(cmd, (unsigned char*) &reply, sizeof(reply));
	printf("cmd setbeacon interval %d end err=%d<<<<<\n", reply.interval,  err);
	return 0;
}

int CMD_SetBaudRate(sat_packet_t *cmd)
{
	int err = 0;
	ISIStrxvuBitrate bitrate = (ISIStrxvuBitrate) cmd->data[0];
	switch(bitrate){
		case trxvu_bitrate_1200:
		case trxvu_bitrate_2400:
		case trxvu_bitrate_4800:
		case trxvu_bitrate_9600:
			{
				err = IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, bitrate);
				printf("Set baud rate to %d  result is: %d\n", (int)bitrate, err);
				if(err == 0){
					vTaskDelay(100);
					TransmitDataAsSPL_Packet(cmd, (unsigned char*) &bitrate, sizeof(bitrate));
				}
			}
			break;

		default:
			err = -1;
	}

	return err;
}

// deployment is done by heating a resistor that will burn out and release the ants
// we have two resistors, try the first one if failed then try the second.
// Note: we should keep trying in the maintenance loop every 30 minutes, since
// the sat might be in a dark night shadow which is cold
// complete orbit takes ~1.5 hours.
int CMD_Re_Deploy_Ants(sat_packet_t *cmd)// Tillawi
{
	(void) cmd;
	IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA, isisants_arm);
	int errA = IsisAntS_autoDeployment(ISIS_TRXVU_I2C_BUS_INDEX,  isisants_sideA, ANTENNA_DEPLOYMENT_TIMEOUT);

	//activate the other side for redundancy
	IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB, isisants_arm);
	int errB = IsisAntS_autoDeployment(ISIS_TRXVU_I2C_BUS_INDEX,  isisants_sideB, ANTENNA_DEPLOYMENT_TIMEOUT);
	return (!errA || !errB) ? 0 : errB;

}

int CMD_Upload_Time(sat_packet_t *cmd)// Danya
{
	if (cmd == NULL || cmd->data == NULL)
	{
	 return E_INPUT_POINTER_NULL;
	}
	time_unix required_time;
	memcpy(&required_time,cmd->data,sizeof(required_time));
	int err =Time_setUnixEpoch(required_time);
	if (0!= err)
	{
	 printf( "time was not set\n");
	}
	else err= FRAM_write((unsigned char*)&required_time,MOST_UPDATED_SAT_TIME_ADDR,MOST_UPDATED_SAT_TIME_SIZE);
	{ // writing new time to the FRAM
		if (0!=err)
		 {
		  # ifdef TESTING
		  printf("an error occurred updating the time in the FRAM , Error = %d \n",err);
		  #endif
		 }
	}
	return 0;

}

int CMD_Echo(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL)
	{
	 return E_INPUT_POINTER_NULL;
	}
	char reply_data[MAX_COMMAND_DATA_LENGTH] = {0};
	const char* sig = "Hello from Sat, wish you all the best Taibeh!";
	sat_packet_t reply_packet = { 0 };
	unsigned int ds = 0;
	strcpy(reply_data, sig);
	ds = strlen(sig) ;
	int	err = AssembleCommand((unsigned char*)&reply_data, ds, trxvu_cmd_type,ECHO_SUBTYPE, cmd->ID, &reply_packet);
		if (0 != err)
		{
			return err;
		}

		err = TransmitSplPacket(&reply_packet , NULL);
		if(err)
			return err;

	return 0;
}

int CMD_GetTxUptime(sat_packet_t *cmd)
{
	int err = 0;
	time_unix uptime = 0;
	err = IsisTrxvu_tcGetUptime(ISIS_TRXVU_I2C_BUS_INDEX, (unsigned int*)&uptime);
	TransmitDataAsSPL_Packet(cmd, (unsigned char*)&uptime, sizeof(uptime));

	return err;
}

int CMD_GetRxUptime(sat_packet_t *cmd)
{
	int err = 0;
	time_unix uptime = 0;
	err = IsisTrxvu_rcGetUptime(ISIS_TRXVU_I2C_BUS_INDEX,(unsigned int*) &uptime);
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &uptime, sizeof(uptime));

	return err;
}

int CMD_GetNumOfDelayedCommands(sat_packet_t *cmd)
{
	int err = 0;
	unsigned char temp = 0;
	temp = GetDelayedCommandBufferCount();
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &temp, sizeof(temp));

	return err;
}

int CMD_GetNumOfOnlineCommands(sat_packet_t *cmd)
{
	int err = 0;
	unsigned short int temp = 0;
	err = IsisTrxvu_rcGetFrameCount(ISIS_TRXVU_I2C_BUS_INDEX, &temp);
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &temp, sizeof(temp));

	return err;
}

int CMD_DeleteDelyedCmdByID(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL)
		{
		 return E_INPUT_POINTER_NULL;
		}
	int err = 0;
	unsigned int index = 0;
	memcpy(&index,cmd->data,sizeof(index));
	err = DeleteDelayedCommandByIndex(index);
	return err;
}

int CMD_DeleteAllDelyedBuffer(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	err = DeleteDelayedBuffer();
	return err;
}

int CMD_AntSetArmStatus(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL)
	{
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	ISISantsSide ant_side = cmd->data[0];

	ISISantsArmStatus status = cmd->data[1];
	err = IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX, ant_side, status);

	return err;
}

int CMD_AntGetArmStatus(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL)
	{
		 return E_INPUT_POINTER_NULL;
	}
	ISISantsSide ant_side;
	memcpy(&ant_side, cmd->data, sizeof(ant_side));


	ISISantsStatus status;
	int err = IsisAntS_getStatusData(ISIS_TRXVU_I2C_BUS_INDEX, ant_side, &status);
	if(0 == err)
	{
		//status.fields.ant1Undeployed && status.fields.ant2Deploying /// explanation about using union
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) &status, sizeof(status));
	}

	return err;
}

int CMD_ArmDisArmAnt(sat_packet_t *cmd)//TODO: need to be completed
{
	if (cmd == NULL || cmd->data == NULL)
	{
	 return E_INPUT_POINTER_NULL;
	}
	int err;
	ISISantsSide Req_Side = cmd->data[0];
	ISISantsArmStatus Req_status = cmd->data[1];
	err= IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX, Req_Side,  Req_status);
	if(0!= err)
	{
		printf("error in %s on side %s\n", (Req_status==0XAD) ? "Arm" :"Disarm",(Req_Side==0x00)? "A":"B");
	}

	return err;
}

int CMD_DisArmAnt(sat_packet_t *cmd)//TODO: need to be completed
{
	if (cmd == NULL || cmd->data == NULL)
	{
		 return E_INPUT_POINTER_NULL;
	}


	return 0;
}

int CMD_AntGetUptime(sat_packet_t *cmd)
{
	int err = 0;
	time_unix uptime = 0;
	ISISantsSide ant_side;
	memcpy(&ant_side, cmd->data, sizeof(ant_side));
	err = IsisAntS_getUptime(ISIS_TRXVU_I2C_BUS_INDEX, ant_side,(unsigned int*) &uptime);
	return err;
}

int CMD_AntCancelDeployment(sat_packet_t *cmd)
{
	int err = 0;
	ISISantsSide ant_side;
	memcpy(&ant_side, cmd->data, sizeof(ant_side));
	err = IsisAntS_cancelDeployment(ISIS_TRXVU_I2C_BUS_INDEX, ant_side);
	return err;
}

