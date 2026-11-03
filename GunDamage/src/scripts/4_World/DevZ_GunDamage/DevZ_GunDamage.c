// Per-gun damage and range floors (DevZ_GunDamageConfig). Bullet flight is never touched.
//
// Just before a gunshot's damage lands on a player, the game reports it to
// EEOnDamageCalculated, which can cancel it. If the gun or bullet has its own multiplier, or the
// bullet slowed down past its range floor, the hit is cancelled and applied again straight away
// with ProcessDirectDamage at the new strength. The re-applied hit goes through the normal damage
// system (armour, bleeding, broken legs, flinch, pain sound, kill credit), so those happen once.
// Blood splashes and impact sounds come from the bullet on players' PCs and aren't affected.
//
// speedCoef is the game's own damage scale for how much the bullet has slowed; the re-applied hit
// uses max(speedCoef, floor) x multiplier instead. LogHits writes both hits' damage, so a test can
// confirm the re-applied hit is exactly that much stronger or weaker than the original.

class DevZ_GunDamage
{
	// The player being hit again right now (ProcessDirectDamage calls EEOnDamageCalculated again)
	protected static PlayerBase s_Reapplying;
	protected static string s_LogStart;
	protected static float s_Scale;
	protected static float s_OldHealth;
	protected static float s_OldShock;
	protected static float s_OldBlood;

	// EEOnDamageCalculated on a player: false = the hit was cancelled and applied again
	static bool OnHit(PlayerBase victim, TotalDamageResult damageResult, int damageType, EntityAI source, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!g_Game.IsServer() || damageType != DamageType.FIRE_ARM || !source || !damageResult)
			return true;
		if (s_Reapplying == victim)
		{
			LogReapplied(damageResult, dmgZone);
			return true;
		}

		DevZ_GunDamageConfig cfg = DevZ_GunDamageConfig.Get();
		if (!cfg.Enabled)
			return true;

		float multiplier = 1;
		float floor = 0;
		DevZ_GunDamageRule rule = cfg.WeaponRule(source);
		if (rule)
		{
			multiplier = rule.DamageMultiplier;
			floor = rule.RangeFloor;
		}
		rule = cfg.AmmoRule(ammo);
		if (rule)
		{
			multiplier *= rule.DamageMultiplier;
			floor = Math.Max(floor, rule.RangeFloor);
		}

		float coef = Math.Max(speedCoef, floor) * multiplier;
		if (Math.AbsFloat(coef - speedCoef) < 0.001 && !cfg.TestReapplyAll)
			return true;
		if (speedCoef <= 0)
			return true;

		if (cfg.LogHits)
		{
			s_Scale = coef / speedCoef;
			s_OldHealth = damageResult.GetDamage(dmgZone, "Health");
			s_OldShock = damageResult.GetDamage(dmgZone, "Shock");
			s_OldBlood = damageResult.GetDamage(dmgZone, "Blood");
			s_LogStart = string.Format("[DevZ_GunDamage] %1 hit by %2 (%3) in %4 at %5 m: speed %6, x%7 (multiplier %8, floor %9)",
				Name(victim), source.GetType(), ammo, dmgZone, Distance(victim, source), speedCoef, s_Scale, multiplier, floor);
		}

		s_Reapplying = victim;
		victim.ProcessDirectDamage(damageType, source, dmgZone, ammo, modelPos, coef);
		s_Reapplying = null;
		return false;
	}

	protected static void LogReapplied(TotalDamageResult damageResult, string dmgZone)
	{
		if (!DevZ_GunDamageConfig.Get().LogHits)
			return;
		float health = damageResult.GetDamage(dmgZone, "Health");
		float shock = damageResult.GetDamage(dmgZone, "Shock");
		float blood = damageResult.GetDamage(dmgZone, "Blood");
		string got = "-";
		if (s_OldHealth > 0)
			got = (health / s_OldHealth).ToString();
		PrintFormat("%1; Health %2 -> %3, Shock %4 -> %5, Blood %6 -> %7 (expected x%8, got x%9)",
			s_LogStart, s_OldHealth, health, s_OldShock, shock, s_OldBlood, blood, s_Scale, got);
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
		return Math.Round(vector.Distance(victim.GetPosition(), shooter.GetPosition()));
	}
}
