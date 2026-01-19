# Known Compatibility Issues
---

This is a list of compatibility issues on Epinephrine with games, application, homebrews and plugins.

## EyePet
---

The camera is not working in this game. That seems to be caused by some buffer in PSPemu that somehow gets invalid.

There are currently no way to solve the issue with this game.

## Titles that ISO need to use `BOOT.BIN`
---

Titles in the ISO/Compressed ISO format that are decrypted or are demo, prototype or modded version of a game will need to be launched while [holding the `R` trigger](./05-PSPPlayback.md#execute-bootbin-instead-of-ebootbin), otherwise the game won't launch properly or crash the system.


## Games with Network issues (requires re-testing)
---

### Hang when trying to get to the AP connection screen
- `Twisted Metal Head On`
- `NBA`
- `ATV Blazin Fury`

### Others

Games that use GameMode API - partial list of games here.
- `Burnout Dominator`: Game becomes too fast for everyone if a Vita player joins, also happens if one of the PSPs is overclocked (video demonstration here).
- `Lego Indiana Jones 2`: Getting a wireless error (Epinephrine) or the game freezes up (ARK).
- `Street Riders`/`Asphalt: Urban GT 2`: Can't see other consoles in lobby (same developer made both games so its most likely the same issue).
- `CSPSP`: Game gets a blue screen/crash when entering multiplayer , note that the game works fine on Epinephrine.
- `Sega Rally Revo`: Game disconnects after it starts in infrastructure mode only.

Some network issues may be fixed with the updated [PRO Online](https://github.com/Kethen/aemu).
