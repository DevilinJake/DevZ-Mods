// Sleeping bags, and where a dead player comes back. The model and textures are Zenarchist's: see data\sleepingbag\CREDIT.txt.
//
// Order: a bag the player has set, then a desk they're authorised on, then vanilla's spawn points. Each can be switched
// off, and one that's switched off, or a bag that's gone or no longer counts, passes to the next. A bag that's cooling down is waited for.
//
// A bag is any object whose class is in Respawn.BagClasses, from this mod or another. So what a player has set is saved
// with them, as the object's class and position, not on the object: nothing about another mod's bag has to change.
// If that object's moved or gone, they simply don't have a bag any more.
class RAPatch_BagRules
{
	static const int ANYONE = 0;
	static const int DEPLOYER = 1;	// only who laid it out
	static const int TEAM = 2;		// any team member: who laid it out, or anyone authorised on the desk it's in

	static bool MayPickUp(int mode, bool isDeployer, bool isAuthorized)
	{
		if (mode == ANYONE)
			return true;
		if (mode == DEPLOYER)
			return isDeployer;
		return isDeployer || isAuthorized;
	}

	// Respawn.BagsNeedOwnTerritory. Empty text means a bag can go there
	static string SpotRefusal(bool inTerritory, bool isOwn)
	{
		if (!inTerritory)
			return "A sleeping bag can only be laid out inside your own territory, and this spot isn't in a territory.";
		if (!isOwn)
			return "A sleeping bag can only be laid out inside your own territory, and this is someone else's.";
		return "";
	}
}

// Packed: carried, and laid out with a hologram
class RAPatch_SleepingBag_Base : ItemBase
{
	override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
	{
		if (g_Game.IsClient())
			return;
		// The last check, at the spot the player's game sent. A refused bag stays packed, on the ground where it was put.
		string refusal = RAPatch_Respawn.BagSpotRefusal(player, position);
		if (refusal != "")
		{
			RAPatch_Settings.Log("refused laying out " + GetType() + " at " + position + ": " + refusal);
			RAPatch_Respawn.TellBagRefused(player, refusal);
			return;
		}
		RAPatch_SleepingBagDeployed_Base bag = RAPatch_SleepingBagDeployed_Base.Cast(g_Game.CreateObject(GetType() + "_Deployed", position, false));
		if (!bag)
		{
			RAPatch_Settings.Log("WARNING could not lay out " + GetType());
			return;
		}
		bag.SetPosition(position);
		bag.SetOrientation(orientation);
		bag.SetHealth("", "", GetHealth("", ""));
		bag.SetWet(GetWet());
		bag.RAPatch_SetDeployer(player);
		float seconds = RAPatch_Settings.Get().Respawn.BagLifetimeHours * 3600;
		bag.SetLifetimeMax(seconds);
		bag.SetLifetime(seconds);
		g_Game.ObjectDelete(this);
	}

	override bool IsDeployable()
	{
		return true;
	}

	override bool CanBeRepairedByCrafting()
	{
		return true;
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionTogglePlaceObject);
		AddAction(ActionDeployObject);
		AddAction(ActionWringClothes);
	}
}

class RAPatch_SleepingBag_Black : RAPatch_SleepingBag_Base {};
class RAPatch_SleepingBag_Blue : RAPatch_SleepingBag_Base {};
class RAPatch_SleepingBag_Green : RAPatch_SleepingBag_Base {};
class RAPatch_SleepingBag_Grey : RAPatch_SleepingBag_Base {};
class RAPatch_SleepingBag_Red : RAPatch_SleepingBag_Base {};

// Laid out: where players can set their respawn
class RAPatch_SleepingBagDeployed_Base : ItemBase
{
	protected string m_RAPatch_Deployer;
	protected int m_RAPatch_DeployerHash;	// clients only need to ask "was it me?"

	void RAPatch_SleepingBagDeployed_Base()
	{
		RegisterNetSyncVariableInt("m_RAPatch_DeployerHash");
	}

