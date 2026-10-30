// DevZ Barrel Storage: a rack that holds 6 barrels. Based on INVICTUZ_Barrel_Storage by Xantarn
// (Workshop 3406014628), fixed for DayZ 1.30.
//
// The item, slot and folder names are still the original mod's (Invictuz_Barrel_Storage,
// BarrelStorage_Kit, SlotBarrel1-6, prefix Invictuz_Barrel_Storage): servers switching from it
// keep their placed racks and types.xml entries, and the models find their textures and barrel
// proxies by that folder name.
class CfgPatches
{
	class DevZ_BarrelStorage
	{
		units[] = {"Invictuz_Barrel_Storage", "BarrelStorage_Kit"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {
			"DZ_Data",
			"DZ_Vehicles_Wheeled",
			"DZ_Vehicles_Parts",
			"DZ_Scripts",
			"DZ_Sounds_Effects",
			"DZ_Gear_Tools",
			"DZ_Gear_Containers",
			"DZ_Gear_Consumables",
			"JM_CF_Scripts"};
	};
};

class CfgMods
{
	class DevZ_BarrelStorage
	{
		dir = "DevZ_BarrelStorage";
		name = "DevZ Barrel Storage";
		author = "DevilinJake";
		credits = "Xantarn (INVICTUZ_Barrel_Storage: models, textures and the original mod)";
		type = "mod";
		dependencies[] = {"World"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"Invictuz_Barrel_Storage/scripts/4_World"};
			};
		};
	};
};

class CfgVehicles
{
	class Container_base;
	// Barrels fit the rack's six slots, on top of the vanilla ones (truck bed)
	class Barrel_ColorBase : Container_base
	{
		inventorySlot[] += {
			"SlotBarrel1",
			"SlotBarrel2",
			"SlotBarrel3",
			"SlotBarrel4",
			"SlotBarrel5",
			"SlotBarrel6"};
	};

	// The placed rack
	class Invictuz_Barrel_Storage : Container_base
	{
		scope = 2;
		displayName = "Barrel Storage";
		descriptionShort = "A rack that holds up to six barrels. Dismantle it with a screwdriver once it's empty.";
		model = "Invictuz_Barrel_Storage\data\BarrelStorage.p3d";
		SingleUseActions[] = {527};
		InteractActions[] = {1025, 1026};
		ContinuousActions[] = {155};
		destroyOnEmpty = 0;
		varQuantityDestroyOnMin = 0;
		quantityBar = 0;
		varQuantityMax = 0;
		carveNavmesh = 1;
		canBeDigged = 0;
		heavyItem = 1;
		weight = 10000;
		itemBehaviour = 2;
		inventoryCondition = "true";
		allowOwnedCargoManipulation = 1;
		physLayer = "item_large";
		itemSize[] = {4, 4};
		attachments[] = {
			"SlotBarrel1",
			"SlotBarrel2",
			"SlotBarrel3",
			"SlotBarrel4",
			"SlotBarrel5",
			"SlotBarrel6"};
		hiddenSelections[] = {"zbytek"};
		hiddenSelectionsTextures[] = {"Invictuz_Barrel_Storage\data\Invictuz_BarrelStorage.paa"};
		hiddenSelectionsMaterials[] = {"Invictuz_Barrel_Storage\data\Invictuz_BarrelStorage.rvmat"};
		class GUIInventoryAttachmentsProps
		{
			class Top
			{
				name = "Top";
				description = "";
				attachmentSlots[] = {"SlotBarrel1", "SlotBarrel2"};
				icon = "barrel";
			};
			class Middle
			{
				name = "Middle";
				description = "";
				attachmentSlots[] = {"SlotBarrel3", "SlotBarrel4"};
				icon = "barrel";
			};
			class Bottom
			{
				name = "Bottom";
				description = "";
				attachmentSlots[] = {"SlotBarrel5", "SlotBarrel6"};
				icon = "barrel";
			};
		};
	};

	// The placement preview shown while deploying the kit
	class BarrelStorage_Holo : Invictuz_Barrel_Storage
	{
		scope = 2;
		physLayer = "item_small";
		hologramMaterial = "holo";
		hologramMaterialPath = "Invictuz_Barrel_Storage\data";
		attachments[] = {};
	};

