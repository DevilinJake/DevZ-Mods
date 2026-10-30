// DayZ Expansion Base Building only (compiled out on servers and clients without it).
#ifdef EXPANSIONMODBASEBUILDING

// Expansion's satchel charge (ExpansionSatchel, an ExpansionExplosive) beeps while it is armed.
// On each client, OnVariablesSynchronized() schedules OnFrame() as a repeating call-queue entry
// when the charge is armed; OnFrame() calls TriggerSound(m_Time), which beeps once m_Timer has
// counted up to m_Time * 100 (m_Time = synced seconds left, only updated below 10 seconds).
//
// Two things go wrong after the bang:
//   1. Nothing ever removes that repeating OnFrame() call. It is only removed in the destructor,
//      and the queued call itself keeps the client-side object alive long after the server has
//      deleted it (RemoveLater, 150 ms after exploding), so OnFrame() keeps running for minutes.
//   2. By then m_Time is 0, so m_Timer < 0 is never true and TriggerSound() plays a fresh beep
//      sound every single frame: the "squeal" at the crater that fades with distance.
//
// The fix: stop beeping, and remove the repeating call, as soon as the charge has exploded or
// is no longer armed; and never let a time of 0 beep every frame (it is treated as 1, the same
// quick beep as the last second of the countdown, until the explosion arrives).
modded class ExpansionExplosive
{
	override void TriggerSound(int time)
	{
		if (m_Exploded || !m_ArmedSynchRemote)
		{
			DevZ_StopBeeping();
			return;
		}

		if (time < 1)
			time = 1;

		super.TriggerSound(time);
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();   // plays the explosion sound and particles

		if (m_ExplodedSynchRemote)
			DevZ_StopBeeping();
	}

	// Remove the repeating per-frame call (harmless if it was never scheduled) and silence any
	// sound the charge still holds.
	void DevZ_StopBeeping()
	{
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OnFrame);

		if (m_Sound)
			m_Sound.SoundStop();
	}
}

#endif