	void RAPatch_SetDeployer(Man player)
	{
		if (!player || !player.GetIdentity())
			return;
		m_RAPatch_Deployer = player.GetIdentity().GetId();
		m_RAPatch_DeployerHash = m_RAPatch_Deployer.Hash();
		SetSynchDirty();
	}

	bool RAPatch_IsDeployer(Man player)
	{
		return player && player.GetIdentity() && player.GetIdentity().GetId().Hash() == m_RAPatch_DeployerHash;
	}

	bool RAPatch_InTerritory()
	{
		return TerritoryHQ.IsWithinTerritoryRange(null, GetPosition());
	}

	bool RAPatch_MayPickUp(Man player)
	{
		bool authorized = RAPatch_InTerritory() && TerritoryHQ.HasPermissionsAtPosition(player, GetPosition());
		return RAPatch_BagRules.MayPickUp(RAPatch_Settings.Get().Respawn.BagPickup, RAPatch_IsDeployer(player), authorized);
	}

	override bool CanBeRepairedByCrafting()
	{
		return true;
	}

	override bool IsTakeable()
	{
		return false;
	}

	override bool CanPutIntoHands(EntityAI parent)
	{
		return false;
	}

	override bool CanPutInCargo(EntityAI parent)
	{
		return false;
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(RAPatch_ActionPackBag);
	}

	override void CF_OnStoreSave(CF_ModStorageMap storage)
	{
		super.CF_OnStoreSave(storage);
		CF_ModStorage ctx = storage.Get(RAPatch_Net.MOD);
		if (!ctx)
			return;
		ctx.Write(m_RAPatch_Deployer);
	}

	override bool CF_OnStoreLoad(CF_ModStorageMap storage)
	{
		if (!super.CF_OnStoreLoad(storage))
			return false;
		CF_ModStorage ctx = storage.Get(RAPatch_Net.MOD);
		if (!ctx)
			return true;
		if (!ctx.Read(m_RAPatch_Deployer))
			return false;
		if (m_RAPatch_Deployer != "")
			m_RAPatch_DeployerHash = m_RAPatch_Deployer.Hash();
		return true;
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		SetSynchDirty();
	}
}

class RAPatch_SleepingBag_Black_Deployed : RAPatch_SleepingBagDeployed_Base {};
class RAPatch_SleepingBag_Blue_Deployed : RAPatch_SleepingBagDeployed_Base {};
class RAPatch_SleepingBag_Green_Deployed : RAPatch_SleepingBagDeployed_Base {};
class RAPatch_SleepingBag_Grey_Deployed : RAPatch_SleepingBagDeployed_Base {};
class RAPatch_SleepingBag_Red_Deployed : RAPatch_SleepingBagDeployed_Base {};

class RAPatch_ActionBagRespawn : ActionInteractBase
{
	void RAPatch_ActionBagRespawn()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_Text = "Set or clear respawn here";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		Object bag = target.GetObject();
		if (!bag || !RAPatch_Settings.Get().Respawn.BagsEnabled)
			return false;
		return RAPatch_Respawn.IsBag(bag) && !bag.IsRuined() && RAPatch_Respawn.MaySet(player, bag);
	}

	override void OnExecuteServer(ActionData action_data)
	{
		super.OnExecuteServer(action_data);
		Object bag = action_data.m_Target.GetObject();
		PlayerIdentity identity = action_data.m_Player.GetIdentity();
		if (!bag || !identity || !RAPatch_Respawn.IsBag(bag) || !RAPatch_Respawn.MaySet(action_data.m_Player, bag))
			return;
		string message = "You'll respawn at this sleeping bag.";
		if (RAPatch_Respawn.Get().FindBag(identity.GetId()) == bag)
		{
			RAPatch_Respawn.Get().ClearBag(identity.GetId());
			message = "You won't respawn at this sleeping bag any more.";
		}
		else
		{
			RAPatch_Respawn.Get().SetBag(identity.GetId(), bag);
		}
		NotificationSystem.SendNotificationToPlayerExtended(action_data.m_Player, 5, "Sleeping bag", message);
	}
}

