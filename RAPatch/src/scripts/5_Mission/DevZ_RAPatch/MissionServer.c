modded class MissionServer
{
	// RA's own hook, meant for init.c. This fills it from the settings file too.
	override void RegisterNoBuildZones(array<ref NoBuildZoneData> no_build_zones)
	{
		super.RegisterNoBuildZones(no_build_zones);
		foreach (RAPatch_ZoneRow zone : RAPatch_Settings.Get().Build.NoBuildZones)
			no_build_zones.Insert(new NoBuildZoneData(Vector(zone.X, 0, zone.Z), zone.Radius, ""));
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();
		RAPatch_DashboardNet.Start();
		RAPatch_VehicleLockNet.Get();
		RAPatch_BenchNet.Start();
		RAPatch_Bench.LogAvailable();
		RAPatch_Settings settings = RAPatch_Settings.Get();
		RAPatch_Settings.Log("loaded: decay " + settings.Decay.Enabled + ", desk radius " + settings.Desk.Radius + ", C4 damage " + settings.Raiding.C4Damage + ", fuse " + settings.Raiding.C4FuseSeconds);
		RAPatch_Modules.WarnAboutMissingClasses();
		if (settings.SelfTest)
			RAPatch_SelfTest.Start();
	}

	// A new character: at their sleeping bag if bags are on, at their desk if bags are off, and otherwise where vanilla chose
	override PlayerBase OnClientNewEvent(PlayerIdentity identity, vector pos, ParamsReadContext ctx)
	{
		vector spot;
		string kind;
		string note;
		bool moved = false;
		if (identity)
			moved = RAPatch_Respawn.Get().Decide(identity.GetId(), spot, kind, note);
		if (moved)
			pos = spot;
		PlayerBase player = super.OnClientNewEvent(identity, pos, ctx);
		if (!player || !identity)
			return player;
		ScriptCallQueue queue = GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY);
		if (moved)
		{
			RAPatch_Respawn.Get().RecordUse(identity.GetId(), kind);
			if (kind == RAPatch_Respawn.BAG)
			{
				EntityAI usedBag = EntityAI.Cast(RAPatch_Respawn.Get().FindBag(identity.GetId()));
				if (usedBag)
				{
					int bagPercent = Math.Round(usedBag.GetHealth01("", "") * 100);
					note = "Your sleeping bag is at " + bagPercent.ToString() + "%.";
				}
				else
				{
					note = "Your sleeping bag has worn out.";
				}
			}
			// After the game has finished setting the new character up
			queue.CallLater(RAPatch_Respawn.Get().ApplyRespawnState, 3000, false, player);
		}
		if (note != "")
			queue.CallLater(RAPatch_Respawn.Get().Tell, 8000, false, identity, note);
		return player;
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);
		RAPatch_Net.SendTo(identity);
	}
}
