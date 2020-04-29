
#include "EPSOperationModes.h"
#include "GlobalStandards.h"

#ifdef ISISEPS
	#include <satellite-subsystems/isis_eps_driver.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif


// after changing the EPS conditioning few of the below states are not need ( only critical and safe mode are relevant )
channel_t g_system_state;
EpsState_t state;
Boolean g_low_volt_flag = FALSE; // this flag get "True" when battery is low

int EnterFullMode()// check with Adi if we can delete this section , to make sure it does not affect and other parts in the code
{
	if(state == FullMode)
	{
		return 0;
	}
	//int err = SetEPS_Channels((channel_t) CHANNELS_OFF);
	state = FullMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterCruiseMode() // check with Adi if we can delete this section , to make sure it doesnt affect and other parts in the code
{
	if(state == CruiseMode)
	{
		return 0;
	}
	//int err = SetEPS_Channels((channel_t) CHANNELS_OFF);
	state = CruiseMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterSafeMode()
{
	if(state == SafeMode)
	{
	 return 0;
	}
	//int err = SetEPS_Channels((channel_t) CHANNELS_OFF);
	state = SafeMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterCriticalMode()
{
	if(state == CriticalMode)
	{
	 return 0;
	}
	//int err = SetEPS_Channels((channel_t) CHANNELS_OFF);
	state = CriticalMode;
	EpsSetLowVoltageFlag(TRUE);
	return 0;
}

// since all components are connected to permanent channels all the below code was commented //////
//int SetEPS_Channels(channel_t channel)
//{
//	(void)channel;
//#ifdef ISISEPS
//	ieps_statcmd_t code;
//	ieps_obus_channel_t chnl;
//	chnl.raw = g_system_state;
//	int err = IsisEPS_outputBusGroupOn(EPS_I2C_BUS_INDEX, chnl, chnl, &code);
//	if (err != 0){
//		return err;
//	}
//	g_system_state = channel;
//
//	chnl.raw = ~g_system_state;	//flip all bits in 'system_state'
//	err = IsisEPS_outputBusGroupOff(EPS_I2C_BUS_INDEX, chnl, chnl, &code);
//	if (err != 0){
//		return err;
//	}
//	return 0;
//#endif
//#ifdef GOMEPS
//#ifdef SET_EPS_CHANNELS
//	gom_eps_channelstates_t st = {0};
//	st.raw = channel;
//	int err = GomEpsSetOutput(EPS_I2C_BUS_INDEX,st);
//	if(0 != err){
//		return err;
//	}
//	g_system_state = channel;
//	return err;
//#endif
//#endif
//	return 0;
//}

EpsState_t GetSystemState()
{
	return state;
}

channel_t GetSystemChannelState()
{
	return g_system_state;
}

Boolean EpsGetLowVoltageFlag()
{
	return g_low_volt_flag;
}

void EpsSetLowVoltageFlag(Boolean low_volt_flag)
{
	g_low_volt_flag = low_volt_flag;
}

