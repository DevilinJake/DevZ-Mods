modded class PlayerBase
{
	// Health moved just before a gunshot lands (DevZ_GunDamage.BeforeHit). Other mods (PvE zones) get to cancel the hit first.
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;
		DevZ_GunDamage.BeforeHit(this, damageResult, damageType, source, dmgZone, ammo, speedCoef);
		return true;
	}

	// Shock put right just after (DevZ_GunDamage.AfterHit)
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		DevZ_GunDamage.AfterHit(this);
	}

	override void EEKilled(Object killer)
	{
		DevZ_GunDamageSelfTest.NoteKilled(killer);
		super.EEKilled(killer);
	}
}
