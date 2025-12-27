# Installation & Updates
---

## Installation
---

It is recommended to do this approach for a fresh installation. Otherwise, use the [Update](#update) section.

1. Remove the Adrenaline bubble and the `ux0:app/PSPEMUCFW/sce_module/adrenaline_kernel.skprx` path from the taiHEN config.txt and finally reboot your device.
2. Download [Adrenaline.vpk](https://github.com/isage/Adrenaline/releases) and install it using [VitaShell](https://github.com/RealYoti/VitaShell/releases).
3. Launch Adrenaline and press `X` to download the 6.61 firmware. After finishing, it will automatically terminate.
4. Relaunch Adrenaline; this time, it will go into PSPemu mode. Follow the instructions on the screen.

> [!TIP] Getting rid of the double launch bug
> Adrenaline needs to launch twice every time you reboot your device. This is done automatically but that increases the boot time for Adrenaline.
>
> To get rid of such a limitation, simply write this line to `*KERNEL` section of you `ur0:/tai/config.txt` file:
>
> ```
> *KERNEL
> ux0:app/PSPEMUCFW/sce_module/adrenaline_kernel.skprx
> ```
>
> After that, Adrenaline will only need to be launched once, improving the time to boot the [vPSP](./XX-Glossary.md) environment.

## Update
---

If you have already been using Adrenaline, you have three update options: Manual, Update PBP, and Network.

### Manual

1. Download [Adrenaline.vpk](https://github.com/isage/Adrenaline/releases)
2. Open the vpk file as a zip
3. Copy all files from `sce_module` directory to `ux0:app/PSPEMUCFW/sce_module`

### Update PBP

1. Download the updater [EBOOT.PBP](http://adrenaline.sarcasticat.com/EBOOT.PBP) (or [updater.zip](https://github.com/isage/Adrenaline/releases))
  1. **If** `updater.zip`, extract and use the `EBOOT.PBP` inside it.
2. Put the file at `ux0:/pspemu/PSP/GAME/UPDATE/` directory (create it if needed)
3. Launch Adrenaline
4. Run the update application

### Network

1. Launch Adrenaline
2. On the XMB/VSH, go to **System Settings** > **System Update**
3. Press `X` to download the update with the console.

> [!IMPORTANT]
> If you have installed a version below v7.1.0, the network update won't work; you will have to update with one of the other two remaining methods.

> [!IMPORTANT] Important 2
> If you have installed a nightly version, the network update won't work; you will have to update with one of the other two remaining methods.