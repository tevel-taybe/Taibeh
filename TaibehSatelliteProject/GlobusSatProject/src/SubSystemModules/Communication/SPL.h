
#ifndef SPL_H_
#define SPL_H_

typedef enum __attribute__ ((__packed__)) spl_command_type_t {
	trxvu_cmd_type,
	eps_cmd_type,
	telemetry_cmd_type,
	filesystem_cmd_type,
	managment_cmd_type,
	maintenace_cmd_type,
	ack_type
}spl_command_type;

//TODO: set all sub-types to be different from one another

typedef enum __attribute__ ((__packed__)) ack_subtype_t
{
	ACK_RECEIVE_COMM = 			    0x01,			// when receive any packet
	ACK_RESET_WAKEUP = 			    0x02,			// after waking up from reset
	ACK_FS_RESET = 				    0x03,			// before going to filesystem reset
	ACK_TRXVU_SOFT_RESET = 		    0x04,			// before going to trxvu reset
	ACK_TRXVU_HARD_RESET =		    0x05,			// before going to trxvu reset
	ACK_ANTS_RESET = 			    0x06,			// before reseting antennas
	ACK_EPS_RESET = 			    0x07,			// before going to eps reset
	ACK_SOFT_RESET = 			    0x08,			// before going to reset
	ACK_HARD_RESET = 		        0x09,			// before starting hardware reset
	ACK_MEMORY_DELETE =             0x10,			// when memory delete is completed success
	ACK_UPDATE_TIME =               0x11,			// after updating time
	ACK_UPDATE_BEACON_BIT_RATE = 	0x12,
	ACK_UPDATE_BEACON_TIME_DELAY =  0x14,
	ACK_UPDATE_EPS_VOLTAGES =       0x15,
	ACK_UPDATE_EPS_HEATER_VALUES =  0x16,
	ACK_UPDATE_EPS_ALPHA =          0x17,
	ACK_MUTE =					    0x18,
	ACK_UNMUTE = 				    0x19,
	ACK_ALLOW_TRANSPONDER =         0x20,
	ACK_DUMP_START = 				0x21,
	ACK_DUMP_ABORT =  			    0x22,
	ACK_DUMP_FINISHED = 			0x23,
	ACK_GENERIC_I2C_CMD = 			0x24,
	ACK_ARM_DISARM = 				0x25,			//after changing arm state of the ants
	ACK_REDEPLOY = 					0x26,
	ACK_RESET_DELAYED_CMD = 		0x27,
	ACK_FRAM_RESET =			    0x28,
	ACK_PING =      			    0x29,
	ACK_UNKNOWN_SUBTYPE = 			0x30,			//when the given subtype is unknown
	ACK_ERROR_MSG = 				0X31, 			// send this ACK when error has occurred
	ACK_CMD_DONE = 					0x32,			// once command executed
	LAST_ACK				// send this ACK when error has occurred
}ack_subtype_t;


typedef enum __attribute__ ((__packed__)) trxvu_subtypes_t
{
	BEACON_SUBTYPE =		  	 0x01,	//0b00000001
	MUTE_TRXVU = 			     0x02,	//0b00000010
	UNMUTE_TRXVU = 			     0x03,	//0b00000011
	DUMP_SUBTYPE =			     0x04,	//0b00000100
	ABORT_DUMP_SUBTYPE= 	     0x05,	//0b00100010
	GET_BAUD_RATE = 		     0x06,	//0b00000110
	GET_BEACON_INTERVAL = 	     0x07,	//0b00000111
	SET_BEACON_INTERVAL = 	     0x08,	//0b00001000
	SET_BAUD_RATE = 		     0x09,	//0b00011001
	SET_BEACON_CYCLE_TIME =	     0x10,	//0b00001010
	GET_BEACON_CYCLE_TIME =      0x37,
	GET_TX_UPTIME = 		     0x11,	//0b00010001
	GET_RX_UPTIME = 		     0x12,	//0b00010010
	ADD_DELAYED_COMMAND_CMD=     0x13,	//0b00010011
	GET_NUM_OF_DELAYED_CMD=      0x14,	//0b00010100
	DELETE_DELAYED_CMD	=	     0x15,	//0b00010101
	DELETE_ALL_DELAYED_CMD=      0x16,	//0b00010110
	GET_NUM_OF_ONLINE_CMD =      0x17,	//0b00010111
	ANT_SET_ARM_STATUS = 	     0x18,	//0b00011000
	ANT_GET_ARM_STATUS = 	     0x19,	//0b00011001
	ANT_GET_UPTIME =		     0x20,	//0b00100000
	ANT_CANCEL_DEPLOY = 	     0x21,	//0b00100001
	ANT_AUTO_DEPLOY = 			 0x38,
	FORCE_ABORT_DUMP_SUBTYPE =   0x22,  //0b00100010
	DELETE_DUMP_TASK =           0x23,	//0b00100011
	ECHO_SUBTYPE =               0x24,
	SET_BEACON_BAUD_RATE =       0x25,
	SET_TRANSPONDER_STATUS =     0x26,
	SET_TRANSPONDER_MIN_RSSI=    0x27,
	TRXVU_IDLE	=   		     0x28,	//0b000101000
	DUMP_STOP_SUBTYPE= 		     0x29,
	ARM_ANT            =         0X30,
	DIS_ARM_ANT         =        0X31,
	SET_ARM_DISARM_ANTS_STATUS = 0x32,
	UPLOAD_TIME =      			 0x33, 	//0b00110011
	RE_DEPLOY_ANTS = 			 0X34,
	Read_From_FRAM =             0X35,
	Write_to_Fram =              0x36,
	LAST_trxvu

}trxvu_subtypes_t;


