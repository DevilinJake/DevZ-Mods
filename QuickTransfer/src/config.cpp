class CfgPatches
{
	class DevZ_QuickTransfer
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class DevZ_QuickTransfer
	{
		dir = "DevZ_QuickTransfer";
		name = "DevZ Quick Transfer";
		author = "DevilinJake";
		type = "mod";
		inputs = "DevZ_QuickTransfer/data/inputs.xml";
		dependencies[] = {"Mission"};

		class defs
		{
			class missionScriptModule
			{
				value = "";
				files[] = {"DevZ_QuickTransfer/scripts/5_Mission"};
			};
		};
	};
};
