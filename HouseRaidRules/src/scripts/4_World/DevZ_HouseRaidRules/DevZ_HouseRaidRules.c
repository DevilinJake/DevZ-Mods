// Decides whether a bullet, explosive or melee hit may damage a rebuilt house part. Everything about
// zones, admins and the attacker comes from Ninjins PvP/PvE Upgraded's public classes at runtime.

class DevZ_HouseRaidRules
{
	static const string LOG = "[DevZ_HouseRaidRules] ";
	protected static ref map<string, float> s_LastNoticeByPlayer;

	// The rebuilt part a damage zone belongs to, or "" when the zone isn't a built construction part
	// (then there's nothing to protect and vanilla carries on)
	static string RebuiltPartName(BuildingBase house, string dmgZone)
	{
		ConstructionBase construction = ConstructionBase.Cast(house.GetConstructionBasic());
		if (!construction)
			return "";
		string partName = dmgZone;
		partName.ToLower();
		if (!construction.GetConstructionPart(partName))
			return "";
		if (!construction.IsPartConstructed(partName))
			return "";
		return partName;
	}

	// True when at least one rebuildable part of the house is built
	static bool HasBuiltPart(BuildingBase house)
	{
		ConstructionBase construction = ConstructionBase.Cast(house.GetConstructionBasic());
		if (!construction)
			return false;
		array<ConstructionPart> parts = construction.GetConstructionPartArray();
		if (!parts)
			return false;
		foreach (ConstructionPart part : parts)
		{
			if (part && part.IsBuilt())
				return true;
		}
		return false;
	}

	// True = let the hit through. partName is the built part hit, or "" for a hit elsewhere on a house
	// that has built parts (blocked too: the engine can spread damage between a house's zones).
	// hitPos is the world position used for the zone lookup (the house's position).
	static bool MayDamagePart(BuildingBase house, string partName, EntityAI source, int damageType, string ammo, vector hitPos)
	{
		DevZ_HouseRaidRulesConfig cfg = DevZ_HouseRaidRulesConfig.Get();
		string hitKind = HitKind(source, damageType, ammo);
		if (cfg.LogAllHits)
			Print(LOG + "HIT house=" + house.GetType() + " part=" + PartLabel(partName) + " damageType=" + damageType + " kind=" + hitKind + " ammo=" + ammo + " source=" + SourceLabel(source));
		if (!cfg.RulesOn())
			return true;
		if (!KindIsBlocked(cfg, hitKind))
			return true;

		PlayerBase attacker = Attacker(source);
		if (attacker && cfg.AdminsBypass() && attacker.NinjinsUpgraded_IsAdmin())
			return true;

		string zoneLabel;
		string notice;
		string reason = BlockReason(cfg, hitPos, zoneLabel, notice);
		if (reason == "")
			return true;

		if (cfg.LogBlocked)
			Print(LOG + "DENIED house=" + house.GetType() + " part=" + PartLabel(partName) + " pos=" + hitPos.ToString() + " zone=" + zoneLabel + " kind=" + hitKind + " ammo=" + ammo + " attacker=" + AttackerLabel(attacker) + " reason=" + reason);
		TellAttacker(cfg, attacker, notice);
		return false;
	}

	// Backstop for damage that reached a part without passing MayDamagePart (another mod applying
	// damage directly, or a path the hook doesn't see). Called when the game is about to destroy a
	// part because its health reached zero without a player action. True = keep the part instead.
	// The attacker is unknown here, so there is no admin bypass and no notice.
	static bool ShouldKeepPart(BuildingBase house, string partName)
	{
		DevZ_HouseRaidRulesConfig cfg = DevZ_HouseRaidRulesConfig.Get();
		if (!cfg.RulesOn())
			return false;
		if (!cfg.BlocksBullets() && !cfg.BlocksExplosives() && !cfg.BlocksMelee())
			return false;
		string zoneLabel;
		string notice;
		string reason = BlockReason(cfg, house.GetPosition(), zoneLabel, notice);
		if (reason == "")
			return false;
		if (cfg.LogBlocked)
			Print(LOG + "RESTORED house=" + house.GetType() + " part=" + partName + " zone=" + zoneLabel + " reason=" + reason + " (health reached zero through damage this mod did not see; part kept and repaired)");
		return true;
	}

