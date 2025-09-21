# Adrenaline Application Settings
---

A tab in the [Adrenaline Menu](../04-AdrenalineMenu.md) that allows adjusting various settings related to Adrenaline Application.

<p align="center">
<img src="../assets/03-epi-menu-settings.png" width="70%" />
</p>

## Graphics Filtering

Applies graphical filters in the Vita/PSTV screen while the Adrenaline App is open.

It offers these options:

- **Original**: No filtering is applied; displays the original PSP graphics.
- **Bilinear**: Smooths out pixel edges for a softer look.
- **Sharp Bilinear**: Slightly sharper smoothing than bilinear and with scanline effect.
- **Advanced AA**: Advanced Anti-Aliasing. Reduces jagged edges, making graphics less blocky.
- **LCD3x**: Simulates the look of an LCD screen, with subtle scanline effect.
- **Sharp Bilinear without Scanlines**: Like Sharp Bilinear, but without the scanline effect.
- **Scale2x**: Doubles the resolution using pixel interpolation, making graphics less blocky.

> [!IMPORTANT]
> When the **Original** option is selected, all graphics-related options will not take effect.

## Smooth Graphics

When enabled, apply a stronger smooth effect filter on top of the filter selected on [Graphical Filtering](#graphics-filtering) configuration.

## f.lux Filter Color

Configures a color filter option that will be applied on top of the other filters.

The available options are:

- **None**
- **Yellow**
- **Blue**
- **Black**

## Screen Scale X/Y (PSP)

Allows to adjust the horizontal (X) and vertical (Y) scaling of the PSP screen within Adrenaline on XMB/VSH and PSP games/homebrew/applications. This allows you to change the size and aspect ratio of the application display to better fit your Vita/PSTV screen and/or the users' preferences.

## Screen Scale X/Y (PS1)

Same as [Screen Scale X/Y (PSP)](#screen-scale-xy-psp), but exclusively on PS1 games/homebrew.

> [!TIP] GrayJack Personal Preference
> For this config, I like to set it to `1.215` on both X and Y axis. Maximizes the size while maintaining the original game proportions.

## Memory Stick Location

Chooses the drive Adrenaline App shows up for the Adrenaline CFW as the Memory Stick drive (`ms0:/`).

The options are:

- `ux0:pspemu`
- `ur0:pspemu`
- `imc0:pspemu`
- `xmc0:pspemu`
- `uma0:pspemu`

## USB device

Selects the device that will show up when connecting the Vita to a system via USB.

The options are:

- **Memory Card**
- **Internal Storage**
- **sd2vita**
- **psvsd**

## Skip Adrenaline Boot Logo

When enabled, it skips the Adrenaline logo at the start of the Adrenaline Application.