// Offered on every item, because the list of bag classes is only known when the game runs. It stops at once on anything else.
modded class ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(RAPatch_ActionBagRespawn);
	}
}

class RAPatch_ActionPackBagCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(5);
	}
}

class RAPatch_ActionPackBag : ActionContinuousBase
{
	void RAPatch_ActionPackBag()
	{
		m_CallbackClass = RAPatch_ActionPackBagCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DEPLOY_2HD;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_HIGH;
		m_Text = "#pack_tent";
	}

	override void CreateConditionComponents()
	{
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
		m_ConditionItem = new CCINotPresent;
	}

	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}

	override bool HasTarget()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		RAPatch_SleepingBagDeployed_Base bag = RAPatch_SleepingBagDeployed_Base.Cast(target.GetObject());
		return bag && bag.RAPatch_MayPickUp(player);
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		RAPatch_SleepingBagDeployed_Base bag = RAPatch_SleepingBagDeployed_Base.Cast(action_data.m_Target.GetObject());
		if (!bag || !bag.RAPatch_MayPickUp(action_data.m_Player))
			return;
		string type = bag.GetType();
		type.Replace("_Deployed", "");
		EntityAI packed = EntityAI.Cast(g_Game.CreateObject(type, action_data.m_Player.GetPosition(), false));
		if (!packed)
			return;
		packed.SetHealth("", "", bag.GetHealth("", ""));
		g_Game.ObjectDelete(bag);
	}
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(RAPatch_ActionBagRespawn);
		actions.Insert(RAPatch_ActionPackBag);
	}
}

modded class Hologram
{
	override string ProjectionBasedOnParent()
	{
		if (m_Parent && m_Parent.IsKindOf("RAPatch_SleepingBag_Base"))
			return m_Parent.GetType() + "_Deployed";
		return super.ProjectionBasedOnParent();
	}

	// Red outline on the player's game. The server runs this as well when it checks the hologram's spot
	override bool IsPlacementPermitted()
	{
		if (m_Player && m_Projection && RAPatch_Respawn.IsBag(m_Projection))
		{
			if (RAPatch_Respawn.BagSpotRefusal(m_Player, GetProjectionPosition()) != "")
				return false;
		}
		return super.IsPlacementPermitted();
	}
}

// The text while holding the outline: this is what tells a player why the bag won't go down
modded class ActionTogglePlaceObject
{
	static const string BAG_DENIED_TEXT = "Can't lay out (your own territory only)";

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!super.ActionCondition(player, target, item))
			return false;
		if (g_Game.IsDedicatedServer())
			return true;
		Hologram hologram = player.GetHologramLocal();
		if (hologram && hologram.GetProjectionEntity() && RAPatch_Respawn.IsBag(hologram.GetProjectionEntity()))
		{
			if (RAPatch_Respawn.BagSpotRefusal(player, hologram.GetProjectionEntity().GetPosition()) != "")
				m_Text = BAG_DENIED_TEXT;
		}
		if (hologram && TerritoryHQ.Cast(hologram.GetProjectionEntity()))
		{
			if (RAPatch_DeskSpacing.Refusal(hologram.GetProjectionEntity().GetPosition()) != "")
				m_Text = RAPatch_DeskSpacing.DENIED_TEXT;
		}
		return true;
	}

	override bool IsIconDisabled()
	{
		return super.IsIconDisabled() || GetText().Contains(BAG_DENIED_TEXT) || GetText().Contains(RAPatch_DeskSpacing.DENIED_TEXT);
	}
}

modded class ModItemRegisterCallbacks
{
	override void RegisterOneHanded(DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior)
	{
		super.RegisterOneHanded(pType, pBehavior);
		pType.AddItemInHandsProfileIK("RAPatch_SleepingBag_Base", "dz/anims/workspaces/player/player_main/player_main_2h.asi", pBehavior, "dz/anims/anm/player/ik/two_handed/BatteryCar.anm");
	}
}