	// "" when raiding is allowed at worldPos right now, else "zone" or "hours". zoneLabel is for the
	// log, notice the text for the attacker.
	protected static string BlockReason(DevZ_HouseRaidRulesConfig cfg, vector worldPos, out string zoneLabel, out string notice)
	{
		string zoneName;
		string zoneType = ZoneTypeAt(worldPos, zoneName);
		zoneLabel = zoneType;
		if (zoneName != "")
			zoneLabel = zoneLabel + "(" + zoneName + ")";
		notice = "";
		if (!cfg.DamageAllowedIn(zoneType))
		{
			notice = cfg.BlockedZoneText;
			return "zone";
		}
		if (cfg.RaidHoursOnly && !cfg.IsRaidTimeNow())
		{
			notice = cfg.BlockedHoursText;
			return "hours";
		}
		return "";
	}

	protected static string PartLabel(string partName)
	{
		if (partName == "")
			return "(house)";
		return partName;
	}

	protected static string SourceLabel(EntityAI source)
	{
		if (!source)
			return "none";
		return source.GetType();
	}

	// "explosive", "bullet", "melee" or "" for anything else (fire, falling, decay...)
	static string HitKind(EntityAI source, int damageType, string ammo)
	{
		if (IsExplosive(source, damageType))
			return "explosive";
		if (damageType == DamageType.FIRE_ARM)
			return "bullet";
		if (ammo != "" && ammo.IndexOf("Bullet") != -1)
			return "bullet";
		if (damageType == DamageType.CLOSE_COMBAT)
			return "melee";
		return "";
	}

	protected static bool IsExplosive(EntityAI source, int damageType)
	{
		if (damageType == DamageType.EXPLOSION)
			return true;
		if (!source)
			return false;
		if (source.IsInherited(ExplosivesBase))
			return true;
		if (source.IsInherited(Grenade_Base))
			return true;
		if (source.IsInherited(TrapBase))
			return true;
		if (source.IsInherited(Ammo_40mm_Base))
			return true;
#ifdef EXPANSIONMODBASEBUILDING
		if (source.IsInherited(Expansion_C4_Explosion))
			return true;
#endif
#ifdef EXPANSIONMODWEAPONS
		if (source.IsInherited(Expansion_RPG_Explosion))
			return true;
		if (source.IsInherited(Expansion_LAW_Explosion))
			return true;
#endif
		return false;
	}

	protected static bool KindIsBlocked(DevZ_HouseRaidRulesConfig cfg, string hitKind)
	{
		if (hitKind == "explosive")
			return cfg.BlocksExplosives();
		if (hitKind == "bullet")
			return cfg.BlocksBullets();
		if (hitKind == "melee")
			return cfg.BlocksMelee();
		return false;
	}

	// The player behind the hit (shooter, thrower, planter), found the way Ninjins finds it
	protected static PlayerBase Attacker(EntityAI source)
	{
		NinjinsUpgraded_ZoneHit hit = NinjinsUpgraded_HitSource.NinjinsUpgraded_HitInfoForDamage(source);
		if (!hit)
			return null;
		return hit.Player;
	}

