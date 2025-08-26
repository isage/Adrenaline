# March33 Driver
---

The Adrenaline `March33 2` driver is a evolution of the March33 Driver of the classic, and very popular, M33 CFW.


## Evolution of the driver

The March33 driver was originally developed by the M33 Team for their custom firmware M33 CFW. The detailed evolution of the driver can't be known besides release notes from the M33 CFW, as back in the day, closed-source development was the norm, and, according to Dark_AleX, the team had other reasons to maintain their software closed-source. So let's do a quick dive and try to give an idea of the evolution of this driver. We will start with 3.51—M33-3, which was the debut of the driver. The following version ludic version number, we will consider all reported changed a the last release of the same base OFW version.

- **`March33 1.0`**: The original `March33` driver. Debuted on 3.51—M33-3 in July 2007. It had support for `ISO` and `CSO` image formats and was the version later version of M33 CFW would improve upon.
- **`March33 1.1`**: An natural evolution fo the `March33` driver. Released in 3.51—M33-7, in the same month as the `1.0`. The release changelog indicated improved loading speed of `ISO` and `CSO`.
- **`March33 1.2`**: Pretty much only bug fixing and some compatibility improvements. Released on 3.71—M33-3 in November of the same year.
- **`March33 1.3`**: Increased compatibility with games and bug fixes. Released on 3.90—M33-3 in March of 2008. There was no further notable mentions on the changelogs of version after this one. This release is the most important as it is the version that was open-sourced years later, in 2017, after all members of the team agreed to release it.
	- Since there was no further notable mentions on the changelogs, we are considering the binary version used on later CFW is this one, or very close to it, patched to work on later version of the PSP Firmware.
	- The Adrenaline version was initially based on this version.
- **`March33 RE`**: A reversed engineered version of the `March33` driver made available in the ME CFW with an added support the `DAX` image format.


Now that we've covered the history and evolution of the `March33` Driver over the years, we can focus on the changes done to it in Adrenaline the justifies it being called `March33 2`.

- **`Support for other compressed formats`**: The new reader was also made to be easily configurable and expandable to support other formats, having been given support for not only `CSO` and `ZSO`, but also adds support for `DAX` as seen on ME's March33 update, as well as the rarely seen before `JSO` and `CSOv2` formats. The driver also supports different block sizes, with 2K and 8K being more common (the higher the block size, the better the compression, but also the more memory used by the driver and thus less available for plugins). All formats benefit equally of the speedup hacks, meaning that performance is mainly left on the decompression algorithm they use.
- **`Emulation of Empty UMD Drive`**: On older versions of Inferno, as well as other ISO Drivers, it was never intended for the driver to run without an ISO file to be mounted, thus several bugs existed that caused malfunction when the driver was used without an ISO file specified, this prevented emulation of an empty UMD drive, which was needed on PS Vita to run certain homebrew that checked for a UMD. The main bug was in the implementation of the function `sceUmdCheckMedium`, a boolean function used to check if a UMD is inserted in the drive, on ISO driver this function was hardcoded to always return true, except if an ISO driver is not specified, then it was hardcoded to wait in an infinite loop until one is (which never happens). The solution used by eCFW such as ARK (up to ARK-3) and TN-CEF (including Adrenaline) was to include a small dummy fake.cso file to mount when no ISO file is specified. On the new `March33 2` driver, among other things, this function has been corrected to behave exactly like the original, simply returning False if no ISO has been specified, thus allowing the ISO driver to simulate an empty UMD drive by loading it without an ISO file specified.
- **`Improved Compressed ISO Read Speeds`**: Not only is the algorithm written with efficiency in mind, it also implements a more aggressive form of the speedup hacks.
- **`Physical UMD seek and read time simulation`**: Configurable option to simulate the seek time and read time of reading a physical UMD to bypass the anti-CFW checks of some games

