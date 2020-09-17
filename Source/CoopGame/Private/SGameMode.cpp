// Made by Alex Jobe


#include "SGameMode.h"
#include "Components/SHealthComponent.h"
#include "EngineUtils.h"
#include "SGameState.h"
#include "SPlayerState.h"
#include "TimerManager.h"

ASGameMode::ASGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f;

	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();

	WaveCount = 1;
	WaveCountMultiplier = 2;
	SpawnRate = 1.f;
	TimeBetweenWaves = 2.f;
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
	CheckAnyPlayerAlive();
}

void ASGameMode::StartWave()
{
	WaveCount++;

	NumBotsToSpawn = WaveCount * WaveCountMultiplier;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, SpawnRate, true, 0.f);

	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);
}

void ASGameMode::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);

	RespawnDeadPlayers();

	SetWaveState(EWaveState::WaitingToStart);
}

void ASGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	if (NumBotsToSpawn > 0 || bIsPreparingForWave) return;

	bool bIsAnyBotAlive = false;

	for (TActorIterator<APawn> PawnIterator(GetWorld()); PawnIterator; ++PawnIterator)
	{
		APawn* TestPawn = *PawnIterator;

		if (!TestPawn || TestPawn->IsPlayerControlled()) continue;

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		if (HealthComp && HealthComp->GetHealth() > 0.f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive) 
	{ 
		SetWaveState(EWaveState::WaveComplete);
		PrepareForNextWave();
	};
}

void ASGameMode::CheckAnyPlayerAlive()
{
	for (TActorIterator<APlayerController> PCIterator(GetWorld()); PCIterator; ++PCIterator)
	{
		APlayerController* PC = *PCIterator;
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.f)
			{
				// A player is still alive
				return;
			}
		}
	}

	// No player alive
	GameOver();
}

void ASGameMode::GameOver()
{
	EndWave();

	SetWaveState(EWaveState::GameOver);
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();

	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}

void ASGameMode::RespawnDeadPlayers()
{
	for (TActorIterator<APlayerController> PCIterator(GetWorld()); PCIterator; ++PCIterator)
	{
		APlayerController* PC = *PCIterator;
		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NumBotsToSpawn--;

	if (NumBotsToSpawn <= 0)
	{
		EndWave();
	}
}
