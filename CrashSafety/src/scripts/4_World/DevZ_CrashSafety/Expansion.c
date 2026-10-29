// Expansion Vehicles only (compiled out on servers without it).
#ifdef EXPANSIONMODVEHICLE

// Expansion runs its own engine simulation for cars and helicopters. Low coolant, and a helicopter
// low on oil, take up to about 20 health a tick straight off the engine, skipping the damage
// system. On a helicopter a ruined engine destroys the whole aircraft at once, before anything can
// put it back. So once the engine is within one tick of its floor, the fluids are reported as fine
// for that tick: they still wear the engine down, but only to the floor.
// Expansion's RevvingOverMaxRPMRuinsEngineInstantly setting also skips the damage system; leave it off.
modded class ExpansionVehicleEngineBase
{
	// Most fluid damage Expansion can do in one tick: coolant up to 10, helicopter oil up to 10
	static const float DEVZ_CS_MAX_FLUID_DAMAGE = 20;

	override void ProcessHealth(float pDt, float pFuel, float pCoolant, float pOil, float pBrake, out float pOutHealth, out float pOutFuel)
	{
		if (g_Game.IsServer() && m_Vehicle && DevZ_CrashSafetyFloors.NearFloor(m_Vehicle, m_DamageZone, DEVZ_CS_MAX_FLUID_DAMAGE))
		{
			pCoolant = 1;
			pOil = 1;
		}
		super.ProcessHealth(pDt, pFuel, pCoolant, pOil, pBrake, pOutHealth, pOutFuel);
	}
}

#endif
