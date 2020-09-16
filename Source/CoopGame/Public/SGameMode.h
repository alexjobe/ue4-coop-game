// Made by Alex Jobe

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ASGameMode();
	
protected:

	FTimerHandle TimerHandle_BotSpawner;

	FTimerHandle TimerHandle_NextWaveStart;

	int32 WaveCount;

	// Number to multiply WaveCount by on each successive wave
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	int32 WaveCountMultiplier;

	// Bots to spawn in current wave
	int32 NumBotsToSpawn;

	// Seconds between each spawn
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float SpawnRate;

	// Seconds between each wave
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

	// Hook for BP to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	// Start spawning bots
	void StartWave();

	// Stop spawning bots
	void EndWave();

	// Start timer for next StartWave
	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

public:

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

};
