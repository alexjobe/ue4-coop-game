// Made by Alex Jobe


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "CoopGame/CoopGame.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing (
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw debug lines for weapons"), 
	ECVF_Cheat
);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleFlashSocketName = "MuzzleFlashSocket";
	TracerTargetName = "BeamEnd";
	BaseDamage = 20.f;
	HeadShotMultiplier = 4.f;
	RateOfFire = 600.f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60.f / RateOfFire;
}

void ASWeapon::Fire()
{
	// Trace the world from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();
	if (!MyOwner) return;

	FVector TraceStart;
	FRotator EyeRotation;

	APlayerController* PC = Cast<APlayerController>(MyOwner->GetInstigatorController());
	if (!PC) return;

	PC->GetPlayerViewPoint(TraceStart, EyeRotation);

	FVector ShotDirection = EyeRotation.Vector();
	FVector TraceEnd = TraceStart + (ShotDirection * 10000);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(MyOwner);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	// Particle "Target" parameter
	FVector TracerEndPoint = TraceEnd;

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, COLLISION_WEAPON, QueryParams))
	{
		// Blocking hit! Process damage
		AActor* HitActor = Hit.GetActor();

		EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		float ActualDamage = BaseDamage;

		if (SurfaceType == SURFACE_FLESH_VULNERABLE)
		{
			ActualDamage = BaseDamage * HeadShotMultiplier;
		}

		UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, PC, this, DamageType);

		UParticleSystem* SelectedImpactEffect = nullptr;

		switch (SurfaceType)
		{
		case SURFACE_FLESH_DEFAULT:
		case SURFACE_FLESH_VULNERABLE:
			SelectedImpactEffect = FleshImpactEffect;
			break;
		default:
			SelectedImpactEffect = DefaultImpactEffect;
			break;
		}

		if (SelectedImpactEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		}

		TracerEndPoint = Hit.ImpactPoint;
	}

	if (DebugWeaponDrawing)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::White, false, 1.f, 0, 1.f);
	}

	PlayFireEffect(TracerEndPoint);

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->GetTimeSeconds(), 0.f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffect(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleFlashSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleFlashSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	AActor* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetInstigatorController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}

	}
}

