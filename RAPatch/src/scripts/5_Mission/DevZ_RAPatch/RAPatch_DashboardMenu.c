// Esc closes any of our menus
class RAPatch_MenuKeys
{
	static bool EscPressed()
	{
		if (RAPatch_MenuKeys.EscPressed())
			return true;
		if (KeyState(KeyCode.KC_ESCAPE))
		{
			ClearKey(KeyCode.KC_ESCAPE);
			return true;
		}
		return false;
	}
}

// The HQ dashboard. It only lists modules this server has in its settings, and says what each one does here.
class RAPatch_DashboardMenu : UIScriptedMenu
{
	protected TerritoryHQ m_Desk;
	protected int m_SeenStamp = -1;
	protected float m_RefreshIn;

	protected TextWidget m_Title;
	protected MultilineTextWidget m_Info;
	protected static const int ROWS = 5;
	protected ref array<Widget> m_Rows = new array<Widget>;
	protected ref array<TextWidget> m_RowNames = new array<TextWidget>;
	protected ref array<MultilineTextWidget> m_RowTexts = new array<MultilineTextWidget>;
	protected ref array<ButtonWidget> m_RowRemove = new array<ButtonWidget>;
	protected ref array<string> m_RowModule = new array<string>;
	protected ButtonWidget m_Prev;
	protected ButtonWidget m_Next;
	protected int m_Page;
	protected string m_Message;
	protected float m_MessageFor;
	protected Widget m_AlarmPanel;
	protected MultilineTextWidget m_AlarmText;
	protected EditBoxWidget m_WebhookBox;
	protected ButtonWidget m_Save;
	protected ButtonWidget m_Remove;
	protected ButtonWidget m_Test;
	protected ButtonWidget m_Close;

