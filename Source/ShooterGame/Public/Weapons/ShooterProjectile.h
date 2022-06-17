// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "ShooterProjectileData.h"
#include "ShooterProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

// 
UCLASS(Abstract, Blueprintable)
class AShooterProjectile : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	/** initial setup */
	virtual void PostInitializeComponents() override;

	/** setup velocity */
	UFUNCTION(BlueprintCallable)
	void InitVelocity(const FVector& ShootDirection, const FVector& AddedVelocity);

	/** handle hit */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);
	
	/** AActor::OnDestroyed's event callback. This will set off the explosion when bExplodeOnImpact is false. */
	UFUNCTION()
	void OnBeingDestroyed(AActor* Actor);

	/** Called from AShooterWeapon_Projectile to Apply its damage data to this projectile. */
	void ApplyWeaponConfig(const FProjectileWeaponData& Config);

private:
	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	UProjectileMovementComponent* MovementComp;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	UParticleSystemComponent* ParticleComp;

	/** A delay time in seconds before this object being destroy after explode. */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float DelayDestroy;
protected:

	/** effects for explosion */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<class AShooterExplosionEffect> ExplosionTemplate;

	/** controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** projectile data */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	struct FProjectileWeaponData WeaponConfig;

	/** If set, the projectile will immediately explode upon impact;
		otherwise, it will only explode the moment it's expired. */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	bool bExplodeOnImpact;

	/** did it explode? */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	/** A hit result cached from the last OnImpact. This is used for the delayed explosion. */
	FHitResult CachedHitResult;

	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

	/** trigger explosion */
	void Explode(const FHitResult& Impact);

	/** shutdown projectile and prepare for destruction */
	void DisableAndDestroy();

	/** update velocity on client */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;

protected:
	/** Returns MovementComp subobject **/
	FORCEINLINE UProjectileMovementComponent* GetMovementComp() const { return MovementComp; }
	/** Returns CollisionComp subobject **/
	FORCEINLINE USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ParticleComp subobject **/
	FORCEINLINE UParticleSystemComponent* GetParticleComp() const { return ParticleComp; }
};
