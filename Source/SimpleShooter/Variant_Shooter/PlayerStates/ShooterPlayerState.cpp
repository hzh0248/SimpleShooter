// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Shooter/PlayerStates/ShooterPlayerState.h"
#include "GameStates/ShooterGameState.h"
#include <Net/UnrealNetwork.h>




void AShooterPlayerState::OnRep_PlayerScore()
{
	OnPlayerScoreUpdated.Broadcast(PlayerScore);
}

void AShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPlayerState, PlayerScore);
}

void AShooterPlayerState::AddPlayerScore(int32 Delta)
{
	PlayerScore += Delta;

	if (HasAuthority())
	{
		OnPlayerScoreUpdated.Broadcast(PlayerScore);
	}

	if (AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>())
	{
		GS->UpdatePlayerScores();
	}
}

