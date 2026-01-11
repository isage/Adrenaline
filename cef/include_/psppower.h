/*
 * PSP Software Development Kit - https://github.com/pspdev
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * psppower.h - Prototypes for the scePower library.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 * Copyright (c) 2005 David Perry <tias_dp@hotmail.com>
 *
 */
#ifndef __POWER_H__
#define __POWER_H__

#include <pspkerneltypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Power callback flags
 */
typedef enum ScePowerCallbackFlags {
	/* Indicates the power switch it pushed, putting the unit into suspend mode */
	PSP_POWER_CB_POWER_SWITCH = 0x80000000,
	/* Indicates the hold switch is on */
	PSP_POWER_CB_HOLD_SWITCH = 0x40000000,
	/* What is standby mode? */
	PSP_POWER_CB_STANDBY  = 0x00080000,
	/* Indicates the resume process has been completed (only seems to be triggered when another event happens) */
	PSP_POWER_CB_RESUME_COMPLETE = 0x00040000,
	/* Indicates the unit is resuming from suspend mode */
	PSP_POWER_CB_RESUMING  = 0x00020000,
	/* Indicates the unit is suspending, seems to occur due to inactivity */
	PSP_POWER_CB_SUSPENDING  = 0x00010000,
	/* Indicates the unit is plugged into an AC outlet */
	PSP_POWER_CB_AC_POWER  = 0x00001000,
	/* Indicates the battery charge level is low */
	PSP_POWER_CB_BATTERY_LOW = 0x00000100,
	/* Indicates there is a battery present in the unit */
	PSP_POWER_CB_BATTERY_EXIST = 0x00000080,
	/*unknown*/
	PSP_POWER_CB_BATTPOWER  = 0x0000007F,
} ScePowerCallbackFlags;


/**
 * Power tick flags
 */
typedef enum ScePowerTickFlags {
	/* All */
	PSP_POWER_TICK_ALL = 0,
	/* Suspend */
	PSP_POWER_TICK_SUSPEND = 1,
	/* Display */
	PSP_POWER_TICK_DISPLAY = 6,
} ScePowerTickFlags;

/**
 * Power Callback Function Definition
 *
 * @param unknown - unknown function, appears to cycle between 1,2 and 3
 * @param powerInfo - combination of PSP_POWER_CB_ flags
 */
typedef void (*powerCallback_t)(int unknown, int powerInfo);

