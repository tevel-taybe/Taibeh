#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/errors.h>

#include <string.h>

#include "EPS.h"
#ifdef ISISEPS
	#include <satellite-subsystems/isis_eps_driver.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif
unsigned short threshold_bat_voltage = 6600;
// y[i] = a * x[i] +(1-a) * y[i-1]
voltage_t prev_filtered_voltage = 0;		// y[i-1]

float alpha = DEFAULT_ALPHA_VALUE;			//<! smoothing constant
EpsThreshVolt_t eps_threshold_voltages = {.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};	// saves the current EPS logic threshold voltages

int EPS_Init()
{
	ISIS_EPS_t subsystem[1]; // One instance to be initialised.// unsigned char i2c_address = EPS_I2C_ADDR;
	subsystem[0].i2cAddr = 0x20; // I2C address defined to 0x20.
	int rv;
#ifdef ISISEPS
	rv = ISIS_EPS_Init( subsystem, 1);//rv=ISIS_EPS_Init(&i2c_address,1);//rv = IsisEPS_initialize(&i2c_address, 1);
#endif
#ifdef GOMEPS
	rv = GomEpsInitialize(&i2c_address, 1);
#endif


	if (rv != E_NO_SS_ERR) {
		return -1;
	}
	rv = IsisSolarPanelv2_initialize(slave0_spi);
	if (rv != 0) {
		return -2;
	}
	IsisSolarPanelv2_sleep();

	rv = GetThresholdVoltages(&eps_threshold_voltages);
	if (0 != rv) {
		return -3;
	}

	rv= GetAlpha(&alpha);
	if (0 != rv) {
		alpha = DEFAULT_ALPHA_VALUE;
		return -4;
	}
	prev_filtered_voltage = 0;	//y[i-1]
	GetBatteryVoltage(&prev_filtered_voltage);

	EPS_Conditioning();

	return 0;
}
#define GetFilterdVoltage(curr_voltage) (voltage_t) (alpha * curr_voltage + (1 - alpha) * prev_filtered_voltage)
///   make sure we still need all the above code
int EPS_Conditioning()
{
	voltage_t curr_voltage = 0;				// x[i]
	GetBatteryVoltage(&curr_voltage);

	voltage_t filtered_voltage = 0;					// the currently filtered voltage; y[i]

	filtered_voltage = GetFilterdVoltage(curr_voltage);

	if (curr_voltage > threshold_bat_voltage )
	{
				EnterSafeMode();
	}
	else
	{
		EnterCriticalMode();
	}


	// discharging
	/*
	if (filtered_voltage > prev_filtered_voltage) {
		if (filtered_voltage < eps_threshold_voltages.fields.Vdown_safe) {
			EnterCriticalMode();
		}
		else if (filtered_voltage < eps_threshold_voltages.fields.Vdown_cruise) {
			EnterSafeMode();
		}
		else if (filtered_voltage < eps_threshold_voltages.fields.Vdown_full) {
			EnterCruiseMode();
		}

	}
	// charging
	else if (filtered_voltage > prev_filtered_voltage) {

		if (filtered_voltage > eps_threshold_voltages.fields.Vup_full) {
			EnterFullMode();
		}
		else if (filtered_voltage > eps_threshold_voltages.fields.Vup_cruise) {
			EnterCruiseMode();
		}
		else if (filtered_voltage > eps_threshold_voltages.fields.Vup_safe) {
			EnterSafeMode();
		}
	}*/
	prev_filtered_voltage = filtered_voltage;
	return 0;
}

int GetBatteryVoltage(voltage_t *vbatt)
{
	int err = 0;
#ifdef ISISEPS
	//isis_eps__gethousekeepingengincdb__from_t
	isis_eps__gethousekeepingengincdb__from_t hk_tlm;//ieps_enghk_data_cdb_t hk_tlm;isis_eps__gethousekeepingengincdb__from_t hk_tlm;
	//ieps_statcmd_t cmd;
	//ieps_board_t board = ieps_board_cdb1;
	err = isis_eps__gethousekeepingengincdb__tm(EPS_I2C_BUS_INDEX, &hk_tlm);//err = IsisEPS_getEngHKDataCDB(EPS_I2C_BUS_INDEX, board, &hk_tlm, &cmd);
	*vbatt= hk_tlm.fields.batt_input.fields.volt; //*vbatt = hk_tlm.fields.bat_voltage;// to check with Adi if the bat voltage is the same as hk_tlm.fields.batt_input.fields.volt
	return 0;
	#endif
#ifdef GOMEPS
	gom_eps_hk_t hk_tlm;
	err = GomEpsGetHkData_general(EPS_I2C_BUS_INDEX,&hk_tlm);
	*vbatt = hk_tlm.fields.vbatt;
#endif
	return err;
}

int UpdateAlpha(float new_alpha)
{
	if (new_alpha > 1 || new_alpha < 0) {
		return -2;
	}
	int err = FRAM_write((unsigned char*) &new_alpha,
			EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE);
	if (0 != err) {
		return err;
	}

	alpha = new_alpha;
	return 0;
}

int UpdateThresholdVoltages(EpsThreshVolt_t *thresh_volts)
{
	if (NULL == thresh_volts) {
		return E_INPUT_POINTER_NULL;
	}

	Boolean valid_dependancies = (thresh_volts->fields.Vup_safe 	< thresh_volts->fields.Vup_cruise
	                           && thresh_volts->fields.Vup_cruise	< thresh_volts->fields.Vup_full);

	Boolean valid_regions = (thresh_volts->fields.Vdown_full 	< thresh_volts->fields.Vup_full)
						&&  (thresh_volts->fields.Vdown_cruise	< thresh_volts->fields.Vup_cruise)
						&&  (thresh_volts->fields.Vdown_safe	< thresh_volts->fields.Vup_safe);

	if (!(valid_dependancies && valid_regions)) {
		return -2;
	}
	int err = FRAM_write((unsigned char*) thresh_volts,
			EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
	if (0 != err) {
		return err;
	}
	memcpy(eps_threshold_voltages.raw, thresh_volts, EPS_THRESH_VOLTAGES_SIZE);
	return E_NO_SS_ERR;
}

int GetThresholdVoltages(EpsThreshVolt_t *thresh_volts)
{
	if (NULL == thresh_volts) {
		return E_INPUT_POINTER_NULL;
	}
	int err = FRAM_read((unsigned char*) thresh_volts, EPS_THRESH_VOLTAGES_ADDR,
			EPS_THRESH_VOLTAGES_SIZE);
	return err;
}

int GetAlpha(float *alpha)
{
	if (NULL == alpha) {
		return E_INPUT_POINTER_NULL;
	}
	int err = FRAM_read((unsigned char*) alpha, EPS_ALPHA_FILTER_VALUE_ADDR,
			EPS_ALPHA_FILTER_VALUE_SIZE);
	return err;
}

int RestoreDefaultAlpha()
{
	int err = 0;
	float def_alpha = DEFAULT_ALPHA_VALUE;
	err = UpdateAlpha(def_alpha);
	return err;
}

int RestoreDefaultThresholdVoltages() //TODO: need to be updated according to our mechanism
{
	int err = 0;
	EpsThreshVolt_t def_thresh =
	{.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
	err = UpdateThresholdVoltages(&def_thresh);
	return err;
}

