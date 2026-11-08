// The workbench: a module that opens a crafting menu on the desk. A recipe can have any number of ingredients, unlike the
// game's own combining. They're paid for when Craft is pressed, from the desk's storage first and then the player's inventory.
// The timer runs on the desk, so the player can walk away, and a restart doesn't lose it. Finished items wait on the desk.
class RAPatch_Bench
{
	static RAPatch_BenchSettings Rules()
	{
		return RAPatch_Settings.Get().Crafting.Bench;
	}

	protected static bool AnyExists(array<string> classes)
	{
		for (int i = 0; i < classes.Count(); i++)
		{
			if (RAPatch_Modules.ClassExists(classes[i]))
				return true;
		}
		return false;
	}

	// A recipe for items this server doesn't have doesn't show
	static bool IsAvailable(RAPatch_BenchRow row)
	{
		if (!row.Enabled || !AnyExists(row.Results))
			return false;
		for (int i = 0; i < row.Ingredients.Count(); i++)
		{
			if (!AnyExists(row.Ingredients[i].Items))
				return false;
		}
		return true;
	}

	static RAPatch_BenchRow RowByName(string name)
	{
		array<ref RAPatch_BenchRow> rows = Rules().Recipes;
		for (int i = 0; i < rows.Count(); i++)
		{
			if (rows[i].Name == name)
				return rows[i];
		}
		return null;
	}

	static int IndexOfName(string name)
	{
		array<ref RAPatch_BenchRow> rows = Rules().Recipes;
		for (int i = 0; i < rows.Count(); i++)
		{
			if (rows[i].Name == name)
				return i;
		}
		return -1;
	}

	static void LogAvailable()
	{
		array<ref RAPatch_BenchRow> rows = Rules().Recipes;
		int available = 0;
		string missing = "";
		for (int i = 0; i < rows.Count(); i++)
		{
			if (IsAvailable(rows[i]))
				available++;
			else if (rows[i].Enabled)
				missing += " " + rows[i].Name + ",";
		}
		RAPatch_Settings.Log("workbench: " + available + " of " + rows.Count() + " recipes available. Off, or items not on this server:" + missing);
	}

	static bool IsFullStack(EntityAI thing)
	{
		Magazine magazine = Magazine.Cast(thing);
		if (magazine)
			return magazine.GetAmmoCount() >= magazine.GetAmmoMax();
		ItemBase item = ItemBase.Cast(thing);
		if (item && item.HasQuantity())
			return item.GetQuantity() >= item.GetQuantityMax();
		return true;
	}

	static float AmountIn(EntityAI thing)
	{
		Magazine magazine = Magazine.Cast(thing);
		if (magazine)
			return magazine.GetAmmoCount();
		ItemBase item = ItemBase.Cast(thing);
		if (item && item.HasQuantity())
			return item.GetQuantity();
		return 1;
	}

	protected static bool Matches(EntityAI thing, RAPatch_BenchItem wanted)
	{
		if (!thing || thing.IsRuined() || !ItemBase.Cast(thing))
			return false;
		for (int i = 0; i < wanted.Items.Count(); i++)
		{
			if (thing.IsKindOf(wanted.Items[i]))
				return true;
		}
		return false;
	}

	static float CountOf(array<EntityAI> things, RAPatch_BenchItem wanted)
	{
		float total = 0;
		for (int i = 0; i < things.Count(); i++)
		{
			if (Matches(things[i], wanted))
				total += AmountIn(things[i]);
		}
		return total;
	}

	static bool CanPay(array<EntityAI> things, RAPatch_BenchRow row)
	{
		for (int i = 0; i < row.Ingredients.Count(); i++)
		{
			if (CountOf(things, row.Ingredients[i]) < row.Ingredients[i].Amount)
				return false;
		}
		return true;
	}

	// In the list's order, so what's first in the list goes first
	static void Pay(array<EntityAI> things, RAPatch_BenchRow row)
	{
		for (int r = 0; r < row.Ingredients.Count(); r++)
		{
			RAPatch_BenchItem wanted = row.Ingredients[r];
			if (!wanted.Consumed)
				continue;
			float left = wanted.Amount;
			for (int i = 0; i < things.Count() && left > 0; i++)
			{
				EntityAI thing = things[i];
				if (!Matches(thing, wanted))
					continue;
				float amount = AmountIn(thing);
				if (amount <= left)
				{
					left -= amount;
					thing.Delete();
					things[i] = null;
					continue;
				}
				Magazine magazine = Magazine.Cast(thing);
				if (magazine)
					magazine.ServerSetAmmoCount(amount - left);
				else
					ItemBase.Cast(thing).SetQuantity(amount - left);
				left = 0;
			}
		}
	}

