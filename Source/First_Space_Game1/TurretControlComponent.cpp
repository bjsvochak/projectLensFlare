// Fill out your copyright notice in the Description page of Project Settings.

#include "First_Space_Game1.h"
#include "PlayState.h"
#include "TurretControlComponent.h"
#include "SeekTargetStrategyA.h"


// Sets default values for this component's properties
UTurretControlComponent::UTurretControlComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	turret = static_cast<ATurret*>(GetOwner());

	static SeekTargetStrategyA strategyA;

	seekTargetStrategy = &strategyA;
	target = nullptr;
}


// Called when the game starts
void UTurretControlComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...	
}

void UTurretControlComponent::Seek()
{
	if (GetWorld()->GetGameState<APlayState>() != nullptr)
	{
		auto addr = &GetWorld()->GetGameState<APlayState>()->GetBlueTargets();

		if (addr == nullptr)
			return;

		auto it = addr->CreateIterator();

		target = seekTargetStrategy->Seek(turret, it);		
	}
}

void UTurretControlComponent::Engage()
{
	if (target)
	{
		FTransform trans = turret->GetTransform();

		FVector actorLoc = target->GetActorLocation();

		if (FVector::DistSquared(actorLoc, turret->GetActorLocation()) > TURRET_SIGHT * TURRET_SIGHT)
		{
			target = nullptr;
			return;
		}

		FVector d = trans.InverseTransformPosition(actorLoc);
		d -= turret->GetBarrelOffset();
		d.Normalize();
		FVector2D d2 = d.UnitCartesianToSpherical();		

		turret->GetChassis()->SetRelativeRotation(FRotator(0.0f, FMath::RadiansToDegrees(d2.Y), 0.0f));

		float barrelRotation = 90.0f - FMath::RadiansToDegrees(d2.X);
		if (barrelRotation < 30.0f && barrelRotation > -30.0f)
		{
			turret->GetBarrel()->SetRelativeRotation(FRotator(barrelRotation, 0.0f, 0.0f));
			PullTrigger();
		}
		else
		{
			target = nullptr;
		}

		/*
		// Chasis
		FTransform trans = turret->GetTransform();
		
		FVector d = target->GetActorLocation() - turret->GetBarrel()->GetComponentLocation();
		FVector chassisD = trans.InverseTransformVector(d);
		chassisD.Normalize();

		FVector upVector = turret->GetActorUpVector();
		FVector chassisProj = FVector::VectorPlaneProject(chassisD, FVector::UpVector);

		FVector2D chassisN(chassisProj);

		chassisN.Normalize();

		float anglerads = FMath::Atan2(chassisN.Y, chassisN.X);
		
		float chassisAngle = FMath::RadiansToDegrees(anglerads);

		turret->GetChassis()->SetRelativeRotation({ 0.0f, chassisAngle , 0.0f });

		// Barrel
		d.Normalize();
		FVector tup = turret->GetActorUpVector();
		tup.Normalize();
		float dp = FVector::DotProduct(d, turret->GetActorUpVector());
		if ( dp > -0.4f && dp < 0.87f)
		{
			turret->GetBarrel()->SetWorldRotation(d.Rotation());
		}
		*/

	}
	else
	{
		Seek();
	}
	
}

// Called every frame
void UTurretControlComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	Engage();
}

void UTurretControlComponent::PullTrigger()
{
	if (turret->IsReady())
	{
		turret->Fire();
	}
}