// Fill out your copyright notice in the Description page of Project Settings.

#include "First_Space_Game1.h"
#include "PlayerShip.h"
#include "PlayerShipController.h"
#include "Projectile.h"
#include "UnrealNetwork.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"


APlayerShip::APlayerShip( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer )
{
	// We do not want the player to see the 3rd person mesh
	GetMesh()->SetOwnerNoSee( true );
	GetMesh()->SetOnlyOwnerSee( false );

	RootComponent = GetCapsuleComponent();

	CockpitCamera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>( this, TEXT( "CockpitCamera" ) );
	CockpitCamera->AttachParent = GetCapsuleComponent();

	CockpitMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>( this, TEXT( "CockpitMesh" ) );
	CockpitMesh->AttachParent = GetCapsuleComponent();
	CockpitMesh->SetCollisionObjectType( ECC_Pawn );
	CockpitMesh->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	CockpitMesh->SetCollisionResponseToAllChannels( ECR_Ignore );
	CockpitMesh->bCastDynamicShadow = false;
	CockpitMesh->bReceivesDecals = false;
	CockpitMesh->SetOwnerNoSee( false );
	CockpitMesh->SetOnlyOwnerSee( true );
	CockpitMesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;

	Reticle = ObjectInitializer.CreateDefaultSubobject<UBillboardComponent>( this, TEXT( "Reticle" ) );
	Reticle->AttachParent = GetCapsuleComponent();

	Laser = ObjectInitializer.CreateDefaultSubobject<ULaser>( this, TEXT( "Laser" ) );
	Laser->AttachParent = RootComponent;
	Laser->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	Laser->PrimaryComponentTick.bCanEverTick = true;
	Laser->bAutoActivate = true;
	Laser->SetComponentTickEnabled( true );
	Laser->RegisterComponent();
	Laser->RegisterAllComponentTickFunctions( true );

	SphereMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>( this, TEXT( "SphereMesh" ) );
	SphereMesh->AttachParent = GetCapsuleComponent();
	SphereMesh->SetVisibility( false );

	PrimaryActorTick.bCanEverTick = true;
	BaseLookLeftRightRate = 9.f;
	BaseLookUpDownRate = 9.f;
	BaseRollLeftRightRate = 9.f;
	BaseEyeHeight = 0.0f;
	TimeOfLastFire = 0.0f;
	TimeBetweenLasers = 0.15f;
	TimeBetweenLasersIfBursting = 0.5f;
	RelativeMuzzleLocation = FVector( 0.5f, 0.0f, -8.f );

	Health = 10000.0f;

	CurrYawSpeed = CurrPitchSpeed = CurrRollSpeed = 0.0f;

	bLeftMenuActive = 0;
	leftMenuTimer = 0.f;
	bLeftMenuTransitioningIn = 0;
	bLeftMenuTransitioningOut = 0;

	LaserRange = 15000;
	AllowedViewDotHitDir = -1.0f;
	ClientSideHitLeeway = 200.0f;
	LaserHitDamage = 30.f;

	bFiring = false;
}

void APlayerShip::Handle3DMenus()
{
	if (Menu3DTrigger( -72, -35, -18, 20 ))
	{
		
	}
}

bool APlayerShip::Menu3DTrigger( float Yaw1, float Yaw2, float Pitch1, float Pitch2 )
{
	FRotator DeviceRotation;
	FVector DevicePosition;
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition( DeviceRotation, DevicePosition );

	return (DeviceRotation.Yaw > Yaw1 && DeviceRotation.Yaw < Yaw2 && DeviceRotation.Pitch > Pitch1 && DeviceRotation.Pitch < Pitch2);
}

void APlayerShip::PostInitComponents()
{
	

	int64 DateInSeconds = FDateTime::Now().ToUnixTimestamp();
	FRandomStream SRand = FRandomStream();
	SRand.Initialize( DateInSeconds );

	e_team = SRand.FRand() >= 0.5f ? ETeamEnum::Team_Blue : ETeamEnum::Team_Red;
}

void APlayerShip::BeginPlay()
{
	Super::BeginPlay();

	TimeOfNextLaser = GetWorld()->GetTimeSeconds();	

	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		APlayerController* PlayerController = GEngine->GetFirstLocalPlayerController( GetWorld() );

		if (nullptr != PlayerController && nullptr != PlayerController->MyHUD)
		{
			PlayerController->MyHUD->AddPostRenderedActor( this );
		}
	}
}