	// In the player's inventory if it fits, otherwise on the ground beside the desk. percent: of a full stack
	static void Give(string type, float percent, PlayerBase player, vector besideDesk)
	{
		EntityAI thing;
		if (player)
			thing = player.GetInventory().CreateInInventory(type);
		if (!thing)
			thing = EntityAI.Cast(g_Game.CreateObjectEx(type, besideDesk, ECE_PLACE_ON_SURFACE));
		if (thing)
			SetPercent(thing, percent);
	}

	protected static void SetPercent(EntityAI thing, float percent)
	{
		Magazine magazine = Magazine.Cast(thing);
		if (magazine)
		{
			magazine.ServerSetAmmoCount(Math.Max(Math.Round(magazine.GetAmmoMax() * percent / 100), 1));
			return;
		}
		ItemBase item = ItemBase.Cast(thing);
		if (item && item.HasQuantity())
			item.SetQuantity(Math.Max(Math.Round(item.GetQuantityMax() * percent / 100), 1));
	}

	// An exact amount back, in as many stacks as it takes
	static void Refund(string type, float amount, PlayerBase player, vector besideDesk)
	{
		float left = amount;
		for (int piece = 0; left > 0 && piece < 40; piece++)
		{
			EntityAI thing;
			if (player)
				thing = player.GetInventory().CreateInInventory(type);
			if (!thing)
				thing = EntityAI.Cast(g_Game.CreateObjectEx(type, besideDesk, ECE_PLACE_ON_SURFACE));
			if (!thing)
				return;
			float stack = 1;
			Magazine magazine = Magazine.Cast(thing);
			ItemBase item = ItemBase.Cast(thing);
			if (magazine)
			{
				stack = Math.Min(left, magazine.GetAmmoMax());
				magazine.ServerSetAmmoCount(stack);
			}
			else if (item && item.HasQuantity())
			{
				stack = Math.Min(left, Math.Max(item.GetQuantityMax(), 1));
				item.SetQuantity(stack);
			}
			left -= stack;
		}
	}

	static void AddPlayerItems(PlayerBase player, array<EntityAI> things)
	{
		if (player)
			player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, things);
	}
}

// What the menu shows. Goes to the player as JSON, in one message, so it has to stay short: at most RESULTS_SHOWN results
class RAPatch_BenchStatus
{
	ref array<int> Q = new array<int>;		// the queue, by place in Crafting.Bench.Recipes
	int L;									// seconds left on the first
	ref array<string> R = new array<string>;// finished items
	int More;								// finished items beyond those listed
	string M;
}

