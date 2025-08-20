# Installation & Updates
---

## Installation

It is recommended to do this approach for a fresh installation. Otherwise, use the [Update](#update) section.

1. Remove the Adrenaline bubble and the `ux0:app/PSPEMUCFW/sce_module/adrenaline_kernel.skprx` path from the taiHEN config.txt and finally reboot your device.
2. Download [Adrenaline.vpk](https://github.com/isage/Adrenaline/releases) and install it using [VitaShell](https://github.com/RealYoti/VitaShell/releases).
3. Launch Adrenaline and press `X` to download the 6.61 firmware. After finishing it will automatically terminate.
4. Relaunch Adrenaline, this time it will go into pspemu mode. Follow the instructions on screen.

> [!TIP] Getting rid of double launch bug
> Adrenaline need to launch twice every time you reboot your device. To get rid of such limitation, simply write this line to `*KERNEL`:
>
> ```
> *KERNEL
> ux0:app/PSPEMUCFW/sce_module/adrenaline_kernel.skprx
> ```
>
> After that, Adrenaline will only need to be launched once.

## Update

If you have already been using Adrenaline, you have three update options: Manual, Update PBP and Network.

### Manual

1. Download [Adrenaline.vpk](https://github.com/isage/Adrenaline/releases)
2. Open the vpk file as zip
3. Copy all files from `sce_module` directory to `ux0:app/PSPEMUCFW/sce_module`

### Update PBP

1. Download the updater [EBOOT.PBP](http://adrenaline.sarcasticat.com/EBOOT.PBP)
2. Put the file at `ux0:/pspemu/PSP/GAME/UPDATE/` directory (create it if needed)
3. Launch Adrenaline
4. Run the update application

### Network

1. Launch Adrenaline
2. On the XMB/VSH, go to **System Settings** > **System Update**
3. Press `X` to download the update with the console.

> [!IMPORTANT]
> If you have installed a version below v7.1.0, network update won't work, you will have to update with one of the other two remaining methods.