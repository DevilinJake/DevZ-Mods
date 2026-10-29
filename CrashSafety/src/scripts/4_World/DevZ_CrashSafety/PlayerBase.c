modded class PlayerBase
{
	protected float m_DevZ_CS_KnockoutEnd;

	// Applies crash damage to this player: health loss that never kills, and a knockout
	// held for a set time. Server only.
	void DevZ_CS_ApplyCrash(float severity, string vehicleType)
	{
		if (!g_Game.IsServer() || !IsAlive())
			return;

		DevZ_CrashSafetyConfig cfg = DevZ_CrashSafetyConfig.Get();

		float maxHealth = GetMaxHealth("", "Health");
		float loss = maxHealth * Math.Lerp(cfg.MinHealthLossPercent, cfg.MaxHealthLossPercent, severity) * 0.01;
		float health = GetHealth("", "Health");
		SetHealth("", "Health", Math.Max(health - loss, 1));

		float seconds = 0;
		if (severity >= cfg.KnockoutFromSeverity)
		{
			float koSeverity = Math.InverseLerp(cfg.KnockoutFromSeverity, 1, severity);
			if (cfg.KnockoutFromSeverity >= 1)
				koSeverity = 1;
			seconds = Math.Lerp(cfg.MinKnockoutSeconds, cfg.MaxKnockoutSeconds, koSeverity);
			DevZ_CS_KnockOut(seconds);
		}

		if (cfg.LogCrashes)
		{
			string name = "unknown";
			if (GetIdentity())
				name = GetIdentity().GetName();
			PrintFormat("[DevZ_CrashSafety] %1 crashed in %2: severity %3, health %4 -> %5, knocked out %6 s", name, vehicleType, severity, health, GetHealth("", "Health"), seconds);
		}
	}

	protected void DevZ_CS_KnockOut(float seconds)
	{
		m_DevZ_CS_KnockoutEnd = Math.Max(m_DevZ_CS_KnockoutEnd, g_Game.GetTickTime() + seconds);
		SetHealth("", "Shock", 0);
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(DevZ_CS_HoldKnockout);
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(DevZ_CS_HoldKnockout, 1000, true);
	}

	// Keeps shock at 0 until the knockout time is up, then wakes the player
	protected void DevZ_CS_HoldKnockout()
	{
		if (!IsAlive())
		{
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(DevZ_CS_HoldKnockout);
			return;
		}

		if (g_Game.GetTickTime() < m_DevZ_CS_KnockoutEnd)
		{
			SetHealth("", "Shock", 0);
			return;
		}

		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(DevZ_CS_HoldKnockout);
		m_DevZ_CS_KnockoutEnd = 0;
		// Above vanilla's conscious threshold (50) so the player comes round straight away, still groggy
		SetHealth("", "Shock", GetMaxHealth("", "Shock") * 0.6);
	}

	override void EEKilled(Object killer)
	{
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(DevZ_CS_HoldKnockout);
		super.EEKilled(killer);
	}

	override void EEDelete(EntityAI parent)
	{
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(DevZ_CS_HoldKnockout);
		super.EEDelete(parent);
	}
}
