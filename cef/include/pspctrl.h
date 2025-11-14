/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspctrl.h - Prototypes for the sceCtrl library.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: pspctrl.h 2433 2008-10-15 10:00:27Z iwn $
 */

/* Note: Some of the structures, types, and definitions in this file were
   extrapolated from symbolic debugging information found in the Japanese
   version of Puzzle Bobble. */

#ifndef __CTRL_H__
#define __CTRL_H__
// Protect from double definition of mixing our modified header with pspsdk header
#define __CTRL_KERNEL_H__

#include <psptypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup Ctrl Controller Kernel Library */
/*@{*/

/** The callback function used by ::sceCtrlSetSpecialButtonCallback(). */
typedef void (*SceKernelButtonCallbackFunction)(u32 cur_buttons, u32 last_buttons, void *opt);

/**
 * Enumeration for the digital controller buttons.
 *
 * @note PSP_CTRL_HOME, PSP_CTRL_NOTE, PSP_CTRL_SCREEN, PSP_CTRL_VOLUP, PSP_CTRL_VOLDOWN, PSP_CTRL_DISC, PSP_CTRL_WLAN_UP, PSP_CTRL_REMOTE, PSP_CTRL_MS can only be read in kernel mode
 */
enum PspCtrlButtons {
	/** Select button. */
	PSP_CTRL_SELECT     = 0x000001,
	/** L3 button. */
	PSP_CTRL_L3			= 0x000002,
	/** R3 button. */
	PSP_CTRL_R3			= 0x000004,
	/** Start button. */
	PSP_CTRL_START      = 0x000008,
	/** Up D-Pad button. */
	PSP_CTRL_UP         = 0x000010,
	/** Right D-Pad button. */
	PSP_CTRL_RIGHT      = 0x000020,
	/** Down D-Pad button. */
	PSP_CTRL_DOWN      	= 0x000040,
	/** Left D-Pad button. */
	PSP_CTRL_LEFT      	= 0x000080,
	/** Left trigger. */
	PSP_CTRL_LTRIGGER   = 0x000100,
	/** Right trigger. */
	PSP_CTRL_RTRIGGER   = 0x000200,
	/** L2 button. */
	PSP_CTRL_L2			= 0x000100,
	/** R2 button. */
	PSP_CTRL_R2			= 0x000200,
	/** L1 button. */
	PSP_CTRL_L1			= 0x000400,
	/** R1 button. */
	PSP_CTRL_R1			= 0x000800,
	/** Triangle button. */
	PSP_CTRL_TRIANGLE   = 0x001000,
	/** Circle button. */
	PSP_CTRL_CIRCLE     = 0x002000,
	/** Cross button. */
	PSP_CTRL_CROSS      = 0x004000,
	/** Square button. */
	PSP_CTRL_SQUARE     = 0x008000,
	/** Home button. In user mode this bit is set if the exit dialog is visible. */
	PSP_CTRL_HOME       = 0x010000,
	/** Hold button. */
	PSP_CTRL_HOLD       = 0x020000,
	/** Music Note button. */
	PSP_CTRL_NOTE       = 0x800000,
	/** Screen button. */
	PSP_CTRL_SCREEN     = 0x400000,
	/** Volume up button. */
	PSP_CTRL_VOLUP      = 0x100000,
	/** Volume down button. */
	PSP_CTRL_VOLDOWN    = 0x200000,
	/** Wlan switch up. */
	PSP_CTRL_WLAN_UP    = 0x040000,
	/** Remote hold position. */
	PSP_CTRL_REMOTE     = 0x080000,
	/** Disc present. */
	PSP_CTRL_DISC       = 0x1000000,
	/** Memory stick present. */
	PSP_CTRL_MS         = 0x2000000,
};

/** Controller input modes. */
enum PspCtrlMode {
	/* Digital input only. No recognizing of analog input. */
	PSP_CTRL_MODE_DIGITAL = 0,
	/* Recognizing of both digital and analog input. */
	PSP_CTRL_MODE_ANALOG = 1,
};

typedef enum PspCtrlButtons SceCtrlButtons;
typedef enum PspCtrlMode SceCtrlMode;

