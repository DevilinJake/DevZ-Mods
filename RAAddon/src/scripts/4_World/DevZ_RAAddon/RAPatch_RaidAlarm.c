// The raid alarm: a desk with a RaidAlarm module posts to its team's own Discord webhook when an enemy damages
// something in its territory. A webhook address lets anyone post to that channel, so it stays on the server: it's saved
// on the desk, never synced to players, and never written to a log.
class RAPatch_WebhookCallback : RestCallback
{
	// Discord answers a good post with no content, and the game reports that as this "error"
	protected static const int DELIVERED = 8;

	PlayerIdentity m_ReportTo;	// a test: tell them how it went

	override void OnError(int errorCode)
	{
		if (errorCode == DELIVERED)
		{
			Delivered();
			return;
		}
		RAPatch_Settings.Log("webhook post failed, error " + errorCode);
		Report("Discord didn't accept that. Check the webhook address, and save it again.");
	}

	override void OnTimeout()
	{
		RAPatch_Settings.Log("webhook post timed out");
		Report("Discord didn't answer. Try again in a moment.");
	}

	override void OnSuccess(string data, int dataSize)
	{
		Delivered();
	}

	protected void Delivered()
	{
		RAPatch_Settings.Log("webhook post sent");
		Report("Test delivered. Check your Discord channel.");
	}

	protected void Report(string message)
	{
		if (m_ReportTo)
			NotificationSystem.SendNotificationToPlayerIdentityExtended(m_ReportTo, 8, "Raid alarm", message);
	}
}

class RAPatch_Webhook
{
	protected static const string BASE = "https://discord.com/api/webhooks/";
	protected static const string BASE_OLD = "https://discordapp.com/api/webhooks/";
	protected static const int MAX_LENGTH = 200;

	static bool DryRun;			// the self test: count what would be sent, and don't send it
	static int DryRunCount;
	static string DryRunLast;
	static ref array<string> DryRunAll = new array<string>;

	protected static bool s_AdminWarned;

	// The staff channel from AdminAlerts.Webhook, or "" when it's off or isn't a real Discord webhook
	static string AdminUrl()
	{
		string url = RAPatch_Settings.Get().AdminAlerts.Webhook;
		url.TrimInPlace();
		if (url == "")
			return "";
		if (!IsValid(url))
		{
			if (!s_AdminWarned)
				RAPatch_Settings.Log("WARNING AdminAlerts.Webhook isn't a Discord webhook address, so staff raid alerts are off");
			s_AdminWarned = true;
			return "";
		}
		return url;
	}

	protected static ref array<ref RAPatch_WebhookCallback> s_Callbacks = new array<ref RAPatch_WebhookCallback>;

	// Only real Discord webhooks, so a player can't make the server post anywhere else
	static bool IsValid(string url)
	{
		if (url.Length() > MAX_LENGTH || url.Contains(" ") || url.Contains("\"") || url.Contains("\\"))
			return false;
		if (url.IndexOf(BASE) == 0)
			return GoodEnding(url.Substring(BASE.Length(), url.Length() - BASE.Length()));
		if (url.IndexOf(BASE_OLD) == 0)
			return GoodEnding(url.Substring(BASE_OLD.Length(), url.Length() - BASE_OLD.Length()));
		return false;
	}

	// The part Discord makes: a long number, a slash, and a long token
	protected static bool GoodEnding(string ending)
	{
		array<string> parts = new array<string>;
		ending.Split("/", parts);
		if (parts.Count() != 2 || parts[0].Length() < 15 || parts[1].Length() < 30)
			return false;
		string number = parts[0];
		for (int i = 0; i < number.Length(); i++)
		{
			int digit = number.Substring(i, 1).ToAscii();
			if (digit < 48 || digit > 57)
				return false;
		}
		string token = parts[1];
		for (int j = 0; j < token.Length(); j++)
		{
			string letter = token.Substring(j, 1);
			int code = letter.ToAscii();
			bool letterOrDigit = (code >= 48 && code <= 57) || (code >= 65 && code <= 90) || (code >= 97 && code <= 122);
			if (!letterOrDigit && letter != "_" && letter != "-")
				return false;
		}
		return true;
	}

