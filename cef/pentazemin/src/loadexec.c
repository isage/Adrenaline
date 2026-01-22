#include <string.h>
#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspinit.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>

void PatchLoadExec(SceModule* mod) {
	u32 text_addr = mod->text_addr;
	u32 text_size = mod->text_size;

	u32 jump = 0;

	for (int i = 0; i < text_size; i += 4) {
		u32 addr = text_addr + i;
		u32 data = VREAD32(addr);

		// Remove apitype check in FW's above 2.60
		if (data == 0x24070200) {
			memset((void *)addr, 0, 0x20);
			continue;
		}

		// Ignore kermit calls
		if (data == 0x17C001D3) {
			MAKE_NOP(addr);
			jump = addr + 8;
			continue;
		}

		// Fix type check
		if (data == 0x34650002) {
			MAKE_INSTRUCTION(addr, 0x24050002); // ori $a1, $v1, 0x2 -> li $a1, 2
			MAKE_INSTRUCTION(addr + 4, 0x12E500B7); // bnez $s7, loc_XXXXXXXX -> beq $s7, $a1, loc_XXXXXXXX
			MAKE_INSTRUCTION(addr + 8, 0xAC570018); // sw $a1, 24($v0) -> sw $s7, 24($v0)
			continue;
		}

		if (data == 0x24100200) {
			// Some registers are reserved. Use other registers to avoid malfunction
			MAKE_INSTRUCTION(addr, 0x24050200); // li $s0, 0x200 -> li $a1, 0x200
			MAKE_INSTRUCTION(addr + 4, 0x12650003); // beq $s3, $s0, loc_XXXXXXXX - > beq $s3, $a1, loc_XXXXXXXX
			MAKE_INSTRUCTION(addr + 8, 0x241E0210); // li $s5, 0x210 -> li $fp, 0x210
			MAKE_INSTRUCTION(addr + 0xC, 0x567EFFDE); // bne $s3, $s5, loc_XXXXXXXX -> bne $s3, $fp, loc_XXXXXXXX

			// Allow LoadExecVSH type 1. Ignore peripheralCommon KERMIT_CMD_ERROR_EXIT
			MAKE_JUMP(addr + 0x14, jump);
			MAKE_INSTRUCTION(addr + 0x18, 0x24170001); // li $s7, 1

			continue;
		}
	}
}