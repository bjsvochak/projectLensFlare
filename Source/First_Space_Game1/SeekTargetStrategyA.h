#pragma once

#include "First_Space_Game1.h"
#include "ISeekTargetStrategy.h"
#include "PlayerPawn.h"

class FIRST_SPACE_GAME1_API SeekTargetStrategyA : public ISeekTargetStrategy
{
	
public:
	virtual APlayerPawn* Seek(const AActor* _seeker, TIndexedContainerIterator<TArray<APlayerPawn*>, APlayerPawn*, int32> &_targetIt) override;
};