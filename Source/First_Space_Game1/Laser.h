// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Projectile.h"
#include "Laser.generated.h"

USTRUCT(BluePrintable)
struct FLaserInstance
{
	GENERATED_USTRUCT_BODY()

	FVector Direction;
	float DistanceToTravel;
	float DistanceAlreadyTraveled;
	unsigned int Muzzle;
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	UStaticMeshComponent* Mesh;
};

UENUM( BlueprintType )		//"BlueprintType" is essential to include
enum class ELaserModeEnum : uint8
{
	LME_Single 	UMETA( DisplayName = "Single" ),
	LME_Double 	UMETA( DisplayName = "Double" ),
	LME_Scatter UMETA( DisplayName = "Scatter" ),
	LME_Burst	UMETA( DisplayName = "Burst" )
};

UCLASS()
class FIRST_SPACE_GAME1_API ULaser : public USceneComponent
{
	GENERATED_BODY()

	enum { numLasers = 16 };
	
	uint16 FlagsActiveInstances;

	class APawn* Instigator;

	FVector MuzzleLocations[3];
	FQuat OwnerRotator;

	FLaserInstance LaserInstances[numLasers];

public:

	UPROPERTY( EditAnywhere )
	float LaserSpeed;	

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "LaserMode", ExposeOnSpawn = true) )
	ELaserModeEnum e_LaserMode;

	virtual void TickComponent( float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction );
	ULaser(const FObjectInitializer& ObjectInitializer);

	void ShowNew( const FVector &Direction, float DistanceToTravel, int Muzzle );
	void UpdateActiveLaserInstances( float DeltaTime );
	bool UpdateLaserInstance( FLaserInstance &LaserInstance, float DeltaTime );

private:

	FLaserInstance* GetAvailableLaserInstance();
};