/** Button mask settings. */
enum SceCtrlButtonMaskMode {
    /** No mask for the specified buttons. Button input is normally recognized. */
    SCE_CTRL_MASK_NO_MASK = 0,
    /**
     * The specified buttons are ignored, that means even if these buttons are pressed by the user
     * they won't be shown as pressed internally.
	 *
	 * You can only block user buttons for applications running in User Mode.
     */
    SCE_CTRL_MASK_IGNORE_BUTTONS = 1,
    /**
     * The specified buttons show up as being pressed, even if the user does not press them.
     *
	 * You can only turn ON user buttons for applications running in User Mode.
     */
    SCE_CTRL_MASK_APPLY_BUTTONS = 2,
};

/** Specifies the type of input data to be obtained. */
enum SceCtrlPort {
    /* Input is only obtained from the PSP. */
    SCE_CRTL_PORT_PSP = 0,
    /* Input is obtained from the PSP and a connected DualShock3 controller. */
    SCE_CTRL_PORT_DS3 = 1,
    /* Input is obtained from the PSP and an unknown connected external device. */
    SCE_CTRL_PORT_UNKNOWN_2 = 2
};

/** Returned controller data */
typedef struct SceCtrlData {
	/**
     * The time stamp of the time during which sampling was performed. Time unit is microseconds.
     * Can be used to get the time period of a button pressing event.
     */
	u32 TimeStamp;
	/** Bit mask containing zero or more of ::PspCtrlButtons. */
	u32 Buttons;
	/** Left analogue stick, X axis (0 - 0xFF). Left = 0, Right = 0xFF. */
	u8 Lx;
	/** Left analogue stick, Y axis (0 - 0xFF). Up = 0, Down = 0xFF. */
	u8 Ly;
	/** Right analogue stick, X axis. Filled with 0 if input source doesn't allow second analog input. */
	u8 Rx;
	/** Right analogue stick, Y axis. Filled with 0 if input source doesn't allow second analog input. */
	u8 Ry;
	/** Reserved. */
	u32	reserved;
} SceCtrlData;

/**
 * This structure is for obtaining button data (button/analog stick information) from the
 * controller using ::sceCtrlPeekBufferPositive2(), ::sceCtrlReadBufferNegative2() etc....
 * In addition to PSP controller state it can contain input state of external input devices
 * such as a wireless controller.
 */
typedef struct {
    /**
     * The time stamp of the time during which sampling was performed. Time unit is microseconds.
     * Can be used to get the time period of a button pressing event.
     */
    u32 timestamp;
    /** The currently pressed button. Bitwise OR'ed values of ::PspCtrlButtons. */
    u32 buttons;
    /** Left analog Stick X-axis offset (0 - 0xFF). Left = 0, Right = 0xFF. */
    u8 lx;
    /** Left analog Stick Y-axis offset (0 - 0xFF). Up = 0, Down = 0xFF. */
    u8 ly;
    /** Right analog x-axis. Filled with 0 if input source doesn't allow second analog input. */
    u8 rx;
    /** Right analog y-axis. Filled with 0 if input source doesn't allow second analog input. */
    u8 ry;
    /** Reserved. */
    u32 reserved;
    /** D-pad pressure sensitivity.
    * Byte 1: D-Pad right.
    * Byte 3: D-Pad left.
    */
    s32 dpad_sense_a;
    /** D-pad pressure sensitivity.
    * Byte 1: D-Pad up.
    * Byte 3: D-Pad down.
    */
    s32 dpad_sense_b;
    /** Gamepad pressure sensitivity.
    * Byte 1: Triangle.
    * Byte 3: Circle.
    */
    s32 gpad_sense_a;
    /** Gamepad pressure sensitivity.
    * Byte 1: Cross.
    * Byte 3: Square.
    */
    s32 gpad_sense_b;
    /** Axis pressure sensitivity.
    * Byte 1: L1.
    * Byte 3: R1.
    */
    s32 axis_sense_a;
    /** Axis pressure sensitivity.
    * Byte 1: L2.
    * Byte 3: R2.
    */
    s32 axis_sense_b;
    /** DS3 sixaxis. This is the return value for tilting the x-axis. */
    s32 tilt_x;
    /** DS3 sixaxis. This is the return value for tilting the y-axis. */
    s32 tilt_y;
} SceCtrlData2;

