# Publishing a DevZ mod to the Workshop

For the repo owner. Everything is uploaded with **DayZ Tools → Publisher** while signed in to
Steam.

## Before uploading

1. **Build it, signed with the DevZ key:**
   ```powershell
   .\build.ps1 <Name> -Key ..\signing\DevZ.biprivatekey
   ```
   The folder to upload is `build\@DevZ_<Name>` (`addons\`, `keys\DevZ.bikey`, `mod.cpp`).
   Server-side mods (AutoStack, InfectedDamage) build without `-Key`, and get the
   **Server-side mod** tag in Publisher.
2. **Refresh the preview and description** if the defaults changed:
   `python tools\make_workshop.py` writes `<Name>\workshop\preview.png` and `description.txt`.

## First upload

In Publisher:

| Box | What to put |
|---|---|
| Item ID | Empty (new item) |
| Title | The mod's name from `mod.cpp`, e.g. `DevZ Inventory Sizes` |
| Folder | `build\@DevZ_<Name>` |
| Preview image | `<Name>\workshop\preview.png` |
| Description | Contents of `<Name>\workshop\description.txt` |
| Visibility | Public |
| Change note | `First release` |

After it uploads, open the item's page in Steam: copy its ID (the number in the address) into
the table below. The first time, Steam may ask you to accept the Workshop agreement before
anyone else can see the item.

## Updating

Pick the item from Publisher's list (so it keeps its ID), point it at the new build folder and
write a short change note. Players get the update the next time they join.

## Published items

| Mod | Workshop ID |
|---|---|
| InventorySizes | |
| AmmoStacks | |
| WeaponDurability | |
| CombineNoSplit | |
| RemoveRecipes | |
| RepairToPristine | |
| QuickTransfer | |
| AutoStack (server-side) | |
| InfectedDamage (server-side) | |
| Essentials (bundle) | |