void APlayerShip::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	//SphereMesh->SetWorldLocation( CockpitMesh->GetSocketLocation( MuzzleAttachPoint ) );

	UpdateRotation( DeltaTime );

}

void APlayerShip::PostRenderFor( APlayerController *PC, UCanvas *Canvas, FVector CameraPosition, FVector CameraDir )
{

	if (nullptr != PlayerState && nullptr != PC && PC->GetViewTarget() != this && (GetWorld()->TimeSeconds - GetLastRenderTime() < 0.5f) && FVector::DotProduct( CameraDir, (GetActorLocation() - CameraPosition) ) > 0.0f)
	{
		FVector ScreenLocation;

		if (PC->ProjectWorldLocationToScreenWithDistance( GetActorLocation(), ScreenLocation ))
		{
			for (auto &TargetBox : TargetBoxes)
			{
				DrawTargetBox( ScreenLocation, TargetBox, Canvas );
			}
		}
	}

	if (IsLocallyControlled())
	{
		if (bFiring)
		{
			TriggerPrimary();
		}
	}
}

void APlayerShip::DrawTargetBox( const FVector &ScreenLocation, const FTargetBox &TargetBox, UCanvas *Canvas )
{
	float ClampedZ = FMath::Clamp( ScreenLocation.Z, TargetBox.NearDist, TargetBox.FarDist );

	float BaseLineSize = TargetBox.Width * TargetBox.BaseLineRatio;



	//GEngine->AddOnScreenDebugMessage( 1, 5.f, FColor::Yellow, FString::SanitizeFloat(1.f - TargetBox.BaseLineRatio) );
	//GEngine->AddOnScreenDebugMessage( 2, 5.f, FColor::Yellow, FString::SanitizeFloat( TargetBox.FarDist ) );
	//GEngine->AddOnScreenDebugMessage( 3, 5.f, FColor::Yellow, FString::SanitizeFloat( ClampedZ) );

	float HorizontalLineSize, VerticalLineSize;

	if (TargetBox.Type == ETargetBoxType::Target_EdgeNoGrow)
	{
		HorizontalLineSize = BaseLineSize;
		VerticalLineSize = TargetBox.Height;
	}
	else if (TargetBox.Type == ETargetBoxType::Target_EdgeGrow)
	{
		float DeltaHorizontalLineSize = TargetBox.Width * (TargetBox.FarDist - ClampedZ) / TargetBox.FarDist * (1.f - TargetBox.BaseLineRatio);
		HorizontalLineSize = BaseLineSize + DeltaHorizontalLineSize;
		VerticalLineSize = TargetBox.Height;
	}
	else if (TargetBox.Type == ETargetBoxType::Target_CornerNoGrow)
	{
		HorizontalLineSize = VerticalLineSize = BaseLineSize;
	}
	else
	{
		float LR = (TargetBox.FarDist - ClampedZ) / TargetBox.FarDist * (1.f - TargetBox.BaseLineRatio);
		float DeltaHorizontalLineSize = TargetBox.Width * LR;
		float DeltaVerticalLineSize = TargetBox.Height * LR;
		HorizontalLineSize = BaseLineSize + DeltaHorizontalLineSize;
		VerticalLineSize = BaseLineSize + DeltaVerticalLineSize;
	}

	float Left = ScreenLocation.X - TargetBox.Width;
	float Right = ScreenLocation.X + TargetBox.Width;
	float Top = ScreenLocation.Y - TargetBox.Height;
	float Bottom = ScreenLocation.Y + TargetBox.Height;




	FCanvasLineItem TargetBoxLines[8] =
	{
		// Top Left
		FCanvasLineItem( FVector2D( Left, Top ), FVector2D( Left + HorizontalLineSize, Top ) ),
		FCanvasLineItem( FVector2D( Left, Top ), FVector2D( Left , Top + VerticalLineSize ) ),

		// Top Right
		FCanvasLineItem( FVector2D( Right, Top ), FVector2D( Right - HorizontalLineSize, Top ) ),
		FCanvasLineItem( FVector2D( Right, Top ), FVector2D( Right, Top + VerticalLineSize ) ),

		// Bottom Left
		FCanvasLineItem( FVector2D( Left, Bottom ), FVector2D( Left + HorizontalLineSize, Bottom ) ),
		FCanvasLineItem( FVector2D( Left, Bottom ), FVector2D( Left, Bottom - VerticalLineSize ) ),

		// Bottom Right
		FCanvasLineItem( FVector2D( Right, Bottom ), FVector2D( Right - HorizontalLineSize, Bottom ) ),
		FCanvasLineItem( FVector2D( Right, Bottom ), FVector2D( Right, Bottom - VerticalLineSize ) )
	};

	if (TargetBox.Font)
	{
		FCanvasTextItem TextItem( FVector2D( ScreenLocation.X, Bottom + BaseLineSize ), FText::FromString( FString::SanitizeFloat( ScreenLocation.Z ) ), TargetBox.Font, TargetBox.Color );

		Canvas->DrawItem( TextItem );
	}


	for (int i = 0; i < 8; ++i)
	{
		TargetBoxLines[i].LineThickness = TargetBox.LineThickness;
		TargetBoxLines[i].SetColor( TargetBox.Color );
		Canvas->DrawItem( TargetBoxLines[i] );
		GetWorld();

	}
}

