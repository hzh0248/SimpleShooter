// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerScoreUpdatedDelegate, int32, PlayerScore);

/**
 * 
 */
UCLASS()
class SIMPLESHOOTER_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerScore, BlueprintGetter = GetPlayerScore)
	int32 PlayerScore = 0;
	
	UFUNCTION()
	void OnRep_PlayerScore();

public:
	FPlayerScoreUpdatedDelegate OnPlayerScoreUpdated;

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

public:
	void AddPlayerScore(int32 Delta);

	UFUNCTION(BlueprintCallable)
	int32 GetPlayerScore() const { return PlayerScore;}
};
