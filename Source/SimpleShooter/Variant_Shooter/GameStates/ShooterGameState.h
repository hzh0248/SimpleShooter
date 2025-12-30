// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterGameState.generated.h"

class AShooterPlayerState;
//class FOnGameOver;

USTRUCT(BlueprintType)
struct FScoreEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	int32 Score;

	UPROPERTY(BlueprintReadOnly)
	int32 Rank;
};

/**
 * 
 */
UCLASS()
class SIMPLESHOOTER_API AShooterGameState : public AGameStateBase
{
	GENERATED_BODY()


protected:
	void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

public:

	UPROPERTY(ReplicatedUsing = OnRep_GameOver, BlueprintReadOnly)
	bool bIsGameOver = false;

	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<FScoreEntry> PlayerScores;

	UFUNCTION()
	void OnRep_GameOver();

	UFUNCTION()
	void ShooterGameOver();

	void UpdatePlayerScores();

	FOnGameOver OnGameOver;
	
};