void APlayerShip::UpdateRotation( float DeltaTime )
{
	if (IsLocallyControlled())
	{
		FQuat AppendRotation( FRotator( CurrPitchSpeed, CurrYawSpeed, CurrRollSpeed ) );
		AddActorLocalRotation( AppendRotation );
		Rotation = GetActorRotation().Quaternion();
		OnRep_Rotation();
	}
	else if (Rotation != LastRotation)
	{
		//SetActorRotation( Rotation );
	}
}

void APlayerShip::OnRep_Rotation()
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		Server_ReportRotation( Rotation );
	}
}

bool APlayerShip::Server_ReportRotation_Validate( const FQuat& NewRotation )
{
	return true;
}

void APlayerShip::Server_ReportRotation_Implementation( const FQuat& NewRotation )
{
	LastRotation = Rotation;
	Rotation = NewRotation;
}

void APlayerShip::SetupPlayerInputComponent( class UInputComponent* InputComponent )
{
	Super::SetupPlayerInputComponent( InputComponent );


	//InputComponent->BindAxis( "Forward_Backward", this, &APlayerShip::MoveForwardBackward );

	/*
	InputComponent->BindAxis( "Look_Left_Right", this, &APlayerShip::LookLeftRight ); // Pitch
	InputComponent->BindAxis( "Look_Up_Down", this, &APlayerShip::LookUpDown ); // Yaw
	InputComponent->BindAxis( "Strafe_Left_Right", this, &APlayerShip::StrafeLeftRight );
	InputComponent->BindAxis( "Roll_Left_Right", this, &APlayerShip::RollLeftRight );
	*/

	InputComponent->BindAction( "Fire", IE_Pressed, this, &APlayerShip::HoldTriggerPrimary );
	InputComponent->BindAction( "Fire", IE_Released, this, &APlayerShip::ReleaseTriggerPrimary );
}

void APlayerShip::MoveForwardBackward( float MovementAmount )
{

	if (0.f != MovementAmount)
	{
		AddMovementInput( GetActorForwardVector(), MovementAmount );
	}
}

void APlayerShip::HoldTriggerPrimary()
{
	bFiring = true;
	TriggerPrimary();
}

void APlayerShip::TriggerPrimary()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	GEngine->AddOnScreenDebugMessage( 8, 5.f, FColor::Green, FString::SanitizeFloat( CurrentTime ) );
	GEngine->AddOnScreenDebugMessage( 9, 5.f, FColor::Red, FString::SanitizeFloat( TimeOfNextLaser ) );

	if (TimeOfNextLaser < CurrentTime)
	{
		GEngine->AddOnScreenDebugMessage( 4, 5.f, FColor::Cyan, "LaserTime" );
		
		TimeOfNextLaser = CurrentTime + TimeBetweenLasers;

		switch (Laser->e_LaserMode)
		{
			case ELaserModeEnum::LME_Single:
			{
				ShootLaser( 2 );
				break;
			}
			case ELaserModeEnum::LME_Double:
			{
				ShootLaser( 0 );
				ShootLaser( 1 );
				break;
			}
			case ELaserModeEnum::LME_Burst:
			{
				ShootLaser( 0 );
				ShootLaser( 1 );
				ShootLaser( 2 );
				TimeOfNextLaser = CurrentTime + TimeBetweenLasersIfBursting;
				break;
			}
			case ELaserModeEnum::LME_Scatter:
			{
				ShootLaser( FMath::Rand() % 3 );
				break;
			}
		}
	}
}

void APlayerShip::ReleaseTriggerPrimary()
{
	bFiring = false;

	if (MuzzleParticles != nullptr)
	{
		MuzzleParticles->DeactivateSystem();
		MuzzleParticles = nullptr;
	}
}

