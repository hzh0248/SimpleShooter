// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/PlayerController/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "ShooterUI.h"
#include "SimpleShooter.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "PlayerStates/ShooterPlayerState.h"
#include "GameStates/ShooterGameState.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController())
	{
		if (SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogSimpleShooter, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}
		ShooterUI = CreateWidget<UShooterUI>(this, ShooterUIClass);

		if (ShooterUI)
		{
			ShooterUI->AddToPlayerScreen(0);
		}
		else {
			UE_LOG(LogSimpleShooter, Error, TEXT("Could not spawn  shooter widget."));
		}

		// create the bullet counter widget and add it to the screen
		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogSimpleShooter, Error, TEXT("Could not spawn bullet counter widget."));

		}
		
		if (AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>())
		{
			GS->OnGameOver.AddDynamic(this, &ThisClass::HandleGameOver);
		}
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PossessPawn(InPawn);
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	if (BulletCounterUI)
	{
		// reset the bullet counter HUD
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

void AShooterPlayerController::OnPlayerScoreUpdated(int32 PlayerScore)
{
	if (IsValid(ShooterUI))
	{
		ShooterUI->BP_UpdateScore(PlayerScore);
	}

	BP_OnScoreboardUpdated();
}

void AShooterPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	APawn* InPawn = GetPawn();

	PossessPawn(InPawn);
}

void AShooterPlayerController::PossessPawn(APawn* InPawn)
{
	InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);
	InPawn->Tags.Add(PlayerPawnTag);

	if (IsLocalPlayerController())
	{
		if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn()))
		{
			// subscribe to the pawn's delegates
			ShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
			ShooterCharacter->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);

			// force update the life bar
			ShooterCharacter->OnDamaged.Broadcast(1.0f);

			if (AShooterPlayerState* PS = GetPlayerState<AShooterPlayerState>())
			{
				PS->OnPlayerScoreUpdated.AddDynamic(this, &AShooterPlayerController::OnPlayerScoreUpdated);
			}
		}
	}
}

void AShooterPlayerController::OnRep_PlayerState()
{
	if (AShooterPlayerState* PS = GetPlayerState<AShooterPlayerState>())
	{
		PS->OnPlayerScoreUpdated.AddDynamic(this, &AShooterPlayerController::OnPlayerScoreUpdated);
	}
}

void AShooterPlayerController::HandleGameOver()
{
	BP_OnGameOver();
}