modded class TerritoryHQ
{
	protected static const int RESULTS_SHOWN = 8;

	protected ref array<string> m_RAPatch_BenchQueue = new array<string>;
	protected float m_RAPatch_BenchLeft;
	protected ref array<string> m_RAPatch_BenchResults = new array<string>;
	protected ref array<float> m_RAPatch_BenchResultPercents = new array<float>;
	protected bool m_RAPatch_BenchTicking;

	void ~TerritoryHQ()
	{
		if (g_Game && m_RAPatch_BenchTicking)
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(RAPatch_BenchTick);
	}

	bool RAPatch_HasBench()
	{
		array<ref RAPatch_ModuleRow> rows = RAPatch_Modules.Rows();
		for (int i = 0; i < rows.Count() && i < RAPatch_Settings.MAX_MODULES; i++)
		{
			if (rows[i].Workbench && RAPatch_HasModule(rows[i].Name))
				return true;
		}
		return false;
	}

	int RAPatch_BenchQueueCount()
	{
		return m_RAPatch_BenchQueue.Count();
	}

	int RAPatch_BenchResultCount()
	{
		return m_RAPatch_BenchResults.Count();
	}

	protected bool RAPatch_BenchBusy()
	{
		return m_RAPatch_BenchQueue.Count() > 0 || m_RAPatch_BenchResults.Count() > 0;
	}

	// The desk's storage first, then the player. Only what's directly in storage: the nails slot is for upkeep, and a box in storage stays shut.
	void RAPatch_BenchItems(PlayerBase player, array<EntityAI> things)
	{
		if (RAPatch_StorageOpen() && GetInventory() && GetInventory().GetCargo())
		{
			CargoBase cargo = GetInventory().GetCargo();
			for (int i = 0; i < cargo.GetItemCount(); i++)
				things.Insert(cargo.GetItem(i));
		}
		RAPatch_Bench.AddPlayerItems(player, things);
	}

	protected vector RAPatch_BenchSpot()
	{
		return GetPosition() + "1 0 0";
	}

	// "" when it started
	string RAPatch_BenchStart(PlayerBase player, string name)
	{
		array<EntityAI> things = new array<EntityAI>;
		RAPatch_BenchItems(player, things);
		return RAPatch_BenchStartWith(things, name);
	}

	string RAPatch_BenchStartWith(array<EntityAI> things, string name)
	{
		if (!g_Game.IsServer() || !RAPatch_HasBench())
			return "This desk has no workbench.";
		RAPatch_BenchSettings rules = RAPatch_Bench.Rules();
		RAPatch_BenchRow row = RAPatch_Bench.RowByName(name);
		if (!row || !RAPatch_Bench.IsAvailable(row))
			return "That recipe isn't available.";
		if (m_RAPatch_BenchQueue.Count() >= rules.QueueSize)
			return "The queue is full.";
		if (m_RAPatch_BenchResults.Count() >= rules.MaxWaiting)
			return "Collect what's finished first.";
		if (!RAPatch_Bench.CanPay(things, row))
			return "You don't have what this needs.";
		RAPatch_Bench.Pay(things, row);
		if (m_RAPatch_BenchQueue.Count() == 0)
			m_RAPatch_BenchLeft = row.Seconds;
		m_RAPatch_BenchQueue.Insert(name);
		RAPatch_BenchStartTicking();
		return "";
	}

	protected void RAPatch_BenchStartTicking()
	{
		if (m_RAPatch_BenchTicking || !g_Game.IsServer())
			return;
		m_RAPatch_BenchTicking = true;
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(RAPatch_BenchTick, 1000, true);
	}

	protected void RAPatch_BenchTick()
	{
		RAPatch_BenchAdvance(1);
		if (m_RAPatch_BenchQueue.Count() == 0)
		{
			g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(RAPatch_BenchTick);
			m_RAPatch_BenchTicking = false;
		}
	}

	void RAPatch_BenchAdvance(float seconds)
	{
		float left = seconds;
		while (left > 0 && m_RAPatch_BenchQueue.Count() > 0)
		{
			if (m_RAPatch_BenchLeft > left)
			{
				m_RAPatch_BenchLeft -= left;
				return;
			}
			left -= m_RAPatch_BenchLeft;
			RAPatch_BenchFinishFirst();
		}
	}

	protected void RAPatch_BenchFinishFirst()
	{
		RAPatch_BenchRow row = RAPatch_Bench.RowByName(m_RAPatch_BenchQueue[0]);
		m_RAPatch_BenchQueue.RemoveOrdered(0);
		if (row && row.Results.Count() > 0)
		{
			for (int i = 0; i < row.Count; i++)
			{
				m_RAPatch_BenchResults.Insert(row.Results[Math.RandomInt(0, row.Results.Count())]);
				m_RAPatch_BenchResultPercents.Insert(row.QuantityPercent);
			}
		}
		m_RAPatch_BenchLeft = 0;
		if (m_RAPatch_BenchQueue.Count() > 0)
		{
			RAPatch_BenchRow next = RAPatch_Bench.RowByName(m_RAPatch_BenchQueue[0]);
			if (next)
				m_RAPatch_BenchLeft = next.Seconds;
		}
	}

	// The last job in the queue comes out, and everything it took goes back
	bool RAPatch_BenchCancelLast(PlayerBase player)
	{
		int last = m_RAPatch_BenchQueue.Count() - 1;
		if (last < 0)
			return false;
		RAPatch_BenchRefund(m_RAPatch_BenchQueue[last], player);
		m_RAPatch_BenchQueue.RemoveOrdered(last);
		if (last == 0)
			m_RAPatch_BenchLeft = 0;
		return true;
	}

	protected void RAPatch_BenchRefund(string name, PlayerBase player)
	{
		RAPatch_BenchRow row = RAPatch_Bench.RowByName(name);
		if (!row)
			return;
		for (int i = 0; i < row.Ingredients.Count(); i++)
		{
			RAPatch_BenchItem wanted = row.Ingredients[i];
			if (wanted.Consumed && wanted.Items.Count() > 0)
				RAPatch_Bench.Refund(wanted.Items[0], wanted.Amount, player, RAPatch_BenchSpot());
		}
	}

	int RAPatch_BenchCollect(PlayerBase player)
	{
		int count = m_RAPatch_BenchResults.Count();
		for (int i = 0; i < count; i++)
			RAPatch_Bench.Give(m_RAPatch_BenchResults[i], m_RAPatch_BenchResultPercents[i], player, RAPatch_BenchSpot());
		m_RAPatch_BenchResults.Clear();
		m_RAPatch_BenchResultPercents.Clear();
		return count;
	}

	RAPatch_BenchStatus RAPatch_BenchBuildStatus()
	{
		RAPatch_BenchStatus status = new RAPatch_BenchStatus;
		for (int q = 0; q < m_RAPatch_BenchQueue.Count(); q++)
			status.Q.Insert(RAPatch_Bench.IndexOfName(m_RAPatch_BenchQueue[q]));
		status.L = Math.Ceil(m_RAPatch_BenchLeft);
		for (int r = 0; r < m_RAPatch_BenchResults.Count(); r++)
		{
			if (r < RESULTS_SHOWN)
				status.R.Insert(m_RAPatch_BenchResults[r]);
			else
				status.More++;
		}
		return status;
	}

	// Packed up or destroyed: finished items and paid-for materials land beside it
	override void RAPatch_DropModules()
	{
		if (g_Game.IsServer())
		{
			RAPatch_BenchCollect(null);
			while (RAPatch_BenchCancelLast(null))
			{
			}
		}
		super.RAPatch_DropModules();
	}

	// The bench can't come out with things in it
	override bool RAPatch_RemoveNamed(string name, PlayerBase player)
	{
		RAPatch_ModuleRow row = RAPatch_Modules.RowByName(name);
		if (row && row.Workbench && RAPatch_BenchBusy())
			return false;
		return super.RAPatch_RemoveNamed(name, player);
	}

	override bool RAPatch_RemoveLast(PlayerBase player)
	{
		RAPatch_ModuleRow row = RAPatch_Modules.RowByName(RAPatch_LastModuleName());
		if (row && row.Workbench && RAPatch_BenchBusy())
			return false;
		return super.RAPatch_RemoveLast(player);
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(RAPatch_ActionOpenBench);
	}

	override void CF_OnStoreSave(CF_ModStorageMap storage)
	{
		super.CF_OnStoreSave(storage);
		CF_ModStorage ctx = storage.Get(RAPatch_Net.MOD);
		if (!ctx)
			return;
		ctx.Write(m_RAPatch_BenchLeft);
		ctx.Write(m_RAPatch_BenchQueue.Count());
		for (int q = 0; q < m_RAPatch_BenchQueue.Count(); q++)
			ctx.Write(m_RAPatch_BenchQueue[q]);
		ctx.Write(m_RAPatch_BenchResults.Count());
		for (int r = 0; r < m_RAPatch_BenchResults.Count(); r++)
		{
			ctx.Write(m_RAPatch_BenchResults[r]);
			ctx.Write(m_RAPatch_BenchResultPercents[r]);
		}
	}

	override bool CF_OnStoreLoad(CF_ModStorageMap storage)
	{
		if (!super.CF_OnStoreLoad(storage))
			return false;
		CF_ModStorage ctx = storage.Get(RAPatch_Net.MOD);
		if (!ctx)
			return true;
		// A desk saved before the workbench existed simply has none of this
		if (!ctx.Read(m_RAPatch_BenchLeft))
			return true;
		int queued;
		if (!ctx.Read(queued))
			return true;
		for (int q = 0; q < queued; q++)
		{
			string name;
			if (!ctx.Read(name))
				return true;
			m_RAPatch_BenchQueue.Insert(name);
		}
		int results;
		if (!ctx.Read(results))
			return true;
		for (int r = 0; r < results; r++)
		{
			string type;
			float percent;
			if (!ctx.Read(type) || !ctx.Read(percent))
				return true;
			m_RAPatch_BenchResults.Insert(type);
			m_RAPatch_BenchResultPercents.Insert(percent);
		}
		return true;
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		if (m_RAPatch_BenchQueue.Count() > 0)
			RAPatch_BenchStartTicking();
	}
}