void APlayerShip::ShootLaser(int Muzzle)
{
	if (bFiring)
	{
		/*
		TimeOfLastFire = GetWorld()->GetTimeSeconds();

		FVector LaserOrigin;
		FRotator LaserRotation;

		Controller->GetPlayerViewPoint( LaserOrigin, LaserRotation );

		FVector LaserDirection = LaserRotation.Vector();
		LaserOrigin += LaserDirection * RelativeMuzzleLocation.X + CockpitMesh->GetUpVector() * RelativeMuzzleLocation.Z;

		if (MuzzleParticles == nullptr)
		{
			MuzzleParticles = UGameplayStatics::SpawnEmitterAttached( MuzzleFlash, CockpitMesh, MuzzleAttachPoint );
			MuzzleParticles->SetRelativeScale3D( FVector( 0.02f, 0.02f, 0.02f ) );
			MuzzleParticles->bOwnerNoSee = false;
			MuzzleParticles->bOnlyOwnerSee = true;
		}

		*/
		const FVector AimDir = GetAim();
		const FVector CameraPos = GetCameraDamageStartLocation(AimDir);
		const FVector EndPos = CameraPos + (AimDir * LaserRange);

		FHitResult Impact = WeaponTrace( CameraPos, EndPos );
		GEngine->AddOnScreenDebugMessage( 4, 5.f, FColor::Cyan, Impact.ToString() );

		if (CockpitMesh)
		{
			const FVector MuzzleOrigin = CockpitMesh->GetSocketLocation( MuzzleAttachPoints[Muzzle] );

			FVector AdjustedAimDir = AimDir;

			if (Impact.bBlockingHit)
			{
				AdjustedAimDir = (Impact.ImpactPoint - MuzzleOrigin).GetSafeNormal();
				Impact = WeaponTrace( MuzzleOrigin, MuzzleOrigin + (AdjustedAimDir * LaserRange) );
				GEngine->AddOnScreenDebugMessage( 4, 5.f, FColor::Cyan, Impact.ToString() );
			}
			else
			{
				Impact.ImpactPoint = FVector_NetQuantize( EndPos );
			}

			ProcessInstantHit( Impact, MuzzleOrigin, AdjustedAimDir, Muzzle );
		}
		
		
		//ServerShootLaser();
		//const float CurrentTime = GetWorld()->GetTimeSeconds();
		//float TimeOfNextFire = TimeOfLastFire + TimeBetweenLasers - CurrentTime;
		//GetWorldTimerManager().SetTimer( LaserTriggerTimerHandle, this, &APlayerShip::ShootLaser, TimeOfNextFire, false );
	}
}

void APlayerShip::ProcessInstantHit( const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int Muzzle )
{
	if (IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		if (Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority)
		{
			ServerNotifyHit( Impact, ShootDir, Muzzle);
		}
		else if (Impact.GetActor() == nullptr)
		{
			if (Impact.bBlockingHit)
			{
				ServerNotifyHit( Impact, ShootDir, Muzzle );
			}
			else
			{
				ServerNotifyMiss( ShootDir, Muzzle );
			}
		}
	}

	ProcessInstantHitConfirmed( Impact, Origin, ShootDir, Muzzle );
}

void APlayerShip::ProcessInstantHitConfirmed( const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int Muzzle )
{
	if (ShouldDealDamage( Impact.GetActor() ))
	{
		DealDamage( Impact, ShootDir );
	}

	if (Role == ROLE_Authority)
	{
		HitImpactNotify = Impact.ImpactPoint;
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		SimulateInstantHit( Impact.ImpactPoint, Muzzle);
	}
}

void APlayerShip::OnRep_HitLocation()
{
	//SimulateInstantHit( HitImpactNotify );
}

void APlayerShip::SimulateInstantHit( const FVector& ImpactPoint, int Muzzle )
{
	const FVector MuzzleOrigin = CockpitMesh->GetSocketLocation( MuzzleAttachPoints[Muzzle] );

	/* Adjust direction based on desired crosshair impact point and muzzle location */
	const FVector AimDir = (ImpactPoint - MuzzleOrigin).GetSafeNormal();

	const FVector EndTrace = MuzzleOrigin + (AimDir * LaserRange);
	const FHitResult Impact = WeaponTrace( MuzzleOrigin, EndTrace );

	if (Impact.bBlockingHit)
	{
		//SpawnImpactEffects( Impact );
		SpawnTrailEffects( Impact.ImpactPoint, Muzzle );
	}
	else
	{
		SpawnTrailEffects( EndTrace, Muzzle );
	}
	
}

