// Crafting from settings. A client registers recipes as the game loads, before it can hear from the server, and
// both sides must have the same list in the same order. So there's a fixed set of 12 recipes. When the settings
// arrive, the client rewrites them and rebuilds the game's recipe cache.
modded class RecipeBase
{
	void RAPatch_ApplySettings()
	{
	}

	protected void RAPatch_ApplyRow(RAPatch_RecipeRow row)
	{
		for (int i = 0; i < MAX_NUMBER_OF_INGREDIENTS; i++)
		{
			m_Ingredients[i].Clear();
			m_SoundCategories[i].Clear();
			m_MinDamageIngredient[i] = -1;
			m_MaxDamageIngredient[i] = 3;
			m_MaxQuantityIngredient[i] = -1;
			m_IngredientAddHealth[i] = 0;
			m_IngredientSetHealth[i] = -1;
			m_IngredientUseSoftSkills[i] = false;
		}
		m_NumberOfResults = 0;
		if (row.Name != "")
			m_Name = row.Name;
		m_IsInstaRecipe = false;
		m_AnimationLength = row.Seconds / CRAFTING_TIME_UNIT_SIZE;
		m_Specialty = 0;

		foreach (string first : row.Ingredient1)
			InsertIngredient(0, first);
		RAPatch_SetIngredient(0, row.Ingredient1Amount, row.Ingredient1Consumed);
		foreach (string second : row.Ingredient2)
			InsertIngredient(1, second);
		RAPatch_SetIngredient(1, row.Ingredient2Amount, row.Ingredient2Consumed);

		if (row.Result == "")
			return;
		AddResult(row.Result);
		m_ResultSetFullQuantity[0] = false;
		m_ResultSetQuantity[0] = row.ResultQuantity;
		m_ResultSetHealth[0] = -1;
		m_ResultInheritsHealth[0] = -1;
		m_ResultInheritsColor[0] = -1;
		if (row.ResultOnGround)
			m_ResultToInventory[0] = -2;
		else
			m_ResultToInventory[0] = -1;
		m_ResultUseSoftSkills[0] = false;
		m_ResultReplacesIngredient[0] = -1;
	}

	protected void RAPatch_SetIngredient(int index, float amount, bool consumed)
	{
		m_MinQuantityIngredient[index] = -1;
		m_IngredientAddQuantity[index] = 0;
		m_IngredientDestroy[index] = false;
		if (amount > 0)
		{
			m_MinQuantityIngredient[index] = amount;
			if (consumed)
				m_IngredientAddQuantity[index] = -amount;
		}
		else if (consumed)
		{
			m_IngredientDestroy[index] = true;
		}
	}
}

class RAPatch_Recipe : RecipeBase
{
	int RAPatch_RowIndex()
	{
		return -1;
	}

	protected RAPatch_RecipeRow RAPatch_Row()
	{
		array<ref RAPatch_RecipeRow> rows = RAPatch_Settings.Get().Crafting.Recipes;
		int index = RAPatch_RowIndex();
		if (index < 0 || index >= rows.Count())
			return null;
		return rows[index];
	}

	override void Init()
	{
		m_Name = "Craft";
		RAPatch_ApplySettings();
	}

	override void RAPatch_ApplySettings()
	{
		RAPatch_RecipeRow row = RAPatch_Row();
		if (row)
			RAPatch_ApplyRow(row);
		else
			RAPatch_ApplyRow(new RAPatch_RecipeRow());
	}

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		RAPatch_RecipeRow row = RAPatch_Row();
		if (!row || !row.Enabled || row.Result == "" || !RAPatch_Modules.ClassExists(row.Result))
			return false;
		if (ingredients[0].GetInventory().IsAttachment() || ingredients[1].GetInventory().IsAttachment())
			return false;
		if (row.Ingredient1MustBeFull && !RAPatch_Bench.IsFullStack(ingredients[0]))
			return false;
		if (row.Ingredient2MustBeFull && !RAPatch_Bench.IsFullStack(ingredients[1]))
			return false;
		if (row.NeedsModule != "" && !RAPatch_Modules.IsNearDeskWith(player, row.NeedsModule))
			return false;
		return true;
	}

	override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
	{
		RAPatch_Settings.Log("crafted: " + m_Name);
	}
}

