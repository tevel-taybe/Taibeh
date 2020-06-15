
#ifndef TELEMETRYFILES_H_
#define TELEMETRYFILES_H_

//	---general
#define FILENAME_WOD_TLM				"wod"
#define FILENAME_Operational_DATA_TLM	"OprData"

//	---eps
#define	FILENAME_EPS_RAW_MB_TLM			"eRwMB"
#define FILENAME_EPS_ENG_MB_TLM			"eEgMB"
#define FILENAME_EPS_RAW_CDB_TLM		"eRwCdb"
#define FILENAME_EPS_ENG_CDB_TLM		"eE_Cdb"
#define	FILENAME_SOLAR_PANELS_TLM		"slrPnl"

//	---trxvu
#define FILENAME_TX_TLM					"tx"
#define FILENAME_TX_REVC				"txRevc"
#define FILENAME_RX_TLM					"rx"
#define FILENAME_RX_REVC				"rxRevC"
#define FILENAME_RX_FRAME				"rxFrame"
#define FILENAME_ANTENNA_TLM			"ant"
#define FILENAME_LOG_TLM    			"LOG"

typedef enum {
	tlm_wod,			//0x00
	tlm_eps_raw_mb,		//0x01
	tlm_eps_eng_mb,		//0x02
	tlm_eps_raw_cdb,	//0x03
	tlm_eps_eng_cdb,	//0x04
	tlm_solar,			//0x05
	tlm_tx,				//0x06
	tlm_tx_revc,		//0x07
	tlm_rx,				//0x08
	tlm_rx_revc,		//0x09
	tlm_rx_frame,		//0x0A
	tlm_antenna,		//0x0b
	//tlm_operation_data,
	tlm_log_file		//0x0c

}tlm_type;
#endif /* TELEMETRYFILES_H_ */
