class CfgPatches
{
	class DevZ_AutoStack
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class DevZ_AutoStack
	{
		dir = "DevZ_AutoStack";
		name = "DevZ Auto Stack";
		author = "DevilinJake";
		type = "mod";
		dependencies[] = {"Game", "World"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"DevZ_AutoStack/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"DevZ_AutoStack/scripts/4_World"};
			};
		};
	};
};