class RAPatch_Recipe0 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 0;
	}
}

class RAPatch_Recipe1 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 1;
	}
}

class RAPatch_Recipe2 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 2;
	}
}

class RAPatch_Recipe3 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 3;
	}
}

class RAPatch_Recipe4 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 4;
	}
}

class RAPatch_Recipe5 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 5;
	}
}

class RAPatch_Recipe6 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 6;
	}
}

class RAPatch_Recipe7 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 7;
	}
}

class RAPatch_Recipe8 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 8;
	}
}

class RAPatch_Recipe9 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 9;
	}
}

class RAPatch_Recipe10 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 10;
	}
}

class RAPatch_Recipe11 : RAPatch_Recipe
{
	override int RAPatch_RowIndex()
	{
		return 11;
	}
}

modded class CraftTerritoryHQKit
{
	override void Init()
	{
		super.Init();
		RAPatch_ApplySettings();
	}

	override void RAPatch_ApplySettings()
	{
		RAPatch_RecipeRow row = RAPatch_Settings.Get().Crafting.DeskKit;
		if (!row)
			return;
		row.Result = "TerritoryHQ_Kit";
		RAPatch_ApplyRow(row);
	}

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		RAPatch_RecipeRow row = RAPatch_Settings.Get().Crafting.DeskKit;
		if (row && !row.Enabled)
			return false;
		return super.CanDo(ingredients, player);
	}
}

modded class PluginRecipesManagerBase
{
	override void RegisterRecipies()
	{
		super.RegisterRecipies();
		RegisterRecipe(new RAPatch_Recipe0);
		RegisterRecipe(new RAPatch_Recipe1);
		RegisterRecipe(new RAPatch_Recipe2);
		RegisterRecipe(new RAPatch_Recipe3);
		RegisterRecipe(new RAPatch_Recipe4);
		RegisterRecipe(new RAPatch_Recipe5);
		RegisterRecipe(new RAPatch_Recipe6);
		RegisterRecipe(new RAPatch_Recipe7);
		RegisterRecipe(new RAPatch_Recipe8);
		RegisterRecipe(new RAPatch_Recipe9);
		RegisterRecipe(new RAPatch_Recipe10);
		RegisterRecipe(new RAPatch_Recipe11);
	}
}

modded class PluginRecipesManager
{
	void PluginRecipesManager()
	{
		RAPatch_Settings.OnReplaced.Insert(RAPatch_Rebuild);
	}

	void ~PluginRecipesManager()
	{
		if (RAPatch_Settings.OnReplaced)
			RAPatch_Settings.OnReplaced.Remove(RAPatch_Rebuild);
	}

	void RAPatch_Rebuild()
	{
		foreach (RecipeBase recipe : m_RecipeList)
		{
			if (recipe)
				recipe.RAPatch_ApplySettings();
		}
		GenerateRecipeCache();
		RAPatch_Settings.Log("client: rebuilt " + m_RecipeList.Count() + " recipes");
		RAPatch_LogFirstRecipe();
	}

	protected void RAPatch_LogFirstRecipe()
	{
		RAPatch_Settings settings = RAPatch_Settings.Get();
		if (settings.Crafting.Recipes.Count() == 0)
			return;
		RAPatch_RecipeRow row = settings.Crafting.Recipes[0];
		int recipeId = RecipeIDFromClassname("RAPatch_Recipe0");
		if (recipeId == -1 || row.Ingredient1.Count() == 0)
			return;
		RecipeBase recipe = m_RecipeList[recipeId];
		string firstIngredient = row.Ingredient1[0];
		CacheObject cached = m_RecipeCache.Get(firstIngredient);
		bool listed = false;
		if (cached)
			listed = cached.IsContainRecipe(recipeId);
		float amount = recipe.m_MinQuantityIngredient[1];
		RAPatch_Settings.Log("client: desk radius " + settings.Desk.Radius + ", recipe 0 needs " + amount + " of ingredient 2, listed for " + firstIngredient + ": " + listed);
	}
}
