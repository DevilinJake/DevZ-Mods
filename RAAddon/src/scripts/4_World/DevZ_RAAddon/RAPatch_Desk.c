// The desk. Radius and player limit are protected members with no setter that reaches the trigger, so the addon
// sets them just before RA's EEInit, which is what builds the trigger at the desk's radius.
//
// Modules: an item only fits a slot if its own config says so, and config can't come from a settings file. So a
// module isn't an attachment. The item is used up on install, and the desk remembers what went in. That lets a server
// owner name any item class. The saved list goes through CF, so a world without this mod's data still loads.
class RAPatch_Modules
{
	static array<ref RAPatch_ModuleRow> Rows()
	{
		return RAPatch_Settings.Get().Desk.Modules;
	}

	static RAPatch_ModuleRow RowForItem(EntityAI item)
	{
		if (!item)
			return null;
		array<ref RAPatch_ModuleRow> rows = Rows();
		for (int i = 0; i < rows.Count(); i++)
		{
			RAPatch_ModuleRow row = rows[i];
			for (int j = 0; j < row.Items.Count(); j++)
			{
				if (item.IsKindOf(row.Items[j]))
					return row;
			}
		}
		return null;
	}

	static int IndexOf(string name)
	{
		array<ref RAPatch_ModuleRow> rows = Rows();
		for (int i = 0; i < rows.Count(); i++)
		{
			if (rows[i].Name == name)
				return i;
		}
		return -1;
	}

	static RAPatch_ModuleRow RowByName(string name)
	{
		int index = IndexOf(name);
		if (index == -1)
			return null;
		return Rows()[index];
	}

	// A test character has no identity, so it can't be asked about permission
	static bool IsNearDeskWith(PlayerBase player, string name)
	{
		if (!player || !TerritoryHQ.All_HQs)
			return false;
		float distance = RAPatch_Settings.Get().Desk.ModuleCraftDistance;
		vector position = player.GetPosition();
		foreach (TerritoryHQ hq : TerritoryHQ.All_HQs)
		{
			if (!hq || vector.Distance(position, hq.GetPosition()) > distance || !hq.RAPatch_HasModule(name))
				continue;
			if (!player.GetIdentity() || hq.IsAuthorized(player))
				return true;
		}
		return false;
	}

	static bool ClassExists(string cls)
	{
		return GetGame().ConfigIsExisting("CfgVehicles " + cls) || GetGame().ConfigIsExisting("CfgMagazines " + cls) || GetGame().ConfigIsExisting("CfgWeapons " + cls);
	}

	static void WarnAboutMissingClasses()
	{
		array<ref RAPatch_RecipeRow> recipes = RAPatch_Settings.Get().Crafting.Recipes;
		for (int r = 0; r < recipes.Count(); r++)
		{
			RAPatch_RecipeRow recipe = recipes[r];
			if (recipe.Enabled && recipe.Result != "" && !ClassExists(recipe.Result))
				RAPatch_Settings.Log("WARNING recipe " + recipe.Name + " is switched off: there's no item class called " + recipe.Result);
		}
		array<string> bags = RAPatch_Settings.Get().Respawn.BagClasses;
		for (int b = 0; b < bags.Count(); b++)
		{
			if (!ClassExists(bags[b]))
				RAPatch_Settings.Log("WARNING Respawn.BagClasses: there's no item class called " + bags[b]);
		}
		array<ref RAPatch_ModuleRow> rows = Rows();
		for (int i = 0; i < rows.Count(); i++)
		{
			RAPatch_ModuleRow row = rows[i];
			for (int j = 0; j < row.Items.Count(); j++)
			{
				if (!ClassExists(row.Items[j]))
					RAPatch_Settings.Log("WARNING module " + row.Name + ": there's no item class called " + row.Items[j]);
			}
		}
	}
}

// Desk.RadiusMayOverlap false: no two territories overlap. RA only refuses a desk placed inside another's radius, so two desks
// could stand a little more than one radius apart and share ground. Works on clients too, for the outline and the action's text.
class RAPatch_DeskSpacing
{
	static const string DENIED_TEXT = "Can't place (territories would overlap)";

