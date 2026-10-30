// Screwdriver on an empty rack: dismantles it and drops the kit on the ground
class DismountInvictuz_Barrel_Storage extends RecipeBase
{
	override void Init()
	{
		m_Name = "#dismantle";
		m_IsInstaRecipe = false;
		m_AnimationLength = 1;
		m_Specialty = 0.02;

		// conditions (-1 = no check)
		m_MinDamageIngredient[0] = -1;
		m_MaxDamageIngredient[0] = -1;
		m_MinQuantityIngredient[0] = 1;
		m_MaxQuantityIngredient[0] = -1;

		m_MinDamageIngredient[1] = -1;
		m_MaxDamageIngredient[1] = -1;
		m_MinQuantityIngredient[1] = 1;
		m_MaxQuantityIngredient[1] = -1;

		// ingredient 1: the rack, used up
		InsertIngredient(0, "Invictuz_Barrel_Storage");
		m_IngredientAddHealth[0] = 0;
		m_IngredientSetHealth[0] = -1;
		m_IngredientAddQuantity[0] = 0;
		m_IngredientDestroy[0] = true;
		m_IngredientUseSoftSkills[0] = false;

		// ingredient 2: the screwdriver, loses a little health
		InsertIngredient(1, "Screwdriver");
		m_IngredientAddHealth[1] = -10;
		m_IngredientSetHealth[1] = -1;
		m_IngredientAddQuantity[1] = 0;
		m_IngredientDestroy[1] = false;
		m_IngredientUseSoftSkills[1] = true;

		// result: the kit, spawned on the ground
		AddResult("BarrelStorage_Kit");
		m_ResultSetFullQuantity[0] = false;
		m_ResultSetQuantity[0] = 10;
		m_ResultSetHealth[0] = -1;
		m_ResultInheritsHealth[0] = -1;
		m_ResultInheritsColor[0] = -1;
		m_ResultToInventory[0] = -2;
		m_ResultUseSoftSkills[0] = false;
		m_ResultReplacesIngredient[0] = -1;
	}

	// Only an empty rack: no barrels on it and nothing in its cargo
	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		EntityAI storage = EntityAI.Cast(ingredients[0]);
		if (!storage || !storage.GetInventory())
			return false;

		if (storage.GetInventory().GetCargo() && storage.GetInventory().GetCargo().GetItemCount() > 0)
			return false;

		return storage.GetInventory().AttachmentCount() == 0;
	}
};
