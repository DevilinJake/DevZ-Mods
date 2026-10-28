// Runs Quick Transfer every frame while the inventory is open
modded class Inventory
{
	protected ref DevZ_QuickTransfer m_DevZ_QuickTransfer = new DevZ_QuickTransfer();

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		VicinityContainer vicinity;
		if (m_LeftArea)
			vicinity = m_LeftArea.GetVicinityContainer();
		m_DevZ_QuickTransfer.Update(vicinity);
	}

	override void OnHide()
	{
		super.OnHide();
		m_DevZ_QuickTransfer.Clear();
	}
}
