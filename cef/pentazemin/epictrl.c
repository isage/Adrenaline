#include <string.h>

#include <systemctrl_adrenaline.h>

PentazeminConfig g_config;

void sctrlPentazeminConfigure(PentazeminConfig* conf){
	memcpy(&g_config, conf, sizeof(PentazeminConfig));
}

int sctrlStartUsb() {
	return sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_START_USB, 0);
}

int sctrlStopUsb() {
	return sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_STOP_USB, 0);
}

int sctrlGetUsbState() {
	int state = sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_GET_USB_STATE, 0);

	// Connected
	if (state & 0x20) {
		return 1;
	}

	// Not connected
	return 2;
}

int sctrlRebootDevice() {
	// can't do it separately, because user might have old systemctrl
	// but this is used only by updater, so that's ok
	sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_UPDATE, 0);
	return sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_POWER_REBOOT, 0);
}