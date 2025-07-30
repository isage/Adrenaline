#include <pspsdk.h>
#include <pspkernel.h>

#include "csoread.h"
#include "umd9660.h"
#include "malloc.h"
#include <string.h>

extern int sceKernelDeflateDecompress(void *dst,int dsize,void *src,int *pparam);

#define DEV_NAME "CSO"

// log output switch
#define OUT_LOG_ISO 0

// thread priority in read
// when READ_PRIORITY >= 48 then OutRun2006 are not work.
//#define READ_PRIORITY 47

// short sleep around read/decompress for higher thread switching
//#define SHORT_SLEEP

// index buffer size
#define CISO_INDEX_SIZE (512/4)

// compresed data buffer cache size
#define CISO_BUF_SIZE 0x2000

/////////////////////////////////////////////////////////////////////////////
#if OUT_LOG_ISO
#define log_iso Kprintf
#else
#define log_iso(text,...) {}
#endif

#ifdef SHORT_SLEEP
#define SWITCH_THREAD() sceKernelDelayThread(1)
#else
#define SWITCH_THREAD() {}
#endif

/****************************************************************************
****************************************************************************/

//#ifdef CISO_BUF_SIZE
//static unsigned char ciso_data_buf[CISO_BUF_SIZE] __attribute__((aligned(64)));
//#else
//static unsigned char ciso_data_buf[0x800] __attribute__((aligned(64)));
//#endif
static unsigned char *ciso_data_buf = NULL;

static unsigned int ciso_index_buf[CISO_INDEX_SIZE] __attribute__((aligned(64)));

static unsigned int ciso_buf_pos;    // file poisiotn of the top of ciso_data_buf
static unsigned int ciso_cur_index;  // index(LBA) number of the top of ciso_index_buf

static u8 *ciso_sector = NULL;
// header buffer
static CISO_H ciso;

/****************************************************************************
	Mount UMD event callback
****************************************************************************/
static int max_sectors;

int CisoOpen(int umdfd)
{
	int result;

	// check CISO header
	ciso.magic[0] = 0;
	ciso_buf_pos  = 0x7FFFFFFF;
	//result = dhReadFileRetry(&ciso_fd, 0, &ciso, sizeof(ciso));
	sceIoLseek(umdfd, 0, PSP_SEEK_SET);
	result = sceIoRead(umdfd, &ciso, sizeof(ciso));
	if(result<0)
	{
		return result;
	}
	if(
		ciso.magic[0]=='C' && 
		ciso.magic[1]=='I' && 
		ciso.magic[2]=='S' && 
		ciso.magic[3]=='O'
	)
	{
log_iso("CISO found\n");
		max_sectors = (int)(ciso.total_bytes) / ciso.block_size;
		ciso_cur_index = 0xffffffff;

		if (!ciso_data_buf)
		{
			ciso_data_buf = (unsigned char *)oe_malloc(CISO_BUF_SIZE+64);
			
			if (!ciso_data_buf)
				return -1;

			if ((((u32)ciso_data_buf) % 64) != 0)
			{
				ciso_data_buf += (64 - (((u32)ciso_data_buf) % 64));
			}			
		}

		if (!ciso_sector)
		{
			ciso_sector = (u8 *)oe_malloc(SECTOR_SIZE);
			if (!ciso_sector)
				return -1;
		}

		return 0;
	}
log_iso("not CISO\n");

	// header check error
	return SCE_KERNEL_ERROR_NOFILE;
}

/****************************************************************************
	get file pointer in sector
****************************************************************************/
static int inline ciso_get_index(u32 sector,int *pindex)
{
	int result;
	int index_off;

	// search index
	index_off = sector - ciso_cur_index;
	if((ciso_cur_index==0xffffffff) || (index_off<0) || (index_off>=CISO_INDEX_SIZE) )
	{
		// out of area
log_iso("CISO READ INDEX sector=%X , loc = %X\n",sector,sizeof(ciso)+sector*4);

		SWITCH_THREAD();
		//result = dhReadFileRetry(&ciso_fd,sizeof(ciso)+sector*4,ciso_index_buf,sizeof(ciso_index_buf));
		result = ReadUmdFileRetry(ciso_index_buf, sizeof(ciso_index_buf), sizeof(ciso)+sector*4);
		SWITCH_THREAD();

		if(result<0) return result;

	// wait for long seek
//	sceKernelDelayThread(20000);

		ciso_cur_index = sector;
		index_off = 0;
	}
log_iso("CISO INDEX base %X offset %X\n",ciso_cur_index,index_off);

	// get file posision and sector size
	*pindex = ciso_index_buf[index_off];
	return 0;
}

