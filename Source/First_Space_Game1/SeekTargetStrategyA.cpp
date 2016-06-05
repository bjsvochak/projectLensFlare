#include "First_Space_Game1.h"
#include "SeekTargetStrategyA.h"


APlayerPawn* SeekTargetStrategyA::Seek(const AActor* _seeker, TIndexedContainerIterator<TArray<APlayerPawn*>, APlayerPawn*, int32> &_targetIt)
{
	APlayerPawn* result = nullptr;
	float max = -1.0f;
	float ubound = TURRET_SIGHT;
	float turretFilter = 0.5f;

	for (; _targetIt; ++_targetIt)
	{
		if (*_targetIt)
		{
			FVector tLocation = (*_targetIt)->GetActorLocation();
			FVector d = tLocation - _seeker->GetActorLocation();
			d.Normalize();
			float dot = FVector::DotProduct(d, _seeker->GetActorUpVector());
			if (turretFilter >= FMath::Abs(dot))
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::SanitizeFloat(dot));
				float distance = d.SizeSquared();

				if (distance > max && distance < ubound)
				{
					result = *_targetIt;
					max = distance;
				}
			}			
		}
	}

	return result;
}