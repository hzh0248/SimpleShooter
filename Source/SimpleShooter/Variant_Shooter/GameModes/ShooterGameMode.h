// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class AShooterPlayerState;
class AShooterNPC;
class AShooterAIController;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);


/**
 *  Simple GameMode for a first person shooter game
 *  Manages game UI
 *  Keeps track of team scores
 */
UCLASS(abstract)
class SIMPLESHOOTER_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	/** Map of scores by team ID */
	TMap<uint8, int32> TeamScores;

	UPROPERTY(EditAnywhere, Category = "Shooter|Rule")
	int32 TargetScore;

	UPROPERTY(EditAnywhere, Category = "Shooter|Rule")
	int32 KillPlayerScore = 3;

	UPROPERTY(EditAnywhere, Category = "Shooter|Rule")
	int32 KillAIScore = 1;

	UPROPERTY(EditAnywhere, Category = "Shooter|AI")
	TSubclassOf<AShooterNPC> AIPawnClass;

	//UPROPERTY(EditAnywhere, Category = "Shooter|AI")
	//TSubclassOf<AShooterAIController> AIControllerClass;

	UPROPERTY(EditAnywhere, Category = "Shooter|Respawn", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	TArray<AActor*> PlayerStarts;

	TArray<AActor*> AISpawnPoints;

	FTimerHandle SpawnAITimer;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter=GetIsGameOver)
	bool bIsGameOver = false;


private:
	int32 PendingAISpawnCount = 0;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	void RespawnPlayer(AController* Controller);

	void RespawnAI(APawn* AIPawn);

	void SpawnAI(int32 Count);

	void SpawnSingleAI();

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	void PostLogin(APlayerController* NewPlayer) override;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_GameOver();

public:

	/** Increases the score for the given team */
	void IncrementTeamScore(uint8 TeamByte);	

	void CkeckWinCondition(AShooterPlayerState* PS);

	void HandleCharacterDeath(AController* Killer, AController* Victim);

	void HandleNPCDeath(AController* Killer, AController* Victim);

	UFUNCTION(BlueprintCallable)
	bool GetIsGameOver() const{ return bIsGameOver; }


	void HandleGameOver();

	FOnGameOver OnGameOver;
};
