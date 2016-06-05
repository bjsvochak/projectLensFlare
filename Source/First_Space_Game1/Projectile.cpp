// Fill out your copyright notice in the Description page of Project Settings.

#include "First_Space_Game1.h"
#include "Projectile.h"
#include "PlayerShip.h"
#include "Casts.h"


AProjectile::AProjectile( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer )
{
	Collision = ObjectInitializer.CreateDefaultSubobject<USphereComponent>( this, TEXT( "SphereComp" ) );
	Collision->InitSphereRadius( 5.0f );
	Collision->AlwaysLoadOnClient = true;
	Collision->AlwaysLoadOnServer = true;
	Collision->bTraceComplexOnMove = true;
	Collision->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	Collision->SetCollisionObjectType( COLLISION_PROJECTILE );
	Collision->SetCollisionResponseToAllChannels( ECR_Ignore );
	Collision->SetCollisionResponseToChannel( ECC_WorldStatic, ECR_Block );
	Collision->SetCollisionResponseToChannel( ECC_WorldDynamic, ECR_Block );
	Collision->SetCollisionResponseToChannel( ECC_Pawn, ECR_Block );
	RootComponent = Collision;

	ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>( this, TEXT( "ProjectileComp" ) );
	ProjectileMovement->UpdatedComponent = Collision;
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat( ROLE_SimulatedProxy );
	bReplicates = true;
	bReplicateMovement = true;

	Damage = 100.f;
}

void AProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ProjectileMovement->OnProjectileStop.AddDynamic( this, &AProjectile::OnCollide );
	Collision->MoveIgnoreActors.Add( Instigator );

	SetLifeSpan( 7.0f );
}

void AProjectile::SetDirection( const FVector &Direction )
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
	}
}

void AProjectile::IgnoreCollisionWithActor( const AActor &Actor )
{
	Collision->MoveIgnoreActors.Add( &Actor );
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectile::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AProjectile::OnCollide( const FHitResult& HitResult )
{
	if (Role == ROLE_Authority & !bPostImpact)
	{
		if ( HitResult.Actor != Instigator && HitResult.Actor->IsA( APlayerShip::StaticClass() ) && HitResult.GetActor()->GetRemoteRole() == ROLE_Authority)
		{
			APlayerShip* HitPlayer = Cast<APlayerShip>(HitResult.GetActor());

			if (HitPlayer)
			{
				FPointDamageEvent HitPlayerDamage;
				HitPlayerDamage.DamageTypeClass = UDamageType::StaticClass();
				HitPlayerDamage.HitInfo = HitResult; 
				HitPlayerDamage.ShotDirection = GetActorForwardVector();
				HitPlayerDamage.Damage = Damage;

				HitPlayer->TakeDamage( HitPlayerDamage.Damage, HitPlayerDamage, HitPlayer->Controller, this );
			}
			DestroyMe();
		}
	}
}

bool AProjectile::DestroyMe_Validate()
{
	return true;
}

void AProjectile::DestroyMe_Implementation()
{
	Destroy();
}

