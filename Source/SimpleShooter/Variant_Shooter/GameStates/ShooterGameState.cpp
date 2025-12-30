// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Shooter/GameStates/ShooterGameState.h"
#include "PlayerStates/ShooterPlayerState.h"
#include "ShooterGameMode.h"
#include <Net/UnrealNetwork.h>




void AShooterGameState::OnRep_GameOver()
{
	if (bIsGameOver)
	{
		OnGameOver.Broadcast();
	}
	
}

void AShooterGameState::ShooterGameOver()
{

	UpdatePlayerScores();

	bIsGameOver = true;
	OnGameOver.Broadcast();
}



void AShooterGameState::BeginPlay()
{
	Super::BeginPlay();

	if (AShooterGameMode* GM = GetWorld()->GetAuthGameMode<AShooterGameMode>())
	{
		GM->OnGameOver.AddDynamic(this, &AShooterGameState::ShooterGameOver);
	}
}

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterGameState, bIsGameOver);
}

void AShooterGameState::UpdatePlayerScores()
{
	PlayerScores.Empty();

	for (APlayerState* PS : PlayerArray)
	{
		if (AShooterPlayerState* SPS = Cast<AShooterPlayerState>(PS))
		{
			FScoreEntry Entry;
			Entry.PlayerName = SPS->GetPlayerName();
			Entry.Score = SPS->GetPlayerScore();
			PlayerScores.Add(Entry);
		}
	}

	PlayerScores.Sort([](const FScoreEntry& A, const FScoreEntry& B)
		{
			return A.Score > B.Score;
		}
	);

	for (int32 i = 0; i < PlayerScores.Num(); ++i)
	{
		PlayerScores[i].Rank = i + 1;
	}
}
