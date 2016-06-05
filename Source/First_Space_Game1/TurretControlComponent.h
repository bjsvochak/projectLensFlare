// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Turret.h"
#include "ISeekTargetStrategy.h"
#include "TurretControlComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FIRST_SPACE_GAME1_API UTurretControlComponent : public UActorComponent
{
	GENERATED_BODY()

	ISeekTargetStrategy* seekTargetStrategy;
	APlayerPawn* target;

public:	
	// Sets default values for this component's properties
	UTurretControlComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void Seek();
	void Engage();
	void PullTrigger();

	ATurret* turret;	
};