/**
 * Register Power Callback Function
 *
 * @param slot - slot of the callback in the list, 0 to 15, pass -1 to get an auto assignment.
 * @param cbid - callback id from calling sceKernelCreateCallback
 *
 * @return 0 on success, the slot number if -1 is passed, < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerRegisterCallback(int slot, SceUID cbid);

/**
 * Unregister Power Callback Function
 *
 * @param slot - slot of the callback
 *
 * @return 0 on success, < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerUnregisterCallback(int slot);

/**
 * Check if unit is plugged in
 *
 * @return 1 if plugged in, 0 if not plugged in, < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerIsPowerOnline(void);

/**
 * Check if a battery is present
 *
 * @return 1 if battery present, 0 if battery not present, < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerIsBatteryExist(void);

/**
 * Check if the battery is charging
 *
 * @return 1 if battery charging, 0 if battery not charging, < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerIsBatteryCharging(void);

/**
 * Get the status of the battery charging
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetBatteryChargingStatus(void);

/**
 * Check if the battery is low
 *
 * @return 1 if the battery is low, 0 if the battery is not low, < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerIsLowBattery(void);

/**
 * Check if a suspend is required
 *
 * @return 1 if suspend is required, 0 otherwise
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerIsSuspendRequired(void);

/**
 * Returns battery remaining capacity
 *
 * @return battery remaining capacity in mAh (milliampere hour)
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetBatteryRemainCapacity(void);

/**
 * Returns battery full capacity
 *
 * @return battery full capacity in mAh (milliampere hour)
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetBatteryFullCapacity(void);

/**
 * Get battery life as integer percent
 *
 * @return Battery charge percentage (0-100), < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetBatteryLifePercent(void);

/**
 * Get battery life as time
 *
 * @return Battery life in minutes, < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetBatteryLifeTime(void);

/**
 * Get temperature of the battery
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetBatteryTemp(void);

/**
 * unknown? - crashes PSP in usermode
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetBatteryElec(void);

/**
 * Get battery volt level
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetBatteryVolt(void);

/**
 * Set CPU Frequency
 *
 * @param cpufreq - new CPU frequency, valid values are 1 - 333
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerSetCpuClockFrequency(int cpufreq);

/**
 * Set Bus Frequency
 *
 * @param busfreq - new BUS frequency, valid values are 1 - 167
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerSetBusClockFrequency(int busfreq);

/**
 * Get CPU Frequency as Integer
 *
 * @return frequency as int
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetCpuClockFrequencyInt(void);

/**
 * Get CPU Frequency as Float
 *
 * @return frequency as float
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
float scePowerGetCpuClockFrequencyFloat(void);

/**
 * Get Bus frequency as Integer
 *
 * @return frequency as int
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetBusClockFrequencyInt(void);

/**
 * Get Bus frequency as Float
 *
 * @return frequency as float
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
float scePowerGetBusClockFrequencyFloat(void);

/**
 * Set Clock Frequencies
 *
 * @param pllfreq - pll frequency, valid from 19-333
 * @param cpufreq - cpu frequency, valid from 1-333
 * @param busfreq - bus frequency, valid from 1-167
 *
 * and:
 *
 * cpufreq <= pllfreq
 * busfreq*2 <= pllfreq
 *
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerSetClockFrequency(int pllfreq, int cpufreq, int busfreq);

/**
 * Lock power switch
 *
 * Note: if the power switch is toggled while locked
 * it will fire immediately after being unlocked.
 *
 * @param unknown - pass 0
 *
 * @return 0 on success, < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerLock(int unknown);

/**
 * Unlock power switch
 *
 * @param unknown - pass 0
 *
 * @return 0 on success, < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerUnlock(int unknown);

/**
 * Generate a power tick, preventing unit from
 * powering off and turning off display.
 *
 * @param type - Either PSP_POWER_TICK_ALL, PSP_POWER_TICK_SUSPEND or PSP_POWER_TICK_DISPLAY
 *
 * @return 0 on success, < 0 on error.
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerTick(int type);

/**
 * Get Idle timer
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerGetIdleTimer(void);

/**
 * Enable Idle timer
 *
 * @param unknown - pass 0
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerIdleTimerEnable(int unknown);

/**
 * Disable Idle timer
 *
 * @param unknown - pass 0
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerIdleTimerDisable(int unknown);

/**
 * Request the PSP to go into standby
 *
 * @return 0 always
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerRequestStandby(void);

/**
 * Request the PSP to go into suspend
 *
 * @return 0 always
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerRequestSuspend(void);

/**
 * Request the PSP to do a cold reset.
 *
 * @param a0 - Unknown
 *
 * @return < 0 on error
 *
 * @attention Needs to link to `psppower_driver` or `psppower` stub.
 */
int scePowerRequestColdReset(int a0);

#ifdef __USER__
/**
 * Alias for scePowerGetBusClockFrequencyInt
 *
 * @return frequency as int
 *
 * @attention Needs to link to `psppower` stub.
 */
int scePowerGetBusClockFrequency(void);

/**
 * Alias for scePowerGetCpuClockFrequencyInt
 *
 * @return frequency as int
 *
 * @attention Needs to link to `psppower` stub.
 */
int scePowerGetCpuClockFrequency(void);

#endif // __USER__

#ifdef __KERNEL__

/**
 * @attention Needs to link to `psppower_driver`.
 */
int scePowerSetIdleCallback(int slot, int flags, u64 time, int (* callback)(int slot, u32 diff, int arg, int *unk), int arg);

#endif // __KERNEL__


#ifdef __cplusplus
}
#endif

#endif
