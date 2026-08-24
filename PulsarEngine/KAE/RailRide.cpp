#include <kamek.hpp>
#include <runtimeWrite.hpp>
#include <core/egg/Math/Math.hpp>
#include <core/egg/Math/Vector.hpp>
#include <core/egg/Math/Quat.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Kart/KartStatus.hpp>
#include <MarioKartWii/Input/InputState.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KMPAREAExpander.hpp>
/*
// Rail Riding [BlueLeopard]
// Pretty cool stuff, but has never been integrated into a track correctly so no idea if it still works with higher speeds.
namespace MKWG {
namespace Race {

void RailRideRotation(Kart::Physics& physics, u8 playerId) {
    Vec3 speed = physics.speed;
    speed.Normalize();
    Vec3 forward = GetForward(physics.mainRot);

    Quat correction;
    correction.MakeVectorRotation(forward, speed);
    correction.Normalise();
    Quat targetRot = MultiplyQuat(correction, physics.mainRot);
    targetRot.Normalise();
    physics.mainRot.SlerpTo(targetRot, physics.mainRot, physics.stabilizationFactor);
}

void RailRideMovements() {
    // find correct routeId
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    for (u16 i = 0; i < kmp->potiSection->pointCount; i++) {
        KMP::Holder<POTI>* holder = kmp->potiSection->holdersArray[i];
        if (!holder || !holder->raw) continue;
        if (kmp->areaSection->GetHolder(KAE.Railride)->raw->routeId == holder->raw->id) {
    // teleport to closest spot on route (not closest point, closest spot. Spot perpendicular to vehicles location)
    // rotate to face next point when just starting on the route and every time a new point is hit (slowly)
    // aquire the correct speed
}

void RailRideEject(Input::State& inputState, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if ((KAE.mtBoost || KAE.mushroom) && inputState.motionControlFlick != 0 && KAE.waitBeforeJump == 0) {
        status.trickableTimer = 0x4;
        KAE.jumped = KAE.jumpedFast = true;
        KAE.onRail = false;
        KAE.waitBeforeJump = 100;
        KAE.height = 1;
        KAE.railRideJumpDir = KAE.xInput;
    } 
    else if (inputState.motionControlFlick != 0 && KAE.waitBeforeJump == 0) {
        status.trickableTimer = 0x4;
        KAE.jumped = KAE.jumpedSlow = true;
        KAE.onRail = false;
        KAE.waitBeforeJump = 100;
        KAE.height = 1;
        KAE.railRideJumpDir = KAE.xInput;
    }
    if (KAE.waitBeforeJump > 0) KAE.waitBeforeJump -= 1;
}

void RailRideJump(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    // height
    Vec3 up = GetUp(physics.mainRot);

    if (KAE.jumped && KAE.height >= 1) KAE.height += 1;
    if (KAE.height > 9) KAE.height = 0;
    if (KAE.height > 0) physics.speed0 = AddVec(physics.speed0, MultiplyVecByFloat(up, 9 - KAE.height));

    if (KAE.jumpedSlow) movement.engineSpeed = 85;
    if (KAE.jumpedFast) movement.engineSpeed = 120;
    
    if (KAE.ground && KAE.waitBeforeJump < 90) {
        KAE.jumped = KAE.jumpedSlow = KAE.jumpedFast = false;
        KAE.height = 0;
    }

    // direction
    if (KAE.jumped) {
        Vec3 right = GetRight(physics.mainRot);

        Vec3 sideVel;
        sideVel.x = right.x * -KAE.railRideJumpDir * 50.0f;
        sideVel.z = right.z * -KAE.railRideJumpDir * 50.0f;
        sideVel.y = right.y * -KAE.railRideJumpDir * 50.0f;

        physics.speed2.x += sideVel.x;
        physics.speed2.z += sideVel.z;
        physics.speed2.y += sideVel.y;
    }
}

void RailRidePhysics(Kart::Physics& physics) {
    physics.engineSpeed = Vec3(0,0,0);
}

} // Race
} // MKWG*/