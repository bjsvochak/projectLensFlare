// Fill out your copyright notice in the Description page of Project Settings.

#include "First_Space_Game1.h"
#include "PlayerShipCameraManager.h"
#include "PlayerShipController.h"

APlayerShipController::APlayerShipController()
{
	PlayerCameraManagerClass = APlayerShipCameraManager::StaticClass();
}

float APlayerShipController::GetSpeed()
{
	return 0.0f;
}