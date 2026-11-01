// Car repairs (doors, engine, chassis, fuel tank and other body parts) don't go through
// PluginRepairing: each action has its own check that stops at Worn. With a listed kit they go
// one step further, Worn to Pristine. Ruined parts stay the game's way.
class DevZ_CarRepairs
{
	// The damage zone at the player's cursor if it's Worn ("" if none). engine = only the
	// Engine zone, otherwise every zone but Engine (the same split the game's two actions use).
	static string WornZoneAt(CarScript car, int component, string geometry, bool engine)
	{
		array<string> selections = new array<string>();
		car.GetActionComponentNameList(component, selections, geometry);

		string zone;
		foreach (string selection : selections)
		{
			if (!DamageSystem.GetDamageZoneFromComponentName(car, selection, zone))
				continue;
			bool isEngine = zone == "Engine";
			if (isEngine != engine)
				continue;
			if (car.GetHealthLevel(zone) == GameConstants.STATE_WORN)
				return zone;
		}
		return "";
	}
}

// Engine, chassis and body zones: the game raises a zone one level and stops at Worn
modded class ActionRepairVehiclePartBase
{
	override void AdjustVehicleHealthServer(ActionData action_data)
	{
		CarScript car = CarScript.Cast(action_data.m_Target.GetObject());
		string damageZone = RepairVehiclePartActionData.Cast(action_data).m_DamageZone;
		if (!g_Game.IsMultiplayer())
			damageZone = m_CurrentDamageZone;

		if (car && damageZone != "" && car.GetHealthLevel(damageZone) == GameConstants.STATE_WORN && DevZ_RepairToPristine.IsListed(action_data.m_MainItem))
		{
			car.SetHealth(damageZone, "", car.GetMaxHealth(damageZone, ""));
			return;
		}

		super.AdjustVehicleHealthServer(action_data);
	}
}

modded class ActionRepairCarEngine
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (super.ActionCondition(player, target, item))
			return true;

		if (!DevZ_RepairToPristine.IsListed(item) || player.GetBrokenLegs() == eBrokenLegs.BROKEN_LEGS)
			return false;

		CarScript car = CarScript.Cast(target.GetObject());
		if (!car || car.EngineIsOn())
			return false;

		string zone = DevZ_CarRepairs.WornZoneAt(car, target.GetComponentIndex(), LOD.NAME_VIEW, true);
		if (zone == "")
			return false;

		m_CurrentDamageZone = zone;
		m_LastValidComponentIndex = target.GetComponentIndex();
		return true;
	}
}

modded class ActionRepairCarChassis
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (super.ActionCondition(player, target, item))
			return true;

		if (!DevZ_RepairToPristine.IsListed(item) || player.GetBrokenLegs() == eBrokenLegs.BROKEN_LEGS)
			return false;

		CarScript car = CarScript.Cast(target.GetObject());
		if (!car)
			return false;

		string zone = DevZ_CarRepairs.WornZoneAt(car, target.GetComponentIndex(), "", false);
		if (zone == "")
			return false;

		// Planks and tarp only repair a truck's wooden/tarp back, as in the game
		if ((WoodenPlank.Cast(item) || Fabric.Cast(item)) && zone != "BackWood" && zone != "BackTarp")
			return false;

		m_CurrentDamageZone = zone;
		m_LastValidComponentIndex = target.GetComponentIndex();
		return true;
	}
}

// Doors, hood and trunk (CarDoor items, on the car or loose)
modded class ActionRepairCarPart
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (super.ActionCondition(player, target, item))
			return true;

		CarDoor carDoor = CarDoor.Cast(target.GetObject());
		if (!carDoor || !player || player.GetBrokenLegs() == eBrokenLegs.BROKEN_LEGS)
			return false;

		if (carDoor.GetHealthLevel("") != GameConstants.STATE_WORN || !DevZ_RepairToPristine.IsListed(item))
			return false;

		// On a car the door has to be open, as in the game
		CarScript car = CarScript.Cast(target.GetParent());
		if (car)
		{
			InventoryLocation loc = new InventoryLocation();
			if (!carDoor.GetInventory().GetCurrentInventoryLocation(loc) || loc.GetSlot() == -1)
				return false;

			string slotName = InventorySlots.GetSlotName(loc.GetSlot());
			if (slotName && car.GetCarDoorsState(slotName) != CarDoorState.DOORS_OPEN)
				return false;
		}
		return true;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		CarDoor carDoor = CarDoor.Cast(action_data.m_Target.GetObject());
		if (!carDoor || carDoor.GetHealthLevel("") != GameConstants.STATE_WORN || !DevZ_RepairToPristine.IsListed(action_data.m_MainItem))
		{
			super.OnFinishProgressServer(action_data);
			return;
		}

		carDoor.SetHealth("", "", carDoor.GetMaxHealth("", ""));

		// Uses up the kit the same way the game's door repair does
		ItemBase kit = action_data.m_MainItem;
		if (kit.HasQuantity())
		{
			if (kit.GetQuantity() > 1)
				kit.SetQuantity(kit.GetQuantity() - kit.GetQuantityMax() * 0.25);
			else
				kit.Delete();
		}
	}
}