void APlayerShip::SpawnTrailEffects( const FVector& EndPoint, int Muzzle )
{
	const FVector Origin = CockpitMesh->GetSocketLocation( MuzzleAttachPoints[Muzzle] );
	FVector ShootDir = EndPoint - Origin;

	if (LaserTracerFX)
	{
		if (IsLocallyControlled())
		{
			ShootDir = ShootDir.GetSafeNormal();
			//UGameplayStatics::SpawnEmitterAttached( LaserTracerFX, CockpitMesh, MuzzleAttachPoints[Muzzle] );
			SphereMesh->SetWorldLocation( EndPoint );
			//GEngine->AddOnScreenDebugMessage( 1, 5.f, FColor::Yellow, FString::SanitizeFloat(FVector::Dist( Origin, EndPoint )) );
			Laser->ShowNew( Rotation.UnrotateVector( ShootDir ), FVector::Dist( Origin, EndPoint ), Muzzle );
		}
		else if(LaserTrailFX)
		{
			UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation( this, LaserTrailFX, Origin );
			if (TrailPSC)
			{
				GEngine->AddOnScreenDebugMessage( 2, 5.f, FColor::Red, "spawned an emitter, non-local." );
				TrailPSC->SetVectorParameter( LaserTrailTargetParam, EndPoint );
			}
		}
	}
		
}


bool APlayerShip::ShouldDealDamage( AActor* TestActor ) const
{
	if (TestActor)
	{
		if (GetNetMode() != NM_Client ||
			TestActor->Role == ROLE_Authority ||
			TestActor->bTearOff)
		{
			return true;
		}
	}

	return false;
}

void APlayerShip::DealDamage( const FHitResult& Impact, const FVector& ShootDir )
{
	float ActualHitDamage = LaserHitDamage;

	/* Handle special damage location on the zombie body (types are setup in the Physics Asset of the zombie */
	UDamageType* DmgType = Cast<UDamageType>( DamageType->GetDefaultObject() );

	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = ActualHitDamage;

	Impact.GetActor()->TakeDamage( PointDmg.Damage, PointDmg, GetController(), this );
}

bool APlayerShip::ServerNotifyHit_Validate( const FHitResult Impact, FVector_NetQuantizeNormal ShootDir, int Muzzle )
{
	return true;
}


void APlayerShip::ServerNotifyHit_Implementation( const FHitResult Impact, FVector_NetQuantizeNormal ShootDir, int Muzzle )
{
	if (Instigator && (Impact.GetActor() || Impact.bBlockingHit))
	{
		const FVector Origin = CockpitMesh->GetSocketLocation( MuzzleAttachPoints[Muzzle] );
		const FVector ViewDir = (Impact.Location - Origin).GetSafeNormal();

		const float ViewDotHitDir = FVector::DotProduct( Instigator->GetViewRotation().Vector(), ViewDir );
		if (ViewDotHitDir > AllowedViewDotHitDir)
		{
			if (Impact.GetActor() == nullptr)
			{
				if (Impact.bBlockingHit)
				{
					ProcessInstantHitConfirmed( Impact, Origin, ShootDir, Muzzle );
				}
			}

			else if (Impact.GetActor()->IsRootComponentStatic() || Impact.GetActor()->IsRootComponentStationary())
			{
				ProcessInstantHitConfirmed( Impact, Origin, ShootDir, Muzzle );
			}
			else
			{
				const FBox HitBox = Impact.GetActor()->GetComponentsBoundingBox();

				FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);
				BoxExtent *= ClientSideHitLeeway;

				BoxExtent.X = FMath::Max( 20.0f, BoxExtent.X );
				BoxExtent.Y = FMath::Max( 20.0f, BoxExtent.Y );
				BoxExtent.Z = FMath::Max( 20.0f, BoxExtent.Z );

				const FVector BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;

				// If we are within client tolerance
				if (FMath::Abs( Impact.Location.Z - BoxCenter.Z ) < BoxExtent.Z &&
					FMath::Abs( Impact.Location.X - BoxCenter.X ) < BoxExtent.X &&
					FMath::Abs( Impact.Location.Y - BoxCenter.Y ) < BoxExtent.Y)
				{
					ProcessInstantHitConfirmed( Impact, Origin, ShootDir, Muzzle);
				}
			}
		}
	}
}

