#include <string.h>
#include <pspsdk.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

PSP_MODULE_INFO("sceIdStorage_Service", 0x1007, 1, 4);

int sceIdStorageCreateLeaf(void) {
	return 0;
}

int sceIdStorageWriteLeaf(void) {
	return 0;
}

int sceIdStorageDeleteLeaf(void) {
	return 0;
}

int sceIdStorageEnd(void) {
	return 0;
}

int sceIdStorageIsReadOnly(void) {
	return 1;
}

int sceIdStorageEnumId(void) {
	return 0;
}

int sceIdStorageGetFreeLeaves(void) {
	return 0;
}

int sceIdStorageFlush(void) {
	return 0;
}

int sceIdStorageUpdate(void) {
	return 0;
}

int sceIdStorageFormat(void) {
	return 0;
}

int sceIdStorageCreateAtomicLeaves(void) {
	return 0;
}

int sceIdStorageInit(void) {
	return 0;
}

int sceIdStorageIsDirty(void) {
	return 0;
}

int sceIdStorageGetLeafSize(void) {
	return 0x200;
}

int sceIdStorageUnformat(void) {
	return 0;
}

int sceIdStorageIsFormatted(void) {
	return 1;
}

int sceKermitPeripheral_driver_C6F8B4E1(short key,int offset,void *buf,int len);
int sceWlanDevGetMacAddr(void* buf);

int sceIdStorageLookup(short key,int offset,void *buf,int len) {
	int res = 0;
	// Vita side doesn't handle keys < 0x100 and > 0x13F
	if (key == 0x44) {
		if (offset == 0) {
			sceWlanDevGetMacAddr(buf);
			res = 0;
			goto exit;
		}

	} else if (key == 0x51) {
		if (offset == 0) {
			strcpy(buf,"6.61");
			res = 0;
			goto exit;
		}

	} else if ((key == 0x100) && (offset == 0)) {
		buf = (void *)((int)buf + 0x38);
		offset = 0x38;
		len = 0xb8;
	}

	res = sceKermitPeripheral_driver_C6F8B4E1(key,offset,buf,len);

exit:
	logmsg4("[INFO]: %s: key=0x%04X, offset=%d, buf=%p, len=%d -> 0x%08X\n", __func__, key, offset, buf, len, res);
	return res;
}

int sceIdStorageReadLeaf(short key,void *buf) {
	return sceIdStorageLookup(key,0,buf,0x200);
}

int module_start(void) {
	logInit("ms0:/log_idstorage.txt");
	logmsg("Kermit ID Storage started...\n");
	return 0;
}

int module_reboot_before(void) {
	return 0;
}
