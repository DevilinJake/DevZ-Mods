// The workbench's menu: recipes on the left, what the chosen one needs on the right, the queue and finished items below.
// Only recipes whose items are on this server are listed.
class RAPatch_WorkbenchMenu : UIScriptedMenu
{
	protected static const int ROWS = 8;

	protected TerritoryHQ m_Desk;
	protected int m_SeenStamp = -1;
	protected float m_RefreshIn;
	protected float m_ShowPartsIn;
	protected int m_First;	// the recipe at the top of the list
	protected Widget m_RecipeArea;
	protected string m_Chosen;
	protected string m_Message;
	protected float m_MessageFor;
	protected ref RAPatch_BenchStatus m_Status;
	protected float m_SinceStatus;

	protected ref array<ButtonWidget> m_Rows = new array<ButtonWidget>;
	protected ref array<TextWidget> m_RowLabels = new array<TextWidget>;
	protected ref array<Widget> m_RowBacks = new array<Widget>;
	protected ref array<string> m_RowNames = new array<string>;
	protected ButtonWidget m_Prev;
	protected ButtonWidget m_Next;
	protected TextWidget m_ChosenName;
	protected TextWidget m_ChosenTime;
	protected MultilineTextWidget m_Parts;
	protected ButtonWidget m_Craft;
	protected MultilineTextWidget m_MessageText;
	protected MultilineTextWidget m_QueueText;
	protected MultilineTextWidget m_ReadyText;
	protected ButtonWidget m_Cancel;
	protected ButtonWidget m_Collect;
	protected ButtonWidget m_Close;

