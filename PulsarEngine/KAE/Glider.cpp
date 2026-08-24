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

    KAE.pitch = 0.0f;
    KAE.previousPitch = 0.0f;

    KAE.yawVel = 0.0f;

    KAE.flightDir = GetForward(physics.mainRot);
}

void ExitGlider(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    KAE.inGlider = false;

    KAE.pitch = 0.0f;
    KAE.yawVel = 0.0f;
    KAE.roll = 0.0f;
    physics.rotVec0.y = 0.0f;
    physics.rotVec1.z = 0.0f;
}

void GliderState(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.inGlider && (KAE.ground || KAE.oob)) {
       ExitGlider(physics, playerId);
    }
    else if (KAE.glider && !KAE.inGlider && !KAE.ground) {
        EnterGlider(physics, playerId);
    }

    if (!kmp) return;
    if (!kmp->areaSection->GetHolder(KAE.Gliding)) return;
    if (kmp->areaSection->GetHolder(KAE.Gliding)->raw->routeId == 2) {
        ExitGlider(physics, playerId);
    }
}

void GliderMovement(Kart::Movement& movement, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    const float dt = 1.0f / 60.0f;

    float gliderSpeed = movement.engineSpeed;
    float targetSpeed = KAE.baseSpeed;
    float speedChange = 5.0f/6.0f * KAE.acceleration;
    float gliderBonusSpeed = 0.0f;

    if (KAE.yInput > 0.1) {
        targetSpeed = 120.0f;
        speedChange = 120.0f / KAE.baseSpeed;
    }
    if (Abs(KAE.xInput) > 0.1f) {
        targetSpeed *= 0.85f;
        speedChange *= 0.85f;
    }
    if (KAE.yInput < -0.1f) {
        targetSpeed *= 0.7f;
        speedChange *= 0.7f;
    }
    
    if (gliderSpeed <= targetSpeed) {
        gliderSpeed += speedChange;

        if (gliderSpeed > targetSpeed) gliderSpeed = targetSpeed;
    }
    else {
        gliderBonusSpeed -= (Abs(KAE.xInput) * gliderSpeed * 0.002) / EGG::Math::Sqrt(KAE.handlingSpeed);
        if (KAE.yInput < -0.05f) gliderBonusSpeed -= (Pow(KAE.yInput, 2)) * gliderSpeed * 0.001; // up deceleration
    }
    if (Abs((gliderSpeed + gliderBonusSpeed) - targetSpeed) < 2.0f) gliderSpeed = targetSpeed;

    movement.engineSpeed = gliderSpeed + gliderBonusSpeed;
}

void UpdatePitch(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    const float maxPitch = 0.18f;
    const float pitchResponse = 0.2f;

    float targetPitch = KAE.zInput * maxPitch;

    // KAE.pitch += (targetPitch - KAE.pitch) * pitchResponse; // weird mechanism, would do next line personally, or a variation of that
    KAE.pitch = targetPitch * pitchResponse;

    float deltaPitch = KAE.pitch - KAE.previousPitch;
    KAE.previousPitch = KAE.pitch;

    Vec3 right = GetRight(physics.mainRot);

    Quat pitchQuat;
    pitchQuat.SetAxisRotation(right, deltaPitch);

    physics.mainRot = MultiplyQuat(pitchQuat, physics.mainRot);
    physics.mainRot.Normalise();
}

void UpdateYaw(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    const float yawAccel = KAE.handling * 0.35f;
    const float maxYawVel = KAE.handling * 0.38f;
    const float targetYawVel = KAE.xInput * maxYawVel;

    if (KAE.yawVel < targetYawVel) KAE.yawVel = Min(KAE.yawVel + yawAccel, targetYawVel);
    else if (KAE.yawVel > targetYawVel) KAE.yawVel = Max(KAE.yawVel - yawAccel, targetYawVel);

    physics.rotVec0.y -= KAE.yawVel;
}

void UpdateRoll(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    float targetRoll = KAE.xInput * 0.18f;
    float damping = 0.98f;

    KAE.roll += (targetRoll - KAE.roll) * 0.04f;
    KAE.roll *= damping;

    physics.rotVec1.z = KAE.roll;
}

void UpdateFlightDirection(Kart::Movement& movement, Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    Vec3 forward = GetForward(physics.mainRot);

    const float follow = 0.0213f;

    KAE.flightDir = AddVec(KAE.flightDir, MultiplyVecByFloat(SubVec(forward, KAE.flightDir), follow));
    KAE.flightDir.Normalize();
    movement.dir.x = KAE.flightDir.x;
    movement.dir.z = KAE.flightDir.z;
    movement.lastDir.x = KAE.flightDir.x;
    movement.lastDir.z = KAE.flightDir.z;
    movement.vel1Dir.x = KAE.flightDir.x;
    movement.vel1Dir.z = KAE.flightDir.z;
    movement.dirDiff.x *= 0.0213f;
    movement.dirDiff.z *= 0.0213f;
}

void UpdateLift(Kart::Movement& movement, Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    
    const float liftCoeff = 0.0000015f;
    float speed = movement.engineSpeed;

    float targetLift = liftCoeff * Pow(speed, 3) * KAE.yInput;

    KAE.liftMemory += (targetLift - KAE.liftMemory) * 0.1f;

    physics.speed0.y += KAE.upDown * KAE.liftMemory;
}

} // Race
} // MKWG