	static void Post(string url, string message, PlayerIdentity reportTo = null)
	{
		if (DryRun)
		{
			DryRunCount++;
			DryRunLast = message;
			DryRunAll.Insert(message);
			return;
		}
		if (!IsValid(url))
			return;
		RestApi api = GetRestApi();
		if (!api)
			api = CreateRestApi();
		string base = BASE;
		if (url.IndexOf(BASE_OLD) == 0)
			base = BASE_OLD;
		RestContext context = api.GetRestContext(base);
		context.SetHeader("application/json");
		// The message is built by this mod, with nothing typed by a player. No mentions, so it can't ping @everyone.
		string body = "{\"content\":\"" + message + "\",\"allowed_mentions\":{\"parse\":[]}}";
		RAPatch_WebhookCallback callback = new RAPatch_WebhookCallback;
		callback.m_ReportTo = reportTo;
		s_Callbacks.Insert(callback);
		if (s_Callbacks.Count() > 16)
			s_Callbacks.Remove(0);
		context.POST(callback, url.Substring(base.Length(), url.Length() - base.Length()), body);
	}
}

// The dashboard's figures. Goes to the player as JSON, in one message, so it has to stay short.
class RAPatch_DeskStatus
{
	float Radius;
	int Authorized;
	int MaxPlayers;
	int NailCount;
	int MinutesLeft;	// -1 = decay is off, -2 = no upkeep
	int Parts;
	int HealthPercent;
	bool Locked;
	bool WebhookSet;
	string Message;
}

modded class ItemBase
{
	// Everything a desk looks after: parts, the desk itself, tents, barrels, crates.
	// Decay takes health off another way and doesn't come through here.
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		bool allowed = super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		if (allowed && g_Game.IsServer() && GetCostToUpkeep() > 0)
			RAPatch_ReportHit(damageType, source);
		return allowed;
	}

	void RAPatch_ReportHitForTest()
	{
		RAPatch_ReportHit(DamageType.CLOSE_COMBAT, null);
	}

	protected void RAPatch_ReportHit(int damageType, EntityAI source)
	{
		TerritoryHQ desk = GetCurrentHQ();
		if (!desk)
			return;
		Man attacker;
		if (source)
			attacker = source.GetHierarchyRootPlayer();
		string kind;
		if (damageType == DamageType.CLOSE_COMBAT)
			kind = "Melee";
		else if (damageType == DamageType.FIRE_ARM)
			kind = "Gunfire";
		else if (damageType == DamageType.EXPLOSION)
			kind = "Explosive";
		else
			return;
		// Melee and gunfire with no player behind them are zombies and animals
		if (!attacker && damageType != DamageType.EXPLOSION)
			return;
		desk.RAPatch_RaidHit(kind, GetDisplayName(), attacker);
	}
}

modded class ScriptedExplosion
{
	// RA's charges take health off directly, so they're reported from here
	override bool OnDamageCalculated(Object target, float damage, int penetration_level)
	{
		bool hurts = super.OnDamageCalculated(target, damage, penetration_level);
		BaseBuilding part = BaseBuilding.Cast(target);
		if (part && part.GetCurrentHQ() && part.RAPatch_CanTakeBlast())
			part.GetCurrentHQ().RAPatch_RaidHit("Explosive charge", part.GetDisplayName(), null);
		return hurts;
	}
}

// Dashboard messages. The server checks every request: authorised, standing at the desk, and the module fitted.
class RAPatch_DashboardNet
{
	static const string STATUS_REQUEST = "RPC_DashStatusRequest";
	static const string STATUS = "RPC_DashStatus";
	static const string SET_WEBHOOK = "RPC_DashSetWebhook";
	static const string TEST_WEBHOOK = "RPC_DashTestWebhook";
	static const string REMOVE_MODULE = "RPC_DashRemoveModule";
	protected static const float MAX_DISTANCE = 5;

	protected static ref RAPatch_DashboardNet s_Instance;

	// Client: what the menu reads
	static TerritoryHQ OpenRequest;
	static ref RAPatch_DeskStatus Latest;
	static int LatestStamp;

	static void Start()
	{
		if (!s_Instance)
			s_Instance = new RAPatch_DashboardNet;
	}

	void RAPatch_DashboardNet()
	{
		GetRPCManager().AddRPC(RAPatch_Net.MOD, STATUS_REQUEST, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, SET_WEBHOOK, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, TEST_WEBHOOK, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, REMOVE_MODULE, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, STATUS, this, SingleplayerExecutionType.Client);
	}

