#include <pspsdk.h>
#include <pspkernel.h>

#include "psperror.h"
#include "umdman.h"

int id_iecallback;
u32 gp_iecallback;
void *arg_iecallback;
int (* iecallback)(int id, void *arg, int unk);

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000
#define SC_OPCODE	0x0000000C
#define JR_RA		0x03e00008

#define NOP	0x00000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a); 
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a); 
#define MAKE_SYSCALL(a, n) _sw(SC_OPCODE | (n << 6), a);
#define JUMP_TARGET(x) (0x80000000 | ((x & 0x03FFFFFF) << 2))

#define REDIRECT_FUNCTION(a, f) _sw(J_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a);  _sw(NOP, a+4);
#define MAKE_DUMMY_FUNCTION0(a) _sw(0x03e00008, a); _sw(0x00001021, a+4);
#define MAKE_DUMMY_FUNCTION1(a) _sw(0x03e00008, a); _sw(0x24020001, a+4);

int sceUmdManRegisterImposeCallBack(int id, void *callback)
{
	//Kprintf("Register Impose.\n");
	
	return 0;
}

int sceUmdManUnRegisterImposeCallback(int id)
{
	//Kprintf("UnRegister Impose.\n");
	return 0;
}

static void NotifyInsertEjectCallback(int u)
{
	if (iecallback)
	{
		asm("lw $gp, 0(%0)\n" :: "r"(&gp_iecallback));
		iecallback(id_iecallback, arg_iecallback, u);	
	}
}

int sceUmdManRegisterInsertEjectUMDCallBack(int id, void *callback, void *arg)
{
	if (id_iecallback != 0)
		return SCE_ERROR_ERRNO_ENOMEM;

	//Kprintf("Register Insert UMD callback.\n");
	
	id_iecallback = id;
	asm("sw $gp, 0(%0)\n" :: "r"(&gp_iecallback));
	arg_iecallback = arg;
	iecallback = callback;	

	u32 *mod =  (u32 *)sceKernelFindModuleByName("sceIsofs_driver");
	u32 text_addr = *(mod+27);

	_sw(0x00001021, text_addr+0x40DC);
	_sw(0x00001021, text_addr+0x4114);
	_sw(0x00001021, text_addr+0x41C8);
	_sw(0x00001021, text_addr+0x43A4);

	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();	

	NotifyInsertEjectCallback(1);
	
	return 0;
}

int sceUmdManUnRegisterInsertEjectUMDCallBack(int id)
{
	if (id != id_iecallback)
	{
		return SCE_ERROR_ERRNO_ENOENT;
	}	

	id_iecallback = 0;
	gp_iecallback = 0;
	arg_iecallback = 0;
	iecallback = NULL;	
	
	return 0;
}

int sceUmdMan_driver_60933ECD()
{
	//Kprintf("6093.\n");
	
	return 0;
}

int InitUmdMan()
{
	//Kprintf("UmdMan Inited.\n");
	
	return 0;
}

