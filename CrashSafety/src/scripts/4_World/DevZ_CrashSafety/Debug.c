// TEMPORARY diagnostics, switched on by DebugLog in the settings. Remove this file and the setting
// once vehicle damage is sorted. Applied before Vehicles.c, so "hit" lines show each hit before the
// floors touch it, and "after" lines show where it actually landed. "contact" lines show raw
// collisions and what the crash code turned them into.

modded class CarScript
{
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		bool apply = super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		if (g_Game.IsServer() && DevZ_CrashSafetyConfig.Get().DebugLog && damageResult)
			PrintFormat("[DevZ_CrashSafety:debug] %1 hit type %2 ammo %3 zone %4: hull -%5 engine -%6 fueltank -%7 | before hull %8 engine %9", GetType(), damageType, ammo, dmgZone, damageResult.GetDamage("", "Health"), damageResult.GetDamage("Engine", "Health"), damageResult.GetDamage("FuelTank", "Health"), GetHealth("", "Health"), GetHealth("Engine", "Health"));
		return apply;
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		if (g_Game.IsServer() && DevZ_CrashSafetyConfig.Get().DebugLog)
			PrintFormat("[DevZ_CrashSafety:debug] %1 after: hull %2 engine %3 fueltank %4", GetType(), GetHealth("", "Health"), GetHealth("Engine", "Health"), GetHealth("FuelTank", "Health"));
	}

	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		if (!g_Game.IsServer() || !DevZ_CrashSafetyConfig.Get().DebugLog)
			return;
		PrintFormat("[DevZ_CrashSafety:debug] %1 zone '%2' level %3 -> %4 (health %5)", GetType(), zone, oldLevel, newLevel, GetHealth(zone, "Health"));
		if (newLevel == GameConstants.STATE_RUINED)
			DumpStack();
	}

	// Every collision the physics reports, before Expansion and vanilla decide what it's worth
	override void OnContact(string zoneName, vector localPos, IEntity other, Contact data)
	{
		bool log = g_Game.IsServer() && DevZ_CrashSafetyConfig.Get().DebugLog;
		int cachedBefore = m_ContactCache.Count();
		if (log)
			PrintFormat("[DevZ_CrashSafety:debug] %1 contact zone '%2' with %3: impulse %4, speed %5 km/h, momentum %6 (prev %7), cached %8", GetType(), zoneName, other, data.Impulse, data.RelativeVelocityBefore.Length() * 3.6, GetMomentum(), m_MomentumPrevTick, cachedBefore);
		super.OnContact(zoneName, localPos, other, data);
		if (log && cachedBefore == 0 && m_ContactCache.Count() > 0)
			PrintFormat("[DevZ_CrashSafety:debug] %1 contact cached: impulse %2 -> damage %3 (min %4, heavy %5)", GetType(), m_ContactCache.GetElement(0)[0].m_Impulse, m_ContactCache.GetElement(0)[0].m_Impulse * m_dmgContactCoef, GameConstants.CARS_CONTACT_DMG_MIN, GameConstants.CARS_CONTACT_DMG_THRESHOLD);
	}

	override void EEKilled(Object killer)
	{
		if (g_Game.IsServer() && DevZ_CrashSafetyConfig.Get().DebugLog)
		{
			PrintFormat("[DevZ_CrashSafety:debug] %1 DESTROYED by %2", GetType(), killer);
			DumpStack();
		}
		super.EEKilled(killer);
	}
}
