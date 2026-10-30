class CfgPatches
{
	class DevZ_SatchelSoundFix
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class DevZ_SatchelSoundFix
	{
		dir = "DevZ_SatchelSoundFix";
		name = "DevZ Satchel Sound Fix";
		author = "DevilinJake";
		type = "mod";
		dependencies[] = {"World"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"DevZ_SatchelSoundFix/scripts/4_World"};
			};
		};
	};
};
