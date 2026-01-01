# Contributing to Adrenaline
---

Contribution to the project is always welcome. The process is straight-forward:

1. Create a fork of the [project on GitHub](https://github.com/isage/Adrenaline)
2. Commit your changes (remember to do good commit messages)
3. Send a Pull Request (PR) to the project

This is the gist of it, but you will need to be able to compile the project to test before sending a PR.

## Project parts overview
---

### VITA-side

- `bubble`: The bubble launcher for Adrenaline
- `user`: The userspace plugin that patches the PSPemu and allows extra communication between the VITA-side with the PSP-side
- `kernel`: The kernel plugin that do the necessary patches to fix the double launch bug
- `usbdevice`: The USB mass storage plugin
- `vsh`: The plugin that patches VITA VSH for Adrenaline

### eCFW (a.k.a. ePSP-side)

The eCFW part can be separated in two parts: The `core` parts and the `extra` parts.

The `core` parts are essential to support the most basic functionality of a CFW. The `extra` parts are not necessary for the CFW, but might provide quality of life features for end-users.

**Core parts:**

- `cef/systemctrl`: The heart of the eCFW, therefore it's name `SystemControl`. It provide patches for unsigned code execution, handles anti-CFW protection in games, and provides extra libraries API that serves the rest of the eCFW module and third-party plugins.
- `cef/vshctrl`: The `VshControl` module handles the main patches for the `XMB`/`VSH` to allow launching unsigned applications from the XMB. It also provides extra API for plugins meant to be run on `XMB`/`VSH` contexts.
- `cef/galaxy`: The `GalaxyController` module provide patches to use ISO and Compressed ISO with the Sony's NP9660 driver.
- `cef/inferno`: The `InfernoDriver` module is the custom driver that allows executing ISO and Compressed ISO.
- `cef/march33`: The `March33Driver` module is the custom driver that allows executing ISO and Compressed ISO.
- `cef/popcorn`: The `PopcornManager` module allows the PS1 emulator (POPS) to launch custom PS1 images.
- `cef/recovery`: The `Recovery Menu` module. It provides the Recovery Menu, that can be used to configure the eCFW, or reset it in case of bad configuration/plugins making the system not boot-up correctly.
- `cef/kermit_idstorage`: A custom implementation of `sceIdStorage_driver` that enables the vPSP environment to work properly.
- `cef/payloadex` and `cef/rebootex`: Handles the actual injection of the CFW in the (re)boot process. `Payloadex` is used by `user` to launch Adrenaline CFW, while `Rebootex` is set to relaunch it during a ePSP reboot.

**Extra Parts:**

- `cef/satelite`: Provides the `VSH Menu` to the eCFW, allowing to quick access the most basic configurations in any place of the XMB.
- `cef/xmbctrl`: The `XMB Control` module provides access to CFW configuration and plugin management directly from the XMB `Settings` section.


## Setting up the development environment
---

To be able to build the project, you will need to setup the necessary development environment parts. Adrenaline depend on four thing: **VITASDK**, **PSPSDK**, **Python3** and **psp-packer**.

For the [VITASDK](https://vitasdk.org/) and [PSPSDK](https://pspdev.github.io/), follow their instructions to install it in your system.

[Python3](https://www.python.org/downloads/) installation will depend on your system and setup. Google is your friend on this one.

For psp-packer, there are two tested implementations that works with Adrenaline, the [Davee's implementation](https://bitbucket.org/DaveeFTW/psp-packer/src/master/) and [GrayJack's implementation](https://github.com/VitaArchive/psp-packer).

Once everything is installed, you can build the project.


## Building the project
---

First, you need to compile the eCFW (ePSP-side) code:

```sh
make -C cef -j1
```

Then, you have to build the VITA-side part:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

For every change, you must recompile the project for the changes to take effect. This may be obvious for many programmers, but absolute novice in compiled languages may not understand this right away.

> [!QUOTE] GrayJack Comment
> I do not recommend to start in a project as complex as this one. Going through the complexity ladder with small steps is statistically a better way to learn.
>
> That being said, over 10 years ago, I started with baremetal software on the 3DS. I had many frustrations as a novice, but I end up just fine. So if you absolutely want to start with Adrenaline, who am I to stop you.

> [!TIP]
> If you **did not** change a thing on the `cef` directory, you **don't** need to recompile the eCFW.

## Manually testing the project
---

After building the project, you have to install it. For that you can follow the [manual update instructions](./02-Install.md#manual).

You need to reboot the system if there was any modifications on the `vsh` or `kernel` code.

Test things your code change affect.

## Logs
---

On the **ePSP-side**, you can pass next the definition to the `make` call that enables logging on the subprojects that form the Adrenaline ePSP CFW. The ones available are:

- `GALAXY_DEBUG`: Enables logs only on the `cef/galaxy` module
- `INFERNO_DEBUG`: Enables logs only on the `cef/inferno` module
- `MARCH33_DEBUG`: Enables logs only on the `cef/march33` module
- `KERMIT_IDSTORAGE_DEBUG`: Enables logs only on the `cef/kermit_idstorage` module
- `RECOVERY_DEBUG`: Enables logs only on the `cef/newrecovery` module
- `POPCORN_DEBUG`: Enables logs only on the `cef/popcorn` module
- `SATELITE_DEBUG`: Enables logs only on the `cef/satelite` module
- `SCTRL_DEBUG`: Enables logs only on the `cef/systemctrl` module
- `VSHCTRL_DEBUG`: Enables logs only on the `cef/vshctrl` module
- `XMBCTRL_DEBUG`: Enables logs only on the `cef/xmbctrl` module
- `DEBUG`: Enables logs to all subprojects with the same level

It is recommended to use the specific ones. Another recommendation is to avoid using more than level `2` unless really necessary, as the excessive logging message can make the vPSP system very slow.

> [!EXAMPLE]
> ```sh
> make -C cef -j1 SCTRL_DEBUG=2 VSHCTRL_DEBUG=1
> ```

On the **VITA-side**, we don't have something super well stablished, you can use `sceClibPrintf` function, and the messages will show up in the `tty`, that you can access through log homebrews (we recommend [catlog](https://github.com/isage/catlog)).

### Accessing the logs

You can access in real-time and in your computer all logs by using logs homebrew such as [catlog](https://github.com/isage/catlog). This is the easiest way to access them.

For the **eCFW** modules, you can later access them by looking into log files at the `???0:/pspemu/` directory. They are all named using the following pattern: `log_⟨MODULE_NAME⟩.txt`.

