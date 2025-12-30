// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/GameModes/ShooterGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterCharacter.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerStates/ShooterPlayerState.h"
#include "AIController.h"
#include "GameStates/ShooterGameState.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		APlayerStart::StaticClass(),
		PlayerStarts
	);

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "AISpawnPoint", AISpawnPoints);

	SpawnAI(4);
}

void AShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	// retrieve the team score if any
	int32 Score = 0;
	if (int32* FoundScore = TeamScores.Find(TeamByte))
	{
		Score = *FoundScore;
	}

	// increment the score for the given team
	++Score;
	TeamScores.Add(TeamByte, Score);
}

void AShooterGameMode::CkeckWinCondition(AShooterPlayerState* PS)
{
	if (PS->GetPlayerScore() >= TargetScore)
	{
		if (!bIsGameOver)
		{
			bIsGameOver = true;
			HandleGameOver();
		}
		
	}
}

void AShooterGameMode::HandleCharacterDeath(AController* Killer, AController* Victim)
{
	if (!Killer || Killer == Victim)
	{
		return;
	}

	if (AShooterPlayerState* PS = Killer->GetPlayerState<AShooterPlayerState>())
	{
		PS->AddPlayerScore(KillPlayerScore);
		
		CkeckWinCondition(PS);
	}

	RespawnPlayer(Victim);
}

void AShooterGameMode::HandleNPCDeath(AController* Killer, AController* Victim)
{
	if (!Killer || Killer == Victim)
	{
		return;
	}

	if (AShooterPlayerState* PS = Killer->GetPlayerState<AShooterPlayerState>())
	{
		PS->AddPlayerScore(KillAIScore);

		CkeckWinCondition(PS);
	}

	RespawnAI(Victim->GetPawn());
}

void AShooterGameMode::HandleGameOver()
{
	OnGameOver.Broadcast();
	BP_GameOver();
}

void AShooterGameMode::RespawnPlayer(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	APawn* Pawn = Controller->GetPawn();

	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(
		Timer,
		FTimerDelegate::CreateLambda([this, Pawn, Controller]()
			{
				if (IsValid(Pawn))
				{
					Pawn->Destroy();
				}

				if (Controller)
				{
					Controller->StartSpot = nullptr;
					RestartPlayer(Controller);
				}
			}),
		RespawnTime,
		false
	);
}

void AShooterGameMode::RespawnAI(APawn* AIPawn)
{

	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(
		Timer,
		FTimerDelegate::CreateLambda([this, AIPawn]()
			{
				if (IsValid(AIPawn))
				{
					AIPawn->Destroy();
				}
				
				SpawnAI(1);
			}),
		RespawnTime,
		false
	);
}

void AShooterGameMode::SpawnAI(int32 Count)
{
	if (!HasAuthority() || !AIPawnClass || AISpawnPoints.Num() == 0)
	{
		return;
	}

	PendingAISpawnCount += Count;

	if (!GetWorld()->GetTimerManager().IsTimerActive(SpawnAITimer))
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnAITimer, this, &AShooterGameMode::SpawnSingleAI, 0.2f, true);
	}
}

void AShooterGameMode::SpawnSingleAI()
{
	if (PendingAISpawnCount <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnAITimer);
		return;
	}

	if (!AIPawnClass || AISpawnPoints.Num() == 0)
	{
		return;
	}

	int32 Index = FMath::RandRange(0, AISpawnPoints.Num() - 1);
	AActor* SpawnPoint = AISpawnPoints[Index];
	if (SpawnPoint == nullptr) return;

	FVector SpawnLocation = SpawnPoint->GetActorLocation();
	FRotator SpawnRotation = SpawnPoint->GetActorRotation();

	SpawnLocation.Z += 100.f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = this;

	APawn* NewAIPawn = GetWorld()->SpawnActor<APawn>(AIPawnClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!NewAIPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnEnemy: Failed to spawn enemy"));
		return;
	}
	PendingAISpawnCount--;

	if (AAIController* AIController = Cast<AAIController>(NewAIPawn->GetController()))
	{
		AIController->Possess(NewAIPawn);
	}
}

AActor* AShooterGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (PlayerStarts.Num() == 0)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	// Ëæ»úÒ»¸ö
	int32 Index = FMath::RandRange(0, PlayerStarts.Num() - 1);
	return PlayerStarts[Index];
}

void AShooterGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AShooterGameState* GS = GetGameState<AShooterGameState>())
	{
		GS->UpdatePlayerScores();
	}
}
