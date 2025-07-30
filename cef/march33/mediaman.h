#ifndef __MEDIAMAN_H__
#define __MEDIAMAN_H__

#define SCE_UMD_INIT				(0x00)
#define SCE_UMD_MEDIA_OUT			(0x01)
#define SCE_UMD_MEDIA_IN			(0x02)
#define SCE_UMD_MEDIA_CHG			(0x04)
#define SCE_UMD_NOT_READY			(0x08)
#define SCE_UMD_READY				(0x10)
#define SCE_UMD_READABLE			(0x20)

#define SCE_UMD_MODE_POWERON		(0x01)
#define SCE_UMD_MODE_POWERCUR		(0x02)

#define SCE_UMD_FMT_UNKNOWN			0x00000	/* UNKNOWN */
#define SCE_UMD_FMT_GAME			0x00010	/* GAME */
#define SCE_UMD_FMT_VIDEO			0x00020	/* VIDEO */
#define SCE_UMD_FMT_AUDIO			0x00040	/* AUDIO */
#define SCE_UMD_FMT_CLEAN			0x00080	/* CLEANNING */


typedef struct SceUmdDiscInfo 
{
	unsigned int uiSize;
	unsigned int uiMediaType;
} SceUmdDiscInfo;

int InitMediaMan();

#endif



