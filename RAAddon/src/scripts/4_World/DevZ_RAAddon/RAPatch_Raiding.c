// Charges. The fuse and all the blast's numbers are virtual functions on RA's classes.
modded class RA_C4
{
	override float GetArmDelay()
	{
		return RAPatch_Settings.Get().Raiding.C4FuseSeconds;
	}
}

modded class RA_Improvised_C4
{
	override float GetArmDelay()
	{
		return RAPatch_Settings.Get().Raiding.ImprovisedFuseSeconds;
	}
}

modded class C4Explosion
{
	override float GetExplosiveDamage()
	{
		return RAPatch_Settings.Get().Raiding.C4Damage;
	}

	override string GetExplosiveType()
	{
		return RAPatch_Settings.Get().Raiding.C4PlayerAmmo;
	}

	override float GetExplosionRadius()
	{
		return RAPatch_Settings.Get().Raiding.BlastRadiusMeters;
	}

	override int GetPenetrationDepth()
	{
		return RAPatch_Settings.Get().Raiding.PenetrationDepth;
	}

	override float GetPenetrationFalloff()
	{
		return RAPatch_Settings.Get().Raiding.PenetrationFalloff;
	}
}

modded class ImprovisedC4Explosion
{
	override float GetExplosiveDamage()
	{
		return RAPatch_Settings.Get().Raiding.ImprovisedDamage;
	}

	override string GetExplosiveType()
	{
		return RAPatch_Settings.Get().Raiding.ImprovisedPlayerAmmo;
	}

	override float GetExplosionRadius()
	{
		return RAPatch_Settings.Get().Raiding.BlastRadiusMeters;
	}

	override int GetPenetrationDepth()
	{
		return RAPatch_Settings.Get().Raiding.PenetrationDepth;
	}

	override float GetPenetrationFalloff()
	{
		return RAPatch_Settings.Get().Raiding.PenetrationFalloff;
	}
}

modded class ScriptedExplosion
{
	// RA's blast only damages building parts, and the desk isn't one. The same search as RA's, before RA's own runs,
	// so a charge touching the desk takes health off it like a wall.
	override void DoExplosion(EntityAI source)
	{
		if (g_Game.IsServer())
			RAPatch_BlastDesks(source);
		super.DoExplosion(source);
	}

	protected void RAPatch_BlastDesks(EntityAI source)
	{
		RAPatch_RaidSettings raid = RAPatch_Settings.Get().Raiding;
		if (!raid.DeskBlastDamage || raid.DoorsOnly || raid.C4Damage <= 0 || raid.DeskC4ToDestroy <= 0 || GetExplosiveDamage() <= 0)
			return;
		array<ref SphereCastResult> results = ShapeCast.Sphere(GetPosition(), GetExplosionRadius(), source, GetExplosionCheckIterations(), GetPenetrationDepth(), ObjIntersectFire);
		map<TerritoryHQ, int> desks = new map<TerritoryHQ, int>;
		foreach (SphereCastResult result : results)
		{
			TerritoryHQ desk = TerritoryHQ.Cast(result.Target);
			if (!desk || desk.IsRuined())
				continue;
			if (!desks.Contains(desk) || result.PenetrationLevel < desks.Get(desk))
				desks.Set(desk, result.PenetrationLevel);
		}
		foreach (TerritoryHQ hit, int level : desks)
		{
			if (RAPatch_IsBlastProof(hit))
				continue;
			float change = RAPatch_DeskBlastDamage(hit, GetExplosiveDamage()) * Math.Pow(GetPenetrationFalloff(), level);
			if (!OnDamageCalculated(hit, change, level))
				continue;
			hit.RAPatch_RaidHit("Explosive charge", hit.GetDisplayName(), null);
			hit.DecreaseHealth("GlobalHealth", "Health", change);
			if (hit)
				hit.SetHealthSynchDirty();
		}
	}

	// The desk's health is in RA's config (500), so the hit is sized to it: one C4 takes 1 / DeskC4ToDestroy of it, a smaller charge less.
	// A hair over, so the last charge always finishes it.
	static float RAPatch_DeskBlastDamage(TerritoryHQ desk, float blastDamage)
	{
		RAPatch_RaidSettings raid = RAPatch_Settings.Get().Raiding;
		float perC4 = desk.GetMaxHealth("GlobalHealth", "Health") / raid.DeskC4ToDestroy;
		return perC4 * blastDamage / raid.C4Damage + 0.01;
	}

	static bool RAPatch_IsBlastProof(Object target)
	{
		foreach (string proof : RAPatch_Settings.Get().Raiding.BlastProofClasses)
		{
			if (target.IsKindOf(proof))
				return true;
		}
		return false;
	}

	// RA asks this just before taking health off, and takes it off itself if the answer's yes. The amount it passes
	// can't be changed, so when a part has a multiplier the addon takes the health off and answers no.
	override bool OnDamageCalculated(Object target, float damage, int penetration_level)
	{
		if (!super.OnDamageCalculated(target, damage, penetration_level))
			return false;
		BaseBuilding part = BaseBuilding.Cast(target);
		if (!part)
			return true;
		if (!part.RAPatch_CanTakeBlast())
			return false;
		float multiplier = part.RAPatch_BlastMultiplier();
		if (multiplier == 1)
			return true;
		part.DecreaseHealth("GlobalHealth", "Health", damage * multiplier);
		part.SetHealthSynchDirty();
		return false;
	}
}