bool APlayerShip::ServerNotifyMiss_Validate( FVector_NetQuantizeNormal ShootDir, int Muzzle )
{
	return true;
}


void APlayerShip::ServerNotifyMiss_Implementation( FVector_NetQuantizeNormal ShootDir, int Muzzle )
{
	const FVector Origin = CockpitMesh->GetSocketLocation( MuzzleAttachPoints[Muzzle] );
	const FVector EndTrace = Origin + (ShootDir * LaserRange);

	// Play on remote clients
	HitImpactNotify = EndTrace;

	if (GetNetMode() != NM_DedicatedServer)
	{
		SpawnTrailEffects( EndTrace, Muzzle );
	}
}

FHitResult APlayerShip::WeaponTrace( const FVector& TraceFrom, const FVector& TraceTo ) const
{
	FCollisionQueryParams TraceParams( TEXT( "WeaponTrace" ), true, Instigator );
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit( ForceInit );
	GetWorld()->LineTraceSingleByChannel( Hit, TraceFrom, TraceTo, COLLISION_INSTANT, TraceParams );

	return Hit;
}

const FVector APlayerShip::GetCameraDamageStartLocation( const FVector &AimDir ) const
{
	APlayerController* PC = Cast<APlayerController>( GetController() );
	FVector OutStartTrace = FVector::ZeroVector;

	if (PC)
	{
		FRotator DummyRot;
		PC->GetPlayerViewPoint( OutStartTrace, DummyRot );

		OutStartTrace = OutStartTrace + AimDir * (FVector::DotProduct( (Instigator->GetActorLocation() - OutStartTrace), AimDir ));
	}

	return OutStartTrace;
}

const FVector APlayerShip::GetAim()
{
	APlayerController* const PC = Instigator ? Cast<APlayerController>( Instigator->Controller ) : nullptr;
	FVector FinalAim = FVector::ZeroVector;

	if (false && UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		FVector DevicePosition;
		FRotator DeviceRotation;
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition( DeviceRotation, DevicePosition );

		FinalAim = GetReticlePosition() - DevicePosition;
		FinalAim.Normalize();
	}
	else if (PC)
	{
		
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint( CamLoc, CamRot );


		FinalAim = GetReticlePosition() - CamLoc;
		FinalAim.Normalize();

		GEngine->AddOnScreenDebugMessage( 3, 5.f, FColor::Blue, FinalAim.ToString() );
	}
	else if (Instigator)
	{
		FinalAim = Instigator->GetBaseAimRotation().Vector();
	}

	return FinalAim;
}

const FVector APlayerShip::GetReticlePosition()
{
	return Reticle->GetComponentLocation();
}

bool APlayerShip::ServerShootLaser_Validate()
{
	return true;
}
void APlayerShip::ServerShootLaser_Implementation()
{
	TimeOfLastFire = GetWorld()->GetTimeSeconds();

	FVector LaserOrigin;
	FRotator LaserRotation;
	Controller->GetPlayerViewPoint( LaserOrigin, LaserRotation );

	FVector LaserDirection = LaserRotation.Vector();
	LaserOrigin += LaserDirection * RelativeMuzzleLocation.X + CockpitMesh->GetUpVector() * RelativeMuzzleLocation.Z;

}


float APlayerShip::TakeDamage( float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser )
{
	ServerTakeDamage( Damage, DamageEvent, EventInstigator, DamageCauser );
	return 0;
}

bool APlayerShip::ServerTakeDamage_Validate( float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser )
{
	return true;
}

void APlayerShip::ServerTakeDamage_Implementation( float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser )
{
	if (Health <= 0.f)
	{
		return;
	}

	const float ActualDamage = Super::TakeDamage( Damage, DamageEvent, EventInstigator, DamageCauser );

	if (ActualDamage > 0.f)
	{
		Health -= ActualDamage;
		if (Health <= 0)
		{
			GEngine->AddOnScreenDebugMessage( 1, 5.f, FColor::Yellow, "someone just took a dirt nap." );
		}
	}
}

const FVector APlayerShip::GetMuzzleLocation( int Muzzle ) const
{
	return CockpitMesh->GetSocketLocation( MuzzleAttachPoints[Muzzle] );
}

void APlayerShip::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME( APlayerShip, Health );
	DOREPLIFETIME( APlayerShip, Rotation );
	DOREPLIFETIME_CONDITION( APlayerShip, HitImpactNotify, COND_SkipOwner );
}