// One player's cooldown clocks. In profiles\DevZ_RAPatch\RespawnData.json, so a restart doesn't clear them.
class RAPatch_RespawnRecord
{
	string Id;
	int BagReadyAt;		// when each cooldown ends, in UTC seconds
	int HQReadyAt;
	int BagPenalties;	// respawns in a row within the penalty window
	int HQPenalties;
	string BagType;		// the object they've set as their bag, and where it is
	vector BagPosition;

	void RAPatch_RespawnRecord(string id = "")
	{
		Id = id;
	}
}

class RAPatch_RespawnData
{
	ref array<ref RAPatch_RespawnRecord> Players = new array<ref RAPatch_RespawnRecord>;
}

class RAPatch_Respawn
{
	static const string BAG = "bag";
	static const string HQ = "hq";
	protected static const string PATH = "$profile:DevZ_RAPatch/RespawnData.json";

	protected static ref RAPatch_Respawn s_Instance;
	protected ref RAPatch_RespawnData m_Data;

	static RAPatch_Respawn Get()
	{
		if (!s_Instance)
			s_Instance = new RAPatch_Respawn;
		return s_Instance;
	}

	void RAPatch_Respawn()
	{
		m_Data = new RAPatch_RespawnData;
		string error;
		if (FileExist(PATH))
			JsonFileLoader<RAPatch_RespawnData>.LoadFile(PATH, m_Data, error);
	}

	protected void Save()
	{
		string error;
		JsonFileLoader<RAPatch_RespawnData>.SaveFile(PATH, m_Data, error);
	}

	protected int Now()
	{
		return CF_Date.Now(true).DateToEpoch();
	}

	protected RAPatch_RespawnRecord Record(string id)
	{
		for (int i = 0; i < m_Data.Players.Count(); i++)
		{
			if (m_Data.Players[i].Id == id)
				return m_Data.Players[i];
		}
		RAPatch_RespawnRecord record = new RAPatch_RespawnRecord(id);
		m_Data.Players.Insert(record);
		return record;
	}

	void Forget(string id)
	{
		for (int i = m_Data.Players.Count() - 1; i >= 0; i--)
		{
			if (m_Data.Players[i].Id == id)
				m_Data.Players.Remove(i);
		}
		Save();
	}

	// Seconds until this kind of respawn is ready: 0 means now
	int SecondsLeft(string id, string kind)
	{
		RAPatch_RespawnRecord record = Record(id);
		int readyAt = record.HQReadyAt;
		if (kind == BAG)
			readyAt = record.BagReadyAt;
		int left = readyAt - Now();
		if (left < 0)
			return 0;
		return left;
	}

	protected string ReadableTime(int seconds)
	{
		if (seconds < 60)
			return seconds.ToString() + " s";
		return Math.Ceil(seconds / 60.0).ToString() + " min";
	}

	static bool IsBag(Object thing)
	{
		array<string> classes = RAPatch_Settings.Get().Respawn.BagClasses;
		for (int i = 0; i < classes.Count(); i++)
		{
			if (thing.IsKindOf(classes[i]))
				return true;
		}
		return false;
	}

	// Inside a territory, only its team. Out in the open, anyone at a bag. The deployer of our own bag always can.
	static bool MaySet(Man player, Object bag)
	{
		RAPatch_SleepingBagDeployed_Base ours = RAPatch_SleepingBagDeployed_Base.Cast(bag);
		if (ours && ours.RAPatch_IsDeployer(player))
			return true;
		return TerritoryHQ.HasPermissionsAtPosition(player, bag.GetPosition());
	}

	// Respawn.BagsNeedOwnTerritory. Empty text means a bag can go here. Works on the player's game and on the server.
	static string BagSpotRefusal(Man player, vector position)
	{
		if (!RAPatch_Settings.Get().Respawn.BagsNeedOwnTerritory)
			return "";
		string id = "";
		if (player && player.GetIdentity())
			id = player.GetIdentity().GetId();
		return BagSpotRefusalFor(id, position);
	}