// For admins: profiles\DevZ_RAAddon\ChargeLog.txt. RA doesn't keep who armed a charge anywhere it can be read later.
class RAPatch_ChargeLog
{
	static const string PATH = "$profile:DevZ_RAAddon/ChargeLog.txt";

	static string Who(Man player)
	{
		if (!player || !player.GetIdentity())
			return "unknown";
		PlayerIdentity identity = player.GetIdentity();
		return identity.GetName() + " (" + identity.GetPlainId() + ")";
	}

	static void Write(string what, string who, Object charge)
	{
		if (!g_Game.IsServer() || !RAPatch_Settings.Get().Raiding.LogCharges)
			return;
		int year, month, day, hour, minute, second;
		GetYearMonthDayUTC(year, month, day);
		GetHourMinuteSecondUTC(hour, minute, second);
		string when = string.Format("%1-%2-%3 %4:%5:%6 UTC", year, month.ToStringLen(2), day.ToStringLen(2), hour.ToStringLen(2), minute.ToStringLen(2), second.ToStringLen(2));
		string where = "no charge";
		string territory = "outside any territory";
		if (charge)
		{
			where = charge.GetType() + " at " + charge.GetPosition().ToString();
			TerritoryHQ desk = RAPatch_Limits.DeskAt(charge.GetPosition());
			if (desk)
				territory = "in the territory of the desk at " + desk.GetPosition().ToString();
		}
		string line = when + " | " + what + " | " + who + " | " + where + " | " + territory;
		FileHandle file = OpenFile(PATH, FileMode.APPEND);
		if (file != 0)
		{
			FPrintln(file, line);
			CloseFile(file);
		}
		RAPatch_Settings.Log("charge: " + line);
	}
}

modded class PlaceableExplosivesBase
{
	protected string m_RAPatch_ArmedBy;

	void RAPatch_NoteArming(Man player, bool wasArmed)
	{
		if (IsArmed() == wasArmed)
			return;
		string who = RAPatch_ChargeLog.Who(player);
		if (IsArmed())
		{
			m_RAPatch_ArmedBy = who;
			RAPatch_ChargeLog.Write("ARMED", who, this);
		}
		else
		{
			RAPatch_ChargeLog.Write("DISARMED", who, this);
		}
	}

	override protected void DoExplosion()
	{
		string who = m_RAPatch_ArmedBy;
		if (who == "")
			who = "unknown";
		RAPatch_ChargeLog.Write("WENT OFF, armed by", who, this);
		super.DoExplosion();
	}

	override void SetArmState(bool state)
	{
		if (state && !RAPatch_RaidClock.IsOpenNow())
		{
			RAPatch_Settings.Log("refused to arm " + GetType() + " at " + GetPosition() + ": outside raid hours");
			return;
		}
		super.SetArmState(state);
	}
}

class RAPatch_RaidMessage
{
	static void Tell(Man player)
	{
		if (!RAPatch_RaidClock.IsOpenNow())
			NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Outside raid hours", "This charge can't be armed yet.");
	}
}

modded class ActionToggleArmExplosive
{
	override void OnExecuteServer(ActionData action_data)
	{
		RAPatch_RaidMessage.Tell(action_data.m_Player);
		PlaceableExplosivesBase charge = PlaceableExplosivesBase.Cast(action_data.m_Target.GetObject());
		bool wasArmed = charge && charge.IsArmed();
		super.OnExecuteServer(action_data);
		if (charge)
			charge.RAPatch_NoteArming(action_data.m_Player, wasArmed);
	}
}

modded class ActionArmAndPlaceC4
{
	override void OnEndServer(ActionData action_data)
	{
		RAPatch_RaidMessage.Tell(action_data.m_Player);
		PlaceableExplosivesBase charge = PlaceableExplosivesBase.Cast(action_data.m_MainItem);
		bool wasArmed = charge && charge.IsArmed();
		super.OnEndServer(action_data);
		if (charge)
			charge.RAPatch_NoteArming(action_data.m_Player, wasArmed);
	}
}