/**
 * This structure represents controller button latch data.
 *
 * With each sampling cycle, the controller service compares the new pressed & released button states
 * with the previously collected pressed button states. This comparison will result in the following possible
 * states for each button:
 *
 *  - `make`: The button has just been pressed with its prior state being the released state. Transition from
 *      'released' state to 'pressed' state.
 *
 *  - `press`: The button is currently in the 'pressed' state.
 *
 *  - `break`: The button has just been released with its prior state being the 'pressed' state. Transition from 'pressed' state to 'release' state.
 *
 *  - `release`: The button is currently in the 'released' state.
 *
 * It is possible for a button to (briefly) be in two states at the same time. Valid combinations are as follows:
 *
 *  - `make` & `press`
 *
 *  - `break` & `release`
 *
 * In other words, if a button is in the `make` state, then it is also in the `press` state. However, this is not the case
 * for the inverse. A button in the `press` state does not need to be in the `make` state.
 *
 * These comparison results are stored internally as latch data and can be retrieved using the APIs ::sceCtrlPeekLatch() and
 * ::sceCtrlReadLatch(). ::PspCtrlButtons can be used to find out the state of each button.
 *
 * @remark The same can be accomplished by using the different sceCtrl[Read/Peek]Buffer[Positive/Negative]() APIs
 * and comparing the currently collected button sampling data with the previously collected one.
 *
 * @see ::sceCtrlPeekLatch()
 * @see ::sceCtrlReadLatch()
 */
typedef struct SceCtrlLatch {
    /** Button transitioned to pressed state. */
    u32 button_make;
    /** Button transitioned to released state. */
    u32 button_break;
    /** Button is in the pressed state. */
    u32 button_press;
    /** Button is in the released state. */
    u32 button_release;
} SceCtrlLatch;

/**
 * Set the controller cycle setting.
 *
 * @param cycle - Cycle.  Normally set to 0.
 *
 * @return The previous cycle setting.
 */
int sceCtrlSetSamplingCycle(int cycle);

/**
 * Get the controller current cycle setting.
 *
 * @param pcycle - Return value.
 *
 * @return 0.
 */
int sceCtrlGetSamplingCycle(int *pcycle);

/**
 * Set the controller mode.
 *
 * @param mode - One of ::PspCtrlMode.
 *
 * @return The previous mode.
 */
int sceCtrlSetSamplingMode(int mode);

/**
 * Get the current controller mode.
 *
 * @param pmode - Return value.
 *
 * @return 0.
 */
int sceCtrlGetSamplingMode(int *pmode);

/**
 * Gets the latch data by polling.
 *
 * This function reads the latch data collected by the controller service. At each sampling
 * interval, the controller service compares the new pressed/released button states with the previously sampled pressed
 * button states and stores that comparison as latch data.
 *
 * Compared to ::sceCtrlReadLatch(), calling this API will not result in clearing the internal latch data. As such,
 * the data returned is the accumulated latch data since the last time ::sceCtrlReadLatch() was called. Consequently,
 * the returned data should not be relied on whether a button is currently in a pressed or released state.
 *
 * @param latch_data Pointer to a ::SceCtrlLatch variable which is to receive the accumulated button latch data.
 *
 * @return On success, the number of times the controller service performed sampling since the last time
 * ::sceCtrlReadLatch() was called. `< 0` on error.
 *
 * @see ::SceCtrlLatch
 * @see ::sceCtrlReadLatch()
 */
int sceCtrlPeekLatch(SceCtrlLatch *latch_data);

