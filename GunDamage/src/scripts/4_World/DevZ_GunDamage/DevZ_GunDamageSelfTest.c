// Testing only (TestOnStart 1 in Config.json): two test characters near the middle of the map (one in a plate
// carrier, one without) take real hits through the game's own damage, each once with this mod's script off and once
// on. The mod's config (bullets, plate carrier) is in force both times, so this checks the script: each "on" hit's
// Health and Shock loss against the "off" one x the multipliers worked out here from the defaults. Then a Longhorn
// shot that only the script makes lethal, which must kill with the gun as the killer.
// The game drops a second hit from the same gun on the same character in the same moment, so hits are 0.3 s apart.
// A hit made by script always reports full speed, so range floors can't be checked here (only with real long shots).
// Results in the script log, "[DevZ_GunDamage] test".
class DevZ_GunDamageCase
{
	string Gun;
	string Ammo;
	string Zone;
	float Speed;
	bool Vest;
	float Health;	// expected multipliers over the script being off
	float Shock;
	float StartHealth = 0;	// 0 = full
	float OffHealth;
	float OffShock;

	void DevZ_GunDamageCase(string gun, string ammo, string zone, float speed, bool vest, float health, float shock)
	{
		Gun = gun;
		Ammo = ammo;
		Zone = zone;
		Speed = speed;
		Vest = vest;
		Health = health;
		Shock = shock;
	}
}

class DevZ_GunDamageSelfTest
{
	protected static ref DevZ_GunDamageSelfTest s_Running;
	protected static Object s_Killer;
	protected static bool s_KillSeen;

	protected ref array<ref DevZ_GunDamageCase> m_Cases = new array<ref DevZ_GunDamageCase>;
	protected PlayerBase m_Vested;
	protected PlayerBase m_Bare;
	protected EntityAI m_Vest;
	protected EntityAI m_Gun;
	protected vector m_At;
	protected int m_Index;
	protected int m_WasEnabled;
	protected int m_Passed;
	protected int m_Failed;
	protected int m_Skipped;

	static void NoteKilled(Object killer)
	{
		if (!s_Running)
			return;
		s_KillSeen = true;
		s_Killer = killer;
	}

	static void Start()
	{
		if (s_Running)
			return;
		s_Running = new DevZ_GunDamageSelfTest;
		if (!s_Running.Begin())
			s_Running = null;
	}

