# PSP Homebrew Support
---

Adrenaline has support for launching unsigned homebrew applications directly from the XMB/VSH, and loading plugins 3 contexts: XMB/VSH, PSP Games/Homebrew, PS1 Games/Homebrew.

All user-mode PSP homebrew since PSP FirmWare (FW) 3.71 onwards should work fine. Kernel-mode PSP homebrew, on the other hand, may require a build targeted specifically to PSP FW 6.60 or 6.61 to work on Adrenaline.

## Homebrew Apps

Homebrew applications must be put on `/pspemu/PSP/GAME/` directory to show up in the XMB/VSH.

## Homebrew plugins

Homebrew plugins must be put in one of three files:

- In `/pspemu/seplugins/VSH.TXT` file for the plugin to loaded on XMB/VSH
- In `/pspemu/seplugins/GAME.TXT` file for the plugin to loaded on PSP games and homebrews
- In `/pspemu/seplugins/POPS.TXT` file for the plugin to loaded on PS1 games and homebrews

> [!INFO]
> PSP filepaths are case insensitive. For that reason, files and directories inside Vita/PSTV `/pspemu/` directory can be all upper case letter, lower case letters or a mix and it will work normally.
>
> So don't worry if you find variations in paths like `/SEPLUGINS/vsh.txt`, or even crazy path like `/SePlUgInS/VsH.TxT`.

## Legacy 1.50 Homebrew

There are some support for legacy FW 1.50 homebrew, but it is not very well tested. If you have any legacy 1.50 homebrew, try and find out if it works on Adrenaline.

> [!QUESTION] What do I do if my legacy 1.50 homebrew fails to run?
> You are encouraged to open an issue on [Adrenaline Issue Tracker](https://github.com/isage/Adrenaline/issues). Please, remember to give as much information as possible so the developers can better find potential solutions for the issue.