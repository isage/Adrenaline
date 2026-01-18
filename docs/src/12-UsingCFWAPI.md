# Using CFW API in your homebrew
---

Adrenaline CFW offers five libraries that can be used by homebrew software: `SystemCtrlForKernel`, `SystemCtrlForUser`, `KUBridge`, `SysclibForUser`, and `VshCtrlLib`.

The `SystemCtrlForKernel` and `SystemCtrlForUser` contain API that allows the user to launch/execute homebrew and games, get and set system information, live patching utilities, and more. It is available in all possible runlevels, but, as the names suggest, `SystemCtrlForKernel` is only available for kernel plugins, while `SystemCtrlForUser` is available for user plugins and homebrew applications.

The `KUBridge` provides APIs to execute functions with kernel access from the user-space. It is also available in all possible runlevels, and as `SystemCtrlForUser`, it is available for user plugins and homebrew applications.

The `SysclibForUser` provides many basic C library API to the user-space. With it, you can avoid linking to the newlibc implementation of those functions, making your executable smaller. It is available in all possible runlevels for user plugins and homebrew applications.

The `VshCtrlLib` extends the API available on the VSH/XMB runlevel; this also means it is only available on that runlevel. It is available for user plugins.

## How to use them in your project
---

To start using these APIs, you can install the [PSP CFW SDK](https://github.com/pspdev/psp-cfw-sdk) to obtain the headers and the stubs for the CFW API.

The easiest way is to use `psp-pacman`:

```sh
psp-pacman -Syy psp-cfw-sdk
```

You can also install it from source, by cloning the git repo linked above, and executing

```sh
make install
```

> [!WARNING]
> `PSPDEV` toolchain must be properly set for it to work.


## My homebrew already uses CFW APIs
---

In the case of your homebrew already using CFW APIs, there is a very good chance that you won't need to modify a thing to support Adrenaline.

The Adrenaline's `SystemCtrlForUser` and `KUBridge` have 100% API compatibility with the same libraries of M33, ME, PRO, and ARK custom firmwares. That means that if the API provided by the original targeted CFW is sufficient for your project, you won't need to change the stubs and header and create a build only targeted to Adrenaline. You will only need to change if you desire to access newly added APIs.

The Adrenaline's `SystemCtrlForKernel` is mostly compatible with M33, ME, PRO, and ARK custom firmwares. Make sure that any function you use from this library also exists for Adrenaline. If you couldn't find any, at worst, you will need to rebuild your kernel plugin project for the 6.61 PSP firmware version to support Adrenaline.

The Adrenaline's `SysclibForUser` only exists on Adrenaline and ARK, and they have total parity. If you already use it, you don't need to change.

The Adrenaline's `VshCtrlLib` is 100% compatible with M33, and mostly compatible with ME, PRO, and ARK custom firmwares. Make sure that any function you use from this library also exists for Adrenaline. If you couldn't find any, you don't need to change a thing to support Adrenaline.

