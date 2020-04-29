
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
	ACK_RECEIVE_COMM = 		0x00,			// when receive any packet

	ACK_RESET_WAKEUP = 		0x7F,			// after waking up from reset

	ACK_FS_RESET = 			0x70,			// before going to filesystem reset
	ACK_TRXVU_SOFT_RESET = 	0x71,			// before going to trxvu reset
	ACK_TRXVU_HARD_RESET =	0x72,			// before going to trxvu reset
	ACK_ANTS_RESET = 		0x73,			// before reseting antennas
	ACK_EPS_RESET = 		0x80,			// before going to eps reset
	ACK_SOFT_RESET = 		0x81,			// before going to reset
	ACK_HARD_RESET = 		0x82,			// before starting hardware reset

	ACK_MEMORY_DELETE = 0x83,				// when memory delete is completed success

	ACK_UPDATE_TIME = 0x12,					// after updating time
	ACK_UPDATE_BEACON_BIT_RATE = 0x13,
	ACK_UPDATE_BEACON_TIME_DELAY = 0x14,
	ACK_UPDATE_EPS_VOLTAGES = 0x15,
	ACK_UPDATE_EPS_HEATER_VALUES = 0x16,
	ACK_UPDATE_EPS_ALPHA = 0x17,

	ACK_MUTE = 0x8D,
	ACK_UNMUTE = 0x8E,
	ACK_ALLOW_TRANSPONDER = 0x8F,

	ACK_DUMP_START = 0x90,
	ACK_DUMP_ABORT = 0x91,
	ACK_DUMP_FINISHED = 0x92,

	ACK_GENERIC_I2C_CMD = 0x93,
	ACK_ARM_DISARM = 0x94,					//after changing arm state of the ants
	ACK_REDEPLOY = 0x95,
	ACK_RESET_DELAYED_CMD = 0x9E,
	ACK_FRAM_RESET = 0xA0,

	ACK_PING = 0xAA,
	ACK_UNKNOWN_SUBTYPE = 0xBB,				//when the given subtype is unknown
	ACK_ERROR_MSG = 0XFF 					// send this ACK when error has occurred
}ack_subtype_t;


typedef enum __attribute__ ((__packed__)) trxvu_subtypes_t
{
	BEACON_SUBTYPE =		0x01,	//0b00000001
	MUTE_TRXVU = 			0x11,	//0b00010001
	UNMUTE_TRXVU = 			0x88,	//0b10001000
	DUMP_SUBTYPE =			0x69,	//0b01101001
	ABORT_DUMP_SUBTYPE= 	0x22,	//0b00100010
	DUMP_STOP_SUBTYPE= 		0x26,
	GET_BAUD_RATE = 		0x13,	//0b00010011
	GET_BEACON_INTERVAL = 	0x23,	//0b00100011
	SET_BEACON_INTERVAL = 	0x24,	//0b00100100
	SET_BAUD_RATE = 		0x15,	//0b00010101
	SET_BEACON_CYCLE_TIME =	0x25,	//0b00100101
	GET_TX_UPTIME = 		0x66,	//0b01100110
	GET_RX_UPTIME = 		0x68,	//0b01101000
	ADD_DELAYED_COMMAND_CMD=0xA0,	//0b10100000
	GET_NUM_OF_DELAYED_CMD= 0xA1,	//0b10100001
	DELETE_DELAYED_CMD	=	0xA2,	//0b10100010
	DELETE_ALL_DELAYED_CMD= 0xA5,	//0b10100101
	GET_NUM_OF_ONLINE_CMD = 0xA7,	//0b10100111
	ANT_SET_ARM_STATUS = 	0xB0,	//0b10110000
	ANT_GET_ARM_STATUS = 	0xB2,	//0b10110010
	ANT_GET_UPTIME =		0xB3,	//0b10110011
	ARM_ANT            =    0XB4,
	DIS_ARM_ANT         =    0XB5,
	SET_ARM_DISARM_ANTS_STATUS = 0xB6,
	ANT_CANCEL_DEPLOY = 	0xB7,	//0b10110111
	UPLOAD_TIME =       0xBB, 		//0b10111011
	FORCE_ABORT_DUMP_SUBTYPE = 0x33,//0b00110011
	DELETE_DUMP_TASK = 0x44,		//0b00100010
	RE_DEPLOY_ANTS = 0X5E,
	ECHO_SUBTYPE = 0x5c

}trxvu_subtypes_t;


typedef enum __attribute__ ((__packed__)) eps_subtypes_t
{
	EPS_NOP_Operation		= 	0x10, //0b00010000
	Get_EPS_Mode			=	0x11, //0b00010001
	EPS_Reset_WDT			=	0x12, //0b00010010
	Solar_Panel_Wakup		=	0x13, //0b00010011
	Solar_Panel_Sleep		=	0x14, //0b00010100
	Solar_panel_Get_State	=	0x15, //0b00010101
	Set_Heater_Temp			=	0x16, //0b00010110

	LAST_eps
}eps_subtypes_t;


typedef enum __attribute__ ((__packed__)) telemetry_subtypes_t
{
	Get_Telemetry_All 					= 0xC9,
    Get_Telemetry_AllREVC			    = 0xC8,
    Get_General_Telemetry			    = 0xc7,
    Eps_Get_Online_TLM					= 0xc6,
    Solar_Panel_Get_Online_Temperature	= 0xc5,

    LAST_TLM
}telemetry_subtypes_t;


typedef enum __attribute__ ((__packed__)) management_subtypes_t
{
	SOFT_RESET_SUBTYPE = 		0xAA,		//0b10101010
	HARD_RESET_SUBTYPE = 		0x99,		//0b10101010
	TRXVU_SOFT_RESET_SUBTYPE =	0xC3,		//0b11000011
	TRXVU_HARD_RESET_SUBTYPE = 	0x3C,		//0b00111100
	EPS_RESET_SUBTYPE =			0xBB,		//0b10111011
	FS_RESET_SUBTYPE =			0xCC		//0b11001100
}management_subtypes_t;

typedef enum __attribute__ ((__packed__)) maintenance_subtypes_t
{
	GENERIC_I2C = 	 	0xAA,		//0b10101010

//	Satellite Hard Reset
//	Satellite Soft Reset
//	FRAM read
//	FRAM write
//	FRAM start
//	FRAM stop
//	FRAM get Device ID
//	Update Sat Time
//	Get Sat Time
//	Get Sat Uptime
//	Hard TRXVU Reset
//	Soft TRXVU Reset
//	Soft TRXVU Component Reset
//	Hard TRXVU Component Reset
//	Antenna Deploy
//	Turn off antenna autodeploy flag
//	Antenna Hard Reset
//	Antenna Soft Reset
//	EPS Hard Reset
//	FS reset
//	Dummy Func


	LAST_XXX
}maintenance_subtypes_t;
//-----------------

#endif /* SPL_H_ */
