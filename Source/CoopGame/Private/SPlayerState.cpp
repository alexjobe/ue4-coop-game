// Made by Alex Jobe


#include "SPlayerState.h"

void ASPlayerState::AddScore(float ScoreDelta)
{
	float NewScore = GetScore() + ScoreDelta;
	SetScore(NewScore);
}
