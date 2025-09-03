# Adrenaline Custom Firmware Settings
---

Adrenaline CFW has several settings options that modify the behavior of the system. The settings can be split in two categories: `General Settings` and `Advanced Settings`.

You can use the [XMB](./10-XmbCfwMenus.md#-adrenaline-cfw-settings) or the [Recovery Menu](./09-RecoveryMenu.md) to easily modify the CFW settings.

> [!CAUTION]
> Not to be confused with [`Adrenaline Application Settings`](./03-AdrenalineMenu/02-AdrenalineSettings.md), which is another thing.

## General Settings
---

### CPU/BUS clock speed in XMB

Configures the CPU/BUS clock speed on when the system is on the XMB/VSH.

#### Options:

- `Default`: Same as `222/111` option
- `20/10`
- `75/37`
- `100/50`
- `133/66`
- `222/111`
- `266/133`
- `300/150`
- `333/166`

### CPU/BUS clock speed in PSP game/app

Configures the CPU/BUS clock speed on when the system is running a PSP game, PSP app or PSP homebrew.

#### Options:

- `Default`: Game/app dependant
- `20/10`
- `75/37`
- `100/50`
- `133/66`
- `222/111`
- `266/133`
- `300/150`
- `333/166`

### UMDemu ISO driver

Configures the UMDemu driver to use with `ISO` and `Compressed ISO` games.

#### Options:

- `Inferno`
- `March33`
- `Sony NP9660`

### Skip Sony logo on coldboot

When enabled, skip the "Sony Computer Entertainment" logo when starting the system.

### Skip Sony logo on gameboot

When enabled, skip the "PSP" logo when starting an PSP game/app.

### Hide corrupt icons in game menu

When enabled, hides corrupt icons in the game menu.

Corrupt icons happens in the case a game folder does not contain an `EBOOT.PBP` file.

### Hide DLCs in game menu

When enabled, hides DLC from showing up as duplicate games when you have DLC installed for your ISO game.

### Hide PIC0/PIC1 in game menu

When enabled, stops displaying PIC0 and PIC1 images on XMB, allowing quicker response time to show the game list.

### Hide MAC address

When enabled, hides the MAC address from the `Settings > System Settings > System Information`.

### Autorun /PSP/GAME/BOOT/EBOOT.PBP

When enabled, the system automatically launches the game/app int the `???0:/pspemu/PSP/GAME/BOOT/EBOOT.PBP`.

### Fake VSH Region

Modifies the perceived region on XMB/VSH, allowing to unlock region-specific apps and features.

#### Options:

- `Disabled`
- `Japan`
- `America`
- `Europe`
- `Korea`
- `United Kingdom`
- `Mexico`
- `Australia/NZ`
- `East`
- `Taiwan`
- `Russia`
- `China`
- `Debug Type I`
- `Debug Type II`

### Extended screen colors mode

Modifies the system to use a extended color space based on the configuration.

#### Options:

- `Disabled`
- `02g`: Use the same color space as the PSP-2000. More vibrant colors, cooler color tone.
- `03g`: Use the same color space as the PSP-3000. More vibrant colors, warmer color tone.

### Use Sony PSP OSK

When enabled, makes the system to use the PSP On Screen Keyboard (OSK) instead of the VITA/PSTV native OSK.

### NoDRM engine

Controls whether to patch the DRM engine of the PSP system.

When enabled, the system will allow to launch application, homebrews and games without a valid license.

### XMB Control

Controls whether to load the extra CFW menus on XMB/VSH.

When enabled, the menus will show up in the XMB in the `Settings` section.

### Recovery color

Configures the color of the Recovery Menu.


## Advanced Settings
---

Advanced settings are called that for a reason. They provide powerful options that can significantly alter system behavior, stability, or compatibility. These settings are intended for experienced users and tinkerers exploring the best set of configuration for cases of bad compatibility with apps and games (anti-CFW checks, slowdowns not solved by general settings, etc).

Incorrect configuration may lead to unexpected issues, so proceed with caution and consult documentation or community resources if unsure.

> [!TIP]
> On the XMB CFW menu, advanced settings are marked with the `▲` symbol.

### Force high memory layout

Controls whether to force the unlock extra RAM space for the user-space RAM partition, that can be used by games and apps.

#### Options:

- `Disabled`: With this option, no specific memory layout is forced.
	- By default, user partition gets the standard 24MB.
	- Homebrew games/apps can request other layout by using the `MEMSIZE` in their `SFO` information or by using the CFW API.
	- Plugin author can use the CFW API to request changing the layout.
- `Stable`: The stable amount of extra RAM is made available to the user RAM partition, a total of 40MB.
	- When this options is set, it truly forces it, any requests to modify it will result in it being "ignored" (the CFW API returns an error informing that is not possible to modify the memory layout).
- `Max`: Makes the maximum possible of RAM available to the user RAM partition, a total of 52MB, at the cost of potential instabilities.
	- When this options is set, it truly forces it, any requests to modify it will result in it being "ignored" (the CFW API returns an error informing that is not possible to modify the memory layout).


> [!WARNING] Max Option Warning
> Some important stuff for the VITA's PSPemu are put in this region of memory. Although Adrenaline protects the sub-region that was already identified, those last 16MB region usage was not yet completely investigated and reversed. That means that some system instabilities may occur once things start to be allocated on this region.

### Execute BOOT.BIN in UMDemu ISO

This configuration makes the the system to execute `BOOT.BIN` file instead of `EBOOT.BIN` when launching ISO game/apps.

### Inferno ISO cache policy

Configures what cache policy the `Inferno` UMDemu driver will use for its overall cache (also known as `Inferno Cache`).

#### Options:

- `LRU`: Least Recently Used policy. Uses less cpu cycles because it will just remove the calls from the queue that have not been used in a timeframe.
- `RR`: Round Robin policy. Uses more cpu cycles making it slower because its more data it has to process in the queue, but can be better on the cases where there are highly uniform, or cyclical, data access or requires predicable cache replacement.

> [!WARNING]
> If you didn't understood the brief explanation, do not change this setting. The default setting (`LRU`) is already the better option for games in general.

### Inferno ISO cache number

Configures how many caches the `Inferno` UMDemu driver will use for `Inferno Cache`.

This could improve the execution of a game/app at the cost of using more memory.

Any option other than `Auto` will always use that value for all ISO games and apps launched with the `Inferno` driver.

#### Options:

- `Auto`: Uses the default value `32`, but also can change based on automatic game patches and fixes, including totally disabling Inferno Cache.
- `1`
- `2`
- `4`
- `8`
- `16`
- `32`
- `64`
- `128`

### Inferno ISO cache size

Configures the size of cache the `Inferno` UMDemu driver will use for `Inferno Cache`.

This also could improve the execution of a game/app at the cost of using more memory.

Any option other than `Auto` will always use that value for all ISO games and apps launched with the `Inferno` driver.

#### Options:

- `Auto`: Uses the default value `32KB`, but also can change based on automatic game patches and fixes, including totally disabling Inferno Cache.
- `1KB`
- `2KB`
- `4KB`
- `8KB`
- `16KB`
- `32KB`
- `64KB`

> [!WARNING]
> Putting the biggest values for **Inferno ISO cache number** and **Inferno ISO cache size** will use half of the stable extra RAM (8MB).
>
> Even though what is left is more than enough for most plugin configurations, if you have a crazy amounts of plugins that uses a lots of memory, memory starvation will happen.

### UMDemu ISO seek time delay factor

Adrenaline's `Inferno` and `March33` UMDemu drivers have the ability to simulate the seek time for the disc reader reach to an disc offset before reading and retrieving data that would happen when reading a physical UMD.

This setting allows to control a factor of how much **slower** is that seek time simulation.

This can be used to bypass the anti-CFW checks in some games. Adrenaline automatically sets for games known to have those checks (when set to `Auto`, of course).

#### Option:

- `Auto`: Disabled by default, but also can change based on automatic game patches and fixes.
- `1x`
- `2x`
- `3x`
- `4x`

> [!WARNING]
> Do not forget to re-set to `Auto` before launching a ISO game/app that do not need this configured to avoid performance issues.

### UMDemu ISO read speed delay factor

Adrenaline's `Inferno` and `March33` UMDemu drivers have the ability to simulate the **slower** read speed of a physical UMD.

This setting allows to control a factor of how much **slower** is that read speed simulation.

This can be used to bypass the anti-CFW checks in some games. Adrenaline automatically sets for games known to have those checks (when set to `Auto`, of course).

#### Option:

- `Auto`: Disabled by default, but also can change based on automatic game patches and fixes.
- `1x`
- `2x`
- `3x`
- `4x`

> [!WARNING]
> Do not forget to re-set to `Auto` before launching a ISO game/app that do not need this configured to avoid performance issues.


> [!WARNING]
> When using the `Inferno` driver, setting **any** of the **UMDemu ISO seek time delay factor** and the **UMDemu ISO read speed delay factor** settings to any value other than `Auto` will consequently disable the `Inferno Cache` feature of the driver.
>
> This can reduce greatly the performance of executing ISO games/apps that do not need them configured if you forget to return them to `Auto`.

### XMB plugins

Controls whether to load plugins on XMB/VSH.

### GAME plugins

Controls whether to load plugins on PSP game/apps.

### POPS plugins

Controls whether to load plugins on PS1 game/apps.