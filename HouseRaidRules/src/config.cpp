class CfgPatches
{
	class DevZ_HouseRaidRules
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		// Needs Ninjins PvP/PvE Upgraded: the zones, the admin flag and the Raid Extension settings
		// are read from it at runtime. Load this mod after it.
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "Ninjins_Core", "Ninjins_PvP_PvE_Upgraded_Zones", "Ninjins_Raid_Extension_v3"};
	};
};

class CfgMods
{
	class DevZ_HouseRaidRules
	{
		dir = "DevZ_HouseRaidRules";
		name = "DevZ House Raid Rules";
		author = "DevilinJake";
		type = "mod";
		dependencies[] = {"Game", "World"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"DevZ_HouseRaidRules/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"DevZ_HouseRaidRules/scripts/4_World"};
			};
		};
	};
};
