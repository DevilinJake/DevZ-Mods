// When a loose-ammo pile lands in a player's inventory or a container, merge it into other piles
// of the same ammo there, fullest first. Runs on the server only; the game syncs the result.
modded class Ammunition_Base
{
	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);

		if (!g_Game.IsServer() || !DevZ_IsStorageLocation(newLoc))
			return;

		DevZ_AutoStackConfig cfg = DevZ_AutoStackConfig.Get();
		if (!cfg.Enabled)
			return;

		// Restart the timer if the pile moves again before it fires
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(DevZ_AutoStack);
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(DevZ_AutoStack, cfg.DelayMs, false);
	}

	override void EEDelete(EntityAI parent)
	{
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(DevZ_AutoStack);
		super.EEDelete(parent);
	}

	protected bool DevZ_IsStorageLocation(InventoryLocation loc)
	{
		int type = loc.GetType();
		return type == InventoryLocationType.CARGO || type == InventoryLocationType.ATTACHMENT;
	}

	protected void DevZ_AutoStack()
	{
		if (IsSetForDeletion() || IsRuined() || GetAmmoCount() <= 0)
			return;

		InventoryLocation loc = new InventoryLocation();
		if (!GetInventory().GetCurrentInventoryLocation(loc) || !DevZ_IsStorageLocation(loc))
			return;

		DevZ_AutoStackConfig cfg = DevZ_AutoStackConfig.Get();
		EntityAI area;
		PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
		if (player)
		{
			if (!cfg.StackInPlayerInventory)
				return;
			area = player;
		}
		else
		{
			if (!cfg.StackInContainers)
				return;
			area = loc.GetParent();
		}
		if (!area)
			return;

		// Other piles of the same ammo with room left, fullest first
		array<EntityAI> items = new array<EntityAI>();
		area.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		array<Ammunition_Base> targets = new array<Ammunition_Base>();
		foreach (EntityAI item : items)
		{
			Ammunition_Base pile = Ammunition_Base.Cast(item);
			if (!pile || pile == this || pile.GetType() != GetType())
				continue;
			if (pile.IsRuined() || pile.IsSetForDeletion() || pile.GetAmmoCount() >= pile.GetAmmoMax())
				continue;
			DevZ_InsertByCount(targets, pile);
		}

		foreach (Ammunition_Base target : targets)
		{
			DevZ_MoveRoundsTo(target);
			if (GetAmmoCount() <= 0)
				break;
		}

		if (GetAmmoCount() <= 0)
			DeleteSafe();
	}

	protected void DevZ_InsertByCount(array<Ammunition_Base> list, Ammunition_Base pile)
	{
		for (int i = 0; i < list.Count(); i++)
		{
			if (pile.GetAmmoCount() > list[i].GetAmmoCount())
			{
				list.InsertAt(pile, i);
				return;
			}
		}
		list.Insert(pile);
	}

	// Moves rounds one at a time so each keeps its own condition, like the game's own combine
	protected void DevZ_MoveRoundsTo(Ammunition_Base target)
	{
		int before = target.GetAmmoCount();
		float targetHealth = target.GetHealth();
		int moved = 0;
		float damage;
		string cartridge;

		while (target.GetAmmoCount() < target.GetAmmoMax() && GetAmmoCount() > 0)
		{
			if (!ServerAcquireCartridge(damage, cartridge))
				break;
			if (!target.ServerStoreCartridge(damage, cartridge))
			{
				ServerStoreCartridge(damage, cartridge);
				break;
			}
			moved++;
		}

		if (moved == 0)
			return;

		target.SetHealth("", "", (before * targetHealth + moved * GetHealth()) / target.GetAmmoCount());
		target.SetSynchDirty();
		SetSynchDirty();
	}
}