	// The kit: place it like a fence kit to build the rack
	class Inventory_Base;
	class BarrelStorage_Kit : Inventory_Base
	{
		scope = 2;
		displayName = "Barrel Storage Kit";
		descriptionShort = "Place it to build a rack that holds up to six barrels.";
		projectionTypename = "BarrelStorage_Holo";
		model = "Invictuz_Barrel_Storage\data\BarrelStorage_Kit.p3d";
		SingleUseActions[] = {527};
		InteractActions[] = {1025, 1026};
		ContinuousActions[] = {155};
		destroyOnEmpty = 0;
		varQuantityDestroyOnMin = 0;
		quantityBar = 0;
		varQuantityMax = 0;
		carveNavmesh = 1;
		canBeDigged = 0;
		heavyItem = 1;
		bounding = "BSphere";
		physLayer = "item_large";
		simulation = "inventoryItem";
		itemSize[] = {10, 5};
		itemBehaviour = 0;
		weight = 6000;
		hiddenSelections[] = {"zbytek"};
		hiddenSelectionsTextures[] = {"Invictuz_Barrel_Storage\data\Invictuz_BarrelStorageKit.paa"};
		hiddenSelectionsMaterials[] = {"Invictuz_Barrel_Storage\data\Invictuz_BarrelStorage_Kit.rvmat"};
		class AnimEvents
		{
			class SoundWeapon
			{
				class movement
				{
					soundSet = "seachest_movement_SoundSet";
					id = 1;
				};
				class pickUpItem_Light
				{
					soundSet = "pickUpSeaChest_SoundSet";
					id = 796;
				};
				class pickUpItem
				{
					soundSet = "pickUpSeaChest_SoundSet";
					id = 797;
				};
				class drop
				{
					soundset = "seachest_drop_SoundSet";
					id = 898;
				};
			};
		};
	};
};

// The six barrel spots in the rack's model (the paths must match the proxies in BarrelStorage.p3d)
class CfgNonAIVehicles
{
	class ProxyAttachment;
	class ProxyBarrelProx1 : ProxyAttachment
	{
		scope = 2;
		inventorySlot = "SlotBarrel1";
		model = "\Invictuz_Barrel_Storage\data\BarrelProx1.p3d";
	};
	class ProxyBarrelProx2 : ProxyAttachment
	{
		scope = 2;
		inventorySlot = "SlotBarrel2";
		model = "\Invictuz_Barrel_Storage\data\BarrelProx2.p3d";
	};
	class ProxyBarrelProx3 : ProxyAttachment
	{
		scope = 2;
		inventorySlot = "SlotBarrel3";
		model = "\Invictuz_Barrel_Storage\data\BarrelProx3.p3d";
	};
	class ProxyBarrelProx4 : ProxyAttachment
	{
		scope = 2;
		inventorySlot = "SlotBarrel4";
		model = "\Invictuz_Barrel_Storage\data\BarrelProx4.p3d";
	};
	class ProxyBarrelProx5 : ProxyAttachment
	{
		scope = 2;
		inventorySlot = "SlotBarrel5";
		model = "\Invictuz_Barrel_Storage\data\BarrelProx5.p3d";
	};
	class ProxyBarrelProx6 : ProxyAttachment
	{
		scope = 2;
		inventorySlot = "SlotBarrel6";
		model = "\Invictuz_Barrel_Storage\data\BarrelProx6.p3d";
	};
};

class CfgSlots
{
	class Slot_SlotBarrel
	{
		name = "SlotBarrel";
		displayName = "Barrel";
		ghostIcon = "barrel";
	};
	class Slot_SlotBarrel1 : Slot_SlotBarrel
	{
		name = "SlotBarrel1";
		displayName = "SlotBarrel1";
		selection = "SlotBarrel1";
		ghostIcon = "barrel";
	};
	class Slot_SlotBarrel2 : Slot_SlotBarrel
	{
		name = "SlotBarrel2";
		displayName = "SlotBarrel2";
		selection = "SlotBarrel2";
		ghostIcon = "barrel";
	};
	class Slot_SlotBarrel3 : Slot_SlotBarrel
	{
		name = "SlotBarrel3";
		displayName = "SlotBarrel3";
		selection = "SlotBarrel3";
		ghostIcon = "barrel";
	};
	class Slot_SlotBarrel4 : Slot_SlotBarrel
	{
		name = "SlotBarrel4";
		displayName = "SlotBarrel4";
		selection = "SlotBarrel4";
		ghostIcon = "barrel";
	};
	class Slot_SlotBarrel5 : Slot_SlotBarrel
	{
		name = "SlotBarrel5";
		displayName = "SlotBarrel5";
		selection = "SlotBarrel5";
		ghostIcon = "barrel";
	};
	class Slot_SlotBarrel6 : Slot_SlotBarrel
	{
		name = "SlotBarrel6";
		displayName = "SlotBarrel6";
		selection = "SlotBarrel6";
		ghostIcon = "barrel";
	};
};
