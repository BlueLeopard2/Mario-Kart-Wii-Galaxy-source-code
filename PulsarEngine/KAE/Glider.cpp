#include <core/egg/Math/Math.hpp>
#include <runtimeWrite.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KMPAREAExpander.hpp>

// Glider [BlueLeopard]

namespace MKWG {
namespace Race {

void EnterGlider(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    KAE.inGlider = true;

    KAE.flightDir = GetForward(physics.mainRot);
}

void ExitGlider(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    KAE.inGlider = false;

    KAE.gliderYaw = 0.0f;
    KAE.gliderRoll = 0.0f;
    KAE.gliderPitch = 0.0f;
    KAE.liftMemory = 0.0f;
    physics.rotVec1 = Vec3(0, physics.rotVec1.y, 0);
    physics.rotVec0.y = 0;
}

void GliderState(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    if (!kmp) return;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.inGlider && (KAE.ground || KAE.oob)) {
       ExitGlider(physics, playerId);
    }
    else if (KAE.glider && !KAE.inGlider && !KAE.ground) {
        EnterGlider(physics, playerId);
    }


    if (!kmp->areaSection->GetHolder(KAE.gliding)) return;
    if (kmp->areaSection->GetHolder(KAE.gliding)->raw->routeId == 2) {
        ExitGlider(physics, playerId);
    }
}

void GliderMovement(Kart::Movement& movement, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    const float dt = 1.0f / 60.0f;

    float gliderSpeed = movement.engineSpeed;
    float targetSpeed = GetTargetSpeed(KAE.baseSpeed, KAE.mega, KAE.tc, KAE.star, KAE.mushroom, KAE.mtBoost, false, KAE.shocked, false, KAE.feather);
    float speedChange = 5.0f/6.0f * KAE.acceleration;
    float gliderBonusSpeed = 0.0f;

    if (KAE.yInput > 0.1) {
        targetSpeed = KAE.hardSpeedLimit;
        speedChange = KAE.hardSpeedLimit / KAE.baseSpeed;
    }
    if (Abs(KAE.xInput) > 0.1f) {
        targetSpeed *= (1/Abs(KAE.xInput)) * 0.9f;
        speedChange *= (1/Abs(KAE.xInput)) * 0.9f;
    }
    if (KAE.yInput < -0.1f) {
        targetSpeed *= (1/Abs(KAE.yInput)) * 0.75f;
        speedChange *= (1/Abs(KAE.yInput)) * 0.75f;
    }
     
    if (gliderSpeed <= targetSpeed) {
        if (KAE.accelerate) gliderSpeed += speedChange;

        if (gliderSpeed > targetSpeed) gliderSpeed = targetSpeed;
    }
    else {
        gliderBonusSpeed -= (Abs(KAE.xInput) * gliderSpeed * 0.002) / EGG::Math::Sqrt(KAE.handlingSpeed);
        if (KAE.yInput < -0.05f) gliderBonusSpeed -= (Pow(KAE.yInput, 2)) * gliderSpeed * 0.001; // up deceleration
    }
    if (Abs((gliderSpeed + gliderBonusSpeed) - targetSpeed) < 2.0f) gliderSpeed = targetSpeed;

    movement.engineSpeed = gliderSpeed + gliderBonusSpeed;
}

/*void UpdatePitch(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    const float maxPitch = 0.18f;
    const float pitchResponse = 0.16f;

    float targetPitch = KAE.zInput * maxPitch;

    KAE.pitchMemory += (targetPitch - KAE.pitchMemory) * pitchResponse;

    physics.rotVec1.x = KAE.pitchMemory;
}*/

void UpdateYaw(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    const float yawAccel = KAE.handling * 0.35f;
    const float maxYawVel = KAE.handling * 0.38f;
    const float targetYawVel = KAE.xInput * maxYawVel;

    if (KAE.gliderYaw < targetYawVel) KAE.gliderYaw = Min(KAE.gliderYaw + yawAccel, targetYawVel);
    else if (KAE.gliderYaw > targetYawVel) KAE.gliderYaw = Max(KAE.gliderYaw - yawAccel, targetYawVel);

    physics.rotVec0.y -= KAE.gliderYaw;
}

void UpdateRoll(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    float targetRoll = KAE.xInput * 0.18f;
    float damping = 0.98f;

    KAE.gliderRoll += (targetRoll - KAE.gliderRoll) * 0.04f;
    KAE.gliderRoll *= damping;

    physics.rotVec1.z = KAE.gliderRoll;
}

void UpdateFlightDirection(Kart::Movement& movement, Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    Vec3 forward = GetForward(physics.mainRot);

    const float follow = 0.03f;

    KAE.flightDir = AddVec(KAE.flightDir, MultiplyVecByFloat(SubVec(forward, KAE.flightDir), follow));
    KAE.flightDir.Normalize();
    movement.dir.x = KAE.flightDir.x;
    movement.dir.z = KAE.flightDir.z;
    movement.lastDir.x = KAE.flightDir.x;
    movement.lastDir.z = KAE.flightDir.z;
    movement.vel1Dir.x = KAE.flightDir.x;
    movement.vel1Dir.z = KAE.flightDir.z;
    movement.dirDiff.x *= follow;
    movement.dirDiff.z *= follow;
}

void UpdateLift(Kart::Movement& movement, Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.glider) {
        KAE.upDown = (kmp->areaSection->GetHolder(KAE.gliding)->raw->routeId == 1) ? 1.0f : -1.0f;
        KAE.gliderGravity = KAE.upDown * kmp->areaSection->GetHolder(KAE.gliding)->raw->setting2 / 100.0f;
    }
    
    const float liftCoeff = 0.0003f;
    const float targetSpeed = GetTargetSpeed(KAE.baseSpeed, KAE.mega, KAE.tc, KAE.star, KAE.mushroom, KAE.mtBoost, false, KAE.shocked, false, KAE.feather);

    float speedCoeff = (movement.engineSpeed / targetSpeed) * 60.0f;
    speedCoeff = Clamp(speedCoeff, 30.0f, 60.0f);

    float targetLift = (liftCoeff * Pow(speedCoeff, 2)) * KAE.yInput;

    float response = 0.3f;
    if (targetLift * KAE.liftMemory < 0.0f) response = 0.5f;

    KAE.liftMemory += (targetLift - KAE.liftMemory) * response;

    physics.gravity = KAE.gliderGravity + KAE.liftMemory;
}

} // Race
} // MKWG