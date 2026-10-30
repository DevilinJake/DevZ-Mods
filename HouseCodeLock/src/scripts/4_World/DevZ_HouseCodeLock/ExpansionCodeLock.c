modded class ExpansionCodeLock
{
	// The house this lock is fitted to, or null when it's on an item (gate, wall, tent...) or loose
	BuildingBase DevZ_GetHouse()
	{
		if (!GetInventory() || !GetInventory().IsAttachment())
			return null;

		return BuildingBase.Cast(GetHierarchyParent());
	}

	// The house door this lock is on, or -1
	int DevZ_GetDoorIndex()
	{
		BuildingBase house = DevZ_GetHouse();
		if (!house)
			return -1;

		InventoryLocation location = new InventoryLocation();
		GetInventory().GetCurrentInventoryLocation(location);
		return DevZ_HouseCodeLock.DoorIndexForSlot(house, InventorySlots.GetSlotName(location.GetSlot()));
	}

	override void SetActions()
	{
		super.SetActions();

		AddAction(DevZ_ActionAttachCodeLockToHouse);
		AddAction(DevZ_ActionHouseCodeLock);
		AddAction(DevZ_ActionHouseChangeCode);
	}

	// Counts as a door lock, like the vanilla locks: a lockpick can't lock or unlock a door with
	// one fitted
	override bool IsExternalLockType()
	{
		if (DevZ_GetHouse())
			return true;

		return super.IsExternalLockType();
	}

	// On a house the lock is "open" while its door is. Expansion uses this to decide whether
	// setting a code locks straight away (door shut) or waits for "Close & lock" (door open),
	// the same as on a gate.
	override bool IsOpen()
	{
		int doorIndex = DevZ_GetDoorIndex();
		if (doorIndex > -1)
			return DevZ_GetHouse().IsDoorOpen(doorIndex);

		return super.IsOpen();
	}

	// "Close & lock" shuts the house door, then Expansion locks the lock
	override void Expansion_Close(string selection)
	{
		int doorIndex = DevZ_GetDoorIndex();
		if (doorIndex > -1)
		{
			BuildingBase house = DevZ_GetHouse();
			if (house.IsDoorOpen(doorIndex))
				house.CloseDoor(doorIndex);
			return;
		}

		super.Expansion_Close(selection);
	}

	// Expansion remembers a player who enters the right code by asking the lock's parent for its
	// lock, which finds nothing when the keypad talks to the lock itself. Remember them here.
	override void RPC_Expansion_Unlock(PlayerIdentity sender, ParamsReadContext ctx)
	{
		bool wasLocked = ExpansionIsLocked();

		super.RPC_Expansion_Unlock(sender, ctx);

		if (!DevZ_GetHouse() || !wasLocked || ExpansionIsLocked() || !sender)
			return;

		if (GetExpansionSettings().GetBaseBuilding().RememberCode)
			AddUser(PlayerBase.Cast(sender.GetPlayer()));
	}

	// After a restart Expansion clears the code of any lock that isn't on an item, which would
	// unlock every house door. Put the code, locked state and known players back.
	override void DeferredInit()
	{
		BuildingBase house = DevZ_GetHouse();
		if (!house || !g_Game.IsServer() || !HasCode())
		{
			super.DeferredInit();
			return;
		}

		string code = m_Expansion_Code;
		bool locked = m_Expansion_Locked;
		TStringArray knownUIDs = new TStringArray();
		knownUIDs.Copy(m_Expansion_KnownUIDs);

		super.DeferredInit();

		m_Expansion_Code = code;
		m_Expansion_CodeLength = code.Length();
		m_Expansion_Locked = locked;
		m_Expansion_KnownUIDs.Copy(knownUIDs);
		SetSlotLock(house, true);
		SetTakeable(false);
		SetSynchDirty();
	}

	// Expansion only lights the lock green when it's on an item; on a house it would always be red
	override protected void UpdateVisuals_Deferred()
	{
		if (!DevZ_GetHouse())
		{
			super.UpdateVisuals_Deferred();
			return;
		}

		ShowSelection("camo");
		ShowSelection("Codelock");
		HideSelection("attach_fence");

		if (!IsMissionClient())
			return;

		if (ExpansionIsLocked())
		{
			SetObjectMaterial(0, DEFAULT_MATERIAL);
			SetObjectMaterial(1, GREEN_LIGHT_GLOW);
			SetObjectMaterial(2, DEFAULT_MATERIAL);
		}
		else
		{
			SetObjectMaterial(0, RED_LIGHT_GLOW);
			SetObjectMaterial(1, DEFAULT_MATERIAL);
			SetObjectMaterial(2, DEFAULT_MATERIAL);
		}
	}
}
