// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Weapons/ShooterProjectile.h"
#include "Particles/ParticleSystemComponent.h"
#include "Effects/ShooterExplosionEffect.h"

AShooterProjectile::AShooterProjectile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CollisionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = CollisionComp;

	ParticleComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticleComp"));
	ParticleComp->bAutoActivate = false;
	ParticleComp->bAutoDestroy = false;
	ParticleComp->SetupAttachment(RootComponent);

	MovementComp = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 2000.0f;
	MovementComp->MaxSpeed = 2000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicatingMovement(true);

	DelayDestroy = 2.0f;
	bExplodeOnImpact = true;
}

void AShooterProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MovementComp->OnProjectileStop.AddDynamic(this, &AShooterProjectile::OnImpact);
	CollisionComp->MoveIgnoreActors.Add(GetInstigator());

	SetLifeSpan( WeaponConfig.ProjectileLife );
	MyController = GetInstigatorController();

	OnDestroyed.AddDynamic(this, &AShooterProjectile::OnBeingDestroyed);
}

void AShooterProjectile::InitVelocity(const FVector& ShootDirection, const FVector& AddedVelocity)
{
	if (MovementComp)
	{
		MovementComp->Velocity = (ShootDirection * MovementComp->InitialSpeed) + AddedVelocity;
	}
}

void AShooterProjectile::OnImpact(const FHitResult& HitResult)
{
	if (bExplodeOnImpact)
	{
		if (GetLocalRole() == ROLE_Authority && !bExploded)
		{
			Explode(HitResult);
			DisableAndDestroy();
		}
	}
	else
	{
		CachedHitResult = HitResult;
	}
}

void AShooterProjectile::OnBeingDestroyed(AActor* Actor)
{
	if (!bExplodeOnImpact && GetLocalRole() == ROLE_Authority && !bExploded)
	{
		if (CachedHitResult.Normal == FVector::ZeroVector && MovementComp)
		{
			// The projectile explodes mid-air.
			CachedHitResult.Normal = MovementComp->Velocity.GetSafeNormal();
			CachedHitResult.ImpactPoint = GetActorLocation();
		}
		Explode(CachedHitResult);
		DisableAndDestroy();
	}
}

void AShooterProjectile::Explode(const FHitResult& Impact)
{
	if (ParticleComp)
	{
		ParticleComp->Deactivate();
	}

	// effects and damage origin shouldn't be placed inside mesh at impact point
	const FVector NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;

	if (WeaponConfig.ExplosionDamage > 0 && WeaponConfig.ExplosionRadius > 0 && WeaponConfig.DamageType)
	{
		UGameplayStatics::ApplyRadialDamage(this, WeaponConfig.ExplosionDamage, NudgedImpactLocation, WeaponConfig.ExplosionRadius, WeaponConfig.DamageType, TArray<AActor*>(), this, MyController.Get());
	}

	if (ExplosionTemplate)
	{
		FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), NudgedImpactLocation);
		AShooterExplosionEffect* const EffectActor = GetWorld()->SpawnActorDeferred<AShooterExplosionEffect>(ExplosionTemplate, SpawnTransform);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
		}
	}

	bExploded = true;
}

void AShooterProjectile::DisableAndDestroy()
{
	UAudioComponent* ProjAudioComp = FindComponentByClass<UAudioComponent>();
	if (ProjAudioComp && ProjAudioComp->IsPlaying())
	{
		ProjAudioComp->FadeOut(0.1f, 0.f);
	}

	MovementComp->StopMovementImmediately();

	if (DelayDestroy > 0)
	{
		// give clients some time to show explosion
		SetLifeSpan(DelayDestroy);
	}
	else
	{
		Destroy();
	}
}

///CODE_SNIPPET_START: AActor::GetActorLocation AActor::GetActorRotation
void AShooterProjectile::OnRep_Exploded()
{
	FVector ProjDirection = GetActorForwardVector();

	const FVector StartTrace = GetActorLocation() - ProjDirection * 200;
	const FVector EndTrace = GetActorLocation() + ProjDirection * 150;
	FHitResult Impact;
	
	if (!GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_PROJECTILE, FCollisionQueryParams(SCENE_QUERY_STAT(ProjClient), true, GetInstigator())))
	{
		// failsafe
		Impact.ImpactPoint = GetActorLocation();
		Impact.ImpactNormal = -ProjDirection;
	}

	Explode(Impact);
}
///CODE_SNIPPET_END

void AShooterProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (MovementComp)
	{
		MovementComp->Velocity = NewVelocity;
	}
}

void AShooterProjectile::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	
	DOREPLIFETIME( AShooterProjectile, bExploded );
}

void AShooterProjectile::ApplyWeaponConfig(const FProjectileWeaponData& Config)
{
	WeaponConfig = Config;
}
