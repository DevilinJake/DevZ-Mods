class CfgPatches
{
	class DevZ_CrashSafety
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class DevZ_CrashSafety
	{
		dir = "DevZ_CrashSafety";
		name = "DevZ Crash Safety";
		author = "DevilinJake";
		type = "mod";
		dependencies[] = {"Game", "World"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"DevZ_CrashSafety/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"DevZ_CrashSafety/scripts/4_World"};
			};
		};
	};
};
