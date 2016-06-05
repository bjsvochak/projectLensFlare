// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseBullet.h"
#include "ETeamEnum.h"
#include "Turret.generated.h"

class UWorld;

UCLASS()
class FIRST_SPACE_GAME1_API ATurret : public APawn
{
	GENERATED_BODY()

	bool isReady = false;
	float timer;
	UWorld* world;
	TArray<const UStaticMeshSocket*> muzzles;
	UStaticMeshComponent* barrel;
	UStaticMeshComponent* chassis;
	FVector barrelOffset;

public:
	// Sets default values for this pawn's properties
	ATurret();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;	

	virtual void Fire();

	virtual bool IsReady() { return isReady; }

	void FindTargetToFireAt();

	UStaticMeshComponent* GetBarrel() { return barrel; }
	UStaticMeshComponent* GetChassis() { return chassis; }
	FVector GetBarrelOffset() { return barrelOffset; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	float reloadTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FRotator rotationOffsetInDegrees;

	UFUNCTION(BlueprintCallable, Category = "Chassis")
	void SetChassis(UStaticMeshComponent* _staticMesh);

	UFUNCTION(BlueprintCallable, Category = "Muzzles")
	void CreateMuzzlesFromSocketedMesh(UStaticMeshComponent* _staticMesh);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enum, Meta = (DisplayName = "Team", ExposeOnSpawn = true))
	ETeamEnum e_team;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void RotateBarrelAndChassis( float ChassisRotation, float BarrelRotation );
	void RotateBarrelAndChassis_Implementation( float ChassisRotation, float BarrelRotation );
	bool RotateBarrelAndChassis_Validate( float ChassisRotation, float BarrelRotation );

	void ReleaseBullet(const FVector &_location, const FRotator &_rotation);

	UPROPERTY( EditDefaultsOnly, Category = Projectile )
	TSubclassOf<class AProjectile> ProjectileClassOfLaser;
};
