// Per-gun damage and range floors (DevZ_GunDamageConfig). Bullet flight is never touched.
//
// The bullets' and the plate carrier's own numbers are in the mod's config. This script changes single guns and
// bullets on top, around the hit itself, which always lands as the game made it:
//
// 1. Just before a gunshot lands, the game reports what it will do (EEOnDamageCalculated, damageResult). Health is
//    moved first by the difference: a hit that should do 1.25 times its damage takes the extra 0.25 off beforehand,
//    one that should do 0.7 gives 0.3 back beforehand. Never down to 0: the hit itself always deals the final blow,
//    so the killer and the kill feed are as normal. A player at full Health has no room for a give-back, so what's
//    left of it comes back right after the hit.
// 2. Right after it lands (EEHitBy), Shock is set to what it should have taken. Knock-out is checked on the player's
//    next tick (vanilla's Unconsciousness modifier), so the in-between value never counts.
// A hit takes nothing off the Blood bar (only the bleeding it may start does), so Blood isn't touched. Bleeding,
// broken legs, flinch, pain sound and the vest's own wear follow the game's own hit.
//
// Range floors use the bullet's own slow-down that the game reports with each hit (speedCoef: 1 at the muzzle).
//
// Until 2026-11-10 this cancelled the hit and applied it again with ProcessDirectDamage at the new strength. Tested
// then: the game drops a second hit from the same gun on the same player in the same moment, so the re-applied hit
// did nothing at all, and every hit this script changed did no damage.

class DevZ_GunDamage
{
	// Between step 1 and step 2 of one hit
	protected static PlayerBase s_Pending;
	protected static float s_ShockBefore;
	protected static float s_WantShock;
	protected static float s_HealLeft;		// Health still to give back after the hit (a weaker hit on a player at full Health)
	protected static float s_HealLeftZone;
	protected static string s_HealZone;
	protected static string s_LogStart;

	// Step 1, from PlayerBase.EEOnDamageCalculated, after other mods (PvE zones) have had their say. The hit always lands.
	static void BeforeHit(PlayerBase victim, TotalDamageResult damageResult, int damageType, EntityAI source, string dmgZone, string ammo, float speedCoef)
	{
		s_Pending = null;
		if (!g_Game.IsServer() || damageType != DamageType.FIRE_ARM || !source || !damageResult || speedCoef <= 0)
			return;
		DevZ_GunDamageConfig cfg = DevZ_GunDamageConfig.Get();
		if (!cfg.Enabled)
			return;

		float multiplier = 1;
		float healthOnly = 1;
		float shockOnly = 1;
		float floor = 0;
		DevZ_GunDamageRule rule = cfg.WeaponRule(source);
		if (rule)
		{
			multiplier = rule.DamageMultiplier;
			healthOnly = rule.HealthMultiplier;
			shockOnly = rule.ShockMultiplier;
			floor = rule.RangeFloor;
		}
		rule = cfg.AmmoRule(ammo);
		if (rule)
		{
			multiplier *= rule.DamageMultiplier;
			healthOnly *= rule.HealthMultiplier;
			shockOnly *= rule.ShockMultiplier;
			if (rule.RangeFloor > floor)
				floor = rule.RangeFloor;
		}

		float range = 1;
		if (floor > speedCoef)
			range = floor / speedCoef;
		float healthScale = range * multiplier * healthOnly;
		float shockScale = range * multiplier * shockOnly;
		bool sameHealth = Math.AbsFloat(healthScale - 1) < 0.001;
		bool sameShock = Math.AbsFloat(shockScale - 1) < 0.001;
		if (sameHealth && sameShock)
			return;

		// What the game's hit will take: the whole body's Health ("" zone) and the zone hit
		float hitHealth = damageResult.GetDamage("", "Health");
		float hitZone = damageResult.GetDamage(dmgZone, "Health");
		float hitShock = damageResult.GetDamage(dmgZone, "Shock");
		float healthBefore = victim.GetHealth("", "Health");
		s_HealLeft = 0;
		s_HealLeftZone = 0;
		s_HealZone = dmgZone;
		if (!sameHealth)
		{
			float extra = (healthScale - 1) * hitHealth;
			float extraZone = (healthScale - 1) * hitZone;
			float moved = MoveHealth(victim, "", extra);
			float movedZone = MoveHealth(victim, dmgZone, extraZone);
			if (extra < 0)
				s_HealLeft = moved - extra;
			if (extraZone < 0)
				s_HealLeftZone = movedZone - extraZone;
		}

		s_Pending = victim;
		s_ShockBefore = victim.GetHealth("", "Shock");
		s_WantShock = hitShock * shockScale;
		s_LogStart = "";
		if (cfg.LogHits)
		{
			// Each call's result in a local first: several calls as one call's arguments can overwrite each other
			string who = Name(victim);
			string gunName = source.GetType();
			float distance = Distance(victim, source);
			float wantHealth = hitHealth * healthScale;
			float wantShock = s_WantShock;
			string line1 = string.Format("[DevZ_GunDamage] %1 hit by %2 (%3) in %4 at %5 m: speed %6, floor %7, Health x%8, Shock x%9", who, gunName, ammo, dmgZone, distance, speedCoef, floor, healthScale, shockScale);
			string line2 = string.Format(". Game's hit: Health %1 (zone %2), Shock %3. Wanted Health %4, Shock %5. Health was %6", hitHealth, hitZone, hitShock, wantHealth, wantShock, healthBefore);
			s_LogStart = line1 + line2;
		}
	}

	// Step 2, from PlayerBase.EEHitBy: the rest of a give-back, and Shock to what it should have taken
	static void AfterHit(PlayerBase victim)
	{
		if (!s_Pending || s_Pending != victim)
			return;
		s_Pending = null;
		if (victim.IsAlive())
		{
			if (s_HealLeft > 0)
				victim.AddHealth("", "Health", s_HealLeft);
			if (s_HealLeftZone > 0)
				victim.AddHealth(s_HealZone, "Health", s_HealLeftZone);
			float wantShock = Math.Min(s_WantShock, s_ShockBefore);
			victim.SetHealth("", "Shock", s_ShockBefore - wantShock);
		}
		if (s_LogStart != "")
		{
			float healthNow = victim.GetHealth("", "Health");
			float shockNow = victim.GetHealth("", "Shock");
			float shockWas = s_ShockBefore;
			float healLeft = s_HealLeft;
			PrintFormat("%1. Now: Health %2 (given back after: %3), Shock %4 (was %5)", s_LogStart, healthNow, healLeft, shockNow, shockWas);
		}
	}

	// Takes "amount" off a zone's Health before the hit (or gives it back, if negative), never to 0. Returns what it moved.
	protected static float MoveHealth(PlayerBase victim, string zone, float amount)
	{
		if (Math.AbsFloat(amount) < 0.001)
			return 0;
		float now = victim.GetHealth(zone, "Health");
		float most = victim.GetMaxHealth(zone, "Health");
		float after = now - amount;
		if (after < 0.01)
			after = 0.01;
		if (after > most)
			after = most;
		victim.SetHealth(zone, "Health", after);
		return now - after;
	}

	protected static string Name(PlayerBase victim)
	{
		if (victim.GetIdentity())
			return victim.GetIdentity().GetName();
		return victim.GetType();
	}

	protected static float Distance(PlayerBase victim, EntityAI source)
	{
		EntityAI shooter = source.GetHierarchyRoot();
		vector victimAt = victim.GetPosition();
		vector shooterAt = shooter.GetPosition();
		return Math.Round(vector.Distance(victimAt, shooterAt));
	}
}
