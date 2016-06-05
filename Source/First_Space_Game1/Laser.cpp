// Fill out your copyright notice in the Description page of Project Settings.

#include "First_Space_Game1.h"
#include "Laser.h"
#include "PlayerShip.h"

ULaser::ULaser( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer )
{
	
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> LaserMesh;
		FConstructorStatics()
			: LaserMesh( TEXT( "/Game/Meshes/SM_SmallAutoCannon_Laser.SM_SmallAutoCannon_Laser" ) )
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	for (int i = 0; i < numLasers; ++i)
	{

		LaserInstances[i].DistanceToTravel = 0.f;
		LaserInstances[i].DistanceAlreadyTraveled = 0.f;

		FString name = "LaserInstance";
		name.AppendInt( i );

		LaserInstances[i].Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>( this, FName( *name ) );
							  
		LaserInstances[i].Mesh->SetStaticMesh( ConstructorStatics.LaserMesh.Get() );
		LaserInstances[i].Mesh->SetWorldScale3D( FVector( 0.1f, 0.01f, 0.01f ) );
		LaserInstances[i].Mesh->AttachParent = GetAttachParent();
		LaserInstances[i].Mesh->SetCollisionObjectType( ECC_Pawn );
		LaserInstances[i].Mesh->SetCollisionEnabled( ECollisionEnabled::NoCollision );
		LaserInstances[i].Mesh->SetCollisionResponseToAllChannels( ECR_Ignore );
		LaserInstances[i].Mesh->bCastDynamicShadow = false;
		LaserInstances[i].Mesh->bReceivesDecals = false;
		LaserInstances[i].Mesh->SetOwnerNoSee( false );
		LaserInstances[i].Mesh->SetOnlyOwnerSee( true );
		LaserInstances[i].Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		
	}

	FlagsActiveInstances = 0;

	LaserSpeed = 1000.f;
}


void ULaser::TickComponent( float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	
	APlayerShip* PS = Cast<APlayerShip>( GetOwner() );
	
	if (nullptr != PS)
	{
		MuzzleLocations[0] = PS->GetMuzzleLocation( 0 );
		MuzzleLocations[1] = PS->GetMuzzleLocation( 1 );
		MuzzleLocations[2] = PS->GetMuzzleLocation( 2 );
		OwnerRotator = PS->GetRotation();
		ULaser::UpdateActiveLaserInstances( DeltaTime );
	}
}

void ULaser::ShowNew( const FVector &Direction, float DistanceToTravel, int Muzzle )
{
	FLaserInstance* NewLaserInstance = GetAvailableLaserInstance();

	if (NewLaserInstance)
	{
		NewLaserInstance->Direction = Direction;
		NewLaserInstance->DistanceToTravel = DistanceToTravel;
		NewLaserInstance->DistanceAlreadyTraveled = 0.f;
		NewLaserInstance->Muzzle = Muzzle;
		NewLaserInstance->Mesh->SetVisibility( true );
		UpdateLaserInstance( *NewLaserInstance, 0.f );
	}
}

FLaserInstance* ULaser::GetAvailableLaserInstance()
{
	for (int i = 0; i < numLasers; ++i)
	{
		uint16 mask = (1 << i);

		if (0 == (FlagsActiveInstances & mask))
		{
			FlagsActiveInstances |= mask; // turn flag on
			GEngine->AddOnScreenDebugMessage( i, 5.f, FColor::Green, "ON" );
			return &LaserInstances[i];
		}
	}

	return nullptr; // no free laser instances
}

void ULaser::UpdateActiveLaserInstances( float DeltaTime )
{
	for (int i = 0; i < numLasers; ++i)
	{
		uint16 mask = (1 << i);

		if (FlagsActiveInstances & mask)
		{
			if (!UpdateLaserInstance( LaserInstances[i], DeltaTime ))
			{
				
				FlagsActiveInstances &= ~(mask);
				GEngine->AddOnScreenDebugMessage( i, 5.f, FColor::Blue, "OFF" );
			}
		}
	}
}

bool ULaser::UpdateLaserInstance( FLaserInstance &LaserInstance, float DeltaTime )
{
	LaserInstance.DistanceAlreadyTraveled += DeltaTime * LaserSpeed;	

	if (LaserInstance.DistanceToTravel > LaserInstance.DistanceAlreadyTraveled)
	{
		APlayerShip* PS = Cast<APlayerShip>( GetOwner() );
		if (nullptr != PS)
		{
			FVector Dir = PS->GetActorRotation().RotateVector( LaserInstance.Direction );
			Dir.Normalize();
			FVector Loc = PS->GetMuzzleLocation(LaserInstance.Muzzle) + Dir * LaserInstance.DistanceAlreadyTraveled * LaserInstance.DistanceAlreadyTraveled / LaserInstance.DistanceToTravel;
			FQuat Rot = Dir.Rotation().Quaternion();

			LaserInstance.Mesh->SetWorldLocationAndRotation( Loc, Rot );
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		LaserInstance.DistanceAlreadyTraveled = 0.f;
		LaserInstance.DistanceToTravel = 0.f;
		LaserInstance.Mesh->SetVisibility( false );
		return false;
	}
}
