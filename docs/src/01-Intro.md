# Adrenaline Project
---

Adrenaline is two software projects in one: **Adrenaline Application** (1) and **Epinephrine CFW** (2).

1. **Adrenaline** is a software that modifies the official PSP Emulator (PSPemu/ePSP) using [taiHEN CFW framework](https://github.com/yifanlu/taiHEN) to make it possible to run a PSP 6.61 Custom Firmware (CFW).
2. **Epinephrine** is a Custom Firmware for the Vita's PSPemu derived from TN-V CFW that, besides usual PSP CFW features, it helps to get access to PSP XMB and most of the PSP native environment in the PSPemu.

> [!INFO]
> Sony's unmodified PSPemu has a more limited environment, tailored to be enough to run what Sony released on the PS Store.

> [!INFO]
> Thanks to the power of taiHEN, Adrenaline can inject custom code into the IPL, which allows unsigned code to run at boot-time.


## Quick Feature Set Overview
---

### Adrenaline Application

- Compatible with all PS Vita models on firmware 3.60 up to 3.74
- Support 8 graphical filtering options
- Support color filter
- Support to PS1 game overlay image filter
- Support for PS2/PS3 <-> PSP game link
- Configurable vPSP Custom Firmware
- Configurable PSP and PS1 screen scale
- Configurable Memory Stick driver location
- Configurable USB device to use when connecting Vita to devices via USB
- Save states

### Epinephrine CFW

- XMB/VSH on ePSP
- Almost entire PSP 6.61 environment available on ePSP (we call it **Virtual PSP**, or **vPSP** for shortness)
- Support executing PSP unsigned code execution (homebrew software, PSP/PS1 backups)
- Support executing PS1 custom games with support for external configuration files that allow easy fixing of games.
- Support manual for game images (ISO/CSO/etc)
- Support for game updates and DLC for both game images (ISO/CSO/etc) and EBOOT
- Configurable CPU/BUS clock for XMB/VSH and PSP games/homebrew/apps
- Three ISO image driver options: Inferno, March33, NP9660
- Support for ISO, CSO, JSO, ZSO, CSOv2, and DAX image formats
- Built-in anti-CFW, fixes some protections and bugs in games.
- Built-in CFW configuration and plugin manager on XMB/VSH for easy CFW management
- Support for autoboot software on system launch if enabled
- Extensive CFW API with good compatibility with M33, ME, PRO, and ARK API
    - Resulting in good compatibility with plugins and homebrew apps and/or easier porting effort for projects using CFW API

## Adrenaline Key Combos Cheat Sheet
---

| Key Combo         | Event                                 | Availability                                  |
|-------------------|---------------------------------------|-----------------------------------------------|
| PS button 2x      | Return to Vita live area              | Anytime                                       |
| Hold R            | Start CFW on Recovery mode            | On Adrenaline App bubble start                |
| L+Select          | Toggle PS1 Fast Forward               | PS1/POPS titles                               |


## Epinephrine CFW Key Combos Cheat Sheet
---

| Key Combo         | Event                                 | Availability                                  |
|-------------------|---------------------------------------|-----------------------------------------------|
| L+R+Down+Select   | Fast exit to XMB/VSH                  | Anytime on XMB or PSP title                   |
| L+R+Down+Start    | Fast exit to XMB/VSH                  | Anytime on XMB or PSP title                   |
| L2+R2+Down+Select | Fast exit to XMB/VSH                  | Anytime on PS1 title on Vita                  |
| L2+R2+Down+Start  | Fast exit to XMB/VSH                  | Anytime on PS1 title on Vita                  |
| Select            | Open VSH Menu                         | XMB/VSH                                       |
| Hold L            | Plugins disabled                      | On PSP/PS1 title Reset/Power-on/launch        |
| Hold R            | Execute BOOT.BIN in UMDemu ISO        | On PSP ISO title and games launch             |