	protected TerritoryHQ AllowedDesk(PlayerIdentity sender, Object target)
	{
		TerritoryHQ desk = TerritoryHQ.Cast(target);
		if (!desk || !sender)
			return null;
		PlayerBase player = PlayerBase.Cast(sender.GetPlayer());
		if (!player || vector.Distance(player.GetPosition(), desk.GetPosition()) > MAX_DISTANCE || !desk.IsAuthorized(player))
			return null;
		return desk;
	}

	protected void Reply(PlayerIdentity to, TerritoryHQ desk, string message)
	{
		RAPatch_DeskStatus status = desk.RAPatch_BuildStatus();
		status.Message = message;
		string text;
		JsonSerializer serializer = new JsonSerializer;
		serializer.WriteToString(status, false, text);
		GetRPCManager().SendRPC(RAPatch_Net.MOD, STATUS, new Param1<string>(text), true, to, desk);
	}

	void RPC_DashStatusRequest(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		TerritoryHQ desk = AllowedDesk(sender, target);
		if (desk && RAPatch_Settings.Get().Desk.DashboardEnabled)
			Reply(sender, desk, "");
	}

	void RPC_DashSetWebhook(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		Param1<string> data;
		if (!ctx.Read(data))
			return;
		TerritoryHQ desk = AllowedDesk(sender, target);
		if (!desk)
			return;
		string url = data.param1;
		url.TrimInPlace();
		if (url == "")
		{
			desk.RAPatch_SetWebhook("");
			Reply(sender, desk, "Webhook removed.");
			return;
		}
		if (!desk.RAPatch_HasRaidAlarm())
		{
			Reply(sender, desk, "Install the raid alarm first.");
			return;
		}
		if (!RAPatch_Webhook.IsValid(url))
		{
			Reply(sender, desk, "That isn't a Discord webhook address.");
			return;
		}
		desk.RAPatch_SetWebhook(url);
		Reply(sender, desk, "Webhook saved.");
	}

	void RPC_DashTestWebhook(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		TerritoryHQ desk = AllowedDesk(sender, target);
		if (!desk)
			return;
		if (!desk.RAPatch_HasRaidAlarm())
		{
			Reply(sender, desk, "Install the raid alarm first.");
			return;
		}
		string refusal = desk.RAPatch_SendTest(sender);
		if (refusal == "")
			refusal = "Sending the test to the saved webhook...";
		Reply(sender, desk, refusal);
	}

	void RPC_DashRemoveModule(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		Param1<string> data;
		if (!ctx.Read(data))
			return;
		TerritoryHQ desk = AllowedDesk(sender, target);
		if (!desk || !RAPatch_Settings.Get().Desk.ModulesRemovable)
			return;
		if (!desk.RAPatch_RemoveNamed(data.param1, PlayerBase.Cast(sender.GetPlayer())))
			NotificationSystem.SendNotificationToPlayerIdentityExtended(sender, 5, "Can't remove module", "Empty the desk's storage and workbench first.");
		Reply(sender, desk, "");
	}

	void RPC_DashStatus(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;
		Param1<string> data;
		if (!ctx.Read(data))
			return;
		RAPatch_DeskStatus status = new RAPatch_DeskStatus;
		string error;
		JsonSerializer serializer = new JsonSerializer;
		if (!serializer.ReadFromString(status, data.param1, error))
			return;
		Latest = status;
		LatestStamp++;
	}
}

class RAPatch_ActionOpenDashboard : ActionInteractBase
{
	void RAPatch_ActionOpenDashboard()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_Text = "Open HQ Dashboard";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!RAPatch_Settings.Get().Desk.DashboardEnabled)
			return false;
		TerritoryHQ hq = TerritoryHQ.Cast(target.GetObject());
		return hq && !hq.IsRuined() && hq.IsAuthorized(player);
	}

	override void OnExecuteClient(ActionData action_data)
	{
		super.OnExecuteClient(action_data);
		TerritoryHQ hq = TerritoryHQ.Cast(action_data.m_Target.GetObject());
		if (hq)
			RAPatch_DashboardNet.OpenRequest = hq;
	}
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(RAPatch_ActionOpenDashboard);
	}
}