	// "" when a new desk can go here. ignore: a desk that's just been put there, which mustn't count against itself
	static string Refusal(vector position, TerritoryHQ ignore = null)
	{
		RAPatch_DeskSettings desk = RAPatch_Settings.Get().Desk;
		if (desk.RadiusMayOverlap || !TerritoryHQ.All_HQs)
			return "";
		foreach (TerritoryHQ other : TerritoryHQ.All_HQs)
		{
			if (!other || other == ignore || other.IsHologram() || other.IsRuined())
				continue;
			float apart = vector.Distance(position, other.GetPosition());
			if (apart < other.GetRadius() + desk.Radius)
			{
				int needed = Math.Ceil(other.GetRadius() + desk.Radius - apart);
				return "Too close to another territory: territories can't overlap. Move " + needed.ToString() + " m further away.";
			}
		}
		return "";
	}
}

modded class TerritoryHQ_Kit
{
	override bool CanBePlaced(Man player, vector position)
	{
		if (RAPatch_DeskSpacing.Refusal(position) != "")
			return false;
		return super.CanBePlaced(player, position);
	}

	// The last check, at the spot the player's game sent. The game deletes the kit as the desk goes down, so a refusal gives a new kit back.
	override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
	{
		if (g_Game.IsServer())
		{
			string refusal = RAPatch_DeskSpacing.Refusal(position);
			if (refusal != "")
			{
				RAPatch_Settings.Log("refused a desk at " + position + ": " + refusal);
				vector kitSpot = position;
				if (player)
					kitSpot = player.GetPosition();
				g_Game.CreateObjectEx("TerritoryHQ_Kit", kitSpot, ECE_PLACE_ON_SURFACE);
				if (player)
					NotificationSystem.SendNotificationToPlayerExtended(player, 8, "Territory HQ", refusal);
				return;
			}
		}
		super.OnPlacementComplete(player, position, orientation);
	}
}

// RA's "[ADMIN] AUTHORIZE" only checks for an admin when VPP Admin Tools is loaded. Without it, it shows to everyone on a desk
// with a code lock, and the server authorises them. So without VPP it's off: admins can use their admin tool's own way in.
#ifndef AVPPAdminTools
modded class ActionAdminTerritoryAuthorize
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		return false;
	}
}
#endif

