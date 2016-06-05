// Fill out your copyright notice in the Description page of Project Settings.

#include "First_Space_Game1.h"
#include "BaseBullet.h"


// Sets default values
ABaseBullet::ABaseBullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseBullet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseBullet::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

