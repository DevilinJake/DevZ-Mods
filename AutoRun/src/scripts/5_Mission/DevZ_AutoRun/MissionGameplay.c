// Runs Auto Run every frame on the client (MissionGameplay only exists there)
modded class MissionGameplay
{
	protected ref DevZ_AutoRun m_DevZ_AutoRun = new DevZ_AutoRun();

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		m_DevZ_AutoRun.Update();
	}
}
