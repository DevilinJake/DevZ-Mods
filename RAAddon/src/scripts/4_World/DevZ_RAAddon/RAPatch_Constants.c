// RA's fixed numbers that have no function around them, and only matter on the player's machine, where the hologram runs.
// Redeclaring a static const in a modded class does change what RA's own code reads (tested on the server with
// BaseBuilding.BUILD_HEIGHT_LIMIT), but the value is set when scripts compile: it can't come from Settings.json, so
// changing one means editing this file, rebuilding, and players getting the new build. These are RA's values.
#ifndef SERVER
modded class BaseBuildingHologram
{
	static const float MAX_SNAP_DISTANCE = 2.8;		// how far a hologram looks for something to snap to
	static const float CAMERA_DISTANCE_MAX = 6.0;	// how far the camera can be from a snap point
}
#endif

// Turning a part while placing it. Parts that RA doesn't let turn get a turn of their own when free placement is on, or a free wall could only face one way.
#ifndef SERVER
modded class BaseBuildingHologram
{
	protected static const float FREE_TURN_STEP = 10;

	override protected void Update(float dt)
	{
		if (m_PreviewItem)
		{
			RAPatch_BuildSettings rules = RAPatch_Settings.Get().Build;
			float step = m_PreviewItem.GetSmallRotationStep();
			bool slow = GetUApi().GetInputByID(UATurbo).LocalValue() != 0;
			// Taken off before RA adds its full step, so it's steady
			float change = 0;
			if (step == 0)
			{
				if (rules.FreePlacement && !m_PreviewItem.IsInherited(BaseBuilding_OpenableBase))
				{
					change = FREE_TURN_STEP;
					if (slow)
						change = FREE_TURN_STEP * rules.SlowRotationMultiplier;
				}
			}
			else if (slow)
			{
				change = step * (rules.SlowRotationMultiplier - 1);
			}
			if (change != 0)
			{
				if (GetUApi().GetInputByID(UANextAction).LocalValue())
					m_SmallRotationAmount += change;
				if (GetUApi().GetInputByID(UAPrevAction).LocalValue())
					m_SmallRotationAmount -= change;
			}
		}
		super.Update(dt);
	}
}
#endif
