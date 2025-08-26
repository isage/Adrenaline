# PSP Playback
---

Adrenaline allows the user to execute PSP game images, making it possible to play digital copies of PSP games on PlayStation Vita or PSTV. To make it possible, it uses PSP UMDemu drivers, having three available drivers, `Inferno 2`, `March33 2`, `Sony's NP9660`

There are three format of PSP game images: `PBP`, `ISO` and `Compressed ISO`.

The `PBP` image format is the official Sony format. It is the format used for PlayStation Network (PSN) digital games, game updates and game DLCs. For PSN games, Adrenaline always use original `Sony's NP9660` driver. Usually, `PBP` files lives on `???0:/pspemu/PSP/GAME/⟨GAME_ID⟩/` directory.

The `ISO` image format is a 1:1 copy of an Universal Media Disc (UMD), while `Compressed ISO` is a `ISO` image compressed in some way to reduce the space usage. There are a few competing formats for `Compressed ISO`: `CSO`, `CSOv2`, `JSO`, `ZSO` and `DAX`, each one with different tradeoffs. Adrenaline supports all of those formats, and allows you to choose between the three UMDemu drivers available. All of those image options should go in `??0:/pspemu/ISO/` directory.

> [!INFO]
> There is a [page](./05-PSPPlayback/01-CompressedIsoFormats.md) explaining the different compressed image formats and their tradeoffs.

## UMDemu ISO Drivers

This is a brief overview of features of each driver as the versions that come with Adrenaline.

### Inferno 2

This is the default UMDemu ISO driver and the one with the most person-hours put into development over the years. This is a small adaptation of the latest work done hosted on `ARK-4`, with minor modifications for better code reuse with other drivers and integration with the Adrenaline CFW.

- **Support for all ISO formats**: `ISO`, `CSO`, `CSOv2`, `JSO`, `ZSO` and `DAX`
- **Emulation of empty UMD drive**: Several bugs existed that caused malfunction when the driver was used without an ISO file specified on other drivers and older versions of this driver.
- **Improved Compressed ISO read speeds**: Usage of more efficient algorithm, block offsets cache and more aggressive form of the speedup hacks.
- **Inferno Cache**: Usage of overall caches as another read-time speedup hack. The amount of cache is configurable in the CFW configuration option, limiting a maximum to allow for enough free space for other features or plugins. It is possible to also configure the cache policy strategy between two algorithms: Least Recently Used (`LRU`, default) and Round Robin (`RR`).
	- **`LRU`**: Uses less cpu cycles because it will just remove the calls from the queue that have not been used in a timeframe.
	- **`RR`**: Uses more cpu cycles making it slower because its more data it has to process in the queue, but can be better on the cases where there are highly uniform, or cyclical, data access or requires predicable cache replacement.
- **Configurable seek and read speed delays**: This allows to simulate the disc seek and read time for the drive to behave more similarly as reading from a physical UMD disc, this allows to bypass some forms of anti-CFW checks in some games.
	- By default, it is automatically set for games known to include such checks, but it is configurable in the case a user finds a game with checks that are not automatically handled.
	- This feature is incompatible with **Inferno Cache**, enabling this will cause **Inferno Cache** to be disabled.

### March 33 2

March33 driver was originally developed for the March33 CFW by the M33 Team. Adrenaline comes with a modified version with ISO and Compressed ISO read improvements, other than that front, there was no other significant changes in the source code.

This driver is made available mostly for compatibility reasons, as historically some games had better time with this driver than `NP9660` and `Inferno`. There are many PSP games and PSP modded games out there, there is a slightly chance it can still have better compatibility on niche cases.

- **Support for all ISO formats**: `ISO`, `CSO`, `CSOv2`, `JSO`, `ZSO` and `DAX`.
- **Improved Compressed ISO read speeds**: Usage of more efficient algorithm, block offsets cache and more aggressive form of the speedup hacks.
- **Configurable seek and read speed delays**: This allows to simulate the disc seek and read time for the drive to behave more similarly as reading from a physical UMD disc, this allows to bypass some forms of anti-CFW checks in some games.
	- By default, it is automatically set for games known to include such checks, but it is configurable in the case a user finds a game with checks that are not automatically handled.

### Sony's NP9660

It is the original PSP UMDemu driver from Sony, but with patches to trick it into running ISO and Compressed ISO formats. This driver is also the driver used on `PBP` games, having an guaranteed compatibility with all games that Sony released on PSN.

This driver is made available mostly for compatibility reasons, as historically some games had better time with this driver than `March33` and `Inferno`. There are many PSP games out there, so there is a slightly chance it can still have better compatibility on niche cases.

- **Support for all ISO formats**: `ISO`, `CSO`, `CSOv2`, `JSO`, `ZSO` and `DAX`.
- **Improved Compressed ISO read speeds**: Usage of more efficient algorithm, block offsets cache and more aggressive form of the speedup hacks.