// Workbench messages. The server checks every request: authorised, standing at the desk, and the module fitted.
class RAPatch_BenchNet
{
	static const string STATUS_REQUEST = "RPC_BenchStatusRequest";
	static const string STATUS = "RPC_BenchStatus";
	static const string START_JOB = "RPC_BenchStart";
	static const string CANCEL = "RPC_BenchCancel";
	static const string COLLECT = "RPC_BenchCollect";
	protected static const float MAX_DISTANCE = 5;

	protected static ref RAPatch_BenchNet s_Instance;

	// Client: what the menu reads
	static TerritoryHQ OpenRequest;
	static ref RAPatch_BenchStatus Latest;
	static int LatestStamp;

	static void Start()
	{
		if (!s_Instance)
			s_Instance = new RAPatch_BenchNet;
	}

	void RAPatch_BenchNet()
	{
		GetRPCManager().AddRPC(RAPatch_Net.MOD, STATUS_REQUEST, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, START_JOB, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, CANCEL, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, COLLECT, this, SingleplayerExecutionType.Server);
		GetRPCManager().AddRPC(RAPatch_Net.MOD, STATUS, this, SingleplayerExecutionType.Client);
	}

	protected TerritoryHQ AllowedDesk(PlayerIdentity sender, Object target)
	{
		TerritoryHQ desk = TerritoryHQ.Cast(target);
		if (!desk || !sender)
			return null;
		PlayerBase player = PlayerBase.Cast(sender.GetPlayer());
		if (!player || vector.Distance(player.GetPosition(), desk.GetPosition()) > MAX_DISTANCE || !desk.IsAuthorized(player) || !desk.RAPatch_HasBench())
			return null;
		return desk;
	}

