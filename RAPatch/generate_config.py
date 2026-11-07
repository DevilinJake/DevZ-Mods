"""Writes src/config.cpp. Part health can't come from the JSON file, because the engine reads a part's hit points
from config, so it comes from part_health.csv: a rebuild, and players need the new build."""
import csv
import os

HERE = os.path.dirname(os.path.abspath(__file__))

# RA's CfgPatches names, so its config loads first and our rows land on top of it.
RA_PATCHES = [
    "RA_BaseBuilding_Scripts", "Rearmed_BaseBuilding", "Gear_CodeLock", "BaseBuilding_CompoundGate",
    "BaseBuilding_CompoundWall", "BaseBuilding_DoorWall", "BaseBuilding_DoorFrame", "BaseBuilding_DoubleDoorWall",
    "BaseBuilding_DoubleDoorFrame", "BaseBuilding_Floor", "BaseBuilding_Foundation", "BaseBuilding_GarageDoor",
    "BaseBuilding_HalfWall", "BaseBuilding_LadderHatch", "BaseBuilding_HatchLadder", "BaseBuilding_Ramp",
    "BaseBuilding_Roof", "BaseBuilding_RoofWall", "BaseBuilding_Stairs", "BaseBuilding_StairsStraight",
    "BaseBuilding_StepLadder", "RearmedServer_BaseBuilding_Storage", "RearmedServer_Gear_Territory",
    "BaseBuilding_TriangleFloor", "BaseBuilding_TriangleFoundation", "BaseBuilding_TriangleLadderHatch",
    "BaseBuilding_Wall", "BaseBuilding_WindowFrame", "BaseBuilding_Windows",
]

# Each part's direct parent in RA's config, by the end of its name. A row for anything else stops the build.
PARENTS = [
    ("TriangleLadderHatch", "BaseBuilding_TriangleLadderHatchBase"),
    ("TriangleFoundation", "BaseBuilding_TriangleFoundationBase"),
    ("TriangleFloor", "BaseBuilding_TriangleFloorBase"),
    ("DoubleDoorFrame", "BaseBuilding_DoubleDoorFrameBase"),
    ("DoorFrame", "BaseBuilding_DoorFrameBase"),
    ("WindowFrame", "BaseBuilding_WindowFrameBase"),
    ("Foundation", "BaseBuilding_FoundationBase"),
    ("LadderHatch", "BaseBuilding_LadderHatchBase"),
    ("HalfWall", "BaseBuilding_HalfWallBase"),
    ("Floor", "BaseBuilding_FloorBase"),
    ("Wall", "BaseBuilding_WallBase"),
]

CONFIG = """// Written by generate_config.py: edit that, or part_health.csv
class CfgPatches
{
	class DevZ_RAPatch
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "JM_CF_Scripts", %(patches)s};
	};
};

class CfgMods
{
	class DevZ_RAPatch
	{
		dir = "DevZ_RAPatch";
		name = "DevZ RA Patch";
		author = "DevilinJake";
		type = "mod";
		storageVersion = 1;		// CF_ModStorage: a desk's modules are saved through CF
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"DevZ_RAPatch/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"DevZ_RAPatch/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"DevZ_RAPatch/scripts/5_Mission"};
			};
		};
	};
};

class CfgVehicles
{
	class Inventory_Base;
	class Container_Base;
	class TerritoryHQ : Container_Base
	{
		// RA lists five upgrade slots here but never defines them. Modules go in through an action, so they're removed.
		attachments[] = {"CodeLock", "HQ_Nails"};
		class GUIInventoryAttachmentsProps
		{
			delete Upgrades;
		};
		// Only opens with a Storage module. A size can't come from the settings file: the engine reads it from config.
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
	};
%(bags)s%(health)s};
"""

# Sleeping bags: Zenarchist's models and textures, data\\sleepingbag\\CREDIT.txt. Only the class names and paths are ours.
BAG_PATH = "DevZ_RAPatch\\data\\sleepingbag\\"
COLOURS = ["Black", "Blue", "Green", "Grey", "Red"]


