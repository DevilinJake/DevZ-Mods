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
};