	// By ID, so the self test can ask without a real player. Overlapping desks: the oldest owns the spot, as everywhere in RA.
	static string BagSpotRefusalFor(string id, vector position)
	{
		array<TerritoryHQ> here = new array<TerritoryHQ>;
		array<TerritoryHQ> desks = TerritoryHQ.All_HQs;
		for (int i = 0; i < desks.Count(); i++)
		{
			TerritoryHQ desk = desks[i];
			if (desk && !desk.IsRuined() && !desk.IsHologram() && vector.Distance(desk.GetPosition(), position) <= desk.GetRadius())
				here.Insert(desk);
		}
		if (here.Count() == 0)
			return RAPatch_BagRules.SpotRefusal(false, false);
		TerritoryHQ owner = TerritoryHQ.GetOldestHQ(here);
		if (!owner)
			owner = here[0];
		return RAPatch_BagRules.SpotRefusal(true, id != "" && owner.IsAuthorized(id));
	}

	static void TellBagRefused(Man player, string refusal)
	{
		if (player && player.GetIdentity())
			NotificationSystem.SendNotificationToPlayerExtended(player, 8, "Sleeping bag", refusal);
	}

	void SetBag(string id, Object bag)
	{
		RAPatch_RespawnRecord record = Record(id);
		record.BagType = bag.GetType();
		record.BagPosition = bag.GetPosition();
		Save();
	}

	void ClearBag(string id)
	{
		RAPatch_RespawnRecord record = Record(id);
		record.BagType = "";
		Save();
	}

	// The object they set, if it's still there and still counts as a bag
	Object FindBag(string id)
	{
		RAPatch_RespawnRecord record = Record(id);
		if (record.BagType == "")
			return null;
		array<Object> nearby = new array<Object>;
		g_Game.GetObjectsAtPosition3D(record.BagPosition, 1, nearby, null);
		for (int i = 0; i < nearby.Count(); i++)
		{
			Object thing = nearby[i];
			if (thing && thing.GetType() == record.BagType && !thing.IsRuined() && IsBag(thing))
				return thing;
		}
		return null;
	}

	TerritoryHQ FindDesk(string id)
	{
		array<TerritoryHQ> desks = TerritoryHQ.All_HQs;
		for (int i = 0; i < desks.Count(); i++)
		{
			if (desks[i] && !desks[i].IsRuined() && !desks[i].IsHologram() && desks[i].IsAuthorized(id))
				return desks[i];
		}
		return null;
	}

	// A clear spot beside the desk, level with it, so a desk on an upper floor keeps you on that floor
	vector SpotBesideDesk(TerritoryHQ desk)
	{
		vector middle = desk.GetPosition() + "0 1 0";
		for (int i = 0; i < 8; i++)
		{
			float angle = i * Math.PI / 4;
			vector spot = middle + Vector(Math.Cos(angle) * 2.5, 0, Math.Sin(angle) * 2.5);
			vector hit;
			vector normal;
			int component;
			if (!DayZPhysics.RaycastRV(middle, spot, hit, normal, component, null, desk, null, false, false, ObjIntersectView, 0.3))
				return spot - "0 1 0";
		}
		return desk.GetPosition() + "0 0.5 0";
	}

	// Where this player would come back, without changing anything. True when they have a bag or a desk to come back to, with
	// how long until it's ready. A chain: their bag if it's set and working, otherwise a desk they're authorised on. A bag that's
	// set and working but cooling down is waited for: the desk isn't a way round its cooldown.
	bool Plan(string id, out vector position, out string kind, out int secondsLeft, out string note)
	{
		RAPatch_RespawnSettings rules = RAPatch_Settings.Get().Respawn;
		if (rules.BagsEnabled)
		{
			Object bag = FindBag(id);
			// With BagsNeedOwnTerritory on, a bag only works while it's in a territory they're authorised on: not after they're taken off the desk, or the desk is gone
			if (bag && rules.BagsNeedOwnTerritory && BagSpotRefusalFor(id, bag.GetPosition()) != "")
			{
				note = "Your sleeping bag isn't in your territory any more.";
				bag = null;
			}
			if (bag)
			{
				position = bag.GetPosition() + "0 0.2 0";
				kind = BAG;
				secondsLeft = SecondsLeft(id, BAG);
				return true;
			}
		}
		if (rules.HQEnabled)
		{
			TerritoryHQ desk = FindDesk(id);
			if (desk)
			{
				position = SpotBesideDesk(desk);
				kind = HQ;
				secondsLeft = SecondsLeft(id, HQ);
				return true;
			}
		}
		return false;
	}

