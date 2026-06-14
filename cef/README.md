# Epinephrine Custom Emulated Firmware

Epinephrine CEF (or eCFW, same thing) is the Custom Firmware that comes integrated with the Adrenaline Project. It is derived from TN-V CFW .

Besides usual PSP CFW features, it helps to get access to PSP XMB and most of the PSP native environment in the PSPemu.


## CFW building blocks

The Epinephrine eCFW part can be separated in two parts: The `core` parts and the `extra` parts.

The `core` parts are essential to support the most basic functionality of a CFW. The `extra` parts are not necessary for the CFW, but might provide quality of life features for end-users.

The ISO drivers are a core part of the custom firmware, but since we support many of them, they received their own folder: `iso_drivers` (mostly motivated to avoid too many folder indentation).

## Project structure

- `core`: The core modules and components for the CFW.
- `extra`: The extra modules for the CFW.
- `cmake`: The project-exclusive cmake definitions.
- `include`: The shared include folder that all modules have access to.
- `bits`: The bits of code shared with more than one module.
- `lib`: The folder that contains static libs/stubs not part of the `PSPSDK` or `psp-cfw-sdk`
