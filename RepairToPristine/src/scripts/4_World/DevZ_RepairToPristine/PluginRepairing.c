// Kits listed in CfgDevZ_RepairToPristine repair items all the way to Pristine.
// The game only asks the damaged item (CanBeRepairedToPristine), never the kit, so the kit
// in use is remembered here while a repair is checked or done.
class DevZ_RepairToPristine
{
	static ItemBase s_Kit;
	protected static ref TStringArray s_Kits;

	static bool KitAllows()
	{
		if (!s_Kit)
			return false;

		if (!s_Kits)
		{
			s_Kits = new TStringArray();
			g_Game.ConfigGetTextArray("CfgDevZ_RepairToPristine kits", s_Kits);
		}

		foreach (string kit : s_Kits)
		{
			if (s_Kit.IsKindOf(kit))
				return true;
		}
		return false;
	}
}

modded class PluginRepairing
{
	override bool Repair(PlayerBase player, ItemBase repair_kit, Object item, float specialty_weight, string damage_zone = "", bool use_kit_qty = true)
	{
		DevZ_RepairToPristine.s_Kit = repair_kit;
		bool repaired = super.Repair(player, repair_kit, item, specialty_weight, damage_zone, use_kit_qty);
		DevZ_RepairToPristine.s_Kit = null;
		return repaired;
	}

	override bool CanRepair(ItemBase repair_kit, Object item, string damage_zone = "")
	{
		// The game's check lets some actions (blowtorch) repair Ruined items only when the item
		// can't go to Pristine, so Ruined items are left to the game's normal rules
		if (item && item.GetHealthLevel(damage_zone) != GameConstants.STATE_RUINED)
			DevZ_RepairToPristine.s_Kit = repair_kit;
		bool can = super.CanRepair(repair_kit, item, damage_zone);
		DevZ_RepairToPristine.s_Kit = null;
		return can;
	}
}

modded class ItemBase
{
	override bool CanBeRepairedToPristine()
	{
		return super.CanBeRepairedToPristine() || DevZ_RepairToPristine.KitAllows();
	}
}
