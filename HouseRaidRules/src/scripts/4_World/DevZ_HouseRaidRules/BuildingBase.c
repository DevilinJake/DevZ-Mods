// Rebuilt walls, doors, windows and barricades are construction parts of the house itself (vanilla
// Rebuilding on BuildingBase), not items, so Ninjins Raid Extension's ItemBase hook never sees hits
// on them. Two hooks:
// - BuildingBase.EEOnDamageCalculated: the same hook Ninjins uses on items. A hit on a built part, or
//   anywhere on a house that has built parts, has to pass DevZ_HouseRaidRules first. Returning false
//   cancels the damage before any health is taken.
// - Rebuilding.DestroyPartServer: the backstop. The game calls it with no player when a part's health
//   reaches zero. If raiding isn't allowed there right now, the part is kept and its health restored,
//   whatever took it down.

modded class BuildingBase
{
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;
		if (!g_Game || !g_Game.IsServer())
			return true;
		string partName = DevZ_HouseRaidRules.RebuiltPartName(this, dmgZone);
		if (partName == "" && !DevZ_HouseRaidRules.HasBuiltPart(this))
			return true;
		// The house's own position, as Ninjins uses the item's position: on buildings modelPos isn't
		// reliably model space, and ModelToWorld on it can land kilometres away, outside every zone
		return DevZ_HouseRaidRules.MayDamagePart(this, partName, source, damageType, ammo, GetPosition());
	}
}

modded class Rebuilding
{
	override void DestroyPartServer(Man player, string part_name, int action_id, bool destroyed_by_connected_part = false)
	{
		BuildingBase house = BuildingBase.Cast(m_EntityParent);
		bool damageDriven = !player && !destroyed_by_connected_part && house && g_Game && g_Game.IsServer();
		if (damageDriven && DevZ_HouseRaidRules.ShouldKeepPart(house, part_name))
		{
			string damageZone;
			if (DamageSystem.GetDamageZoneFromComponentName(house, part_name, damageZone))
				house.SetHealthMax(damageZone);
			return;
		}
		super.DestroyPartServer(player, part_name, action_id, destroyed_by_connected_part);
	}
}