	void SetDesk(TerritoryHQ desk)
	{
		m_Desk = desk;
		ShowRecipes();
		RequestStatus();
	}

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("DevZ_RAAddon/gui/workbench.layout");
		for (int i = 0; i < ROWS; i++)
		{
			m_Rows.Insert(ButtonWidget.Cast(layoutRoot.FindAnyWidget("Recipe" + i)));
			m_RowLabels.Insert(TextWidget.Cast(layoutRoot.FindAnyWidget("Recipe" + i + "Label")));
			m_RowBacks.Insert(layoutRoot.FindAnyWidget("Recipe" + i + "Back"));
			m_RowNames.Insert("");
		}
		m_RecipeArea = layoutRoot.FindAnyWidget("RecipeArea");
		m_Prev = ButtonWidget.Cast(layoutRoot.FindAnyWidget("RecipesPrev"));
		m_Next = ButtonWidget.Cast(layoutRoot.FindAnyWidget("RecipesNext"));
		m_ChosenName = TextWidget.Cast(layoutRoot.FindAnyWidget("ChosenName"));
		m_ChosenTime = TextWidget.Cast(layoutRoot.FindAnyWidget("ChosenTime"));
		m_Parts = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("PartsText"));
		m_Craft = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CraftButton"));
		m_MessageText = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("MessageText"));
		m_QueueText = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("QueueText"));
		m_ReadyText = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("ReadyText"));
		m_Cancel = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CancelButton"));
		m_Collect = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CollectButton"));
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

	// The wheel scrolls the recipe list while the mouse is over it
	override bool OnMouseWheel(Widget w, int x, int y, int wheel)
	{
		super.OnMouseWheel(w, x, y, wheel);
		if (!m_RecipeArea || wheel == 0)
			return false;
		float left, top, width, height;
		m_RecipeArea.GetScreenPos(left, top);
		m_RecipeArea.GetScreenSize(width, height);
		if (x < left || x > left + width || y < top || y > top + height)
			return false;
		if (wheel > 0)
			m_First--;
		else
			m_First++;
		ShowRecipes();
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
			GetRPCManager().SendRPC(RAPatch_Net.MOD, RAPatch_BenchNet.STATUS_REQUEST, null, true, null, m_Desk);
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);
		if (!m_Desk || RAPatch_MenuKeys.EscPressed())
		{
			Close();
			return;
		}
		m_RefreshIn -= timeslice;
		if (m_RefreshIn <= 0)
		{
			m_RefreshIn = 2;
			RequestStatus();
		}
		m_ShowPartsIn -= timeslice;
		if (m_ShowPartsIn <= 0)
		{
			m_ShowPartsIn = 1;
			ShowChosen();
			ShowProgress();
		}
		m_SinceStatus += timeslice;
		if (m_MessageFor > 0)
		{
			m_MessageFor -= timeslice;
			if (m_MessageFor <= 0)
				m_MessageText.SetText("");
		}
		if (m_SeenStamp != RAPatch_BenchNet.LatestStamp && RAPatch_BenchNet.Latest)
		{
			m_SeenStamp = RAPatch_BenchNet.LatestStamp;
			m_Status = RAPatch_BenchNet.Latest;
			m_SinceStatus = 0;
			if (m_Status.M != "")
			{
				m_Message = m_Status.M;
				m_MessageFor = 10;
				m_MessageText.SetText(m_Message);
			}
			ShowProgress();
			ShowChosen();
		}
	}

	protected void AvailableRows(array<RAPatch_BenchRow> rows)
	{
		array<ref RAPatch_BenchRow> all = RAPatch_Bench.Rules().Recipes;
		for (int i = 0; i < all.Count(); i++)
		{
			if (RAPatch_Bench.IsAvailable(all[i]))
				rows.Insert(all[i]);
		}
	}

	protected void ShowRecipes()
	{
		array<RAPatch_BenchRow> rows = new array<RAPatch_BenchRow>;
		AvailableRows(rows);
		int lastFirst = Math.Max(rows.Count() - ROWS, 0);
		m_First = Math.Clamp(m_First, 0, lastFirst);
		m_Prev.Show(lastFirst > 0);
		m_Next.Show(lastFirst > 0);
		if (m_Chosen == "" && rows.Count() > 0)
			m_Chosen = rows[0].Name;
		for (int i = 0; i < ROWS; i++)
		{
			int index = m_First + i;
			if (index >= rows.Count())
			{
				m_Rows[i].Show(false);
				m_RowNames[i] = "";
				continue;
			}
			string name = rows[index].Name;
			m_RowNames[i] = name;
			m_RowLabels[i].SetText(name);
			if (name == m_Chosen)
				m_RowBacks[i].SetColor(ARGB(255, 166, 107, 30));
			else
				m_RowBacks[i].SetColor(ARGB(255, 46, 54, 66));
			m_Rows[i].Show(true);
		}
		ShowChosen();
	}

	protected string ItemName(string type)
	{
		string name;
		if (GetGame().ConfigGetText("CfgVehicles " + type + " displayName", name) && name != "")
			return name;
		if (GetGame().ConfigGetText("CfgMagazines " + type + " displayName", name) && name != "")
			return name;
		return type;
	}

	protected string TimeText(int seconds)
	{
		if (seconds >= 60)
			return (seconds / 60).ToString() + " min " + (seconds % 60).ToString() + " s";
		return seconds.ToString() + " s";
	}

	// Refreshed each second, as the player's inventory and the desk's storage change
	protected void ShowChosen()
	{
		RAPatch_BenchRow row = RAPatch_Bench.RowByName(m_Chosen);
		if (!row || !m_Desk)
		{
			m_ChosenName.SetText("This server has no workbench recipes.");
			m_ChosenTime.SetText("");
			m_Parts.SetText("");
			m_Craft.Show(false);
			return;
		}
		m_Craft.Show(true);
		m_ChosenName.SetText(row.Name);
		string makes = "";
		if (row.Results.Count() > 0)
			makes = "Makes " + row.Count.ToString() + " x " + ItemName(row.Results[0]);
		m_ChosenTime.SetText(TimeText(row.Seconds) + ".   " + makes);

		array<EntityAI> things = new array<EntityAI>;
		m_Desk.RAPatch_BenchItems(PlayerBase.Cast(GetGame().GetPlayer()), things);
		string parts = "";
		for (int i = 0; i < row.Ingredients.Count(); i++)
		{
			RAPatch_BenchItem wanted = row.Ingredients[i];
			if (wanted.Items.Count() == 0)
				continue;
			float have = RAPatch_Bench.CountOf(things, wanted);
			parts += ItemName(wanted.Items[0]) + ":  " + Math.Floor(have).ToString() + " of " + Math.Floor(wanted.Amount).ToString();
			if (have < wanted.Amount)
				parts += "   NEED MORE";
			if (!wanted.Consumed)
				parts += "   (kept)";
			parts += "\n";
		}
		m_Parts.SetText(parts);
	}

	protected void ShowProgress()
	{
		if (!m_Status)
			return;
		array<ref RAPatch_BenchRow> all = RAPatch_Bench.Rules().Recipes;
		string queue = "";
		for (int q = 0; q < m_Status.Q.Count(); q++)
		{
			int index = m_Status.Q[q];
			string name = "Unknown recipe";
			if (index >= 0 && index < all.Count())
				name = all[index].Name;
			queue += (q + 1).ToString() + ". " + name;
			if (q == 0)
				queue += ":  " + TimeText(Math.Max(m_Status.L - m_SinceStatus, 0));
			queue += "\n";
		}
		if (queue == "")
			queue = "Nothing being made.";
		m_QueueText.SetText(queue);

		string ready = "";
		for (int r = 0; r < m_Status.R.Count(); r++)
			ready += ItemName(m_Status.R[r]) + "\n";
		if (m_Status.More > 0)
			ready += "and " + m_Status.More.ToString() + " more\n";
		if (ready == "")
			ready = "Nothing to collect.";
		m_ReadyText.SetText(ready);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (w == m_Close)
		{
			Close();
			return true;
		}
		if (!m_Desk)
			return false;
		if (w == m_Prev || w == m_Next)
		{
			if (w == m_Next)
				m_First += ROWS;
			else
				m_First -= ROWS;
			ShowRecipes();
			return true;
		}
		for (int i = 0; i < ROWS; i++)
		{
			if (w == m_Rows[i] && m_RowNames[i] != "")
			{
				m_Chosen = m_RowNames[i];
				ShowRecipes();
				return true;
			}
		}
		if (w == m_Craft && m_Chosen != "")
		{
			GetRPCManager().SendRPC(RAPatch_Net.MOD, RAPatch_BenchNet.START_JOB, new Param1<string>(m_Chosen), true, null, m_Desk);
			return true;
		}
		if (w == m_Cancel)
		{
			GetRPCManager().SendRPC(RAPatch_Net.MOD, RAPatch_BenchNet.CANCEL, null, true, null, m_Desk);
			return true;
		}
		if (w == m_Collect)
		{
			GetRPCManager().SendRPC(RAPatch_Net.MOD, RAPatch_BenchNet.COLLECT, null, true, null, m_Desk);
			return true;
		}
		return false;
	}
}

modded class MissionGameplay
{
	protected ref RAPatch_WorkbenchMenu m_RAPatch_Workbench;

	override void OnInit()
	{
		super.OnInit();
		RAPatch_BenchNet.Start();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		TerritoryHQ desk = RAPatch_BenchNet.OpenRequest;
		if (!desk)
			return;
		RAPatch_BenchNet.OpenRequest = null;
		if (GetGame().GetUIManager().GetMenu())
			return;
		m_RAPatch_Workbench = new RAPatch_WorkbenchMenu;
		GetGame().GetUIManager().ShowScriptedMenu(m_RAPatch_Workbench, null);
		m_RAPatch_Workbench.SetDesk(desk);
	}
}