modded class TerritoryHQ
{
	protected ref array<string> m_RAPatch_InstalledNames = new array<string>;
	protected ref array<string> m_RAPatch_InstalledItems = new array<string>;
	protected ref array<float> m_RAPatch_InstalledHealth = new array<float>;
	protected int m_RAPatch_ModuleMask;		// by row in the settings file, so clients can read it
	protected int m_RAPatch_ModuleCount;

	void TerritoryHQ()
	{
		RegisterNetSyncVariableInt("m_RAPatch_ModuleMask");
		RegisterNetSyncVariableInt("m_RAPatch_DoorCount");
		RegisterNetSyncVariableInt("m_RAPatch_GateCount");
		RegisterNetSyncVariableInt("m_RAPatch_ModuleCount");
	}

	override float GetCostToUpkeep()
	{
		return RAPatch_Decay.Upkeep(RAPatch_Settings.Get().Decay.DeskNailsPerDay);
	}

	override float GetTimeToDecay()
	{
		return RAPatch_Decay.Seconds(RAPatch_Settings.Get().Decay.DeskHours);
	}

	override bool ConsumeNails(float amount)
	{
		if (!RAPatch_Settings.Get().Decay.Enabled)
			return true;
		return super.ConsumeNails(amount * RAPatch_UpkeepMultiplier());
	}

	// What the desk's name uses to work out the time left
	override float GetBaseUpkeepCost()
	{
		return super.GetBaseUpkeepCost() * RAPatch_UpkeepMultiplier();
	}

	override void EEInit()
	{
		#ifdef SERVER
		RAPatch_ApplyRules();
		#endif
		super.EEInit();
	}

	int RAPatch_AuthorizedCount()
	{
		if (!m_AuthorizationList)
			return 0;
		return m_AuthorizationList.Count();
	}

	int RAPatch_ModuleCount()
	{
		return m_RAPatch_ModuleCount;
	}

	bool RAPatch_HasModule(string name)
	{
		int index = RAPatch_Modules.IndexOf(name);
		if (index == -1)
			return false;
		return (m_RAPatch_ModuleMask & (1 << index)) != 0;
	}

	// Works on clients too, from the synced mask
	protected float RAPatch_UpkeepMultiplier()
	{
		float multiplier = 1;
		array<ref RAPatch_ModuleRow> rows = RAPatch_Modules.Rows();
		for (int i = 0; i < rows.Count() && i < RAPatch_Settings.MAX_MODULES; i++)
		{
			if ((m_RAPatch_ModuleMask & (1 << i)) != 0)
				multiplier *= rows[i].UpkeepMultiplier;
		}
		return multiplier;
	}

	protected bool RAPatch_StorageOpen()
	{
		array<ref RAPatch_ModuleRow> rows = RAPatch_Modules.Rows();
		for (int i = 0; i < rows.Count() && i < RAPatch_Settings.MAX_MODULES; i++)
		{
			if ((m_RAPatch_ModuleMask & (1 << i)) != 0 && rows[i].Storage)
				return true;
		}
		return false;
	}

	int RAPatch_CargoCount()
	{
		CargoBase cargo = GetInventory().GetCargo();
		if (!cargo)
			return 0;
		return cargo.GetItemCount();
	}

	// Name of the module that would come out next, for the action's text. Works on clients.
	string RAPatch_LastModuleName()
	{
		if (m_RAPatch_InstalledNames.Count() == 0)
			return "module";
		return m_RAPatch_InstalledNames[m_RAPatch_InstalledNames.Count() - 1];
	}

	bool RAPatch_LastIsStorage()
	{
		if (m_RAPatch_InstalledNames.Count() == 0)
			return false;
		RAPatch_ModuleRow row = RAPatch_Modules.RowByName(m_RAPatch_InstalledNames[m_RAPatch_InstalledNames.Count() - 1]);
		return row && row.Storage;
	}

	// "" means it can go in. Works on clients too, so the action can say why not.
	string RAPatch_WhyNotInstall(ItemBase item)
	{
		RAPatch_ModuleRow row = RAPatch_Modules.RowForItem(item);
		if (!row)
			return "not a module";
		if (RAPatch_HasModule(row.Name))
			return "already installed";
		if (item.IsRuined())
			return "ruined";
		CargoBase cargo = item.GetInventory().GetCargo();
		if (item.GetInventory().AttachmentCount() > 0 || (cargo && cargo.GetItemCount() > 0))
			return "empty it first";
		if (row.RadiusAdded > 0 && !RAPatch_Settings.Get().Desk.RadiusMayOverlap)
		{
			float newRadius = GetRadius() + row.RadiusAdded;
			foreach (TerritoryHQ other : All_HQs)
			{
				if (other && other != this && !other.IsHologram() && vector.Distance(other.GetPosition(), GetPosition()) < other.GetRadius() + newRadius)
					return "would overlap another territory";
			}
		}
		return "";
	}

	bool RAPatch_Install(ItemBase item)
	{
		if (!g_Game.IsServer() || RAPatch_WhyNotInstall(item) != "")
			return false;
		RAPatch_ModuleRow row = RAPatch_Modules.RowForItem(item);
		m_RAPatch_InstalledNames.Insert(row.Name);
		m_RAPatch_InstalledItems.Insert(item.GetType());
		m_RAPatch_InstalledHealth.Insert(item.GetHealth("", ""));
		item.Delete();
		RAPatch_Refresh();
		RAPatch_Settings.Log("installed " + row.Name + " in the desk at " + GetPosition());
		return true;
	}

	// player null: on the ground beside the desk
	bool RAPatch_RemoveLast(PlayerBase player)
	{
		if (!g_Game.IsServer() || m_RAPatch_InstalledNames.Count() == 0)
			return false;
		if (RAPatch_LastIsStorage() && RAPatch_CargoCount() > 0)
			return false;
		int last = m_RAPatch_InstalledNames.Count() - 1;
		RAPatch_GiveBack(m_RAPatch_InstalledItems[last], m_RAPatch_InstalledHealth[last], player);
		m_RAPatch_InstalledNames.Remove(last);
		m_RAPatch_InstalledItems.Remove(last);
		m_RAPatch_InstalledHealth.Remove(last);
		RAPatch_Refresh();
		return true;
	}

	// The dashboard's Remove button: any module, not just the last
	bool RAPatch_RemoveNamed(string name, PlayerBase player)
	{
		if (!g_Game.IsServer())
			return false;
		int index = m_RAPatch_InstalledNames.Find(name);
		if (index == -1)
			return false;
		RAPatch_ModuleRow row = RAPatch_Modules.RowByName(name);
		if (row && row.Storage && RAPatch_CargoCount() > 0)
			return false;
		RAPatch_GiveBack(m_RAPatch_InstalledItems[index], m_RAPatch_InstalledHealth[index], player);
		m_RAPatch_InstalledNames.Remove(index);
		m_RAPatch_InstalledItems.Remove(index);
		m_RAPatch_InstalledHealth.Remove(index);
		RAPatch_Refresh();
		return true;
	}

	// The desk is being packed up or was destroyed
	void RAPatch_DropModules()
	{
		if (!g_Game.IsServer())
			return;
		for (int i = 0; i < m_RAPatch_InstalledItems.Count(); i++)
			RAPatch_GiveBack(m_RAPatch_InstalledItems[i], m_RAPatch_InstalledHealth[i], null);
		m_RAPatch_InstalledNames.Clear();
		m_RAPatch_InstalledItems.Clear();
		m_RAPatch_InstalledHealth.Clear();
		m_RAPatch_ModuleMask = 0;
		m_RAPatch_ModuleCount = 0;
		SetSynchDirty();
	}

	protected void RAPatch_GiveBack(string type, float health, PlayerBase player)
	{
		EntityAI item;
		if (player)
		{
			item = player.GetHumanInventory().CreateInHands(type);
			if (!item)
				item = player.GetInventory().CreateInInventory(type);
		}
		if (!item)
			item = EntityAI.Cast(g_Game.CreateObjectEx(type, GetPosition() + "1 0 0", ECE_PLACE_ON_SURFACE));
		if (item)
			item.SetHealth("", "", health);
	}

	protected void RAPatch_Refresh()
	{
		RAPatch_ApplyRules();
		if (m_TerritoryTrigger)
			m_TerritoryTrigger.SetRadius(m_Radius);
	}

	protected void RAPatch_ApplyRules()
	{
		RAPatch_DeskSettings desk = RAPatch_Settings.Get().Desk;
		float radius = desk.Radius;
		int players = desk.MaxPlayers;
		int mask = 0;
		foreach (string name : m_RAPatch_InstalledNames)
		{
			int index = RAPatch_Modules.IndexOf(name);
			if (index == -1 || index >= RAPatch_Settings.MAX_MODULES)
				continue;
			mask = mask | (1 << index);
			RAPatch_ModuleRow row = RAPatch_Modules.Rows()[index];
			radius += row.RadiusAdded;
			players += row.PlayersAdded;
		}
		m_Radius = radius;
		if (desk.MaxPlayers > 0)
			m_MaxPlayers = players;
		m_RAPatch_ModuleMask = mask;
		m_RAPatch_ModuleCount = m_RAPatch_InstalledNames.Count();
		SetSynchDirty();
	}

	override bool CanDisplayCargo()
	{
		if (!IsRuined() && !RAPatch_StorageOpen())
			return false;
		return super.CanDisplayCargo();
	}

	override bool CanReceiveItemIntoCargo(EntityAI item)
	{
		if (!RAPatch_StorageOpen())
			return false;
		return super.CanReceiveItemIntoCargo(item);
	}

	override bool CanLoadItemIntoCargo(EntityAI item)
	{
		if (!RAPatch_StorageOpen())
			return false;
		return super.CanLoadItemIntoCargo(item);
	}

	// A charge also sets off a small vanilla explosion for players. The desk ignores that one: with Raiding.DeskBlastDamage on,
	// the addon's own hit makes DeskC4ToDestroy exact, and with it off, charges can't hurt the desk. Grenades still count.
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		RAPatch_RaidSettings raid = RAPatch_Settings.Get().Raiding;
		if (damageType == DamageType.EXPLOSION && (ammo == raid.C4PlayerAmmo || ammo == raid.ImprovisedPlayerAmmo))
			return false;
		return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	}

	override void OnDamageDestroyed(int oldLevel)
	{
		RAPatch_AlarmDestroyed();
		RAPatch_DropModules();
		super.OnDamageDestroyed(oldLevel);
	}

	// Some ways of killing a desk come through here and not there. Both are safe to run twice.
	override void EEKilled(Object killer)
	{
		RAPatch_AlarmDestroyed();
		RAPatch_DropModules();
		super.EEKilled(killer);
	}

	override void CF_OnStoreSave(CF_ModStorageMap storage)
	{
		super.CF_OnStoreSave(storage);
		CF_ModStorage ctx = storage.Get(RAPatch_Net.MOD);
		if (!ctx)
			return;
		ctx.Write(m_RAPatch_InstalledNames.Count());
		for (int i = 0; i < m_RAPatch_InstalledNames.Count(); i++)
		{
			ctx.Write(m_RAPatch_InstalledNames[i]);
			ctx.Write(m_RAPatch_InstalledItems[i]);
			ctx.Write(m_RAPatch_InstalledHealth[i]);
		}
		ctx.Write(m_RAPatch_Webhook);
	}

	override bool CF_OnStoreLoad(CF_ModStorageMap storage)
	{
		if (!super.CF_OnStoreLoad(storage))
			return false;
		CF_ModStorage ctx = storage.Get(RAPatch_Net.MOD);
		if (!ctx)
			return true;
		int count;
		if (!ctx.Read(count))
			return false;
		for (int i = 0; i < count; i++)
		{
			string name;
			string type;
			float health;
			if (!ctx.Read(name) || !ctx.Read(type) || !ctx.Read(health))
				return false;
			m_RAPatch_InstalledNames.Insert(name);
			m_RAPatch_InstalledItems.Insert(type);
			m_RAPatch_InstalledHealth.Insert(health);
		}
		return ctx.Read(m_RAPatch_Webhook);
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		RAPatch_Refresh();
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(RAPatch_ActionInstallModule);
		AddAction(RAPatch_ActionRemoveModule);
		AddAction(RAPatch_ActionOpenDashboard);
	}

	// The raid alarm and the dashboard's figures. In this class because a modded class can't use what a later file adds to it.
	protected string m_RAPatch_Webhook;		// server only. Never synced, and never logged
	protected int m_RAPatch_AlarmReadyAt;	// in game milliseconds
	protected int m_RAPatch_AlarmHits;
	protected int m_RAPatch_TestReadyAt;
	protected int m_RAPatch_PartCount;
	protected int m_RAPatch_DoorCount;
	protected int m_RAPatch_GateCount;

	int RAPatch_DoorCount()
	{
		return m_RAPatch_DoorCount;
	}

	int RAPatch_GateCount()
	{
		return m_RAPatch_GateCount;
	}

	protected void RAPatch_CountDoorOrGate(ItemBase item, int change)
	{
		int kind = RAPatch_Limits.KindOf(item.GetType());
		if (kind == RAPatch_Limits.DOOR)
			m_RAPatch_DoorCount = Math.Max(m_RAPatch_DoorCount + change, 0);
		else if (kind == RAPatch_Limits.GATE)
			m_RAPatch_GateCount = Math.Max(m_RAPatch_GateCount + change, 0);
		else
			return;
		SetSynchDirty();
	}

	override void RegisterItem(ItemBase base_item)
	{
		super.RegisterItem(base_item);
		if (base_item && base_item.IsInherited(BaseBuilding))
		{
			m_RAPatch_PartCount++;
			RAPatch_CountDoorOrGate(base_item, 1);
		}
	}

	override void UnregisterItem(ItemBase base_item)
	{
		super.UnregisterItem(base_item);
		if (base_item && base_item.IsInherited(BaseBuilding))
		{
			m_RAPatch_PartCount--;
			RAPatch_CountDoorOrGate(base_item, -1);
		}
	}

	bool RAPatch_HasRaidAlarm()
	{
		array<ref RAPatch_ModuleRow> rows = RAPatch_Modules.Rows();
		for (int i = 0; i < rows.Count() && i < RAPatch_Settings.MAX_MODULES; i++)
		{
			if (rows[i].RaidAlarm && (m_RAPatch_ModuleMask & (1 << i)) != 0)
				return true;
		}
		return false;
	}

	void RAPatch_SetWebhook(string url)
	{
		m_RAPatch_Webhook = url;
	}

	bool RAPatch_WebhookIsSet()
	{
		return m_RAPatch_Webhook != "";
	}

	protected string RAPatch_Where()
	{
		vector position = GetPosition();
		return Math.Round(position[0]).ToString() + ", " + Math.Round(position[2]).ToString();
	}

	void RAPatch_RaidHit(string kind, string what, Man attacker)
	{
		if (!g_Game.IsServer() || m_RAPatch_Webhook == "" || !RAPatch_HasRaidAlarm())
			return;
		// A team's own building and repairs aren't a raid. RA's charges don't say who armed them, so those always alert.
		if (attacker && attacker.GetIdentity() && IsAuthorized(attacker))
			return;
		int now = g_Game.GetTime();
		if (now < m_RAPatch_AlarmReadyAt)
		{
			m_RAPatch_AlarmHits++;
			return;
		}
		m_RAPatch_AlarmReadyAt = now + RAPatch_Settings.Get().Desk.RaidAlarmCooldownSeconds * 1000;
		string message = "RAID ALARM: your base at map position " + RAPatch_Where() + " is under attack. " + kind + " hit: " + RAPatch_Clean(what) + ".";
		if (m_RAPatch_AlarmHits > 0)
			message += " " + m_RAPatch_AlarmHits + " more hits since the last alert.";
		m_RAPatch_AlarmHits = 0;
		RAPatch_Webhook.Post(m_RAPatch_Webhook, message);
	}

	// Item names go inside a JSON string, so nothing that could break out of it
	protected string RAPatch_Clean(string text)
	{
		text.Replace("\"", "");
		text.Replace("\\", "");
		return text;
	}

	// The last message. The desk's gone, so its webhook goes too.
	protected void RAPatch_AlarmDestroyed()
	{
		if (!g_Game.IsServer() || m_RAPatch_Webhook == "")
			return;
		if (RAPatch_HasRaidAlarm())
			RAPatch_Webhook.Post(m_RAPatch_Webhook, "RAID ALARM: your Territory HQ at map position " + RAPatch_Where() + " has been destroyed.");
		m_RAPatch_Webhook = "";
	}

	// "" when it went out
	string RAPatch_SendTest(PlayerIdentity reportTo = null)
	{
		if (m_RAPatch_Webhook == "")
			return "No webhook saved yet.";
		int now = g_Game.GetTime();
		if (now < m_RAPatch_TestReadyAt)
			return "Wait a moment before testing again.";
		m_RAPatch_TestReadyAt = now + 30000;
		RAPatch_Webhook.Post(m_RAPatch_Webhook, "Raid alarm test from your Territory HQ. This channel will get the alerts.", reportTo);
		return "";
	}

	RAPatch_DeskStatus RAPatch_BuildStatus()
	{
		RAPatch_DeskStatus status = new RAPatch_DeskStatus;
		status.Radius = GetRadius();
		status.Authorized = RAPatch_AuthorizedCount();
		status.MaxPlayers = GetMaxPlayers();
		status.NailCount = GetEquity();
		status.Parts = m_RAPatch_PartCount;
		status.HealthPercent = Math.Round(GetHealth01("", "") * 100);
		RA_CodeLock code_lock = RA_CodeLock.Cast(FindCodeLock());
		status.Locked = code_lock && code_lock.IsLocked();
		status.WebhookSet = m_RAPatch_Webhook != "";
		if (!RAPatch_Settings.Get().Decay.Enabled)
			status.MinutesLeft = -1;
		else if (GetRemainingTimeUntilDecay() == int.MAX)
			status.MinutesLeft = -2;
		else
			status.MinutesLeft = GetRemainingTimeUntilDecay();
		return status;
	}
}

