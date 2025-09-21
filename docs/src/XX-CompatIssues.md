# Known Compatibility Issues
---

This is a list of compatibility issues on Adrenaline with games, application, homebrews and plugins.

## EyePet
---

The camera is not working in this game. That seems to be caused by some buffer in PSPemu that somehow gets invalid.

There are currently no way to solve the issue with this game.

## The Simpson Game
---

The game crashes when the keyboard is opened.

To fix, enable the [`Use Sony PSP OSK`](./08-CfwConfiguration.md#use-sony-psp-osk) CFW option.


## Games with Network issues (requires re-testing)
---

### Hang when trying to get to the AP connection screen
- `Twisted Metal Head On`
- `NBA`
- `ATV Blazin Fury`

### Others

Games that use GameMode API - partial list of games here.
- `Burnout Dominator`: Game becomes too fast for everyone if a Vita player joins, also happens if one of the PSPs is overclocked (video demonstration here).
- `Lego Star Wars II`/`Lego Indiana Jones 2`: Getting a wireless error (Adrenaline) or the game freezes up (ARK4).
- `Street Riders`/`Asphalt: Urban GT 2`: Can't see other consoles in lobby (same developer made both games so its most likely the same issue).
- `CSPSP`: Game gets a blue screen/crash when entering multiplayer , note that the game works fine on Adrenaline.
- `Sega Rally Revo`: Game disconnects after it starts in infrastructure mode only.

Some network issues may be fixed with the updated [PRO Online](https://github.com/Kethen/aemu).