/**
 * Gets the latch data by blocking.
 *
 * This function reads the most recent latch data collected by the controller service. At each sampling
 * interval, the controller service compares the new pressed/released button states with the previously sampled pressed
 * button states and stores that comparison as latch data.
 *
 * Compared to ::sceCtrlPeekLatch(), calling this API will result in clearing the internal latch data. As such,
 * calling code might have to explicitly wait for the controller service to update its collected latch data.
 *
 * @param latch_data Pointer to a ::SceCtrlLatch variable which is to receive the current button latch data.
 *
 * @return On success, the number of times the controller service performed sampling since the last time
 * ::sceCtrlReadLatch() was called. `< 0` on error.
 *
 * @par Example:
 * @code
 * SceCtrlLatch latchData;
 *
 * while (1) {
 *     // Obtain latch data
 *     sceCtrlReadLatch(&latchData);
 *
 *     if (latchData.button_make & SCE_CTRL_CROSS)
 *     {
 *         // The Cross button has just been pressed (transition from 'released' state to 'pressed' state)
 *     }
 *
 *     if (latchData.button_press & SCE_CTRL_SQUARE)
 *     {
 *         // The Square button is currently in the 'pressed' state
 *     }
 *
 *    if (latchData.button_break & SCE_CTRL_TRIANGLE)
 *    {
 *        // The Triangle button has just been released (transition from 'pressed' state to 'released' state)
 *    }
 *
 *    if (latchData.button_release & SCE_CTRL_CIRCLE)
 *    {
 *        // The Circle button is currently in the 'released' state
 *    }
 *
 *    // As we clear the internal latch data with the ReadLatch() call, we can explicitly wait for the VBLANK interval
 *    // to give the controller service the time it needs to collect new latch data again. This guarantees the next call
 *    // to sceCtrlReadLatch() will return collected data again.
 *    //
 *    // Note: The sceCtrlReadBuffer*() APIs are implicitly waiting for a VBLANK interval if necessary.
 *    sceDisplayWaitVBlank();
 * }
 * @endcode
 *
 * @see ::SceCtrlLatch
 * @see ::sceCtrlPeekLatch()
 */
int sceCtrlReadLatch(SceCtrlLatch *latch_data);

/**
 * Retrieves controller state data by polling (positive logic).
 *
 * This function obtains button data stored in the internal controller buffers. Does not wait for the next
 * update interval to be performed.
 * The obtained data will be the latest transferred button data into the internal controller buffers.
 *
 * @param pad_data Pointer to one or more ::SceCtrlData variables to receive controller state data. The obtained
 * button data is represented in positive logic.
 * @param num_bufs The number of internal controller buffers to read. There are 64 internal controller
 * buffers which can be read. Has to be set to a value in the range of 1 - 64.
 *
 * @return The number of read internal controller buffers on success.
 *
 * @par Example:
 * @code
 * SceCtrlData data;

 * sceCtrlSetSamplingCycle(0);
 * sceCtrlSetSamplingMode(SCE_CTRL_INPUT_DIGITAL_ANALOG);
 *
 * while (1) {
 *        sceCtrlPeekBufferPositive(&data, 1);
 *        // Cross button pressed
 *        if (data.buttons & SCE_CTRL_CROSS) {
 *            // do something
 *        }
 * }
 * @endcode
 */
int sceCtrlPeekBufferPositive(SceCtrlData *pad_data, int num_bufs);

/**
 * Retrieves controller state data by polling (negative logic).
 *
 * This function obtains button data stored in the internal controller buffers. Does not wait for the next
 * update interval to be performed.
 * The obtained data will be the latest transferred button data into the internal controller buffers.
 *
 * @param pad_data Pointer to one or more ::SceCtrlData variables to receive controller state data. The obtained
 * button data is represented in negative logic.
 * @param num_bufs The number of internal controller buffers to read. There are 64 internal controller
 * buffers which can be read. Has to be set to a value in the range of 1 - 64.
 *
 * @return The number of read internal controller buffers on success.
 *
 * @par Example:
 * @code
 * SceCtrlData data;

 * sceCtrlSetSamplingCycle(0);
 * sceCtrlSetSamplingMode(SCE_CTRL_INPUT_DIGITAL_ANALOG);
 *
 * while (1) {
 *        sceCtrlPeekBufferNegative(&data, 1);
 *        // Cross button pressed
 *        if (data.buttons & ~SCE_CTRL_CROSS) {
 *            // do something
 *        }
 * }
 * @endcode
 */
int sceCtrlPeekBufferNegative(SceCtrlData *pad_data, int num_bufs);

