# Minimal Edition Driver (ME Driver)
---

The Epinephrine `Minimal Edition 2` driver is an evolution of the Minimal Edition Driver of the popular, Minimal Edition CFW also known as (L)ME CFW.


## Evolution of the driver

The ME driver was originally developed by neur0n for their custom firmware (L)ME CFW. It started as a reverse engineer of the `March33 Driver` when it was closed-source, but then it started to evolve as the ME CFW was being developed. Back then, the ME CFW also called this driver as "March33 Drive" for it's history of the March33 driver reversed engineered. The following version is a ludic version number; we will consider all reported changes as the last release of the same base OFW version.

- **`ME Driver 1.0`**: A reverse-engineered version of the `March33` driver made available in the ME CFW.
- **`ME Driver 1.5`**: An natural evolution fo the `MinimalEdition` driver by the latest release of the ME CFW.

Now that we've covered the history and evolution of the `MinimalEdition` Driver over the years, we can focus on the changes made to it in EPI that justify it being called `MinimalEdition Driver 2`.

- **`Support for other compressed formats`**: The new reader was also made to be easily configurable and expandable to support other formats, having been given support for not only `CSO` and `ZSO`, but also adds support for `DAX` as seen on ME's March33 update, as well as the rarely seen before `JSO` and `CSOv2` formats. The driver also supports different block sizes, with 2K and 8K being more common (the higher the block size, the better the compression, but also the more memory used by the driver and thus less available for plugins). All formats benefit equally from the speedup hacks, meaning that performance is mainly left to the decompression algorithm they use.
- **`Emulation of Empty UMD Drive`**: On older versions of ME Driver, as well as other ISO Drivers except Inferno 2, it was never intended for the driver to run without an ISO file to be mounted, thus several bugs existed that caused malfunction when the driver was used without an ISO file specified, this prevented emulation of an empty UMD drive, which was needed on PS Vita to run certain homebrew that checked for a UMD. The main bug was in the implementation of the function `sceUmdCheckMedium`, a boolean function used to check if a UMD is inserted in the drive, on ISO driver this function was hardcoded to always return true, except if an ISO driver is not specified, then it was hardcoded to wait in an infinite loop until one is (which never happens). The solution used by eCFW such as ARK (up to ARK-3) and TN-CEF (including Adrenaline) was to include a small dummy fake.cso file to mount when no ISO file is specified. On the new `ME Driver 2` driver, among other things, this function has been corrected to behave exactly like the original, simply returning False if no ISO has been specified, thus allowing the ISO driver to simulate an empty UMD drive by loading it without an ISO file specified.
- **`Improved Compressed ISO Read Speeds`**: Not only is the algorithm written with efficiency in mind, it also implements a more aggressive form of the speedup hacks.
- **`Physical UMD seek and read time simulation`**: Configurable option to simulate the seek time and read time of reading a physical UMD to bypass the anti-CFW checks of some games, or fix games with synchronization issues due to faster read times.