	protected void Reply(PlayerIdentity to, TerritoryHQ desk, string message)
	{
		RAPatch_BenchStatus status = desk.RAPatch_BenchBuildStatus();
		status.M = message;
		string text;
		JsonSerializer serializer = new JsonSerializer;
		serializer.WriteToString(status, false, text);
		GetRPCManager().SendRPC(RAPatch_Net.MOD, STATUS, new Param1<string>(text), true, to, desk);
	}

	void RPC_BenchStatusRequest(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		TerritoryHQ desk = AllowedDesk(sender, target);
		if (desk)
			Reply(sender, desk, "");
	}

	void RPC_BenchStart(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		Param1<string> data;
		if (!ctx.Read(data))
			return;
		TerritoryHQ desk = AllowedDesk(sender, target);
		if (!desk)
			return;
		string refusal = desk.RAPatch_BenchStart(PlayerBase.Cast(sender.GetPlayer()), data.param1);
		if (refusal == "")
			refusal = "Started: " + data.param1 + ".";
		Reply(sender, desk, refusal);
	}

	void RPC_BenchCancel(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		TerritoryHQ desk = AllowedDesk(sender, target);
		if (!desk)
			return;
		string message = "Nothing to cancel.";
		if (desk.RAPatch_BenchCancelLast(PlayerBase.Cast(sender.GetPlayer())))
			message = "Cancelled, and the materials came back.";
		Reply(sender, desk, message);
	}

	void RPC_BenchCollect(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;
		TerritoryHQ desk = AllowedDesk(sender, target);
		if (!desk)
			return;
		int count = desk.RAPatch_BenchCollect(PlayerBase.Cast(sender.GetPlayer()));
		string message = "Nothing to collect yet.";
		if (count > 0)
			message = "Collected. Anything that didn't fit is on the ground beside the desk.";
		Reply(sender, desk, message);
	}

	void RPC_BenchStatus(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;
		Param1<string> data;
		if (!ctx.Read(data))
			return;
		RAPatch_BenchStatus status = new RAPatch_BenchStatus;
		string error;
		JsonSerializer serializer = new JsonSerializer;
		if (!serializer.ReadFromString(status, data.param1, error))
			return;
		Latest = status;
		LatestStamp++;
	}
}

class RAPatch_ActionOpenBench : ActionInteractBase
{
	void RAPatch_ActionOpenBench()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_Text = "Open Workbench";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		TerritoryHQ hq = TerritoryHQ.Cast(target.GetObject());
		return hq && !hq.IsRuined() && hq.RAPatch_HasBench() && hq.IsAuthorized(player);
	}

	override void OnExecuteClient(ActionData action_data)
	{
		super.OnExecuteClient(action_data);
		TerritoryHQ hq = TerritoryHQ.Cast(action_data.m_Target.GetObject());
		if (hq)
			RAPatch_BenchNet.OpenRequest = hq;
	}
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(RAPatch_ActionOpenBench);
	}
}
