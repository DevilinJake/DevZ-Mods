class CfgPatches
{
	class DevZ_GPSArrowFix
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		// Expansion's navigation scripts first, so its IngameHud members exist when ours compiles
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DayZExpansion_Navigation_Scripts"};
	};
};

class CfgMods
{
	class DevZ_GPSArrowFix
	{
		dir = "DevZ_GPSArrowFix";
		name = "DevZ GPS Arrow Fix";
		author = "DevilinJake";
		type = "mod";
		dependencies[] = {"Mission"};

		class defs
		{
			class missionScriptModule
			{
				value = "";
				files[] = {"DevZ_GPSArrowFix/scripts/5_Mission"};
			};
		};
	};
};