/****************************************************************************
	Read one sector
****************************************************************************/
static int ciso_read_one(void *buf,int sector)
{
	int result;
	int index,index2;
	int dpos,dsize;


log_iso("CISO READ sector=%X\n",sector);

	// get current index
	result = ciso_get_index(sector,&index);
	if(result<0) 
	{
		return result;
	}

	// get file posision and sector size
	dpos  = (index & 0x7fffffff) << ciso.align;

	if(index & 0x80000000)
	{
		// plain sector
log_iso("CISO plain read fpos=%08X\n",dpos);
#ifdef SHORT_SLEEP
	sceKernelDelayThread(1);
#endif
		//result = dhReadFileRetry(&ciso_fd,dpos,buf,0x800);
	result = ReadUmdFileRetry(buf, 0x800, dpos);
	
#ifdef SHORT_SLEEP
	sceKernelDelayThread(1);
#endif
		return result;
	}

	// compressed sector
log_iso("CISO compress read\n");

	// get sectoer size from next index
	result = ciso_get_index(sector+1,&index2);
	if(result<0) return result;
	
	dsize = ((index2 & 0x7fffffff) << ciso.align) - dpos;
	
	// adjust to maximum size for scramble(shared) sector index
	if( (dsize <= 0) || (dsize > 0x800) ) dsize = 0x800;

#ifdef CISO_BUF_SIZE
	SWITCH_THREAD();
	// read sector buffer
	if( (dpos < ciso_buf_pos) || ( (dpos+dsize) > (ciso_buf_pos+CISO_BUF_SIZE))  )
	{
		// seek & read
		//result = dhReadFileRetry(&ciso_fd,dpos,ciso_data_buf,CISO_BUF_SIZE);
		result = ReadUmdFileRetry(ciso_data_buf, CISO_BUF_SIZE, dpos);
		SWITCH_THREAD();
		if(result<0)
		{
			ciso_buf_pos = 0xfff00000; // set invalid position
			return result;
		}
		ciso_buf_pos = dpos;
	}
	result = sceKernelDeflateDecompress(buf,0x800,ciso_data_buf + dpos - ciso_buf_pos , NULL);
	SWITCH_THREAD();

#else
	// seek
	// read compressed data
	SWITCH_THREAD();
	//result = dhReadFileRetry(&ciso_fd,dpos,ciso_data_buf,dsize);
	result = ReadUmdFileRetry(ciso_data_buf, dsize, dpos);
	SWITCH_THREAD();
	if(result<0) return result;

	result = sceKernelDeflateDecompress(buf,0x800,ciso_data_buf,NULL);
	SWITCH_THREAD();

#endif
log_iso("result sceKernelDeflateDecompress %08X\n",result);
	if(result<0) return result;

	return 0x800;
}

/****************************************************************************
	Read Request
****************************************************************************/

/*int CisofileReadSectors(int lba, int nsectors, void *buf)
{
	int result;
	int i;

	int num_bytes = nsectors * SECTOR_SIZE;

#ifdef READ_PRIORITY
	int cur_prio = sceKernelGetThreadCurrentPriority();
	sceKernelChangeThreadPriority(0,READ_PRIORITY);
#endif
	for(i=0;i<num_bytes;i+=0x800)
	{
		result = ciso_read_one(buf,lba);
		if(result < 0)
		{
			nsectors = result;
			break;
		}
		buf += 0x800;
		lba++;
	}
#ifdef READ_PRIORITY
	sceKernelChangeThreadPriority(0, cur_prio);
#endif	

	return nsectors;
}*/
int CisoReadFile(void *buf, int size, int offset)
{
	int lba = offset / SECTOR_SIZE;
	u8  *p = (u8 *)buf;
	int i;
	int remaining = size;
	int read = 0;
	int result;
	int x = (offset & 0x7FF);	

	if (x)
	{
		result = ciso_read_one(ciso_sector, lba);
		if (result < 0)
			return result;

		int m = 0x800-x;

		if (m > remaining)
			m = remaining;
		
		memcpy(p, ciso_sector+x, m);
		p += m;
		read += m;
		remaining -= m;
		lba++;
	}

	int nsectors = remaining / SECTOR_SIZE;

	for (i = 0; i < nsectors; i++)
	{
		result = ciso_read_one(p, lba);
		if (result < 0)
			return result;

		p += SECTOR_SIZE;
		read += SECTOR_SIZE;
		remaining -= SECTOR_SIZE;
		lba++;
	}

	if (remaining)
	{
		result = ciso_read_one(ciso_sector, lba);
		if (result < 0)
			return result;

		 memcpy(p, ciso_sector, remaining);
		 read += remaining;
	}	

	return read;
}

/****************************************************************************
****************************************************************************/
int CisofileGetDiscSize(int umdfd)
{
	return (int)(ciso.total_bytes) / ciso.block_size;
}

