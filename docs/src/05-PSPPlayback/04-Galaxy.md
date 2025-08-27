# Galaxy Controller
---

> [!QUESTION] Wait... Not NP9660?
> Yes, Sony's `NP9660` driver is very closed-source and without much information about improvements over time, so there is nothing to talk about it. On the other hand, we have something to talk about, even if little, of the thing that tricks Sony's `NP9660` into executing `ISO` and `Compressed ISO` image files.


The `Galaxy Controller` is the software that tricks Sony's `NP9660` Driver into executing `ISO` and `Compressed ISO` image files.


The Adrenaline `Galaxy Controller` is a mix of the Galaxy Controller of ARK-3 and M33 CFW since version `8.0.0`. On previous versions of Adrenaline, it came with a binary source of M33 Galaxy Controller.

Historically, there were many versions of `Galaxy Controller` created. Most of them seem to have come into existence in their own way, except for the ARK-1/2/3 version, which was a direct continuation from the PRO CFW version. That doesn't matter much for this plugin, though, since all of them do pretty much the same thing, and the performance differences came down to the reading speed of PSP image formats being dependent more on the image-reading implementation of each project than on the patches to the `NP9660` Driver.

The current Adrenaline version reuses the image-reading code for all ISO drivers (and also to show the images on XMB/VSH); for that reason, it contains all the improvements related to reading PSP image files in the `NP9660` Driver once `Galaxy Controller` patches it.

- **`Support for other compressed formats`**: The new reader was also made to be easily configurable and expandable to support other formats, having been given support for not only `CSO` and `ZSO`, but also adds support for `DAX` as seen on ME's March33 update, as well as the rarely seen before `JSO` and `CSOv2` formats. The driver also supports different block sizes, with 2K and 8K being more common (the higher the block size, the better the compression, but also the more memory used by the driver and thus less available for plugins). All formats benefit equally from the speedup hacks, meaning that performance is mainly left to the decompression algorithm they use.
- **`Improved Compressed ISO Read Speeds`**: Not only is the algorithm written with efficiency in mind, it also implements a more aggressive form of the speedup hacks.
