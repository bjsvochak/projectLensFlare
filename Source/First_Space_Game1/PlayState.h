// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "PlayerPawn.h"
#include "PlayState.generated.h"

/**
 * 
 */
UCLASS()
class FIRST_SPACE_GAME1_API APlayState : public AGameState
{
	GENERATED_BODY()

	TArray<APlayerPawn*>* teams;
	
public:

	void UpdateTrackedPlayers(TArray<APlayerPawn*>* _teams);
	
	UFUNCTION(BlueprintCallable, Category = "Player Targets")
	TArray<APlayerPawn*>& GetBlueTargets() { return teams[0]; }

	UFUNCTION(BlueprintCallable, Category = "Player Targets")
	TArray<APlayerPawn*>& GetRedTargets() { return teams[1]; }
};
