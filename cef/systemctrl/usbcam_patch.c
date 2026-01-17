#include <systemctrl_adrenaline.h>
#include <pspusbcam.h>

void sceUsb_driver_ED8C8695();
void sceUsb_driver_63E55EBE();

int (* _sceUsbCamStillInput)(u8 *buf, SceSize size);
int sceUsbCamStillInput_Patched(u8 *buf, SceSize size) {
	int k1 = pspSdkSetK1(0);
	int ret = _sceUsbCamStillInput(buf, size);
	sceUsb_driver_ED8C8695(); // force camera stop

	pspSdkSetK1(k1);

	return ret;
}

int sceUsbCamSetupStillEx_Patched(PspUsbCamSetupStillExParam *exparam) {
	int res = 0;

	int k1 = pspSdkSetK1(0);

	PspUsbCamSetupStillParam param = {0};
	param.size = sizeof(PspUsbCamSetupStillParam);
	switch(exparam->resolution)
	{
		case 0:
			param.resolution = 0;
		break;
		case 1:
			param.resolution = 1;
		break;
		case 2:
			param.resolution = 2;
		break;
		case 3:
			param.resolution = 3;
		break;
		case 4:
			param.resolution = 8;
		break;
		case 5:
			param.resolution = 7;
		break;
		case 6:
			param.resolution = 4;
		break;
		case 7:
			param.resolution = 5;
		break;
		case 8:
			// Vita camera doesn't support 1280x960
			param.resolution = 7;
			//param.resolution = 6;
		break;
	}

	param.jpegsize = exparam->jpegsize;
	param.complevel = exparam->complevel;
	param.delay = exparam->delay;
	param.reverseflags = 0x101;

	res = sceUsbCamSetupStill(&param);

	sceUsb_driver_63E55EBE(); // force camera start

	pspSdkSetK1(k1);

	return res;
}

int sceUsbCamSetupVideoEx_Patched(PspUsbCamSetupVideoExParam *exparam, void *workarea, int wasize) {
	int res = 0;

	int k1 = pspSdkSetK1(0);

	PspUsbCamSetupVideoParam param = {0};
	param.size = sizeof(PspUsbCamSetupVideoParam);
	switch(exparam->resolution)
	{
		case 0:
			param.resolution = 0;
		break;
		case 1:
			param.resolution = 1;
		break;
		case 2:
			param.resolution = 2;
		break;
		case 3:
			param.resolution = 3;
		break;
		case 4:
			param.resolution = 8;
		break;
		case 5:
			param.resolution = 7;
		break;
		case 6:
			param.resolution = 4;
		break;
		case 7:
			param.resolution = 5;
		break;
		case 8:
			// Vita camera doesn't support 1280x960
			param.resolution = 7;
//			param.resolution = 6;
		break;
	}

	param.framerate = exparam->framerate;
	param.wb = exparam->wb;
	param.saturation = exparam->saturation;
	param.brightness = exparam->brightness;
	param.contrast = exparam->contrast;
	param.sharpness = exparam->sharpness;
	param.effectmode = exparam->effectmode;
	param.framesize = exparam->framesize;
	param.evlevel = exparam->evlevel;

	res = sceUsbCamSetupVideo(&param, workarea, wasize);

	pspSdkSetK1(k1);

	return res;
}


static int mute_mic = 0;
static void* mic_buf = NULL;
static SceSize mic_size = 0;

int (* _sceUsbCamReadMic)(void *buf, SceSize size);
int sceUsbCamReadMic_Patched(void *buf, SceSize size) {
	int k1 = pspSdkSetK1(0);
	int res = _sceUsbCamReadMic(buf, size);
	mic_buf = buf;
	mic_size = size;
	pspSdkSetK1(k1);
	return res;
}

int (* _sceUsbCamWaitReadMicEnd)(void);
int sceUsbCamWaitReadMicEnd_Patched() {
	int k1 = pspSdkSetK1(0);
	int res = _sceUsbCamWaitReadMicEnd();
	if (mute_mic && mic_buf) {
		memset(mic_buf, 0, mic_size);
	}
	pspSdkSetK1(k1);
	return res;
}