def health(material):
    levels = [(1, material), (0.7, material), (0.5, material + "_damage"), (0.3, material + "_damage"), (0, material + "_destruct")]
    rows = ",\n".join('\t\t\t\t\t\t{%g, {"%s%s.rvmat"}}' % (level, BAG_PATH, name) for level, name in levels)
    return """		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 250;
					healthLevels[] =
					{
%s
					};
				};
			};
		};
""" % rows


def bags():
    text = """	class RAPatch_SleepingBag_Base : Inventory_Base
	{
		scope = 1;
		displayName = "Sleeping Bag";
		descriptionShort = "Lay it out and set your respawn on it. Based on Zenarchist's sleeping bag.";
		model = "%(path)ssleepingbag.p3d";
		soundImpactType = "textile";
		hiddenSelections[] = {"packed"};
		hiddenSelectionsMaterials[] = {"%(path)ssleepingbag_packed.rvmat"};
		inventorySlot[] = {"Backpack_1"};
		weight = 1000;
		absorbency = 0.3;
		varWetMax = 0.49;
		itemSize[] = {4, 2};
		repairableWithKits[] = {5, 2};
		repairCosts[] = {30, 25};
		itemBehaviour = 2;
		rotationFlags = 16;
%(packed)s	};
	class RAPatch_SleepingBagDeployed_Base : Inventory_Base
	{
		scope = 1;
		displayName = "Sleeping Bag";
		descriptionShort = "Set your respawn here, or pack it up.";
		model = "%(path)ssleepingbag_deployed.p3d";
		soundImpactType = "textile";
		hiddenSelections[] = {"deployed"};
		hiddenSelectionsMaterials[] = {"%(path)ssleepingbag.rvmat"};
		weight = 1000;
		absorbency = 0.3;
		varWetMax = 0.49;
		itemSize[] = {4, 2};
		repairableWithKits[] = {5, 2};
		repairCosts[] = {30, 25};
		itemBehaviour = 2;
		rotationFlags = 16;
		slopeTolerance = 0.3;
		yawPitchRollLimit[] = {30, 30, 30};
%(deployed)s	};
""" % {"path": BAG_PATH, "packed": health("sleepingbag_packed"), "deployed": health("sleepingbag")}
    for colour in COLOURS:
        low = colour.lower()
        text += """	class RAPatch_SleepingBag_%(c)s : RAPatch_SleepingBag_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"%(p)ssleepingbag_packed_%(l)s_co.paa"};
	};
	class RAPatch_SleepingBag_%(c)s_Deployed : RAPatch_SleepingBagDeployed_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"%(p)ssleepingbag_%(l)s_co.paa"};
	};
""" % {"c": colour, "l": low, "p": BAG_PATH}
    return text

HEALTH_ROW = """	class %(part)s : %(parent)s
	{
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = %(hitpoints)g;
				};
			};
		};
	};
"""


def parent_of(part):
    for ending, parent in PARENTS:
        if part.endswith(ending):
            return parent
    raise SystemExit(f"part_health.csv: don't know the parent of {part}: add it to PARENTS in generate_config.py")


def main():
    parents, rows = [], []
    with open(os.path.join(HERE, "part_health.csv"), newline="") as f:
        for row in csv.DictReader(f):
            part = row["Class"].strip()
            parent = parent_of(part)
            if parent not in parents:
                parents.append(parent)
            rows.append(HEALTH_ROW % {"part": part, "parent": parent, "hitpoints": float(row["Hitpoints"])})
    text = CONFIG % {
        "patches": ", ".join(f'"{p}"' for p in RA_PATCHES),
        "bags": bags(),
        "health": "".join(f"\tclass {p};\n" for p in parents) + "".join(rows),
    }
    with open(os.path.join(HERE, "src", "config.cpp"), "w", newline="\n") as f:
        f.write(text)


main()
