# Publishing a DevZ mod to the Workshop

For the repo owner. Everything is uploaded with **DayZ Tools → Publisher** while signed in to
Steam.

## Before uploading

1. **Build it, signed with the DevZ key:**
   ```powershell
   .\build.ps1 <Name> -Key ..\signing\DevZ.biprivatekey
   ```
   The folder to upload is `build\@DevZ_<Name>` (`addons\`, `keys\DevZ.bikey`, `mod.cpp`).
   Server-side mods (AutoStack, InfectedDamage, CrashSafety) build without `-Key`, and get the
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
the table below **and into `workshop_ids.txt`**, then build and upload the mod once more. The
build writes that ID into `meta.cpp` (`publishedid`), which servers send to players' launchers so
they can find the mod; Publisher's own first-upload `meta.cpp` says 0, which breaks that (a
player reported exactly this on Weapon Durability). The first time, Steam may ask you to accept the Workshop agreement before
anyone else can see the item.

## Updating

Pick the item from Publisher's list (so it keeps its ID), point it at the new build folder and
write a short change note. Players get the update the next time they join.

## Published items

The same IDs live in `workshop_ids.txt`, which the build scripts read.

| Mod | Workshop ID |
|---|---|
| InventorySizes | [3828750627](https://steamcommunity.com/sharedfiles/filedetails/?id=3828750627) |
| AmmoStacks | [3828759176](https://steamcommunity.com/sharedfiles/filedetails/?id=3828759176) |
| WeaponDurability | [3828759569](https://steamcommunity.com/sharedfiles/filedetails/?id=3828759569) |
| CombineNoSplit | [3828760325](https://steamcommunity.com/sharedfiles/filedetails/?id=3828760325) |
| RemoveRecipes | [3828760950](https://steamcommunity.com/sharedfiles/filedetails/?id=3828760950) |
| RepairToPristine | [3828762289](https://steamcommunity.com/sharedfiles/filedetails/?id=3828762289) |
| QuickTransfer | [3829388553](https://steamcommunity.com/sharedfiles/filedetails/?id=3829388553) |
| AutoRun | [3829432690](https://steamcommunity.com/sharedfiles/filedetails/?id=3829432690) |
| SatchelSoundFix | [3829979817](https://steamcommunity.com/sharedfiles/filedetails/?id=3829979817) |
| HouseCodeLock | [3829980528](https://steamcommunity.com/sharedfiles/filedetails/?id=3829980528) |
| AutoStack (server-side) | [3828762770](https://steamcommunity.com/sharedfiles/filedetails/?id=3828762770) |
| InfectedDamage (server-side) | [3828763099](https://steamcommunity.com/sharedfiles/filedetails/?id=3828763099) |
| CrashSafety (server-side) | [3829389536](https://steamcommunity.com/sharedfiles/filedetails/?id=3829389536) |
| HouseRaidRules (server-side, on branch `house-raid-rules`) | [3829986096](https://steamcommunity.com/sharedfiles/filedetails/?id=3829986096) |
| Essentials (bundle) | [3828767191](https://steamcommunity.com/sharedfiles/filedetails/?id=3828767191) |
