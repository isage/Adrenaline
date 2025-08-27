# PSP Homebrew Support
---

Adrenaline support launching unsigned homebrew applications directly from the XMB/VSH, and loading plugins with a powerful configuration format.

All user-mode PSP homebrew since PSP Firmware (FW) 3.71 onwards should work fine. Kernel-mode PSP homebrew, on the other hand, may require a build specifically targeted to PSP firmware 6.60 or 6.61 to work on Adrenaline.

## Homebrew Apps

PSP homebrew applications are custom software created by the community that can run on the PSP hardware, ranging from games, emulators, and utilities (like media players and file managers).

Homebrew applications must be live on `???0:/pspemu/PSP/GAME/` directory to appear in the XMB/VSH.

## Homebrew plugins

Homebrew plugins are programs in the form of `PRX` files that can add extra features and/or modify the behavior of the PSP system and games.

Homebrew plugins `.prx` files must be put in the `???0:/pspemu/seplugins/` folder and be added to the configuration file `???0:/pspemu/seplugins/EPIplugins.txt`.

> [!INFO]
> PSP filepaths are case insensitive. For that reason, files and directories inside the Vita/PSTV `???0:/pspemu/` directory can be all uppercase letters, lowercase letters, or a mix, and it will work normally.
>
> So don't worry if you find variations in paths like `/SEPLUGINS/epiplugins.txt`, or even crazy paths like `/SePlUgInS/EpIpLuGiNs.TxT`.

The following pages provide more detailed information on plugins.


## Legacy 1.50 Homebrew

There is some support for legacy FW 1.50 homebrew, but it is not very well tested. If you have any legacy 1.50 homebrew, try to find out if it works on Adrenaline.

> [!QUESTION] What do I do if my legacy 1.50 homebrew fails to run?
> You are encouraged to open an issue on [Adrenaline Issue Tracker](https://github.com/isage/Adrenaline/issues). Please, remember to give as much information as possible so the developers can better find potential solutions for the issue.