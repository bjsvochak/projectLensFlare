// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "Turret.h"
#include "PlayerPawn.h"
#include "GameMode_Current.generated.h"

/**
 * 
 */
UCLASS()
class FIRST_SPACE_GAME1_API AGameMode_Current : public AGameMode
{
	GENERATED_BODY()

	enum{ e_numTeams = 2};

	virtual void Tick(float DeltaSeconds);
	void UpdatePlayerLocationToFleet(APlayerPawn* _playerPawn, unsigned int _e_team);
	TArray<APlayerPawn*> teams[e_numTeams]; // one for red, one for blue

public:
	
};