/**
 * Retrieves controller state data by blocking (positive logic).
 *
 * This function obtains button data stored in the internal controller buffers. Waits for the next update interval
 * before obtaining the data. The read data is the newest transferred data into the internal controller
 * buffers.
 *
 * @param pad_data Pointer to one or more ::SceCtrlData variables to receive controller state data. The obtained
 * data is represented in positive logic.
 * @param num_bufs The number of internal controller buffers to read. There are 64 internal controller
 * buffers which can be read. Has to be set to a value in the range of 1 - 64.
 *
 * @return The number of read internal controller buffers on success.
 */
int sceCtrlReadBufferPositive(SceCtrlData *pad_data, int num_bufs);

/**
 * Retrieves controller state data by blocking (negative logic).
 *
 * This function obtains button data stored in the internal controller buffers. Waits for the next update interval
 * before obtaining the data. The read data is the newest transferred data into the internal controller
 * buffers.
 *
 * @param pad_data Pointer to one or more ::SceCtrlData variables to receive controller state data. The obtained
 * data is represented in negative logic.
 * @param num_bufs The number of internal controller buffers to read. There are 64 internal controller
 * buffers which can be read. Has to be set to a value in the range of 1 - 64.
 *
 * @return The number of read internal controller buffers on success.
 */
int sceCtrlReadBufferNegative(SceCtrlData *pad_data, int num_bufs);

/**
 * Retrieves controller state data by polling (positive logic) with support for wireless controllers.
 *
 * This function Obtains button data stored in the internal controller buffers. Waits for the next update interval
 * before obtaining the data. The read data is the newest transferred data into the internal controller
 * buffers and can contain input state provided by external input devices such as a wireless controller.
 *
 * @remark You need to call ::sceCtrl_driver_E467BEC8() before initial use of this API or its related ones.
 *
 * @param port Pass a valid element of ::SceCtrlPort (either 1 or 2).
 * @param pad_data Pointer to one or more ::SceCtrlData2 variables to receive controller state data. The obtained
 * button data is represented in positive logic.
 * @param num_bufs The number of internal controller buffers to read. There are 64 internal controller
 * buffers which can be read. Has to be set to a value in the range of 1 - 64.
 *
 * @return The number of read internal controller buffers on success.
 */
int sceCtrlPeekBufferPositive2(u32 port, SceCtrlData2 *pad_data, int num_bufs);

/**
 * Retrieves controller state data by polling (negative logic) with support for wireless controllers.
 *
 * This function obtains button data stored in the internal controller buffers. Waits for the next update interval
 * before obtaining the data. The read data is the newest transferred data into the internal controller
 * buffers and can contain input state provided by external input devices such as a wireless controller.
 *
 * @remark You need to call ::sceCtrl_driver_E467BEC8() before initial use of this API or its related ones.
 *
 * @param port Pass a valid element of ::SceCtrlPort (either 1 or 2).
 * @param pad_data Pointer to one or more ::SceCtrlData2 variables to receive controller state data. The obtained
 * button data is represented in negative logic.
 * @param num_bufs The number of internal controller buffers to read. There are 64 internal controller
 * buffers which can be read. Has to be set to a value in the range of 1 - 64.
 *
 * @return The number of read internal controller buffers on success.
 */
int sceCtrlPeekBufferNegative2(u32 port, SceCtrlData2 *pad_data, int num_bufs);

/**
 * Retrieves controller state data by blocking (positive logic) with support for wireless controllers.
 *
 * This function obtains button data stored in the internal controller buffers. Waits for the next update interval
 * before obtaining the data. The read data is the newest transferred data into the internal controller
 * buffers and can contain input state provided by external input devices such as a wireless controller.
 *
 * @remark You need to call ::sceCtrl_driver_E467BEC8() before initial use of this API or its related ones.
 *
 * @param port Pass a valid element of ::SceCtrlPort (either 1 or 2).
 * @param pad_data Pointer to one or more ::SceCtrlData2 variables to receive controller state data. The obtained
 * button data is represented in positive logic.
 * @param num_bufs The number of internal controller buffers to read. There are 64 internal controller
 * buffers which can be read. Has to be set to a value in the range of 1 - 64.
 *
 * @return The number of read internal controller buffers on success.
 */
