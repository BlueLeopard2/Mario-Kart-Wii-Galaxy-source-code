#include <core/egg/Math/Math.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <KAE/KMPAREAExpander.hpp>

// Glider [BlueLeopard]
namespace MKWG {
namespace Race {

void GliderState(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.glider && !KAE.inGlider) KAE.inGlider = true;
    if (KAE.ground || KAE.oob) {
        KAE.inGlider = false;
        KAE.zMemory = 0.0f;
        KAE.xSideSpeed = 0.0f;
        KAE.zSideSpeed = 0.0f;
        physics.rotVec1.z = 0.0f;
    }
}

void GliderMovement(Kart::Movement& movement, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    const float dt = 1.0f / 60.0f;

    if (KAE.inGlider) {
        movement.hopDir.x = 0;
        movement.hopDir.y = 0;
        movement.hopDir.z = 0;

        float accel = 350 * KAE.acceleration;

        if (movement.engineSpeed < KAE.baseSpeed) movement.engineSpeed += accel * dt;
        movement.engineSpeed -= Abs(KAE.xInput) / EGG::Math::Sqrt(KAE.handlingSpeed);
        if (KAE.yInput < 0.0f) movement.engineSpeed += -(Pow(KAE.yInput)) * 2.0f;
        else if (KAE.yInput > 0.0f) movement.engineSpeed += KAE.yInput;
    }
}

void GliderGravity(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    
    // up/down movements
    KAE.zMemory = KAE.zMemory * 0.96f + KAE.zInput * 0.04f; // the bigger the last value, the faster vehicle can't go higher with inputs.

    if (KAE.zMemory > -0.8) physics.gravity = KAE.upDown * KAE.AREAGravity - KAE.zInput * (1.0f - fmaxf(0.0f, -KAE.zMemory - 0.4f));
    else physics.gravity = KAE.upDown * KAE.AREAGravity;
}

void GliderRotation(Kart::Movement& movement, Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    const float dt = 1.0f / 60.0f;

    // Yaw
    //if (KAE.brake) {
        const float accel   = 3.0f * KAE.handling;   // responsiveness
        const float maxVel  = 1.0f * KAE.handling;   // max rad/sec

        KAE.yawVel += KAE.xInput * accel * dt;
        KAE.yawVel = Clamp(KAE.yawVel, -maxVel, maxVel);
        if (KAE.xInput < 0.1f && KAE.xInput > -0.1f) KAE.yawVel *= KAE.yawVel;
        physics.rotVec0.y -= KAE.yawVel;
        // move kart with the new rotation
        SnapSpeedToRotation(movement, physics);
    //}
    /*else { // moving left/right instead of rotating a lot
        const float accel   = 0.5f * KAE.handling;   // responsiveness
        const float maxVel  = 0.16666667f * KAE.handling;   // max rad/sec

        KAE.yawVel += KAE.xInput * accel * dt;
        KAE.yawVel = Clamp(KAE.yawVel, -maxVel, maxVel);
        if (KAE.xInput < 0.1f && KAE.xInput > -0.1f) KAE.yawVel *= KAE.yawVel;
        float deltaYaw = -KAE.yawVel;

        physics.rotVec0.y += deltaYaw;

        const float maxSideSpeed = KAE.handling * 10.0f;
        const float sideAccel    = KAE.handling * 3.0f;
        const float sideDamping  = 0.3f;

        Vec3 right = GetRight(physics.mainRot);
        right.y = 0.0f;
        right.Normalize();

        float targetSideVel = KAE.xInput * maxSideSpeed;
        float currentSideVel = 0;

        currentSideVel += (targetSideVel - currentSideVel) * sideAccel;
        currentSideVel -= currentSideVel * sideDamping;

        float currentSide = Dot(physics.speed2, right);

        physics.speed2.x += right.x * (currentSideVel - currentSide);
        physics.speed2.z += right.z * (currentSideVel - currentSide);
        
    }
    */
    // Roll
    const float maxLeanVel  = 0.2f;    // max rotVec.z
    const float leanAccel   = 0.05f;   // how fast it ramps
    const float leanDamping = 0.03f;   // slows it naturally

    float targetLeanVel = KAE.xInput * maxLeanVel;
    KAE.leanVel += (targetLeanVel - KAE.leanVel) * leanAccel;
    KAE.leanVel -= KAE.leanVel * leanDamping;      
    physics.rotVec1.z = KAE.leanVel;
}

} // Race
} // MKWG
