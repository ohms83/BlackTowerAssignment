# Black Tower's Shooter Game Assignment
I assumed that this test is aiming for my knowledge of Unreal's network and multiplayer system so I make sure that the added codes are functioning on both multiplayer and standalone games.

## My Approach
Noticing that the `AShooterProjectile` class already has almost everything I need to achieve the goal, I based my codes from there. My principal is prioritizing __reusability__ over everything. So, if there are any codes ready for me to use, I will just use them. With that in mind, I also develop my own classes by reducing the redundancies as much as possible.

## Development Environment
- Windows 11 Home, Windows 10 Home
- Unreal Engine 4.27.2
- Microsoft Visual Studio 2022 (already tested with the version 2019)

## Control
Press 'G' button to throw the grenade.

## Change List
### PlayerPawn BP
- Added a new `ThrowGrenade` and `ServerThrowGrenade` functions.
- Handles `Grenade` input event.
### ProjGrenade
- Introduced a new projectile class representing the flag grenade.
- Added a `StaticMeshComponent` for the visual
- Changed `Explosion Effect` to `ProjGrenade_Explosion`
- Simulating rotation (can be turned on/off)
### ProjGrenade_Explosion
- Introduced a new explosion effect class
- `Explosion FX` parameter to use the alternative explosion effect (from the Unreal Marketplaec).
### ShooterProjectileData
- Moved `FProjectileWeaponData` out to the new header file (reduced cyclic redundancy).
### ShooterWeapon_Projectile
- Moved `ApplyWeaponConfig` to `AShooterProjectile` class. I see no reason this should be here since it's confusing and makes more sense to be there (again, it introduced a cyclic redundancy).
### ShooterProjectile
- Modified `WeaponConfig` to be editable from the Blueprint
- Introduced `bExplodeOnImpact` flag. I believe the flag grenade shouldn't explode upon impact and should be time-based instead.
- Added `OnBeingDestroyed` to handle the `OnDestroyed` event. This will trigger the explosion in case `bExplodeOnImpact` is set to `false`.
- Modified `OnImpact` to function according to `bExplodeOnImpact` flag.
- Introduced `CachedHitResult` to be used from `OnBeingDestroyed`.
- Introduced the `DelayDestroy` parameter to be used by the `DisableAndDestroy` function.
- `DisableAndDestroy` to immediately destroy the actor in case the `DelayDestroy` is zero.
### ShooterGame_TeamDeathMatch
- During play testing, I noticed that the grenade doesn't deal damage to allies. Worrying that this might fail the assignment, I introduced the `bAllowFriendlyFire` to the class and have it turn off by default.
### GameMode_TeamDeathMatch
- Introduced a new Team Deathmatch game mode that has friendly fire turned on.

## Time Spent
- 1st Day: Took 6 hours to read and understand the project's structures. and UE's network system.
- 2nd Day: Took 4 hours to develop the prototype.
- 3rd Day: Took 4 hours to finish up and debugging.
- 4th Day: Took 4 hours to clean up the codes and implement the Friendly Fire mode.

## Assets
- Grenade: https://free3d.com/3d-model/grenade-443268.html
- Explosion Effect: https://www.unrealengine.com/marketplace/en-US/product/realistic-starter-vfx-pack-vol

## Repository
- https://github.com/ohms83/BlackTowerAssignment.git (currently private)