	// The Ninjins zone type at a world position: "Safe", "Raid", "PvP", "PvE", "Visual" or "None".
	// A safe zone wins over everything, then a raid zone, then PvP against PvE by zone priority, then
	// visual zones. Outside every zone the map-wide PvE/PvP schedule from GlobalSettings decides.
	// Only zones whose own schedule is active right now count.
	static string ZoneTypeAt(vector worldPos, out string zoneName)
	{
		zoneName = "";
		NinjinsUpgraded_ZoneRuntime bestSafe;
		NinjinsUpgraded_ZoneRuntime bestRaid;
		NinjinsUpgraded_ZoneRuntime bestPvp;
		NinjinsUpgraded_ZoneRuntime bestPve;
		NinjinsUpgraded_ZoneRuntime bestVisual;

		NinjinsUpgraded_ZoneEngine engine = NinjinsUpgraded_ZoneEngineModule.GetEngine();
		array<ref NinjinsUpgraded_ZoneRuntime> zones;
		if (engine)
			zones = engine.NinjinsUpgraded_GetRuntimeZones();
		if (zones)
		{
			foreach (NinjinsUpgraded_ZoneRuntime zone : zones)
			{
				if (!zone)
					continue;
				if (!zone.IsScheduleActive())
					continue;
				if (!zone.IsInside(worldPos))
					continue;
				if (zone.m_Type == NinjinsUpgraded_ZoneType.SafeZone)
					bestSafe = HigherPriority(bestSafe, zone);
				else if (zone.m_Type == NinjinsUpgraded_ZoneType.RaidZone)
					bestRaid = HigherPriority(bestRaid, zone);
				else if (zone.m_Type == NinjinsUpgraded_ZoneType.PvPZone)
					bestPvp = HigherPriority(bestPvp, zone);
				else if (zone.m_Type == NinjinsUpgraded_ZoneType.PvEZone)
					bestPve = HigherPriority(bestPve, zone);
				else if (zone.m_Type == NinjinsUpgraded_ZoneType.Visual)
					bestVisual = HigherPriority(bestVisual, zone);
			}
		}

		NinjinsUpgraded_ZoneRuntime winner;
		if (bestSafe)
			winner = bestSafe;
		else if (bestRaid)
			winner = bestRaid;
		else if (bestPvp && bestPve)
			winner = HigherPriority(bestPvp, bestPve);
		else if (bestPvp)
			winner = bestPvp;
		else if (bestPve)
			winner = bestPve;
		else if (bestVisual)
			winner = bestVisual;
		if (winner)
		{
			zoneName = winner.m_Name;
			return TypeLabel(winner.m_Type);
		}

		if (g_NinjinsUpgraded_GlobalSettings)
		{
			if (g_NinjinsUpgraded_GlobalSettings.IsPvEEverywhereEffectiveNow())
			{
				zoneName = "map-wide";
				return "PvE";
			}
			if (g_NinjinsUpgraded_GlobalSettings.IsPvPEverywhereEffectiveNow())
			{
				zoneName = "map-wide";
				return "PvP";
			}
		}
		return "None";
	}

	protected static NinjinsUpgraded_ZoneRuntime HigherPriority(NinjinsUpgraded_ZoneRuntime current, NinjinsUpgraded_ZoneRuntime candidate)
	{
		if (!current)
			return candidate;
		if (candidate.m_Priority >= current.m_Priority)
			return candidate;
		return current;
	}

	static string TypeLabel(NinjinsUpgraded_ZoneType type)
	{
		if (type == NinjinsUpgraded_ZoneType.SafeZone)
			return "Safe";
		if (type == NinjinsUpgraded_ZoneType.RaidZone)
			return "Raid";
		if (type == NinjinsUpgraded_ZoneType.PvPZone)
			return "PvP";
		if (type == NinjinsUpgraded_ZoneType.PvEZone)
			return "PvE";
		if (type == NinjinsUpgraded_ZoneType.Visual)
			return "Visual";
		return "None";
	}

	protected static string AttackerLabel(PlayerBase attacker)
	{
		if (!attacker)
			return "unknown";
		PlayerIdentity identity = attacker.GetIdentity();
		if (!identity)
			return attacker.ToString();
		return identity.GetName() + " (" + identity.GetPlainId() + ")";
	}

	protected static void TellAttacker(DevZ_HouseRaidRulesConfig cfg, PlayerBase attacker, string message)
	{
		if (!attacker || message == "" || !cfg.ShowBlockedNotify)
			return;
		PlayerIdentity identity = attacker.GetIdentity();
		if (!identity)
			return;
		if (!s_LastNoticeByPlayer)
			s_LastNoticeByPlayer = new map<string, float>();
		string key = identity.GetId();
		float now = g_Game.GetTickTime();
		if (s_LastNoticeByPlayer.Contains(key) && now - s_LastNoticeByPlayer.Get(key) < cfg.NoticeWaitSeconds)
			return;
		s_LastNoticeByPlayer.Set(key, now);
		NotificationSystem.Create(new StringLocaliser(message), new StringLocaliser(""), "set:dayz_gui image:cross", ARGB(255, 220, 70, 70), 5.0, identity);
	}
}
