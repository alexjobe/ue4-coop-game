// Made by Alex Jobe


#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "CoopGame/CoopGame.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"

// Command to display debug lines in game
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

	MuzzleSocketName = "MuzzleFlashSocket";
	TracerTargetName = "BeamEnd";

	BaseDamage = 20.f;
	HeadShotMultiplier = 4.f;
	RateOfFire = 600.f;

	SetReplicates(true);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60.f / RateOfFire;
}

void ASWeapon::Fire()
{
	// Trace the world from pawn eyes to crosshair location

	if (!HasAuthority())
	{
		ServerFire();
	}

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

	EPhysicalSurface SurfaceType = SurfaceType_Default;

	FHitResult Hit;

	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, COLLISION_WEAPON, QueryParams))
	{
		// Blocking hit! Process damage
		AActor* HitActor = Hit.GetActor();

		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		float ActualDamage = BaseDamage;

		if (SurfaceType == SURFACE_FLESH_VULNERABLE)
		{
			ActualDamage = BaseDamage * HeadShotMultiplier;
		}

		UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, PC, this, DamageType);

		PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

		TracerEndPoint = Hit.ImpactPoint;
	}

	if (DebugWeaponDrawing)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::White, false, 1.f, 0, 1.f);
	}

	PlayFireEffects(TracerEndPoint);

	if (HasAuthority())
	{
		HitScanTrace.TraceTo = TracerEndPoint;
		HitScanTrace.SurfaceType = SurfaceType;
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
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

void ASWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic effects
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ASWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
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

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
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
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedImpactEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}