typedef enum __attribute__ ((__packed__)) eps_subtypes_t
{
	Get_EPS_Raw_MBoard = 			0x01, // Get housekeeping data for the mainboard in raw values. siham
	Get_EPS_Eng_MBoard = 			0x02, // Get housekeeping data for the mainboard in engineering values. maisan
	Get_EPS_RAverega_Eng_MBoard = 	0x03, //Get running-average housekeeping data for the mainboard in engineering values. azal
	Get_EPS_RAW_DBoard = 			0x04, //Get housekeeping data for the companion/daughterboard (cdb) in raw values. jana
	EPS_RAverage_DBoard =			0x05, //mhmd.T
	Get_EPS_EngHK_CDBoard = 		0x06, //Get running-average housekeeping data for the companion/daughterboard (cdb) in engineering values. mhmd.j
	Get_EPS_Mode = 					0x07,
	EPS_Reset_WDT = 				0x08,
	Solar_Panel_Wakeup =			0x09,
	Solar_panel_Sleep =				0x10,
	Solar_Panel_Get_State = 		0x11,
	Set_Heater_Temp =   			0x12,
	EPS_NOP_Operation =				0x13,
	EPS_Reset_WTD     =             0x14,
	LAST_eps
}eps_subtypes_t;


typedef enum __attribute__ ((__packed__)) telemetry_subtypes_t
{
	Get_Telemetry_All = 					0x01,
    Get_Telemetry_AllREVC = 				0x02,
    Get_EPS_Online_TLM =    				0x03,
    Get_Solar_Panel_Online_Temperature = 	0x04,
    LAST_TLM
}telemetry_subtypes_t;


typedef enum __attribute__ ((__packed__)) management_subtypes_t
{
	SOFT_RESET_SUBTYPE = 		0x01,		//0b10101010
	HARD_RESET_SUBTYPE = 		0x02,		//0b10101010
	TRXVU_SOFT_RESET_SUBTYPE =	0x03,		//0b11000011
	TRXVU_HARD_RESET_SUBTYPE = 	0x04,		//0b00111100
	EPS_RESET_SUBTYPE =			0x05,		//0b10111011
	FS_RESET_SUBTYPE =			0x06,		//0b11001100
	LAST_management
}management_subtypes_t;

typedef enum __attribute__ ((__packed__)) maintenance_subtypes_t
{
	Satellite_Hard_Reset=				0x01,
	Satellite_Soft_Reset=				0x02,
	FRAM_get_Device_ID=					0x07,
	Update_Sat_Time= 					0x08,
	Get_Sat_Time=						0x09,
	Get_Sat_Uptime=						0x10,
	Hard_TRXVU_Reset=					0x11,
	Soft_TRXVU_Reset=					0x12,
	Soft_TRXVU_Component_Reset=			0x13,
	Hard_TRXVU_Component_Reset= 		0x14,
	Antenna_Deploy=						0x15,
	Turn_off_antenna_autodeploy_flag= 	0x16,
	Antenna_Hard_Reset=					0x17,
	Antenna_Soft_Reset=					0x18,
	EPS_Hard_Reset=						0x19,
	FS_reset=							0x20,
	Dummy_Func=							0x21,
	I2C_Generic_CMD = 					0x22, //0b10101010
	Restart_Fram_CMD =					0x23,
	LAST_maintenance

}maintenance_subtypes_t;
//-----------------

#endif /* SPL_H_ */
