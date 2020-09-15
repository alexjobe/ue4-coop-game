// Made by Alex Jobe


#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.f;
	TotalNumOfTicks = 0;
	TicksProcessed = 0;

	bIsPowerupActive = false;

	SetReplicates(true);
}

void ASPowerupActor::ActivatePowerup(AActor* ActiveFor)
{
	OnActivated(ActiveFor);

	bIsPowerupActive = true;
	// OnRep is automatically called on clients, but we still need to call it on the server
	OnRep_PowerupActive();

	if (PowerupInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::ProcessTick, PowerupInterval, true);
	}
	else
	{
		ProcessTick();
	}
}

void ASPowerupActor::ProcessTick()
{
	TicksProcessed++;

	OnPowerupTicked();

	if (TicksProcessed >= TotalNumOfTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		// OnRep is automatically called on clients, but we still need to call it on the server
		OnRep_PowerupActive();

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}