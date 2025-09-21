# Notable Plugins
---

## CXMB
---

One of the most important plugins for customization. With this plugin you can install custom XMB themes made by the community.

This plugin should be enabled for `VSH` (also known as `XMB`) mode only.

> [!IMPORTANT]
> This plugins must be installed in the `/seplugins/cxmb/cxmb.prx` path or else it will refuse to work.

[Download CXMB Here](https://wololo.net/talk/download/file.php?id=3995&sid=e959a08b2f9096913455fac40e7ef84c)

> [!EXAMPLE]
> On `???0:/pspemu/seplugins/EPIplugins.txt`, add the following line:
>
> ```
> xmb, ms0:/seplugins/cxmb/cxmb.prx, on
> ```

## Game Categories Lite
---

This plugin lets you organize your games in subfolders where you can more easily find them and have the XMB load them faster. An important plugin these days of high capacity memory sticks that fit hundreds of games, way more than the PSP was supposed to handle with its limited memory.

This plugin should be enabled for `VSH` (also known as `XMB`) mode only.

> [!EXAMPLE]
> On `???0:/pspemu/seplugins/EPIplugins.txt`, add the following line:
>
> ```
> vsh, ms0:/seplugins/category_lite.prx, on
> ```

## PRO Online
---

An important plugin that brings back Online functionality to the PSP on many games.

On Adrenaline this plugin is compatible with the "Inferno Cache" settings.

> [!IMPORTANT]
> This plugin should be enabled for `TITLE ID`, or limited testing of `GAME` mode **ONLY**

[Download PRO Online Here](https://github.com/Kethen/aemu/releases)

> [!EXAMPLE]
> Note that this example if for **Motorstorm Artic Edge** US version, change or add `TITLE ID`s according to the games you want that it enabled.
>
> On `???0:/pspemu/seplugins/EPIplugins.txt`, add the following line:
>
> ```
> UCES01250, ms0:/seplugins/atpro.prx, on
> ```

## GePatch
---

An experimental plugin for Adrenaline that allows to run a few games in native resolution.

> [!IMPORTANT]
> This plugin should be enabled for `TITLE ID`, or limited testing of `GAME` mode **ONLY**.
>
> And this plugins in incompatible with the `Force High Memory` CFW configuration, so it is best to disable this configuration before starting a game with this plugin

[Download GePatch Here](https://github.com/TheOfficialFloW/GePatch/releases)

[Link to the compatibility list spreadsheet](https://docs.google.com/spreadsheets/d/1aZlmKwELcdpCb9ezI5iRfgcX9hoGxgL4tNC-673aKqk/edit#gid=0)

> [!EXAMPLE]
> Note that this example if for **Final Fantasy III** US version, change or add `TITLE ID`s according to the games you want that it enabled.
>
> On `???0:/pspemu/seplugins/EPIplugins.txt`, add the following line:
>
> ```
> NPUH10125, ms0:/seplugins/ge_patch.prx, on
> ```

## Remastered Controls Collection
---

A collection of plugins for Adrenaline and ARK (on PSVita/PSTV) that enhance the controls of various PSP titles to better use the PSVita/PSTV controller inputs (like the right joystick).

It includes plugins for:

- `Kingdom Hearts: Birth by Sleep` and `Kingdom Hearts: Birth by Sleep Final Mix`
- `Grand Theft Auto: Liberty City Stories` and `Grand Theft Auto: Vice City Stories`
- `Tomb Raider: Anniversary` and `Tomb Raider: Legend`
- `Splinter Cell: Essentials`
- `Resitance: Retribution`
- `Prince Of Persia: Revelations` and `Prince Of Persia: Rival Swords`
- `Metal Gear Solid: Peace Walker`, `Metal Gear Solid: Portable Ops` and `Metal Gear Solid: Portable Ops Plus`
- `The Warriors`
- `Tony Hawk's Project 8`

> [!IMPORTANT]
> This plugin should be enabled for `TITLE ID` **ONLY**.

[Download Remastered Controls Plugins Here](https://github.com/TheOfficialFloW/RemasteredControls/releases)

> [!EXAMPLE]
> Note that this example if for **Kingdom Hearts: Birth By Sleep** US version, change or add `TITLE ID`s and plugin path according to the games you want that it enabled.
>
> On `???0:/pspemu/seplugins/EPIplugins.txt`, add the following line:
>
> ```
> ULUS10505, ms0:/seplugins/khbbs_remastered.prx, on
> ```

## TempAR PSP
---

TempAR is a complete rewrite of the popular PSP cheat device NitePR/MKULTRA, with additional features and improvements created by raing3.

[Download TempAR PSP Here](https://github.com/raing3/tempar/releases/latest)

> [!EXAMPLE]
> On `???0:/pspemu/seplugins/EPIplugins.txt`, add the following lines:
>
> **For PSP games:**
> ```
> psp, ms0:/seplugins/TempAR/tempar.prx, on
> ```
>
> **For PS1 games:**
> ```
> pops, ms0:/seplugins/TempAR/tempar_lite.prx, on
> ```

## CWCheat
---

A cheat device compatible with every game. Requires an updated cheat database to recognize most games.

It can be enabled on every game, but should be disabled for GTA games where their specific cheatdevice is preferred.

[Download CWCheat Here](https://drive.google.com/open?id=1-PF-CmVGVvW5i-IXFxan4HLXZP8bUrm1)

[Download latest cheat database Here](https://github.com/Saramagrean/CWCheat-Database-Plus-)

> [!EXAMPLE]
> On `???0:/pspemu/seplugins/EPIplugins.txt`, add the following lines:
>
> **For PSP games:**
> ```
> psp, ms0:/seplugins/cwcheat/cwcheat.prx, on
> ```
>
> **For PS1 games:**
> ```
> pops, ms0:/seplugins/cwcheat/cwcheatpops.prx, on
> ```

## GTA LCS and VCS cheatdevice remastered
---

A must have plugin for every fan of GTA on the PSP console. Allows for cheats as well as mods.

It is recommended to enable these plugins only on the specific GTA games where you'll be loading them using Adrenaline's per-game plugins configuration.

> [!IMPORTANT]
> If you have cwcheat also installed and enabled, it is recommended to have it disabled for the specific GTA games that you'll be using with cheat device, you can use ARK's per-game plugins configuration.

[Download CheatDeviceRemastered Here](https://github.com/Freakler/CheatDeviceRemastered/releases/latest)

> [!EXAMPLE]
> Note that this example is for the US version of those games. Change them according to your region.
>
> On `???0:/pspemu/seplugins/EPIplugins.txt`, add the following lines:
>
> ```
> ULUS10041 ULUS10160, ms0:seplugins/cheatdevice_remastered.prx, on
> ```

