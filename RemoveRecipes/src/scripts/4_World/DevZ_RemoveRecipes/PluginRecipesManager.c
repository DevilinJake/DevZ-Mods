// Recipes listed in CfgDevZ_RemoveRecipes are taken out of the right-click crafting list.
// Runs after every mod has registered its recipes, so load order doesn't matter.
modded class PluginRecipesManager
{
	override protected void CreateAllRecipes()
	{
		super.CreateAllRecipes();
		DevZ_RemoveListedRecipes();
	}

	protected void DevZ_RemoveListedRecipes()
	{
		TStringArray wanted = new TStringArray();
		g_Game.ConfigGetTextArray("CfgDevZ_RemoveRecipes recipes", wanted);

		// Class names are matched without caring about upper/lower case
		map<string, string> registered = new map<string, string>();
		foreach (string name, int id : m_RecipeNamesList)
		{
			string lower = name;
			lower.ToLower();
			registered.Insert(lower, name);
		}

		foreach (string recipe : wanted)
		{
			string key = recipe;
			key.ToLower();
			string found;
			if (registered.Find(key, found))
			{
				UnregisterRecipe(found);
				Print("[DevZ_RemoveRecipes] Removed recipe " + found);
			}
			else
			{
				Print("[DevZ_RemoveRecipes] Recipe not found (mod not loaded or name misspelt): " + recipe);
			}
		}
	}
}
