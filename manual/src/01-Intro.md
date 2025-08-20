# Adrenaline
---

Adrenaline is two softwares projects in one: Adrenaline Application (1) and Adrenaline CFW (2).

1. A software that modifies the official PSP Emulator (PSPemu/ePSP) using [taiHEN CFW framework](https://github.com/yifanlu/taiHEN) to make possible to run a PSP 6.61 Custom FirmWare (CFW).
2. A Custom FirmWare for the Vita's PSPemu that, besides usual PSP CFW features, enables access to PSP XMB and most of the PSP native environment in the PSPemu

> [!INFO]
> Sony's unmodified PSPemu has a more limited environment, tailored to be enough to run what Sony released on PS Store.

> [!INFO]
> Thanks to the power of taiHEN, Adrenaline can inject custom code into the IPL which allows unsigned code to run at boot-time.


## Quick Feature Set Overview

### Adrenaline Application

- Compatible with all PS Vita models on firmware 3.60 up to 3.74
- Support to 7 graphical filtering options
- Support to color filter
- Support for PS2/PS3 <-> PSP game link
- Configurable PSP and PS1 screen scale
- Configurable Memory Stick driver location
- Configurable USB device to use when connecting Vita to devices via USB
- Save states

### Adrenaline CFW

- XMB/VSH on ePSP
- Almost entire PSP 6.61 environment available on ePSP (we call it **Virtual PSP**, or **vPSP** for shortness)
- Support executing PSP unsigned code execution (homebrew software, PSP/PS1 backups)
- Support executing PS1 custom games with support for external configuration files that allows easy fixing of games.
- Support manual for game images (ISO/CSO/etc)
- Support for game updates and DLC for both game images (ISO/CSO/etc) and EBOOT
- Configurable CPU/BUS clock for XMB/VSH and PSP games/homebrew/apps
- Three ISO image driver options: Inferno, March33, NP9660
	- Inferno and NP9660 supports launching ISO, CSO, JSO, ZSO, CSOv2 and DAX image formats
	- March33 supports launching ISO and CSO image formats

> [!IMPORTANT]
> Support to showing up JSO, ZSO, CSOv2 and DAX on XMB/VSH are not implemented yet.

- Built-in anti-CFW, fixes some protections and bugs in games. Compatible with NoDRM plugins
- Built-in CFW configuration and plugin manager on XMB/VSH for easy CFW management
- Support to autoboot software on system launch if enabled
- Extensive CFW API with good compatibility with M33, ME, PRO and ARK-4 API
	- Resulting in good compatibility with plugins and homebrew apps and/or easier porting effort for projects using CFW API

## Key Combos Cheat Sheet

| Key Combo       | Event                                 | Availability                   |
|-----------------|---------------------------------------|--------------------------------|
| L+R+Down+Select | Fast exit to VSH                      | Anytime                        |
| L+Select        | Toggle PS1 Fast Forward               | PS1/POPS Apps and Games        |
| Select          | Open VSH Menu                         | XMB/VSH                        |
| Hold R          | Start Adrenaline CFW on Recovery mode | On Adrenaline App bubble start |