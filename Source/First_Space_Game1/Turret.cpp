// Fill out your copyright notice in the Description page of Project Settings.

#include "First_Space_Game1.h"
#include "Turret.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "BaseBullet.h"
#include "PlayerShip.h"
#include "Projectile.h"


// Sets default values
ATurret::ATurret()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	timer = -3.0f;

	if (reloadTime == 0.0f)
	{
		reloadTime = 1.0f;
	}

	rotationOffsetInDegrees.Pitch = rotationOffsetInDegrees.Yaw = rotationOffsetInDegrees.Roll = 1.0f;
}

// Called when the game starts or when spawned
void ATurret::BeginPlay()
{
	Super::BeginPlay();	
}

void ATurret::SetChassis(UStaticMeshComponent* _staticMesh)
{
	chassis = _staticMesh; 
	TArray<FName> sockets = _staticMesh->GetAllSocketNames();
	barrelOffset = _staticMesh->GetSocketTransform(_staticMesh->GetAllSocketNames()[0], RTS_Actor).GetTranslation();
}

void ATurret::CreateMuzzlesFromSocketedMesh(UStaticMeshComponent* _staticMesh)
{
	muzzles.Empty();
	barrel = _staticMesh;

	for (auto& socketName : _staticMesh->GetAllSocketNames())
	{		
		muzzles.Add(barrel->GetSocketByName(socketName));
	}
}

// Called every frame
void ATurret::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	
		timer += DeltaTime;

		if (timer >= reloadTime)
		{
			FindTargetToFireAt();
		}
	
}

void ATurret::FindTargetToFireAt()
{
	float closestDsq = INFINITY;
	FTransform trans = GetTransform();
	float finalChassisRotation = 0.0f;
	float finalBarrelRotation = 0.0f;

	for (auto it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		FVector TestTarget = ( *it )->GetPawn()->GetActorLocation();

		float dsq = FVector::DistSquared( TestTarget, GetActorLocation() );

		if (dsq < TURRET_SIGHT * TURRET_SIGHT && dsq < closestDsq)
		{
			FVector d = trans.InverseTransformPosition( TestTarget );
			d -= GetBarrelOffset();
			d.Normalize();
			FVector2D d2 = d.UnitCartesianToSpherical();

			float barrelRotation = 90.0f - FMath::RadiansToDegrees( d2.X );
			if (barrelRotation < 30.0f && barrelRotation > -30.0f)
			{
				finalChassisRotation = d2.Y;
				finalBarrelRotation = barrelRotation;
				closestDsq = dsq;
			}
		}
	}

	if (closestDsq < INFINITY)
	{
		GetChassis()->SetRelativeRotation( FRotator( 0.0f, FMath::RadiansToDegrees( finalChassisRotation ), 0.0f ) );
		GetBarrel()->SetRelativeRotation( FRotator( finalBarrelRotation, 0.0f, 0.0f ) );
		if (Role == ROLE_Authority)
		{
			Fire();
		}
	}
}

void ATurret::Fire()
{
	timer = 0.0f;

	for (auto& muzzle : muzzles)
	{				
		FTransform trans;
		muzzle->GetSocketTransform(trans, barrel);
		FVector loc = GetTransform().GetLocation() + GetActorUpVector() * barrelOffset;
		ReleaseBullet(loc, trans.GetRotation().Rotator().Add(
			FMath::FRand() * rotationOffsetInDegrees.Pitch - (rotationOffsetInDegrees.Pitch * .5f), 
			FMath::FRand() * rotationOffsetInDegrees.Yaw - (rotationOffsetInDegrees.Yaw * .5f),
			FMath::FRand() * rotationOffsetInDegrees.Roll - (rotationOffsetInDegrees.Roll * .5f) ));
	}
}

void ATurret::RotateBarrelAndChassis_Implementation( float ChassisRotation, float BarrelRotation )
{
	
	//GEngine->AddOnScreenDebugMessage( 0, 5.f, FColor::Yellow, FRotator( 0.0f, ChassisRotation, 0.0f ).ToString() );
	//GEngine->AddOnScreenDebugMessage( 0, 5.f, FColor::Yellow, FRotator( BarrelRotation, 0.0f, 0.0f ).ToString() );
}

bool ATurret::ATurret::RotateBarrelAndChassis_Validate( float ChassisRotation, float BarrelRotation )
{
	return true;
}

void ATurret::ReleaseBullet(const FVector &LaserOrigin, const FRotator &LaserRotation)
{
	FVector LaserDirection = GetActorForwardVector();

	FTransform LaserTransform( LaserRotation, LaserOrigin );

	AProjectile* Laser = Cast<AProjectile>( UGameplayStatics::BeginDeferredActorSpawnFromClass( this, ProjectileClassOfLaser, LaserTransform ) );

	if (Laser)
	{
		Laser->SetOwner( this );
		Laser->Instigator = this;
		Laser->SetDirection( FVector::ForwardVector );
		Laser->IgnoreCollisionWithActor( *this );
	}	

	UGameplayStatics::FinishSpawningActor( Laser, LaserTransform );
}



