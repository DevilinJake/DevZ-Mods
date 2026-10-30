class CfgPatches
{
	class DevZ_HouseCodeLock
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DayZExpansion_Objects_BaseBuilding_Items", "DayZExpansion_BaseBuilding_Scripts"};
	};
};

class CfgMods
{
	class DevZ_HouseCodeLock
	{
		dir = "DevZ_HouseCodeLock";
		name = "DevZ Expansion House CodeLock";
		author = "DevilinJake";
		type = "mod";
		dependencies[] = {"World"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"DevZ_HouseCodeLock/scripts/4_World"};
			};
		};
	};
};

class CfgVehicles
{
	class Inventory_Base;

	// The vanilla digital code lock's spots on rebuilt house doors (doors 1-10, wood and metal).
	// Added to Expansion's own list, so the lock still fits everything it fitted before.
	class ExpansionCodeLock: Inventory_Base
	{
		inventorySlot[] += {
			"Att_CodeLock1_Wood", "Att_CodeLock1_Metal",
			"Att_CodeLock2_Wood", "Att_CodeLock2_Metal",
			"Att_CodeLock3_Wood", "Att_CodeLock3_Metal",
			"Att_CodeLock4_Wood", "Att_CodeLock4_Metal",
			"Att_CodeLock5_Wood", "Att_CodeLock5_Metal",
			"Att_CodeLock6_Wood", "Att_CodeLock6_Metal",
			"Att_CodeLock7_Wood", "Att_CodeLock7_Metal",
			"Att_CodeLock8_Wood", "Att_CodeLock8_Metal",
			"Att_CodeLock9_Wood", "Att_CodeLock9_Metal",
			"Att_CodeLock10_Wood", "Att_CodeLock10_Metal"
		};
	};

	// The keypad players see on a house door: one on each side of the door, drawn with Expansion's
	// lock model. Client-side only, never saved, never networked. The real lock sits hidden in the
	// door's slot and holds the code. (The door's slot points the lock model into the door, and a
	// door needs a keypad on both sides, like Expansion's two-sided lock on a fence gate.)
	class DevZ_HouseCodeLockVisual: Inventory_Base
	{
		scope = 1;
		displayName = "$STR_EXPANSION_BB_CODE_LOCK";
		model = "\DayZExpansion\Objects\Basebuilding\Items\Codelock\CodeLock.p3d";
		itemSize[] = {2, 2};
		weight = 0;
		hiddenSelections[] = {"light_red", "light_green", "light_screen"};
		hiddenSelectionsTextures[] = {
			"\DayZExpansion\Objects\Basebuilding\Items\Codelock\data\codelock_co.paa",
			"\DayZExpansion\Objects\Basebuilding\Items\Codelock\data\codelock_co.paa",
			"\DayZExpansion\Objects\Basebuilding\Items\Codelock\data\codelock_co.paa"
		};
		hiddenSelectionsMaterials[] = {
			"\DayZExpansion\Objects\Basebuilding\Items\Codelock\data\codelock.rvmat",
			"\DayZExpansion\Objects\Basebuilding\Items\Codelock\data\codelock.rvmat",
			"\DayZExpansion\Objects\Basebuilding\Items\Codelock\data\codelock.rvmat"
		};
		class AnimationSources
		{
			class camo { source = "user"; animPeriod = 0.01; initPhase = 0; };
			class Codelock { source = "user"; animPeriod = 0.01; initPhase = 0; };
			class attach_fence { source = "user"; animPeriod = 0.01; initPhase = 1; };
		};

		// Where the two keypads go, in metres. The door's own geometry gives the direction through
		// the door, but its box includes the frame trim, so the thickness comes from here: the
		// inside keypad sits devz_doorThickness + devz_lockDepth behind the outside one (9 cm
		// measured on Nasdara's rebuilt house doors). devz_keypadDir is the keypad's direction in
		// the lock model's own axes, only used when the door's geometry can't be read.
		devz_doorThickness = 0.09;
		devz_lockDepth = 0.03;
		devz_keypadDir = "0 0 1";
	};
};