// How long installing or removing takes
class RAPatch_ActionModuleCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(RAPatch_Settings.Get().Desk.ModuleInstallSeconds);
	}
}

class RAPatch_ActionModuleBase : ActionContinuousBase
{
	void RAPatch_ActionModuleBase()
	{
		m_CallbackClass = RAPatch_ActionModuleCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
	}

	// The desk offers this, not the item, so it needs the input that looks at the target
	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}

	override bool HasTarget()
	{
		return true;
	}

	override void CreateConditionComponents()
	{
		m_ConditionTarget = new CCTCursor;
		m_ConditionItem = new CCINone;
	}
}

class RAPatch_ActionInstallModule : RAPatch_ActionModuleBase
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		TerritoryHQ hq = TerritoryHQ.Cast(target.GetObject());
		if (!hq || !item || hq.IsRuined() || !hq.IsAuthorized(player))
			return false;
		RAPatch_ModuleRow row = RAPatch_Modules.RowForItem(item);
		if (!row)
			return false;
		string refusal = hq.RAPatch_WhyNotInstall(item);
		if (refusal != "" && g_Game.IsDedicatedServer())
			return false;
		if (refusal != "")
			m_Text = "Can't install " + row.Name + " (" + refusal + ")";
		else
			m_Text = "Install " + row.Name;
		return true;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		TerritoryHQ hq = TerritoryHQ.Cast(action_data.m_Target.GetObject());
		if (hq && hq.IsAuthorized(action_data.m_Player))
			hq.RAPatch_Install(action_data.m_MainItem);
	}
}

