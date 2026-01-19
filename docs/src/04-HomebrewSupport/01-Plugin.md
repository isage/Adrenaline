# PSP Plugin
---

PSP plugins are small programs (usually `.prx` files) that add new features or modify the behavior of the PSP system or games.

Epinephrine should be compatible with most of the plugins made for classic PSP CFW, but incompatibilities may happen. Plugins that replace the VSH Menu (i.e. System Menu, Ultimate VSH Menu, etc) are not supported.

> [!TIP]
> If you find a plugin that is not working, first open an issue on the plugin project issue tracker before opening an issue on the Adrenaline issue tracker.
>
> If you end up opening an issue on Adrenaline's issue tracker, be mindful that issue resolution depends on many factors, as developers' time, developers' interest in the issue, external contributions, and more; as with any hobbyist open-source project.

## Launch without plugins
---

Epinephrine allows launching software without plugins by holding the `L` trigger while launching apps/games/XMB.

This feature can be useful for quickly checking whether issues with a software are related to broken/bugged plugins or to plugins that don't interact well with the software.

## Adding a plugin
---

1. Put the `.prx` file somewhere on `???0:/pspemu/seplugins/` directory (create if it doesn't exist)
2. Open the `???0:/pspemu/seplugins/plugins.txt` (create if it doesn't exist)
  - This is the same file as ARK CFW uses, that will be shared with both CFWs
  - Optionally, you can create and use `???0:/pspemu/seplugins/EPIplugins.txt` to have a separate plugin configuration file from ARK
3. Add to the configuration line in the general format: `⟨RUNLEVELS⟩, ⟨PLUGIN PATH⟩, ⟨on/off⟩`

    - Example: `psp, ms0:/seplugins/my_plugin.prx, on`
    - You can find detailed information on the format [on the next page](./02-PluginConfigFormat.md)
    - You can find a configuration example [here](./02-PluginConfigFormat.md#example)

> [!IMPORTANT]
> Before version 8.0.0, EPI used another format and files for the plugins
>
> If you just updated from the previous version, EPI provides an easy way to import the old style to the new:
>
> 1. Open Recovery Menu
> 2. Go to the `Advanced` section
> 3. Select and click on `Import classic plugins`
>
> <p align="center"> <img src="../assets/04-import-plugins.png" width="700em" /> </p>

## Managing Plugins
---

Epinephrine offers two ways to manage plugins already added.

### First method: XMB Settings

1. On XMB, go to the `Settings`
2. Find the `★ Plugins Manager` and the confirmation button, and a list of added plugins will show up
3. Select the plugin, click the confirmation button.
4. Select the state (`on`/`off`/`remove`), confirm with the confirmation button.

    - The `remove` option will remove the line from the configuration file and restart the system
5. Return to save configuration.

<p align="center">
<img src="../assets/04-xmb-plugin.png" width="49%" alt="XMB Plugin Manager - Plugin list" />
<img src="../assets/04-xmb-plugin-state.png" width="49%" alt="XMB Plugin Manager - Plugin states" />
</p>

<!-- ![Plugin List](../assets/04-xmb-plugin.png) ![Plugin state](../assets/04-xmb-plugin-state.png) -->

> [!WARNING]
> Modification of the plugin state on any plugin that has XMB/VSH runlevel (`xmb`/`vsh`) needs to restart the XMB/VSH to take effect.

### Second method: Recovery Menu

1. Open [Recovery Menu](../09-RecoveryMenu.md)
2. Go to the `Plugins` section
3. Go over your plugins and use `←`/`→` to select the plugin state
4. Go back to the `General` section
5. Exit the Recovery Menu

<p align="center">
<img src="../assets/04-rec-plugin.png" width="49%" alt="Recovery Menu plugin section" />
<img src="../assets/04-rec-exit.png" width="49%" alt="Recovery Menu exit" />
</p>
