class CfgPatches
{
	class DevZ_AutoRun
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class DevZ_AutoRun
	{
		dir = "DevZ_AutoRun";
		name = "DevZ Auto Run";
		author = "DevilinJake";
		type = "mod";
		inputs = "DevZ_AutoRun/data/inputs.xml";
		dependencies[] = {"World", "Mission"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"DevZ_AutoRun/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"DevZ_AutoRun/scripts/5_Mission"};
			};
		};
	};
};
