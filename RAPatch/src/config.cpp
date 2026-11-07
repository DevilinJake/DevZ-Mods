// Written by generate_config.py: edit that, or part_health.csv
class CfgPatches
{
	class DevZ_RAPatch
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "JM_CF_Scripts", "RA_BaseBuilding_Scripts", "Rearmed_BaseBuilding", "Gear_CodeLock", "BaseBuilding_CompoundGate", "BaseBuilding_CompoundWall", "BaseBuilding_DoorWall", "BaseBuilding_DoorFrame", "BaseBuilding_DoubleDoorWall", "BaseBuilding_DoubleDoorFrame", "BaseBuilding_Floor", "BaseBuilding_Foundation", "BaseBuilding_GarageDoor", "BaseBuilding_HalfWall", "BaseBuilding_LadderHatch", "BaseBuilding_HatchLadder", "BaseBuilding_Ramp", "BaseBuilding_Roof", "BaseBuilding_RoofWall", "BaseBuilding_Stairs", "BaseBuilding_StairsStraight", "BaseBuilding_StepLadder", "RearmedServer_BaseBuilding_Storage", "RearmedServer_Gear_Territory", "BaseBuilding_TriangleFloor", "BaseBuilding_TriangleFoundation", "BaseBuilding_TriangleLadderHatch", "BaseBuilding_Wall", "BaseBuilding_WindowFrame", "BaseBuilding_Windows"};
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
	class RAPatch_SleepingBag_Base : Inventory_Base
	{
		scope = 1;
		displayName = "Sleeping Bag";
		descriptionShort = "Lay it out and set your respawn on it. Based on Zenarchist's sleeping bag.";
		model = "DevZ_RAPatch\data\sleepingbag\sleepingbag.p3d";
		soundImpactType = "textile";
		hiddenSelections[] = {"packed"};
		hiddenSelectionsMaterials[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed.rvmat"};
		inventorySlot[] = {"Backpack_1"};
		weight = 1000;
		absorbency = 0.3;
		varWetMax = 0.49;
		itemSize[] = {4, 2};
		repairableWithKits[] = {5, 2};
		repairCosts[] = {30, 25};
		itemBehaviour = 2;
		rotationFlags = 16;
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 250;
					healthLevels[] =
					{
						{1, {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed.rvmat"}},
						{0.7, {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed.rvmat"}},
						{0.5, {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed_damage.rvmat"}},
						{0.3, {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed_damage.rvmat"}},
						{0, {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed_destruct.rvmat"}}
					};
				};
			};
		};
	};
	class RAPatch_SleepingBagDeployed_Base : Inventory_Base
	{
		scope = 1;
		displayName = "Sleeping Bag";
		descriptionShort = "Set your respawn here, or pack it up.";
		model = "DevZ_RAPatch\data\sleepingbag\sleepingbag_deployed.p3d";
		soundImpactType = "textile";
		hiddenSelections[] = {"deployed"};
		hiddenSelectionsMaterials[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag.rvmat"};
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
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 250;
					healthLevels[] =
					{
						{1, {"DevZ_RAPatch\data\sleepingbag\sleepingbag.rvmat"}},
						{0.7, {"DevZ_RAPatch\data\sleepingbag\sleepingbag.rvmat"}},
						{0.5, {"DevZ_RAPatch\data\sleepingbag\sleepingbag_damage.rvmat"}},
						{0.3, {"DevZ_RAPatch\data\sleepingbag\sleepingbag_damage.rvmat"}},
						{0, {"DevZ_RAPatch\data\sleepingbag\sleepingbag_destruct.rvmat"}}
					};
				};
			};
		};
	};
	class RAPatch_SleepingBag_Black : RAPatch_SleepingBag_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed_black_co.paa"};
	};
	class RAPatch_SleepingBag_Black_Deployed : RAPatch_SleepingBagDeployed_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_black_co.paa"};
	};
	class RAPatch_SleepingBag_Blue : RAPatch_SleepingBag_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed_blue_co.paa"};
	};
	class RAPatch_SleepingBag_Blue_Deployed : RAPatch_SleepingBagDeployed_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_blue_co.paa"};
	};
	class RAPatch_SleepingBag_Green : RAPatch_SleepingBag_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed_green_co.paa"};
	};
	class RAPatch_SleepingBag_Green_Deployed : RAPatch_SleepingBagDeployed_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_green_co.paa"};
	};
	class RAPatch_SleepingBag_Grey : RAPatch_SleepingBag_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed_grey_co.paa"};
	};
	class RAPatch_SleepingBag_Grey_Deployed : RAPatch_SleepingBagDeployed_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_grey_co.paa"};
	};
	class RAPatch_SleepingBag_Red : RAPatch_SleepingBag_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_packed_red_co.paa"};
	};
	class RAPatch_SleepingBag_Red_Deployed : RAPatch_SleepingBagDeployed_Base
	{
		scope = 2;
		hiddenSelectionsTextures[] = {"DevZ_RAPatch\data\sleepingbag\sleepingbag_red_co.paa"};
	};
};
