// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class FIRST_SPACE_GAME1_API AProjectile : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY( VisibleDefaultsOnly, Category = Projectile )
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY( VisibleDefaultsOnly, Category = Projectile )
	USphereComponent* Collision;
	float Damage;

	uint8 bPostImpact;

public:	
	AProjectile( const FObjectInitializer& ObjectInitializer );

	virtual void PostInitializeComponents();

	void SetDirection( const FVector &Direction );
	void IgnoreCollisionWithActor( const AActor &Actor );

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION()
	void OnCollide( const FHitResult &HitResult );

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void DestroyMe();
};