class RAPatch_ActionRemoveModule : RAPatch_ActionModuleBase
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		TerritoryHQ hq = TerritoryHQ.Cast(target.GetObject());
		if (!hq || item || hq.IsRuined() || hq.RAPatch_ModuleCount() == 0 || !hq.IsAuthorized(player))
			return false;
		if (!RAPatch_Settings.Get().Desk.ModulesRemovable)
			return false;
		m_Text = "Remove " + hq.RAPatch_LastModuleName();
		return true;
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		TerritoryHQ hq = TerritoryHQ.Cast(action_data.m_Target.GetObject());
		if (!hq || !hq.IsAuthorized(action_data.m_Player) || !RAPatch_Settings.Get().Desk.ModulesRemovable)
			return;
		if (!hq.RAPatch_RemoveLast(action_data.m_Player))
			NotificationSystem.SendNotificationToPlayerExtended(action_data.m_Player, 5, "Can't remove module", "Empty the desk's storage and workbench first.");
	}
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(RAPatch_ActionInstallModule);
		actions.Insert(RAPatch_ActionRemoveModule);
	}
}

modded class TerritoryTrigger
{
	// RA's version squares the radius before making its search box, so a 36 m desk searches 1296 m each way,
	// and a bigger desk searches far more. Same job, with the box the right size.
	override void SetRadius(float radius)
	{
		if (!g_Game.IsServer())
			return;
		SetCollisionSphere(radius);
		vector position = GetWorldPosition();
		array<EntityAI> entities = {};
		DayZPlayerUtils.SceneGetEntitiesInBox(position - Vector(radius, radius, radius), position + Vector(radius, radius, radius), entities);
		float radiusSq = radius * radius;
		for (int i = 0; i < entities.Count(); i++)
		{
			if (vector.DistanceSq(position, entities[i].GetWorldPosition()) <= radiusSq && !entities[i].IsPendingDeletion())
				SendEvent(entities[i], EntityEvent.ENTER, 0);
		}
	}
}

modded class ActionTerritoryAuthorize
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!super.ActionCondition(player, target, item))
			return false;
		if (RAPatch_Settings.Get().Desk.OpenAuthorization)
			return true;
		TerritoryHQ hq = TerritoryHQ.Cast(target.GetObject());
		// Past RA's check, a locked lock has already let this player through
		RA_CodeLock code_lock = RA_CodeLock.Cast(hq.FindAttachmentBySlotName("CodeLock"));
		if (code_lock && code_lock.IsLocked())
			return true;
		return hq.RAPatch_AuthorizedCount() == 0;
	}
}
