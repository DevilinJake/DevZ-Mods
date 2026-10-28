# DevZ Quick Transfer

Move loot without dragging every item. Open your inventory next to a crate, hold **Left Alt**
and sweep the mouse over the items in your backpack: each one the cursor touches drops into
the crate. Sweep over the crate's items instead and they come into your inventory.

- **Players need it too** (it's a normal mod, not a server-only one): the key and the mouse
  are on the player's side.
- **Works with items and containers from any mod**, and with bigger inventories such as
  [Inventory Sizes](../InventorySizes).
- **The server checks every move** exactly like a drag and drop, so it can't move anything a
  player couldn't move by hand.

## What it does

With the inventory open, hold the key and move the mouse over items:

| The item is... | It moves to... |
|---|---|
| In your clothing or backpack | The nearest container you have **open** in the vicinity list (crate, tent, barrel, car trunk, backpack on the ground) |
| In a vicinity container, or on the ground | Your clothing or backpack, wherever there's room |

- **Only what fits moves.** If the nearest open container is full, the item goes to the next
  nearest one with room; if none has room, it stays where it is.
- **Nothing you wear or hold moves**: clothing, your backpack itself, the item in your hands
  and attachments (a scope on a gun) are left alone.
- **No ping-pong.** An item moves once per key press, so it never jumps back when the cursor
  passes over it again in its new place.
- **Bodies aren't a target.** You can sweep items out of a dead player's clothing, but your
  own items never go into a body.
- **Containers you have open aren't picked up** when the cursor crosses their header.
- Nothing happens while you're dragging an item.

Fast sweeps are fine: the game sends one inventory move at a time, so items the cursor touched
are moved one after another over the next few frames.

## The key

Default: **Left Alt**. Change it in **Options > Controls**, under **DevZ Mods**, "Quick
transfer". The mod only listens to the key while the inventory is open, so outside it Left
Alt keeps doing what it always does (free look).

Not right mouse on purpose: right mouse is aim.

## Installing

1. **Build it** from the repo folder:
   ```powershell
   .\build.ps1 QuickTransfer -Key C:\path\to\YourServer.biprivatekey
   ```
2. **Copy `build\@DevZ_QuickTransfer`** into your server folder, add it to `-mod=`, and copy
   its key from `keys\` into your server's `keys\`.
3. Players get the same copy (Workshop or your server pack).

## Settings

None. There's nothing for admins to set, and players pick their own key in the game's
controls.

## How it works

The inventory screen already knows which item the mouse is over (it's the item the tooltip
is for). While the key is held, every new item under the cursor is put in a queue. Each frame
the mod takes the next item from the queue, finds a free spot for it (in your clothing and
backpack, or in the nearest open vicinity container) and asks the game to move it there: the
same request a drag and drop sends, so the server checks it the same way.

Code: `src\scripts\5_Mission\DevZ_QuickTransfer\` (one class, `DevZ_QuickTransfer`, plus a
small hook into the inventory screen's update).
