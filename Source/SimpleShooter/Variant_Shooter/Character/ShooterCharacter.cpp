// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"
#include <Net/UnrealNetwork.h>


AShooterCharacter::AShooterCharacter()
{
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// reset HP to max
	CurrentHP = MaxHP;
	bIsAlive = true;

	// update the HUD
	//OnDamaged.Broadcast(1.0f);

	if (HasAuthority())
	{
		if (AShooterGameMode* GM = GetWorld()->GetAuthGameMode<AShooterGameMode>())
		{
			GM->OnGameOver.AddDynamic(this, &AShooterCharacter::OnGameOver);
		}
	}
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);

		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);


		// Scoreboard
		EnhancedInputComponent->BindAction(ShowScoreboardAction, ETriggerEvent::Started, this, &AShooterCharacter::BP_ShowScoreboard);
		EnhancedInputComponent->BindAction(ShowScoreboardAction, ETriggerEvent::Completed, this, &AShooterCharacter::BP_CloseScoreboard);
	}

}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.0f;
	}

	// ignore if already dead
	if (!bIsAlive)
	{
		return 0.0f;
	}
	if (AShooterGameMode* GM = GetWorld()->GetAuthGameMode<AShooterGameMode>())
	{
		if (GM->GetIsGameOver())
		{
			return 0.0f;
		}
	}

	LastEventInstigator = EventInstigator;

	// Reduce HP
	CurrentHP -= Damage;

	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	// update the HUD
	UpdateHealthHUD();

	return Damage;
}

void AShooterCharacter::DoStartFiring()
{
	// fire the current weapon
	if (HasAuthority())
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->StartFiring();
		}
	}
	else {
		ServerDoStartFiring();
	}
}

void AShooterCharacter::DoStopFiring()
{
	// stop firing the current weapon
	if (HasAuthority())
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFiring();
		}
	}
	else {
		ServerDoStopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	if (HasAuthority())
	{
		// ensure we have at least two weapons two switch between
		if (OwnedWeapons.Num() > 1)
		{
			// deactivate the old weapon
			CurrentWeapon->DeactivateWeapon();

			// find the index of the current weapon in the owned list
			int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

			// is this the last weapon?
			if (WeaponIndex == OwnedWeapons.Num() - 1)
			{
				// loop back to the beginning of the array
				WeaponIndex = 0;
			}
			else {
				// select the next weapon index
				++WeaponIndex;
			}

			// set the new weapon as current
			CurrentWeapon = OwnedWeapons[WeaponIndex];

			// activate the new weapon
			CurrentWeapon->ActivateWeapon();
		}
	}
	else {
		ServerDoSwitchWeapon();
	}
}

void AShooterCharacter::ServerDoStartFiring_Implementation()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFiring();
	}
}

void AShooterCharacter::ServerDoStopFiring_Implementation()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::ServerDoSwitchWeapon_Implementation()
{
	DoSwitchWeapon();
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
	
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	AController* PC = GetController();
	if (PC && PC->IsLocalController())
	{
		OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
	}
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	// do we already own this weapon?
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// spawn the new weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// add the weapon to the owned list
			OwnedWeapons.Add(AddedWeapon);

			// if we have an existing weapon, deactivate it
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// switch to the new weapon
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// update the bullet counter
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// set the character mesh AnimInstances
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// unused
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;

}

void AShooterCharacter::Die()
{
	if (!HasAuthority() || !bIsAlive)
	{
		return;
	}

	bIsAlive = false;

	// deactivate the weapon
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	// reset the bullet counter UI
	OnBulletCountUpdated.Broadcast(0, 0);

	Multicast_OnDeath();

	if (AShooterGameMode* GM = GetWorld()->GetAuthGameMode<AShooterGameMode>())
	{
		GM->HandleCharacterDeath(LastEventInstigator, GetController());
	}
}

void AShooterCharacter::UpdateHealthHUD()
{
	AController* PC = GetController();
	if (PC && PC->IsLocalController())
	{

		OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));
	}
}

void AShooterCharacter::Multicast_OnDeath_Implementation()
{
	UpdateWeaponHUD(0, 0);

	// stop character movement
	GetCharacterMovement()->StopMovementImmediately();

	// disable controls
	DisableInput(nullptr);

	// call the BP handler
	BP_OnDeath();
}

void AShooterCharacter::OnGameOver()
{
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	GetCharacterMovement()->StopMovementImmediately();

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

}

void AShooterCharacter::OnRep_CurrentHP()
{
	UpdateHealthHUD();
}

void AShooterCharacter::OnRep_CurrentWeapon()
{
	if (CurrentWeapon)
	{
		OnWeaponActivated(CurrentWeapon);
	}
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterCharacter, CurrentHP);
	DOREPLIFETIME(AShooterCharacter, CurrentWeapon);
}
