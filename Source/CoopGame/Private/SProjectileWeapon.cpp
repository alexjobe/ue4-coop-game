// Made by Alex Jobe


#include "SProjectileWeapon.h"

void ASProjectileWeapon::Fire()
{
	// Trace the world from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();
	if (MyOwner && ProjectileClass)
	{
		FVector TraceStart;
		FRotator EyeRotation;

		APlayerController* PC = Cast<APlayerController>(MyOwner->GetInstigatorController());
		if (!PC) return;

		PC->GetPlayerViewPoint(TraceStart, EyeRotation);

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleFlashSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
	}
}