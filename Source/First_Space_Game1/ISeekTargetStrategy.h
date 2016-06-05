// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "First_Space_Game1.h"
#include "PlayerPawn.h"
/**
 * 
 */

class FIRST_SPACE_GAME1_API ISeekTargetStrategy
{
public:
	virtual APlayerPawn* Seek(const AActor* _seekerTransform, TIndexedContainerIterator<TArray<APlayerPawn*>, APlayerPawn*, int32> &_targetIt) = 0;
};
