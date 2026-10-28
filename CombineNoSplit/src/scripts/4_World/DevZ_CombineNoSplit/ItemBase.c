// Items listed in CfgDevZ_CombineNoSplit can be combined but never split.
// The game sets both from one config value (canBeSplit), so this sets them separately.
modded class ItemBase
{
	protected static ref set<string> s_DevZ_CombineNoSplit;

	override void InitItemVariables()
	{
		super.InitItemVariables();

		if (DevZ_IsCombineNoSplit(GetType()))
		{
			can_this_be_combined = true;
			m_CanThisBeSplit = false;
		}
	}

	static bool DevZ_IsCombineNoSplit(string type)
	{
		if (!s_DevZ_CombineNoSplit)
		{
			s_DevZ_CombineNoSplit = new set<string>();
			TStringArray items = new TStringArray();
			g_Game.ConfigGetTextArray("CfgDevZ_CombineNoSplit items", items);
			foreach (string item : items)
			{
				item.ToLower();
				s_DevZ_CombineNoSplit.Insert(item);
			}
		}

		type.ToLower();
		return s_DevZ_CombineNoSplit.Find(type) != -1;
	}
}
