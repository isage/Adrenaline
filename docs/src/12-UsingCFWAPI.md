# Using CFW API in your homebrew
---

Adrenaline CFW offers five libraries that can be used by homebrew software: `SystemCtrlForKernel`, `SystemCtrlForUser`, `KUBridge`, `SysclibForUser`, and `VshCtrlLib`.

The `SystemCtrlForKernel` and `SystemCtrlForUser` contain API that allows the user to launch/execute homebrew and games, get and set system information, live patching utilities, and more. It is available in all possible runlevels, but, as the names suggest, `SystemCtrlForKernel` is only available for kernel plugins, while `SystemCtrlForUser` is available for user plugins and homebrew applications.

The `KUBridge` provides APIs to execute functions with kernel access from the user-space. It is also available in all possible runlevels, and as `SystemCtrlForUser`, it is available for user plugins and homebrew applications.

The `SysclibForUser` provides many basic C library API to the user-space. With it, you can avoid linking to the newlibc implementation of those functions, making your executable smaller. It is available in all possible runlevels for user plugins and homebrew applications.

The `VshCtrlLib` extends the API available on the VSH/XMB runlevel; this also means it is only available on that runlevel. It is available for user plugins.

## How to use them in your project
---

<!-- TODO: LINK -->
To start using these APIs, you can download from here the stubs and headers of these libraries. Or alternatively, produce the stubs from the source, as it will be explained below.

**Producing the CFW API stubs:**

1. Clone the Adrenaline repository
2. Enter the project folder
3. Prepare to build with:
```sh
cmake -S . -B build
```
4. Generate the stubs and headers
```sh
cmake --build build --target cfw-libs
```
5. Grab the `include` directory and the `lib` directory from the `build/hb-dev-setup/` directory and add them to your project source


After that, you should include the `lib` directory in the library directory list. If you are using `Makefile`, it is as simple as defining the `LIBDIR` variable to the path.

> [!EXAMPLE]
> ```makefile
> LIBDIR = ../lib
> ```

Then, to link to the stub, you should pass the `-l` argument to your compilation flags. If you are using `Makefile`, you can use the `LIBS` variable.

> [!EXAMPLE]
> ```makefile
> LIBS = -lpspsystemctrl_user -lpspsysc_user -lpspkubridge
> ```

### System Control Headers

The `SystemControl` headers (`systemctrl.h` and `systemctrl_se.h`) limit the visualization of some function definitions because some functions are exclusive to user-space and some are exclusive to kernel-space. To make them appear, it uses macro definitions to make them available. If your project is a user plugin, or a homebrew app, pass the `-D__USER__` flag into your `CFLAGS`; if your project is a kernel plugin, pass the `-D__KERNEL__` flag into your `CFLAGS`.

Alternatively, to facilitate things, you can import the `mak` (`build.mak` and `build_prx.mak`) files at the generated `libs` folder in your project `Makefile`. With those, the setup of the macros are automatic.


## My homebrew already uses CFW APIs
---

In the case of your homebrew already using CFW APIs, there is a very good chance that you won't need to modify a thing to support Adrenaline.

The Adrenaline's `SystemCtrlForUser` and `KUBridge` have 100% API compatibility with the same libraries of M33, ME, PRO, and ARK custom firmwares. That means that if the API provided by the original targeted CFW is sufficient for your project, you won't need to change the stubs and header and create a build only targeted to Adrenaline. You will only need to change if you desire to access newly added APIs.

The Adrenaline's `SystemCtrlForKernel` is mostly compatible with M33, ME, PRO, and ARK custom firmwares. Make sure that any function you use from this library also exists for Adrenaline. If you couldn't find any, at worst, you will need to rebuild your kernel plugin project for the 6.61 PSP firmware version to support Adrenaline.

The Adrenaline's `SysclibForUser` only exists on Adrenaline and ARK, and they have total parity. If you already use it, you don't need to change.

The Adrenaline's `VshCtrlLib` is 100% compatible with M33, and mostly compatible with ME, PRO, and ARK custom firmwares. Make sure that any function you use from this library also exists for Adrenaline. If you couldn't find any, you don't need to change a thing to support Adrenaline.

