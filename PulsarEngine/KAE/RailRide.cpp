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

void RailRideReset(Kart::Movement& movement, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.isRailRiding || KAE.lastPointReached) movement.engineSpeed = KAE.baseSpeed;\

    KAE.isRailRiding = false;
    KAE.lastPointReached = false;
}

void RailRideEject(Kart::Movement& movement, u8 playerId) {   
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    movement.engineSpeed = KAE.baseSpeed;
    KAE.nextPoint = 0;
}

void DetermineNextPoint(Kart::Physics& physics, Kart::Movement& movement, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    // start of nextPoint is already detemined in RailRideStart
    Vec3 distance = kmp->potiSection->GetHolder(KAE.routeId)->raw->points[KAE.nextPoint].position - physics.position;
    float length = VecLength(distance);

    if (length < 250.0f) {
        KAE.nextPoint +=1;
    }
    if (KAE.nextPoint == (kmp->potiSection->GetHolder(KAE.routeId)->raw->pointCount)) {
        RailRideEject(movement, playerId);
        KAE.lastPointReached = true;
        KAE.isRailRiding = false;
    }
}

void RailRideStart(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    for (u16 i = 0; i < kmp->areaSection->pointCount; i++) {
        if (kmp->FindAREA(physics.position, i, (u8)0x13) != i) continue;

        KAE.routeId = kmp->areaSection->GetHolder(i)->raw->routeId;

        if (!kmp->potiSection->GetHolder(KAE.routeId)) continue;

        bool pointFound = false;
        float smallestDistance = 300.0f;

        for (u8 j = 0; j < kmp->potiSection->GetHolder(KAE.routeId)->raw->pointCount - 1; j++) {
            Vec3 A = kmp->potiSection->GetHolder(KAE.routeId)->raw->points[j].position;
            Vec3 B = kmp->potiSection->GetHolder(KAE.routeId)->raw->points[j + 1].position;
        
            Vec3 closestPoint = FindClosestPoint(A, B, physics.position);
            Vec3 distance = SubVec(physics.position, closestPoint);
            float length = VecLength(distance);
        
            if (length < smallestDistance) {
                smallestDistance = length;
                pointFound = true;
                continue;
            }

            if (pointFound) {
                Stopper(physics);
                KAE.isRailRiding = true;
                KAE.nextPoint = j + 1;
                break;
            }
        }
    if (KAE.isRailRiding) break;
    }
}

void RailRideStop(Kart::Movement& movement, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    
    if (KAE.oob) {
        KAE.nextPoint = 0;
        KAE.isRailRiding = false;
        KAE.jumped = false;
        KAE.lastPointReached = false;
    }
}

void RailRidePhysics(Kart::Physics& physics, Kart::Movement& movement, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.bullet || KAE.hit) return;
    movement.engineSpeed = movement.acceleration = 0.0f;

    Vec3 target = kmp->potiSection->GetHolder(KAE.routeId)->raw->points[KAE.nextPoint].position;

    Vec3 desiredDir = target - physics.position;
    desiredDir.Normalize();

    Vec3 floorNormal = KAE.floorNormal;
    if (!KAE.ground) floorNormal = Vec3(0,0,0);

    Vec3 forward = desiredDir - MultiplyVecByFloat(floorNormal, Dot(desiredDir, floorNormal));
    forward.Normalize();

    float speed = GetTargetSpeed(KAE.baseSpeed, KAE.mega, KAE.tc, KAE.star, KAE.mushroom, KAE.mtBoost, false, KAE.shocked, false, KAE.feather);
    physics.engineSpeed = AddVec(physics.engineSpeed, MultiplyVecByFloat(forward, speed));
}

void RailRideRotation(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    Vec3 target = kmp->potiSection->GetHolder(KAE.routeId)->raw->points[KAE.nextPoint].position;

    Vec3 desiredDir = target - physics.position;
    desiredDir.Normalize();

    Vec3 floorNormal = KAE.floorNormal;
    if (!KAE.ground) floorNormal = Vec3(0,0,0);

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

void RailRideJumpStart(Input::State* inputState, Kart::Physics& physics, Kart::Movement& movement, Kart::Status& status, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    
    if (inputState->motionControlFlick != 0) {
        RailRideEject(movement, playerId);

        KAE.isRailRiding = false;
        KAE.lastPointReached = false;
        KAE.jumped = true;

        float railRideJumpDir = KAE.xInput;
        status.airtime = 5;
        status.trickableTimer = 0x4;
        
        KAE.jumpSpeed = GetTargetSpeed(KAE.baseSpeed, KAE.mega, KAE.tc, KAE.star, KAE.mushroom, KAE.mtBoost, false, KAE.shocked, false, KAE.feather);

        Vec3 up = GetUp(physics.mainRot);
        Vec3 right = GetRight(physics.mainRot);

        float rightForce = 4.0f;
        KAE.sideVel = MultiplyVecByFloat(right, -railRideJumpDir * rightForce);
        float upForce = 4.0f;
        KAE.upVel = MultiplyVecByFloat(up, upForce);
    }
}

void RailRideJumpPhysics(Kart::Movement& movement, Kart::Physics& physics, Kart::Status& status, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    KAE.upVel = MultiplyVecByFloat(KAE.upVel, 0.8f);
    KAE.sideVel = MultiplyVecByFloat(KAE.sideVel, 0.9f);

    Vec3 addedVel = AddVec(KAE.upVel, KAE.sideVel);

    physics.speed0 = AddVec(physics.speed0, addedVel);
    movement.engineSpeed = KAE.jumpSpeed;

    if (VecLength(KAE.upVel) < 0.8f && KAE.ground) KAE.jumped = false;
}

} // Race
} // MKWG