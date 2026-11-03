modded class PlayerBase
{
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		// Other mods (PvE zones) get to cancel the hit first
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;
		return DevZ_GunDamage.OnHit(this, damageResult, damageType, source, dmgZone, ammo, modelPos, speedCoef);
	}
}
