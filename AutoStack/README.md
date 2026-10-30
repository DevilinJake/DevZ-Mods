# DevZ Auto Stack

Loose ammo merges into your existing stacks automatically. Pick up 12 rounds of 9×19 while you
already carry a stack of 40, and you end up with one stack of 52 instead of two.

- **Server-side only.** Players don't download anything, and it needs no signing.
- **Works with any ammo**, vanilla or modded, and with any stack size, including
  [Ammo Stacks](../AmmoStacks).
- **Each round keeps its own condition.** Rounds are moved one at a time, the same way the game
  does it when you combine stacks by hand.

## What it does

When a loose-ammo pile arrives somewhere (picked up, looted, or put into a container):

| Where it goes | What happens |
|---|---|
| Anywhere in your clothing or backpack | Merges into other stacks of the same ammo you're carrying, fullest first |
| Into a container (crate, tent, car trunk, backpack on the ground) | Merges into stacks of the same ammo in that container |
| Into your hands | Nothing, so you can still hold a single stack |
| Split off a stack, or moved around inside your own inventory | Nothing, so you can sort and split ammo by hand |

If the stacks it merges into fill up, whatever's left stays as its own smaller stack. Ruined
ammo is never merged.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 AutoStack
   ```
2. **Copy `build\@DevZ_AutoStack`** into your server folder.
3. **Load it as a server mod**, not a normal mod: add it to `-servermod=` in your start line,
   e.g. `-servermod=@DevZ_AutoStack;`

## Settings

Created on first start at `<profiles folder>\DevZ_AutoStack\Config.json`. Edit it with the
server stopped.

| Setting | Default | Meaning |
|---|---|---|
| `Enabled` | `1` | `0` turns the mod off |
| `StackInPlayerInventory` | `1` | Merge ammo anywhere in a player's clothing and backpack |
| `StackInContainers` | `1` | Merge ammo put into crates, tents, car trunks and dropped backpacks |
| `DelayMs` | `300` | How long after an item moves before merging, so the move finishes first |

If players notice their inventory "jumping" while dragging ammo around, raise `DelayMs`.
