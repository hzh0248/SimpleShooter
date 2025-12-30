// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShooterBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLESHOOTER_API UShooterBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Shooter|Sort")
	static void SortPlayerStatesByScore(UPARAM(ref) TArray<APlayerState*>& PlayerStates);
	
	
};