	// True when the player comes back somewhere other than vanilla's spawn points. Note says what happened, if it needs saying.
	bool Decide(string id, out vector position, out string kind, out string note)
	{
		int secondsLeft;
		if (!Plan(id, position, kind, secondsLeft, note))
			return false;
		if (secondsLeft == 0)
			return true;
		if (kind == BAG)
			note = "Your sleeping bag is ready in " + ReadableTime(secondsLeft) + ".";
		else
			note = "Your Territory HQ is ready in " + ReadableTime(secondsLeft) + ".";
		return false;
	}

	// The cooldown starts, longer if this respawn came soon after the last one's ended, and the bag wears
	void RecordUse(string id, string kind)
	{
		RAPatch_RespawnSettings rules = RAPatch_Settings.Get().Respawn;
		RAPatch_RespawnRecord record = Record(id);
		int now = Now();
		int readyAt = record.HQReadyAt;
		int penalties = record.HQPenalties;
		float cooldown = rules.HQCooldownSeconds;
		if (kind == BAG)
		{
			readyAt = record.BagReadyAt;
			penalties = record.BagPenalties;
			cooldown = rules.BagCooldownSeconds;
		}
		int window = rules.CooldownPenaltyWindowSeconds;
		if (readyAt > 0 && now - readyAt <= window)
			penalties++;
		else
			penalties = 0;
		cooldown += penalties * rules.CooldownPenaltySeconds;
		if (rules.CooldownMaxSeconds > 0 && cooldown > rules.CooldownMaxSeconds)
			cooldown = rules.CooldownMaxSeconds;
		// Whole numbers: a float can't hold a date this big, and would round it by up to a minute
		int seconds = cooldown;
		if (kind == BAG)
		{
			record.BagReadyAt = now + seconds;
			record.BagPenalties = penalties;
		}
		else
		{
			record.HQReadyAt = now + seconds;
			record.HQPenalties = penalties;
		}
		Save();
		if (kind != BAG)
			return;
		Object bag = FindBag(id);
		float wear = rules.BagWearPercent;
		if (bag && wear > 0)
			bag.DecreaseHealth("", "", bag.GetMaxHealth("", "") * wear / 100);
	}

	// The self test: pretend the cooldown ended this long ago
	void EndCooldownForTest(string id, int secondsAgo)
	{
		Record(id).BagReadyAt = Now() - secondsAgo;
	}

	// For an admin's test: both cooldowns over now, and no penalty building up
	void EndCooldownsNow(string id)
	{
		RAPatch_RespawnRecord record = Record(id);
		record.BagReadyAt = 0;
		record.HQReadyAt = 0;
		record.BagPenalties = 0;
		record.HQPenalties = 0;
		Save();
	}

	// Health and blood stay full
	void ApplyRespawnState(PlayerBase player)
	{
		if (!player || !player.IsAlive())
			return;
		RAPatch_RespawnSettings rules = RAPatch_Settings.Get().Respawn;
		player.GetStatWater().Set(player.GetStatWater().GetMax() * rules.RespawnWaterPercent / 100);
		player.GetStatEnergy().Set(player.GetStatEnergy().GetMax() * rules.RespawnEnergyPercent / 100);
	}

	void Tell(PlayerIdentity identity, string message)
	{
		if (identity)
			NotificationSystem.SendNotificationToPlayerIdentityExtended(identity, 8, "Respawn", message);
	}
}
