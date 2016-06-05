// Fill out your copyright notice in the Description page of Project Settings.

#include "First_Space_Game1.h"
#include "PlayerPawn.h"
#include "PlayState.h"
#include "GameMode_Current.h"
#include "ETeamEnum.h"

void AGameMode_Current::Tick(float _dt)
{
	/*
	teams[0].Empty();
	teams[1].Empty();

	
	for (auto controller = GetWorld()->GetPlayerControllerIterator(); controller; ++controller)
	{
		if (controller->IsValid())
		{
			APlayerPawn* pawn = static_cast<APlayerPawn*>(controller->Get()->GetPawn());
		}
	}
	*/
	
}

void AGameMode_Current::UpdatePlayerLocationToFleet(APlayerPawn* _playerPawn, unsigned int _e_team)
{
	// TODO: replace this code with actual fleet code (the array is sent through all the way to the turret level (filtered along the way), for now, just getting turrets firing
	teams[_e_team].Push(_playerPawn);
}