	void SetDesk(TerritoryHQ desk)
	{
		m_Desk = desk;
		RequestStatus();
	}

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("DevZ_RAPatch/gui/dashboard.layout");
		m_Title = TextWidget.Cast(layoutRoot.FindAnyWidget("TitleText"));
		m_Info = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("InfoText"));
		for (int i = 0; i < ROWS; i++)
		{
			m_Rows.Insert(layoutRoot.FindAnyWidget("ModuleRow" + i));
			m_RowNames.Insert(TextWidget.Cast(layoutRoot.FindAnyWidget("ModuleName" + i)));
			m_RowTexts.Insert(MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("ModuleDesc" + i)));
			m_RowRemove.Insert(ButtonWidget.Cast(layoutRoot.FindAnyWidget("ModuleRemove" + i)));
			m_RowModule.Insert("");
		}
		m_Prev = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ModulesPrev"));
		m_Next = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ModulesNext"));
		m_AlarmPanel = layoutRoot.FindAnyWidget("AlarmPanel");
		m_AlarmText = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("AlarmText"));
		m_WebhookBox = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("WebhookBox"));
		m_Save = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SaveButton"));
		m_Remove = ButtonWidget.Cast(layoutRoot.FindAnyWidget("RemoveButton"));
		m_Test = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TestButton"));
		m_Close = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseButton"));
		return layoutRoot;
	}

	override bool UseMouse()
	{
		return true;
	}

	override bool UseKeyboard()
	{
		return true;
	}

	override void OnShow()
	{
		super.OnShow();
		GetGame().GetInput().ChangeGameFocus(1);
		GetGame().GetUIManager().ShowUICursor(true);
		GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
	}

	override void OnHide()
	{
		super.OnHide();
		GetGame().GetInput().ResetGameFocus();
		GetGame().GetUIManager().ShowUICursor(false);
		GetGame().GetMission().PlayerControlEnable(true);
	}

	protected void RequestStatus()
	{
		if (m_Desk)
			GetRPCManager().SendRPC(RAPatch_Net.MOD, RAPatch_DashboardNet.STATUS_REQUEST, null, true, null, m_Desk);
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);
		if (!m_Desk)
		{
			Close();
			return;
		}
		if (GetUApi().GetInputByID(UAUIBack).LocalPress())
		{
			Close();
			return;
		}
		m_RefreshIn -= timeslice;
		if (m_MessageFor > 0)
			m_MessageFor -= timeslice;
		if (m_RefreshIn <= 0)
		{
			m_RefreshIn = 5;
			RequestStatus();
		}
		if (m_SeenStamp != RAPatch_DashboardNet.LatestStamp && RAPatch_DashboardNet.Latest)
		{
			m_SeenStamp = RAPatch_DashboardNet.LatestStamp;
			Show(RAPatch_DashboardNet.Latest);
		}
	}

	protected string TimeLeft(int minutes)
	{
		if (minutes == -1)
			return "No decay on this server";
		if (minutes == -2)
			return "No upkeep due";
		if (minutes <= 0)
			return "DECAYING: add nails";
		int days = minutes / 1440;
		int hours = (minutes % 1440) / 60;
		return days.ToString() + " d " + hours.ToString() + " h";
	}

	protected string CountOf(int count, int limit)
	{
		if (limit > 0)
			return count.ToString() + " of " + limit.ToString();
		return count.ToString();
	}

	protected void Show(RAPatch_DeskStatus status)
	{
		string players = status.Authorized.ToString();
		if (status.MaxPlayers > 0 && status.MaxPlayers != int.MAX)
			players += " of " + status.MaxPlayers.ToString();
		string info = "Radius: " + Math.Round(status.Radius).ToString() + " m\n";
		info += "Authorised players: " + players + "\n";
		info += "Building parts: " + status.Parts.ToString() + "\n";
		info += "Doors: " + CountOf(m_Desk.RAPatch_DoorCount(), RAPatch_Settings.Get().Desk.MaxDoors) + "\n";
		info += "Gates: " + CountOf(m_Desk.RAPatch_GateCount(), RAPatch_Settings.Get().Desk.MaxGates) + "\n";
		info += "Nails: " + status.NailCount.ToString() + "\n";
		info += "Upkeep left: " + TimeLeft(status.MinutesLeft) + "\n";
		info += "Desk health: " + status.HealthPercent.ToString() + "%\n";
		if (status.Locked)
			info += "Code lock: locked";
		else
			info += "Code lock: NOT LOCKED";
		m_Info.SetText(info);

		bool alarmOnServer = false;
		array<ref RAPatch_ModuleRow> all = RAPatch_Modules.Rows();
		for (int a = 0; a < all.Count() && a < RAPatch_Settings.MAX_MODULES; a++)
		{
			if (all[a].RaidAlarm)
				alarmOnServer = true;
		}
		ShowModules();

		m_AlarmPanel.Show(alarmOnServer);
		if (alarmOnServer)
		{
			if (status.Message != "")
			{
				m_Message = status.Message;
				m_MessageFor = 15;
			}
			string alarm;
			if (!m_Desk.RAPatch_HasRaidAlarm())
				alarm = "Install the raid alarm to use this.";
			else if (status.WebhookSet)
				alarm = "Alerts are on. A webhook is saved for this base.";
			else
				alarm = "Paste your team's Discord webhook address here.";
			if (m_MessageFor > 0)
				alarm = m_Message;
			m_AlarmText.SetText(alarm);
		}
	}

	// One page of modules, each with its own Remove button
	protected void ShowModules()
	{
		array<ref RAPatch_ModuleRow> rows = RAPatch_Modules.Rows();
		int count = Math.Min(rows.Count(), RAPatch_Settings.MAX_MODULES);
		int pages = (count + ROWS - 1) / ROWS;
		if (m_Page >= pages)
			m_Page = Math.Max(pages - 1, 0);
		m_Prev.Show(pages > 1);
		m_Next.Show(pages > 1);
		bool removable = RAPatch_Settings.Get().Desk.ModulesRemovable;
		for (int i = 0; i < ROWS; i++)
		{
			int index = m_Page * ROWS + i;
			if (index >= count)
			{
				m_Rows[i].Show(false);
				m_RowModule[i] = "";
				continue;
			}
			RAPatch_ModuleRow row = rows[index];
			bool installed = m_Desk.RAPatch_HasModule(row.Name);
			string state = "not installed";
			if (installed)
				state = "INSTALLED";
			m_RowNames[i].SetText(row.Name + ": " + state);
			string about = Describe(row);
			about.TrimInPlace();
			m_RowTexts[i].SetText(about);
			m_RowRemove[i].Show(installed && removable);
			m_RowModule[i] = row.Name;
			m_Rows[i].Show(true);
		}
	}

	// What this module does on this server, from the settings
	protected string Describe(RAPatch_ModuleRow row)
	{
		string text = "";
		if (row.RadiusAdded > 0)
			text += "   +" + Math.Round(row.RadiusAdded).ToString() + " m radius.";
		if (row.PlayersAdded > 0)
			text += "   +" + row.PlayersAdded.ToString() + " players.";
		if (row.UpkeepMultiplier != 1)
			text += "   Upkeep x " + row.UpkeepMultiplier.ToString() + ".";
		if (row.Storage)
			text += "   Adds storage.";
		if (row.RaidAlarm)
			text += "   Discord alerts when your base is attacked.";
		if (row.Workbench)
			text += "   Opens the workbench.";
		foreach (RAPatch_RecipeRow recipe : RAPatch_Settings.Get().Crafting.Recipes)
		{
			if (recipe.Enabled && recipe.NeedsModule == row.Name)
				text += "   Crafts: " + recipe.Name + ".";
		}
		return text;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (w == m_Close)
		{
			Close();
			return true;
		}
		if (w == m_Prev || w == m_Next)
		{
			if (w == m_Next)
				m_Page++;
			else if (m_Page > 0)
				m_Page--;
			if (m_Desk)
				ShowModules();
			return true;
		}
		for (int i = 0; i < ROWS; i++)
		{
			if (w == m_RowRemove[i] && m_Desk && m_RowModule[i] != "")
			{
				GetRPCManager().SendRPC(RAPatch_Net.MOD, RAPatch_DashboardNet.REMOVE_MODULE, new Param1<string>(m_RowModule[i]), true, null, m_Desk);
				m_RefreshIn = 0.5;
				return true;
			}
		}
		if (w == m_Save)
		{
			SendWebhook(m_WebhookBox.GetText());
			m_WebhookBox.SetText("");
			return true;
		}
		if (w == m_Remove)
		{
			SendWebhook("");
			return true;
		}
		if (w == m_Test && m_Desk)
		{
			GetRPCManager().SendRPC(RAPatch_Net.MOD, RAPatch_DashboardNet.TEST_WEBHOOK, null, true, null, m_Desk);
			return true;
		}
		return false;
	}

	protected void SendWebhook(string url)
	{
		if (m_Desk)
			GetRPCManager().SendRPC(RAPatch_Net.MOD, RAPatch_DashboardNet.SET_WEBHOOK, new Param1<string>(url), true, null, m_Desk);
	}
}

modded class MissionGameplay
{
	protected ref RAPatch_DashboardMenu m_RAPatch_Dashboard;

	override void OnInit()
	{
		super.OnInit();
		RAPatch_DashboardNet.Start();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		TerritoryHQ desk = RAPatch_DashboardNet.OpenRequest;
		if (!desk)
			return;
		RAPatch_DashboardNet.OpenRequest = null;
		if (GetGame().GetUIManager().GetMenu())
			return;
		m_RAPatch_Dashboard = new RAPatch_DashboardMenu;
		GetGame().GetUIManager().ShowScriptedMenu(m_RAPatch_Dashboard, null);
		m_RAPatch_Dashboard.SetDesk(desk);
	}
}
