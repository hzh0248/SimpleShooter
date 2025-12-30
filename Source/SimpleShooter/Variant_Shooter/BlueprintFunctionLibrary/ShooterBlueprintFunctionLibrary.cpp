// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Shooter/BlueprintFunctionLibrary/ShooterBlueprintFunctionLibrary.h"
#include "PlayerStates/ShooterPlayerState.h"

void UShooterBlueprintFunctionLibrary::SortPlayerStatesByScore(TArray<APlayerState*>& PlayerStates)
{
	PlayerStates.Sort(
		[](const APlayerState& A, const APlayerState& B)
		{
			const AShooterPlayerState* SA = Cast<AShooterPlayerState>(&A);
			const AShooterPlayerState* SB = Cast<AShooterPlayerState>(&B);

			if (!SA && !SB) return false;
			if (!SA) return false;
			if (!SB) return true;

			return SA->GetPlayerScore() > SB->GetPlayerScore();
		}
	);
}


