// Made by Alex Jobe

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class ASPowerupActor;
class UDecalComponent;
class USphereComponent;

UCLASS()
class COOPGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	TSubclassOf<ASPowerupActor> PowerUpClass;

	ASPowerupActor* PowerUpInstance;

	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
	float CooldownDuration;

	FTimerHandle TimerHandle_RespawnTimer;

	UFUNCTION()
	void Respawn();

public:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