int (* _sceUsbCamSetupMic)(void *param, void *workarea, int wasize);
int sceUsbCamSetupMic_Patched(void *param, void *workarea, int wasize) {
	int res = 0;
	int k1 = pspSdkSetK1(0);
	res = _sceUsbCamSetupMic(param, workarea, wasize);
	pspSdkSetK1(k1);
	mute_mic = 0;
	return res;

}

int sceUsbCamSetupMicEx_Patched(PspUsbCamSetupMicExParam *exparam, void *workarea, int wasize) {
	int res = 0;

	// TODO: xmb wants 22050, but it causes crash for some reason
	// TODO: pspemu resampler looks broken (see invizimals voice chat, for example)
	int k1 = pspSdkSetK1(0);
	PspUsbCamSetupMicParam param = {0};
	param.size = sizeof(PspUsbCamSetupMicParam);
	param.alc = exparam->alc;
	param.gain = exparam->gain;
	param.freq = 11025;//exparam->freq;

	res = _sceUsbCamSetupMic(&param, workarea, wasize);

	pspSdkSetK1(k1);

	mute_mic = 1;

	return res;
}

static int dummy_read_cam = 0;

int (* _sceUsbCamSetEvLevel)(int level);
int sceUsbCamSetEvLevel_Patched(int level) {
	int res = 0;
	int k1 = pspSdkSetK1(0);
	dummy_read_cam = 1;
	sceKernelDelayThread(100000);
	res = _sceUsbCamSetEvLevel(level);
	sceKernelDelayThread(100000);
	dummy_read_cam = 0;
	sceKernelDelayThread(100000);
	pspSdkSetK1(k1);
	return res;
}

int (* _sceUsbCamReadVideoFrame)(u8 *buf, SceSize size);
int sceUsbCamReadVideoFrame_Patched(u8 *buf, SceSize size) {
	int k1 = pspSdkSetK1(0);

	if (dummy_read_cam) {
		sceKernelDelayThread(300000);
		pspSdkSetK1(k1);
		return 0;
	}
	int res = _sceUsbCamReadVideoFrame(buf, size);
	pspSdkSetK1(k1);
	return res;
}

int (* _sceUsbCamWaitReadVideoFrameEnd)(void);
int sceUsbCamWaitReadVideoFrameEnd_Patched() {
	int k1 = pspSdkSetK1(0);
	if (dummy_read_cam) {
		sceKernelDelayThread(300000);
		pspSdkSetK1(k1);
		return 0x8000;
	}
	int res = _sceUsbCamWaitReadVideoFrameEnd();
	pspSdkSetK1(k1);
	return res;
}

void PatchUSBCamDriver(SceModule* mod) {
	REDIRECT_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceUsbCam", 0x0A41A298), sceUsbCamSetupStillEx_Patched);
	REDIRECT_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceUsbCam", 0xCFE9E999), sceUsbCamSetupVideoEx_Patched);
	REDIRECT_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceUsbCam", 0x2E930264), sceUsbCamSetupMicEx_Patched);
	HIJACK_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceUsbCam", 0xFB0A6C5D), sceUsbCamStillInput_Patched, _sceUsbCamStillInput);
	HIJACK_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceUsbCam", 0x1D686870), sceUsbCamSetEvLevel_Patched, _sceUsbCamSetEvLevel);
	HIJACK_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceUsbCam", 0x99D86281), sceUsbCamReadVideoFrame_Patched, _sceUsbCamReadVideoFrame);
	HIJACK_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceUsbCam", 0xF90B2293), sceUsbCamWaitReadVideoFrameEnd_Patched, _sceUsbCamWaitReadVideoFrameEnd);
	HIJACK_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceUsbCam", 0x3DC0088E), sceUsbCamReadMic_Patched, _sceUsbCamReadMic);
	HIJACK_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceUsbCam", 0x03ED7A82), sceUsbCamSetupMic_Patched, _sceUsbCamSetupMic);
	HIJACK_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceUsbCam", 0xB048A67D), sceUsbCamWaitReadMicEnd_Patched, _sceUsbCamWaitReadMicEnd);
	sctrlFlushCache();
}