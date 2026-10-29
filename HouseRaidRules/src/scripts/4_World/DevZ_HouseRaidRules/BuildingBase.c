// Rebuilt walls, doors, windows and barricades are construction parts of the house itself (vanilla
// Rebuilding on BuildingBase), not items, so Ninjins Raid Extension's ItemBase hook never sees hits
// on them. This is the same hook on the house: when the damaged zone is a built part, the hit has to
// pass DevZ_HouseRaidRules first. Returning false cancels the damage.

modded class BuildingBase
{
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;
		if (!g_Game || !g_Game.IsServer())
			return true;
		string partName = DevZ_HouseRaidRules.RebuiltPartName(this, dmgZone);
		if (partName == "")
			return true;
		return DevZ_HouseRaidRules.MayDamagePart(this, partName, source, damageType, ammo, ModelToWorld(modelPos));
	}
}
