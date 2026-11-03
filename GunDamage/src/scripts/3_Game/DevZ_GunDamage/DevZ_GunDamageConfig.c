// One gun or bullet type with its own damage. For guns, ClassName matches that class and every class
// built on it, so "BoltActionRifle_Base" covers every bolt-action rifle; bullets match by exact
// name. The first matching entry wins.
class DevZ_GunDamageRule
{
	string ClassName;
	// Damage x this, on top of the bullet's own damage in the mod's config. 1 = no change.
	float DamageMultiplier = 1;
	// A bullet does less damage as it slows down over distance. It never drops below this share of
	// its full damage (0.85 = 85%). 0 = the game's normal drop-off.
	float RangeFloor = 0;

	void DevZ_GunDamageRule(string cls = "", float multiplier = 1, float floor = 0)
	{
		ClassName = cls;
		DamageMultiplier = multiplier;
		RangeFloor = floor;
	}
}

// Settings, saved to $profile:DevZ_GunDamage\Config.json on first start. Edit there and restart.
class DevZ_GunDamageConfig
{
	static const string FOLDER = "$profile:DevZ_GunDamage\\";
	static const string FILE = FOLDER + "Config.json";
	static const int VERSION = 1;

	// The settings version this file was written for. Don't edit.
	int ConfigVersion = 0;

	// 0 = guns only get the ammo and armour changes from the mod's config (no per-gun damage,
	// no range floors)
	int Enabled = 1;

	// Guns with their own damage or range floor (matched on the gun that fired)
	ref array<ref DevZ_GunDamageRule> Weapons;
	// Bullet types with their own damage or range floor (matched on the bullet). Combined with a
	// gun's entry: multipliers multiply, the higher floor wins.
	ref array<ref DevZ_GunDamageRule> Ammo;

	// 1 = write a line to the server's script log for every hit this mod changes
	int LogHits = 1;
	// Testing only: 1 = also re-apply hits this mod wouldn't change, at their normal damage, and
	// log them, to check a re-applied hit does exactly what the original would have
	int TestReapplyAll = 0;

	protected static ref DevZ_GunDamageConfig s_Instance;

	void DevZ_GunDamageConfig()
	{
		Weapons = DefaultWeapons();
		Ammo = DefaultAmmo();
	}

	static array<ref DevZ_GunDamageRule> DefaultWeapons()
	{
		array<ref DevZ_GunDamageRule> rules = new array<ref DevZ_GunDamageRule>();
		// .308 battle rifles: the .308 buff in the config is for snipers, these keep about vanilla damage
		rules.Insert(new DevZ_GunDamageRule("FAL_Base", 0.7));
		rules.Insert(new DevZ_GunDamageRule("M14_Base", 0.7));
		rules.Insert(new DevZ_GunDamageRule("SCARH_Base", 0.7));
		// The Longhorn pistol (.308, but slower than a rifle)
		rules.Insert(new DevZ_GunDamageRule("LongHorn_Base", 1.25));
		// SMGs: their ammo is +25% in the config (shared with pistols), this takes them to +40%
		rules.Insert(new DevZ_GunDamageRule("MP5K_Base", 1.12));
		rules.Insert(new DevZ_GunDamageRule("UMP45_Base", 1.12));
		rules.Insert(new DevZ_GunDamageRule("CZ61_Base", 1.12));
		rules.Insert(new DevZ_GunDamageRule("PP19_Base", 1.12));
		// Sniper rifles keep most of their damage at long range
		rules.Insert(new DevZ_GunDamageRule("BoltActionRifle_Base", 1, 0.85));
		rules.Insert(new DevZ_GunDamageRule("SVD_Base", 1, 0.85));
		rules.Insert(new DevZ_GunDamageRule("B95_Base", 1, 0.85));
		return rules;
	}

	static array<ref DevZ_GunDamageRule> DefaultAmmo()
	{
		array<ref DevZ_GunDamageRule> rules = new array<ref DevZ_GunDamageRule>();
		// Slugs stay useful out to about 150 m
		rules.Insert(new DevZ_GunDamageRule("Bullet_12GaugeSlug", 1, 0.7));
		return rules;
	}

	// Fills in settings added since the file was written
	protected void Upgrade()
	{
		if (!Weapons)
			Weapons = DefaultWeapons();
		if (!Ammo)
			Ammo = DefaultAmmo();
		ConfigVersion = VERSION;
	}

	static DevZ_GunDamageConfig Get()
	{
		if (!s_Instance)
		{
			s_Instance = new DevZ_GunDamageConfig();
			if (FileExist(FILE))
			{
				JsonFileLoader<DevZ_GunDamageConfig>.JsonLoadFile(FILE, s_Instance);
				s_Instance.Upgrade();
			}
			else
			{
				MakeDirectory(FOLDER);
				s_Instance.ConfigVersion = VERSION;
			}
			// Written back every start, so settings added in an update show up in the file
			JsonFileLoader<DevZ_GunDamageConfig>.JsonSaveFile(FILE, s_Instance);
		}
		return s_Instance;
	}

	DevZ_GunDamageRule WeaponRule(EntityAI weapon)
	{
		foreach (DevZ_GunDamageRule rule : Weapons)
		{
			if (weapon.IsKindOf(rule.ClassName))
				return rule;
		}
		return null;
	}

	// Exact bullet names only: rubber slugs and beanbags are built on the slug and shouldn't follow it
	DevZ_GunDamageRule AmmoRule(string ammo)
	{
		ammo.ToLower();
		foreach (DevZ_GunDamageRule rule : Ammo)
		{
			string cls = rule.ClassName;
			cls.ToLower();
			if (cls == ammo)
				return rule;
		}
		return null;
	}

	// Writes the values the game actually loaded, to check the mod's config took effect
	static void LogLoadedValues()
	{
		string vest = "CfgVehicles PlateCarrierVest DamageSystem ";
		PrintFormat("[DevZ_GunDamage] loaded: Bullet_308Win Health %1, Bullet_556x45 Health %2 Shock %3, Bullet_9x19 Health %4",
			g_Game.ConfigGetFloat("CfgAmmo Bullet_308Win DamageApplied Health damage"),
			g_Game.ConfigGetFloat("CfgAmmo Bullet_556x45 DamageApplied Health damage"),
			g_Game.ConfigGetFloat("CfgAmmo Bullet_556x45 DamageApplied Shock damage"),
			g_Game.ConfigGetFloat("CfgAmmo Bullet_9x19 DamageApplied Health damage"));
		PrintFormat("[DevZ_GunDamage] loaded: PlateCarrierVest bullet armour Health %1 Shock %2, melee Shock %3, vest hitpoints %4",
			g_Game.ConfigGetFloat(vest + "GlobalArmor Projectile Health damage"),
			g_Game.ConfigGetFloat(vest + "GlobalArmor Projectile Shock damage"),
			g_Game.ConfigGetFloat(vest + "GlobalArmor Melee Shock damage"),
			g_Game.ConfigGetFloat(vest + "GlobalHealth Health hitpoints"));
	}
}
