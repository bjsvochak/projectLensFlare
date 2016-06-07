// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "ETeamEnum.h"
#include "Laser.h"
#include "PlayerShip.generated.h"

UENUM( BlueprintType )
enum class ETargetBoxType : uint8
{
	Target_EdgeNoGrow 	UMETA( DisplayName = "Edge No Grow" ),
	Target_EdgeGrow	UMETA( DisplayName = "Edge Grow" ),
	Target_CornerNoGrow UMETA( DisplayName = "Corner No Grow"),
	Target_CornerGrow UMETA( DisplayName = "Corner Grow")
};

USTRUCT(BluePrintable)
struct FTargetBox
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETargetBoxType Type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NearDist;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FarDist;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Width;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Height;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseLineRatio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LineThickness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UFont* Font;
};

UCLASS()
class FIRST_SPACE_GAME1_API APlayerShip : public ACharacter
{
	GENERATED_BODY()

private:

	float TimeOfLastFire;
	float TimeOfNextLaser;
	FTimerHandle LaserTriggerTimerHandle;

	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	UStaticMeshComponent* CockpitMesh;

	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	UStaticMeshComponent* SphereMesh;

	UPROPERTY(VisibleDefaultsOnly, Category=Camera)
	UCameraComponent* CockpitCamera;

	UPROPERTY( EditDefaultsOnly )
	float TimeBetweenLasers;

	UPROPERTY( EditDefaultsOnly )
	float TimeBetweenLasersIfBursting;

	FVector RelativeMuzzleLocation;

	/// Exposed Weapon Settings
	UPROPERTY( EditDefaultsOnly, Category = Projectile )
	TSubclassOf<class AProjectile> ProjectileClassOfLaser;

	/// Steering Settings
	float BaseLookLeftRightRate;
	float BaseLookUpDownRate;
	float BaseRollLeftRightRate;

	uint8 bFiring;

	uint8 bLeftMenuActive;
	uint8 bLeftMenuTransitioningIn;
	uint8 bLeftMenuTransitioningOut;

	/// Exposed Weapon Settings
	//UPROPERTY( EditDefaultsOnly, Category = Menu )
	//TSubclassOf<class UWidget> LeftMenuClass;

	//UWidget* LeftMenu;

	float leftMenuTimer;

	void Handle3DMenus();

	bool Menu3DTrigger( float Yaw1, float Yaw2, float Pitch1, float Pitch2 );
	
	/// Input Binding Functions
	void MoveForwardBackward( float MovementAmount );
	
	void RollLeftRight( float RollAmount );
	void LookLeftRight( float LookAmount );
	void LookUpDown( float LookAmount );
	void StrafeLeftRight( float StrafeAmount );
	

	void HoldTriggerPrimary();
	void ReleaseTriggerPrimary();

	FQuat LastRotation;

	UPROPERTY( Replicated )
	FQuat Rotation;
	
	void OnRep_Rotation();

	const FVector GetAim();
	
	UFUNCTION( Server, Unreliable, WithValidation )
	void Server_ReportRotation( const FQuat& NewRotation );

	/// Actions
	void ShootLaser(int Muzzle);

	UFUNCTION( Reliable, Server, WithValidation )
	void ServerShootLaser();

	FQuat QuatMult( const FQuat &q1, const FQuat &q2 );

	const FVector GetReticlePosition();
	const FVector GetCameraDamageStartLocation( const FVector &AimDir ) const;
	void ProcessInstantHit( const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int Muzzle );
	FHitResult WeaponTrace( const FVector& TraceFrom, const FVector& TraceTo ) const;
	void ProcessInstantHitConfirmed( const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int Muzzle );
	bool ShouldDealDamage( AActor* TestActor ) const;
	void DealDamage( const FHitResult& Impact, const FVector& ShootDir );
	void SimulateInstantHit( const FVector& ImpactPoint, int Muzzle );
	void SpawnTrailEffects( const FVector& EndPoint, int Muzzle );

	UFUNCTION( Reliable, Server, WithValidation )
	void ServerNotifyHit( const FHitResult Impact, FVector_NetQuantizeNormal ShootDir, int Muzzle );

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerNotifyMiss(FVector_NetQuantizeNormal ShootDir, int Muzzle );

	UPROPERTY(EditDefaultsOnly)
	float LaserRange;

	UPROPERTY(EditDefaultsOnly)
	float AllowedViewDotHitDir;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_HitLocation)
	FVector HitImpactNotify;

	UFUNCTION()
	void OnRep_HitLocation();

	UPROPERTY(EditDefaultsOnly)
	float LaserHitDamage;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly)
	float ClientSideHitLeeway;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* LaserTrailFX;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* LaserTracerFX;

	UPROPERTY(EditDefaultsOnly)
	FName LaserTrailTargetParam;

public:
	APlayerShip(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitComponents();

	/// Override Functions
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	virtual float TakeDamage( float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser ) override;
	virtual void PostRenderFor( APlayerController *PC, UCanvas *Canvas, FVector CameraPosition, FVector CameraDir ) override;

	void DrawTargetBox( const FVector &ScreenLocation, const FTargetBox &TargetBox, UCanvas *Canvas);
	
	UFUNCTION( Reliable, Server, WithValidation )
	void ServerTakeDamage( float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser );
	
	void UpdateRotation( float DeltaTime );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Team", ExposeOnSpawn = true) )
	ETeamEnum e_team;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category=Health)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category=Rotation)
	float CurrYawSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category=Rotation)
	float CurrPitchSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category=Rotation)
	float CurrRollSpeed;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, Category=Weapon)
	TArray<FName> MuzzleAttachPoints;

	float CurrYaw;
	float CurrPitch;
	float CurrRoll;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzleParticles;

	// Target Box
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Targeting)
	TArray<FTargetBox> TargetBoxes;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Reticle )
	UBillboardComponent* Reticle;

	UPROPERTY( EditDefaultsOnly, Category = Weapons )
	ULaser* Laser;

	const FVector GetMuzzleLocation( int Muzzle ) const;

	void TriggerPrimary();

	const FQuat &GetRotation() const { return Rotation; }
};
