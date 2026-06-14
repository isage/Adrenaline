#include <pspsdk.h>

extern int main(int argc, char *argv[]);

int start_thread(SceSize args, void *argp)
{
    main(0, NULL);    
    
    return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{

    SceUID thid = sceKernelCreateThread("start_thread", start_thread, 0x10, 0x4000, 0, NULL);

    if (thid < 0)
        return thid;

    sceKernelStartThread(thid, args, argp);
    
    return 0;
}
