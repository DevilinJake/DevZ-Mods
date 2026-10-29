# DevZ Auto Run

Press **Z** and your character keeps moving forward at the speed you had: walking, jogging or
sprinting. Steer with the mouse, as when holding W. Press W, A, S or D (or Z again) to stop.

- **Players need it too** (it's a normal mod, not a server-only one): the key is on the
  player's side.
- **Works on any map and with any other mods.** It changes no items and no vanilla settings.
- **Multiplayer-safe.** The server is told the same state as the player's game, so the two
  agree on how the character moves and nobody is pulled back.

## What it does

| You press Z while... | You then... |
|---|---|
| Walking | Keep walking |
| Jogging | Keep jogging |
| Sprinting | Keep sprinting until your stamina runs out, then jog, and sprint again when it comes back (as holding Shift does) |
| Standing still | Jog (or walk, if you have walk toggled on) |

Direction is always forward; turn the camera to steer. Free look (Alt) doesn't change your
direction, the same as in vanilla.

**Stops auto-run:**

- Any movement key: W, A, S, D.
- Z again.
- Dying, being knocked out, being restrained, getting into a vehicle.
- Opening a menu you can't walk in: the Esc menu, a trader, another mod's screens.

**Doesn't stop it:**

- Jumping and vaulting: after a vault you carry on.
- Crouching, going prone and standing back up: you move the way that stance normally moves
  (prone can't sprint, for example).
- The inventory, the map, reading a note, the gesture and quickbar wheels, typing in chat:
  everything the game lets you walk through with the movement keys. Typed letters don't count
  as keys, so chatting "wasd" won't stop you. The game slows you to a walk while the
  inventory is open, as it does when you hold W.
- Swimming, ladders and falling: you keep going, as you would with W held.

Holding Shift or Ctrl while auto-running does nothing: the speed is the one you started with.
Stop and start again to change it.

## The key

Default: **Z**. Change it in **Options > Controls**, under **DevZ Mods**, "Auto run". In
vanilla DayZ, Z is "move down", which is only used by console presets (swimming and ladders
use S on PC), so the default doesn't clash with anything on a keyboard.

Running **DayZ Expansion**? Expansion has its own auto-run on Z too. Switch one of them off:
set `EnableAutoRun` to `0` in Expansion's `GeneralSettings.json`, or rebind this one.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 AutoRun -Key C:\path\to\YourServer.biprivatekey
   ```
2. **Copy `build\@DevZ_AutoRun`** into your server folder, add it to `-mod=`, and copy its
   key from `keys\` into your server's `keys\`.
3. Players get the same copy (Workshop or your server pack).

## Settings

None. There's nothing for admins to set, and players pick their own key in the game's
controls.

## How it works

DayZ's input controller has an override for movement speed and direction (the game uses it for
its own bots and debug tools). Pressing the key reads the speed you're moving at and sets that
override on your game, and sends one small message so the server sets the same override on its
copy of your character. From then on no per-frame work is needed beyond watching the movement
keys; stopping clears both overrides with one more message. Nothing is sent while auto-run is
off, and nothing is sent per frame while it's on (sprint dropping to a jog on empty stamina, or
coming back, is one message each time).

Code: `src\scripts\5_Mission\DevZ_AutoRun\` (the key handling, client only) and
`src\scripts\4_World\DevZ_AutoRun\PlayerBase.c` (the override and its server message).
