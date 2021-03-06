#ifndef TELEMETRYCOLLECTOR_H_
#define TELEMETRYCOLLECTOR_H_
#include <stdint.h>
#include "GlobalStandards.h"
#include "TelemetryFiles.h"
#include "TLM_management.h"

typedef enum{
	eps_tlm,
	trxvu_tlm,
	ant_tlm,
	solar_panel_tlm,
	wod_tlm,
	log_tlm
	//operational_data_tlm
}subsystem_tlm;

typedef struct __attribute__ ((__packed__)) OperationalData
{
	voltage_t vbat;					///< the current voltage on the battery [mV]
	current_t electric_current;		///< the up-to-date electric current of the battery [mA]
	current_t current_3V3;			///< the up-to-date 3.3 Volt bus current of the battery [mA]
	current_t current_5V;			///< the up-to-date 5 Volt bus current of the battery [mA]
	int16_t temp2; /*!< 2 and 4 cell battery pack: Battery pack temperature in between the center battery cells. */
	int16_t temp3; /*!< 2 cell battery pack: not used 4 cell battery pack: Battery pack temperature on the front of the battery pack. */
	long    data[6]; /*array to hold all 6 panel temprature. */


} OperationalData_t;

typedef struct __attribute__ ((__packed__)) WOD_Telemetry_t
{
	time_unix sat_time;				///< current Unix time of the satellites clock [sec]
	voltage_t vbat;					///< the current voltage on the battery [mV]
	voltage_t volt_5V;				///< the current voltage on the 5V bus [mV]
	voltage_t volt_3V3;				///< the current voltage on the 3V3 bus [mV]
	power_t charging_power;			///< the current charging power [mW]
	power_t consumed_power;			///< the power consumed by the satellite [mW]
	current_t electric_current;		///< the up-to-date electric current of the battery [mA]
	current_t current_3V3;			///< the up-to-date 3.3 Volt bus current of the battery [mA]
	current_t current_5V;			///< the up-to-date 5 Volt bus current of the battery [mA]

	unsigned int free_memory;		///< number of bytes free in the satellites SD [byte]
	unsigned int corrupt_bytes;		///< number of currpted bytes in the memory	[bytes]
	unsigned short number_of_resets;///< counts the number of resets the satellite has gone through [#]

	temp_t bat_temp ;
	temp_t solar_panels[NUMBER_OF_SOLAR_PANELS];
	temp_t mcu_temp ;

} WOD_Telemetry_t;

#define NUMBER_OF_TELEMETRIES 11	///< number of telemetries the satellite saves
#define NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS 5			///<
/*!
 * @brief copies the corresponding filename into a buffer.
 * @return	-1 on NULL input
 * 			-2 on unknown  tlm_type
 */
int GetTelemetryFilenameByType(tlm_type tlm_type,char filename[MAX_F_FILE_NAME_SIZE]);


/*!
 * @brief Creates all telemetry files,
 * @param[out]	tlms_created states whether the files were created successful
 */
void TelemetryCreateFiles(Boolean8bit tlms_created[NUMBER_OF_TELEMETRIES]);

/*!
 * @brief saves all telemetries into the appropriate TLM files
 */
void TelemetryCollectorLogic();

/*!
 *  @brief saves current EPS telemetry into file
 */
void TelemetrySaveEPS();

/*!
 *  @brief saves current TRXVU telemetry into file
 */
void TelemetrySaveTRXVU();

/*!
 *  @brief saves current Antenna telemetry into file
 */
void TelemetrySaveANT();

/*!
 *  @brief saves current solar panel telemetry into file
 */
void TelemetrySaveSolarPanels();

/*!
 *  @brief saves current WOD telemetry into file
 */
void TelemetrySaveWOD();

/*!
 * @brief Gets all necessary telemetry and arranges it into a WOD structure
 * @param[out] output WOD telemetry. If an error occurred while getting TLM the fields will be zero
 */
void GetCurrentWODTelemetry(WOD_Telemetry_t *wod);

#endif /* TELEMETRYCOLLECTOR_H_ */
