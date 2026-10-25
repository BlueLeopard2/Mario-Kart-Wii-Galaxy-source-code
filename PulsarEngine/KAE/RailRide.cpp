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
#include <core/rvl/OS/OS.hpp>

// Rail Riding [BlueLeopard]

namespace MKWG {
namespace Race {

void RailRideEject(Kart::Movement& movement, u8 playerId) {   
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    movement.engineSpeed = KAE.baseSpeed;
    KAE.isRailRiding = false;
}

void DetermineNextPoint(Kart::Physics& physics, Kart::Movement& movement, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    for (u16 i = 0; i < kmp->potiSection->pointCount; i++) {
        // start of nextPoint is already detemined in RailRideStart
        Vec3 distance = kmp->potiSection->GetHolder(KAE.routeId)->raw->points[KAE.nextPoint].position - physics.position;
        float length = VecLength(distance);

        if (length < 250.0f) {
            KAE.nextPoint +=1;  
        }
        if (KAE.nextPoint > kmp->potiSection->GetHolder(KAE.routeId)->raw->pointCount) RailRideEject(movement, playerId);   
    }
}

void RailRideStart(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    for (u16 i = 0; i < kmp->areaSection->pointCount; i++) {
        if (kmp->FindAREA(physics.position, i, (u8)0x13) != i) continue;
    
        KAE.routeId = kmp->areaSection->GetHolder(i)->raw->routeId;

        if (!kmp->potiSection->GetHolder(KAE.routeId)) return;
        float vecLength[255]; // this is not optimal but does the job
        
        for (u8 j = 0; j < kmp->potiSection->GetHolder(KAE.routeId)->raw->pointCount; j++) { // find all points on the route
            Vec3 distance = physics.position - kmp->potiSection->GetHolder(KAE.routeId)->raw->points[j].position;
            vecLength[j] = VecLength(distance);
        }
        u8 nearestIndex;
        u8 secondNearestIndex;

        FindSmallest2(vecLength, kmp->potiSection->GetHolder(KAE.routeId)->raw->pointCount, nearestIndex, secondNearestIndex);

        Vec3 positionNearest = kmp->potiSection->GetHolder(KAE.routeId)->raw->points[nearestIndex].position;
        Vec3 positionSecondNearest = kmp->potiSection->GetHolder(KAE.routeId)->raw->points[secondNearestIndex].position;
        Vec3 closestPoint = FindClosestPoint(positionNearest, positionSecondNearest, physics.position); // Math for finding closest spot
        Vec3 distance = SubVec(physics.position, closestPoint);
        float distanceSquared = distance.x * distance.x + distance.y * distance.y + distance.z * distance.z;
        if (distanceSquared < 300.0f * 300.0f) {
            KAE.isRailRiding = true;
            KAE.nextPoint = Max(nearestIndex, secondNearestIndex);
        }
    }
}

void RailRideStop(Kart::Movement& movement, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    
    if (KAE.oob || !KAE.ground) RailRideEject(movement, playerId);
}

void RailRidePhysics(Kart::Physics& physics, Kart::Movement& movement, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    physics.engineSpeed = Vec3(0,0,0);
    movement.engineSpeed = 0.0f;

    Vec3 target = kmp->potiSection->GetHolder(KAE.routeId)->raw->points[KAE.nextPoint].position;

    Vec3 desiredDir = target - physics.position;
    desiredDir.y = 0.0f;
    desiredDir.Normalize();

    Vec3 floorNormal = KAE.floorNormal;

    Vec3 forward = desiredDir - MultiplyVecByFloat(floorNormal, Dot(desiredDir, floorNormal));

    forward.Normalize();

    physics.speed2 = AddVec(physics.speed2, MultiplyVecByFloat(forward, KAE.baseSpeed));
}

void RailRideRotation(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    Vec3 target = kmp->potiSection->GetHolder(KAE.routeId)->raw->points[KAE.nextPoint].position;

    Vec3 desiredDir = target - physics.position;
    desiredDir.y = 0.0f;
    desiredDir.Normalize();

    Vec3 floorNormal = KAE.floorNormal;

    Vec3 dir = desiredDir - MultiplyVecByFloat(floorNormal, Dot(desiredDir, floorNormal));

    dir.Normalize();

    Vec3 forward = GetForward(physics.mainRot);
    forward.Normalize();

    Quat correction;
    correction.MakeVectorRotation(forward, dir);
    correction.Normalise();

    Quat targetRot = MultiplyQuat(correction, physics.mainRot);
    targetRot.Normalise();

    physics.mainRot.SlerpTo(
        targetRot,
        physics.mainRot,
        physics.stabilizationFactor
    );
}

void RailRideState(Input::State* inputState, Kart::Movement& movement, Kart::Status& status, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if ((KAE.mtBoost || KAE.mushroom) && inputState->motionControlFlick != 0) {
        RailRideEject(movement, playerId);
        KAE.jumpedFast = true;
        status.trickableTimer = 0x4;
        KAE.jumped = true;
        KAE.height = 9;
        KAE.railRideJumpDir = KAE.xInput;
    }
    else if (inputState->motionControlFlick != 0) {
        RailRideEject(movement, playerId);
        KAE.jumpedSlow = true;
        status.trickableTimer = 0x4;
        KAE.jumped = true;
        KAE.height = 9;
        KAE.railRideJumpDir = KAE.xInput;
    }
}

void RailRideJump(Kart::Movement& movement, Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    // relative height
    Vec3 up = GetUp(physics.mainRot);

    if (KAE.height > 0) KAE.height -= 1;
    if (KAE.height > 0) physics.speed0 = AddVec(physics.speed0, MultiplyVecByFloat(up, KAE.height));

    if (KAE.jumpedSlow) movement.engineSpeed = 85.0f;
    if (KAE.jumpedFast) movement.engineSpeed = 120.0f;

    // direction
    Vec3 right = GetRight(physics.mainRot);

    float jumpForce = 30.0f;
    Vec3 sideVel;
    sideVel.x = right.x * -KAE.railRideJumpDir * jumpForce;
    sideVel.z = right.z * -KAE.railRideJumpDir * jumpForce;
    sideVel.y = right.y * -KAE.railRideJumpDir * jumpForce;

    physics.speed2.x += sideVel.x;
    physics.speed2.z += sideVel.z;
    physics.speed2.y += sideVel.y;

    if ((KAE.ground && KAE.height < 2) || KAE.oob) {
        KAE.jumped = KAE.jumpedSlow = KAE.jumpedFast = false;
        KAE.height = 0;
    }
}

} // Race
} // MKWG