	protected bool Begin()
	{
		m_Cases.Insert(new DevZ_GunDamageCase("FAL", "Bullet_308Win", "Torso", 1, true, 0.7, 0.7));
		m_Cases.Insert(new DevZ_GunDamageCase("M14", "Bullet_308Win", "Torso", 1, true, 0.7, 0.7));
		DevZ_GunDamageCase hurt = new DevZ_GunDamageCase("FAL", "Bullet_308Win", "Torso", 1, true, 0.7, 0.7);
		hurt.StartHealth = 70;	// room for the give-back before the hit
		m_Cases.Insert(hurt);
		m_Cases.Insert(new DevZ_GunDamageCase("LongHorn", "Bullet_308Win", "Torso", 1, true, 1.25, 1.25));
		m_Cases.Insert(new DevZ_GunDamageCase("MP5K", "Bullet_9x19", "Torso", 1, false, 1.12, 1.12));
		m_Cases.Insert(new DevZ_GunDamageCase("UMP45", "Bullet_45ACP", "Torso", 1, false, 1.12, 1.12));
		m_Cases.Insert(new DevZ_GunDamageCase("Glock19", "Bullet_9x19", "LeftLeg", 1, false, 1, 1));
		m_Cases.Insert(new DevZ_GunDamageCase("M4A1", "Bullet_556x45", "Torso", 1, true, 1, 1));
		m_Cases.Insert(new DevZ_GunDamageCase("Mosin9130", "Bullet_762x54", "Torso", 1, true, 1, 1));
		m_Cases.Insert(new DevZ_GunDamageCase("Expansion_AWM", "Bullet_Expansion_338", "Torso", 1, true, 1.479592, 1.2));

		m_At = "7520 0 7480";
		m_At[1] = g_Game.SurfaceY(m_At[0], m_At[2]);
		m_Vested = PlayerBase.Cast(g_Game.CreatePlayer(null, "SurvivorM_Mirek", m_At, 0, "NONE"));
		vector bareAt = m_At;
		bareAt[2] = bareAt[2] + 4;
		m_Bare = PlayerBase.Cast(g_Game.CreatePlayer(null, "SurvivorM_Mirek", bareAt, 0, "NONE"));
		if (!m_Vested || !m_Bare)
		{
			Print("[DevZ_GunDamage] test: couldn't make the test characters.");
			Cleanup();
			return false;
		}
		m_Vest = m_Vested.GetInventory().CreateAttachment("PlateCarrierVest");
		m_WasEnabled = DevZ_GunDamageConfig.Get().Enabled;
		Print("[DevZ_GunDamage] test: started.");
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OffHit, 300, false);
		return true;
	}

	protected void OffHit()
	{
		if (m_Index >= m_Cases.Count())
		{
			KillTest();
			return;
		}
		DevZ_GunDamageCase c = m_Cases[m_Index];
		vector gunAt = m_At;
		gunAt[0] = gunAt[0] + 20;
		if (g_Game.ConfigIsExisting("CfgAmmo " + c.Ammo))
			m_Gun = EntityAI.Cast(g_Game.CreateObjectEx(c.Gun, gunAt, ECE_PLACE_ON_SURFACE));
		if (!m_Gun)
		{
			PrintFormat("[DevZ_GunDamage] test: SKIP %1 %2, not on this server.", c.Gun, c.Ammo);
			m_Skipped++;
			Next();
			return;
		}
		DevZ_GunDamageConfig.Get().Enabled = 0;
		float health, shock;
		Hit(c, health, shock);
		DevZ_GunDamageConfig.Get().Enabled = m_WasEnabled;
		c.OffHealth = health;
		c.OffShock = shock;
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OnHit, 300, false);
	}

	protected void OnHit()
	{
		DevZ_GunDamageCase c = m_Cases[m_Index];
		DevZ_GunDamageConfig.Get().Enabled = 1;
		float onHealth, onShock;
		Hit(c, onHealth, onShock);
		DevZ_GunDamageConfig.Get().Enabled = m_WasEnabled;
		float wantHealth = c.OffHealth * c.Health;
		float wantShock = c.OffShock * c.Shock;
		bool ok = Close(onHealth, wantHealth);
		if (!Close(onShock, wantShock))
			ok = false;
		string result = "FAIL";
		if (ok)
		{
			result = "ok";
			m_Passed++;
		}
		else
		{
			m_Failed++;
		}
		string what = string.Format("[DevZ_GunDamage] test: %1 %2 %3 (%4), plate carrier %5, Health at start %6:", result, c.Gun, c.Ammo, c.Zone, c.Vest, c.StartHealth);
		float offHealth = c.OffHealth;
		float offShock = c.OffShock;
		string got = string.Format(" script off took Health %1 Shock %2; on took Health %3 Shock %4; expected Health %5 Shock %6", offHealth, offShock, onHealth, onShock, wantHealth, wantShock);
		Print(what + got);
		Next();
	}

	protected void Next()
	{
		if (m_Gun)
			g_Game.ObjectDelete(m_Gun);
		m_Gun = null;
		m_Index++;
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OffHit, 300, false);
	}

	// Longhorn through the plate carrier: about 65 Health with the script off, 82 with it on. At 70 Health only the
	// script's hit kills, and the Longhorn must be the killer (the hit itself still deals the final blow).
	protected void KillTest()
	{
		vector gunAt = m_At;
		gunAt[0] = gunAt[0] + 20;
		m_Gun = EntityAI.Cast(g_Game.CreateObjectEx("LongHorn", gunAt, ECE_PLACE_ON_SURFACE));
		Refill(m_Vested);
		m_Vested.SetHealth("", "Health", 70);
		s_KillSeen = false;
		s_Killer = null;
		DevZ_GunDamageConfig.Get().Enabled = 1;
		m_Vested.ProcessDirectDamage(DamageType.FIRE_ARM, m_Gun, "Torso", "Bullet_308Win", "0 0 0", 1);
		DevZ_GunDamageConfig.Get().Enabled = m_WasEnabled;
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KillCheck, 500, false);
	}

	protected void KillCheck()
	{
		bool dead = !m_Vested.IsAlive();
		string killerName = "nobody";
		if (s_Killer)
			killerName = s_Killer.GetType();
		bool ok = dead && s_KillSeen && s_Killer == m_Gun;
		string result = "FAIL";
		if (ok)
		{
			result = "ok";
			m_Passed++;
		}
		else
		{
			m_Failed++;
		}
		PrintFormat("[DevZ_GunDamage] test: %1 kill: Longhorn chest shot at 70 Health through the plate carrier: dead %2, killer %3.", result, dead, killerName);
		DevZ_GunDamageConfig.Get().Enabled = m_WasEnabled;
		Cleanup();
		int passed = m_Passed;
		int failed = m_Failed;
		int skipped = m_Skipped;
		PrintFormat("[DevZ_GunDamage] test: %1 passed, %2 failed, %3 skipped.", passed, failed, skipped);
		s_Running = null;
	}

	protected void Cleanup()
	{
		if (m_Vested)
			g_Game.ObjectDelete(m_Vested);
		if (m_Bare)
			g_Game.ObjectDelete(m_Bare);
		if (m_Gun)
			g_Game.ObjectDelete(m_Gun);
		m_Gun = null;
	}

	protected void Hit(DevZ_GunDamageCase c, out float health, out float shock)
	{
		PlayerBase body = m_Bare;
		if (c.Vest)
			body = m_Vested;
		Refill(body);
		if (c.StartHealth > 0)
			body.SetHealth("", "Health", c.StartHealth);
		if (m_Vest)
			m_Vest.SetHealth("", "Health", m_Vest.GetMaxHealth("", "Health"));
		float h = body.GetHealth("", "Health");
		float s = body.GetHealth("", "Shock");
		body.ProcessDirectDamage(DamageType.FIRE_ARM, m_Gun, c.Zone, c.Ammo, "0 0 0", c.Speed);
		health = h - body.GetHealth("", "Health");
		shock = s - body.GetHealth("", "Shock");
	}

	protected static void Refill(PlayerBase body)
	{
		array<string> zones = new array<string>;
		body.GetDamageZones(zones);
		foreach (string zone : zones)
		{
			body.SetHealth(zone, "Health", body.GetMaxHealth(zone, "Health"));
		}
		body.SetHealth("", "Health", body.GetMaxHealth("", "Health"));
		body.SetHealth("", "Shock", body.GetMaxHealth("", "Shock"));
	}

	protected static bool Close(float got, float want)
	{
		float diff = Math.AbsFloat(got - want);
		return diff <= 0.02 * Math.Max(Math.AbsFloat(want), 1);
	}
}
