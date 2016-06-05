// Fill out your copyright notice in the Description page of Project Settings.

#include "First_Space_Game1.h"
#include "PlayState.h"

void APlayState::UpdateTrackedPlayers(TArray<APlayerPawn*>* _teams)
{
	teams = _teams;	
}