int sceCtrlReadBufferPositive2(u32 port, SceCtrlData2 *pad_data, int num_bufs);

/**
 * Retrieves controller state data by blocking (negative logic) with support for wireless controllers.
 *
 * This function obtains button data stored in the internal controller buffers. Waits for the next update interval
 * before obtaining the data. The read data is the newest transferred data into the internal controller
 * buffers and can contain input state provided by external input devices such as a wireless controller.
 *
 * @remark You need to call ::sceCtrl_driver_E467BEC8() before initial use of this API or its related ones.
 *
 * @param port Pass a valid element of ::SceCtrlPort (either 1 or 2).
 * @param pad_data Pointer to one or more ::SceCtrlData2 variables to receive controller state data. The obtained
 * button data is represented in negative logic.
 * @param num_bufs The number of internal controller buffers to read. There are 64 internal controller
 * buffers which can be read. Has to be set to a value in the range of 1 - 64.
 *
 * @return The number of read internal controller buffers on success.
 */
int sceCtrlReadBufferNegative2(u32 port, SceCtrlData2 *pad_data, int num_bufs);

/**
 * Set analog threshold relating to the idle timer.
 *
 * @param idlereset - Movement needed by the analog to reset the idle timer.
 * @param idleback - Movement needed by the analog to bring the PSP back from an idle state.
 *
 * Set to -1 for analog to not cancel idle timer.
 * Set to 0 for idle timer to be cancelled even if the analog is not moved.
 * Set between 1 - 128 to specify the movement on either axis needed by the analog to fire the event.
 *
 * @return < 0 on error.
 */
int sceCtrlSetIdleCancelThreshold(int idlereset, int idleback);

/**
 * Get the idle threshold values.
 *
 * @param idlerest - Movement needed by the analog to reset the idle timer.
 * @param idleback - Movement needed by the analog to bring the PSP back from an idle state.
 *
 * @return < 0 on error.
 */
int sceCtrlGetIdleCancelThreshold(int *idlerest, int *idleback);

#ifdef __KERNEL__

/**
 * Sets a button mask mode for one or more buttons.
 *
 * You can only mask user mode buttons in user applications. Masking of kernel
 * mode buttons is ignored as well as buttons used in kernel mode applications.
 *
 * @param buttons The button value for which the button mask mode will be applied for. One or more buttons of ::PspCtrlButtons.
 * @param mask_mode Specifies the type of the button mask. One of ::SceCtrlButtonMaskMode.
 *
 * @return The previous button mask type for the given buttons. One of ::SceCtrlButtonMaskMode.
 *
 * @example Example:
 * @code
 * sceCtrlSetButtonIntercept(0xFFFF, 1);  // Mask lower 16bits
 * sceCtrlSetButtonIntercept(0x10000, 2); // Always return HOME key
 * // Do something
 * sceCtrlSetButtonIntercept(0x10000, 0); // Unset HOME key
 * sceCtrlSetButtonIntercept(0xFFFF, 0);  // Unset mask
 * @endcode
 */
u32 sceCtrlSetButtonIntercept(u32 buttons, u32 mask_mode);

/**
 * Get button mask mode
 *
 * @param buttons The buttons to check for. One or more buttons of ::PspCtrlButtons.
 *
 * @return The button mask mode for the given buttons. One of ::SceCtrlButtonMaskMode.
 */
u32 sceCtrlGetButtonIntercept(u32 buttons);

/**
 * Registers a button callback.
 *
 * @param slot The slot used to register the callback. Between 0 - 3.
 * @param button_mask Bitwise OR'ed button values which will be checked for being pressed. One or more buttons of ::PspCtrlButtons.
 * @param callback Pointer to the callback function handling the button callbacks.
 * @param opt Optional user argument. Passed to the callback function as its third argument.
 *
 * @return 0 on success.
 */
s32 sceCtrlSetSpecialButtonCallback(u32 slot, u32 button_mask, SceKernelButtonCallbackFunction callback, void *opt);

#endif // __KERNEL__

/*@}*/

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __CTRL_H__
