# PSP Playback
---

Adrenaline allows the user to execute PSP game images, making it possible to play digital copies of PSP games on PlayStation Vita or PSTV. To make it possible, it uses PSP UMDemu drivers, having three available drivers: `Inferno 2`, `March33 2`, and `Sony's NP9660`

There are three formats of PSP game images: `PBP`, `ISO`, and `Compressed ISO`.

The `PBP` image format is the official Sony format. It is the format used for PlayStation Network (PSN) digital games, game updates, and game DLCs. For PSN games, Adrenaline always uses the original `Sony's NP9660` driver. Usually, `PBP` files live on `???0:/pspemu/PSP/GAME/⟨TITLE_ID⟩/` directory.

The `ISO` image format is a 1:1 copy of a Universal Media Disc (UMD), while `Compressed ISO` is an `ISO` image compressed in some way to reduce space usage. There are a few competing formats for `Compressed ISO`: `CSO`, `CSOv2`, `JSO`, `ZSO`, and `DAX`, each with different tradeoffs. Adrenaline supports all of those formats and allows you to choose between the three UMDemu drivers available. All of those image options should go in `??0:/pspemu/ISO/` directory.

> [!INFO]
> There is a [page](./05-PSPPlayback/01-CompressedIsoFormats.md) explaining the different compressed image formats and their tradeoffs.

## UMDemu ISO Drivers
---

This is a brief overview of the features of each driver in the versions that come with Adrenaline.

### Inferno 2

This is the default UMDemu ISO driver and the one with the most person-hours put into development over the years. This is a small adaptation of the latest work done hosted on `ARK CFW`, with minor modifications for better code reuse with other drivers and integration with the Adrenaline CFW.

- **Support for all ISO formats**: `ISO`, `CSO`, `CSOv2`, `JSO`, `ZSO`, and `DAX`
- **Emulation of empty UMD drive**: Several bugs existed that caused malfunction when the driver was used without an ISO file specified on other drivers and older versions of this driver.
- **Improved Compressed ISO read speeds**: Usage of a more efficient algorithm, block offsets cache, and a more aggressive form of the speedup hacks.
- **Inferno Cache**: Usage of overall caches as another read-time speedup hack. The amount of cache is configurable in the CFW configuration option, limiting a maximum to allow for enough free space for other features or plugins. It is also possible to configure the cache policy strategy between two algorithms: Least Recently Used (`LRU`, default) and Random Replacement (`RR`).
    - **`LRU`**: Uses fewer CPU cycles because it will discards the cache slot from the queue that have not been used in a timeframe when new cache item needs to be added.
    - **`RR`**: Uses slightly less memory as it doesn't need to keep track of the least used cache slot.
- **Configurable seek and read speed delays**: This allows for simulating the disc seek and read time for the drive to behave more similarly as reading from a physical UMD disc, which allows bypassing some forms of anti-CFW checks in some games.
    - By default, it is automatically set for games known to include such checks, but it is configurable in the case a user finds a game with checks that are not automatically handled.
    - This feature is incompatible with **Inferno Cache**, enabling this will cause **Inferno Cache** to be disabled.

### March 33 2

March33 driver was originally developed for the March33 CFW by the M33 Team. Adrenaline comes with a modified version with ISO and Compressed ISO read improvements and bug fixes; other than that, there were no other significant changes in the source code.

This driver is made available mostly for compatibility reasons, as historically some games had a better time with this driver than `NP9660` and `Inferno`. There are many PSP games and PSP modded games out there; there is a slight chance it can still have better compatibility in niche cases.

- **Support for all ISO formats**: `ISO`, `CSO`, `CSOv2`, `JSO`, `ZSO`, and `DAX`.
- **Improved Compressed ISO read speeds**: Usage of a more efficient algorithm, block offsets cache, and a more aggressive form of the speedup hacks.
- **Configurable seek and read speed delays**: This allows for simulating the disc seek and read time for the drive to behave more similarly as reading from a physical UMD disc, which allows bypassing some forms of anti-CFW checks in some games.
    - By default, it is automatically set for games known to include such checks, but it is configurable in the case a user finds a game with checks that are not automatically handled.

### Sony's NP9660

It is the original PSP UMDemu driver from Sony, but with patches to trick it into running ISO and Compressed ISO formats. This driver is also the one used on `PBP` games; it has guaranteed compatibility with all games that Sony released on PSN.

This driver is made available for ISO mostly for compatibility reasons, as historically some games had a better time with this driver than `March33` and `Inferno`. There are many PSP games out there, so there is a slight chance it can still have better compatibility in niche cases.

- **Support for all ISO formats**: `ISO`, `CSO`, `CSOv2`, `JSO`, `ZSO`, and `DAX`.
- **Improved Compressed ISO read speeds**: Usage of a more efficient algorithm, block offsets cache, and a more aggressive form of the speedup hacks.
- **Configurable seek and read speed delays**: This allows for simulating the disc seek and read time for the drive to behave more similarly as reading from a physical UMD disc, which allows bypassing some forms of anti-CFW checks in some games.
    - By default, it is automatically set for games known to include such checks, but it is configurable in the case a user finds a game with checks that are not automatically handled.

## Other ISO features
---

Adrenaline offers a few other functionalities related to `ISO` and `Compressed ISO` that are worth mentioning.

### Support for game update and DLC

Adrenaline supports launching ISO games with updates and DLC when installed in their usual place (`???0:/pspemu/PSP/GAME/⟨TITLE_ID⟩/PBOOT.PBP`). But, as a downside, having this file without `EBOOT.PBP` in the same folder causes the XMB/VSH to show duplicated items; to solve that, you can enable the [Hide DLCs in game menu](./08-CfwConfiguration.md#hide-dlcs-in-game-menu) CFW setting.

### Support for manuals

Adrenaline also supports manuals for ISO games. For it to work, the `.DAT` file must be in the `???0:/pspemu/ISO/` directory with the same name as the `ISO` game file.

> [!EXAMPLE]
> For the ISO file in `???0:/pspemu/ISO/my_game.cso`, the `DAT` file should be at `???0:/pspemu/ISO/my_game.DAT`