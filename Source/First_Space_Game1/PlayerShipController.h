// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "PlayerShipController.generated.h"

/**
 * 
 */
UCLASS()
class FIRST_SPACE_GAME1_API APlayerShipController : public APlayerController
{
	GENERATED_BODY()
	
public:

	APlayerShipController();
	
	float GetSpeed();
};
