#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Kart/KartStatus.hpp>
#include <KAE/KMPAREAExpander.hpp>


// Under Water [BlueLeopard]
// Currently under construction
namespace MKWG {
namespace Race {

void UnderWaterPhysics(Kart::Movement& movement, Kart::Physics& physics, const Kart::Status& status, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    float handlingStat = 0.02;
    if (KAE.underWater) {
        if (!KAE.prevUnderWater) {
            movement.engineSpeed *= 0.8;
            KAE.normalSoftSpeedLimit = movement.softSpeedLimit;
        }       
        if (!KAE.ground) {
            // turning
            const float accel   = 4.0f;   // responsiveness
            const float damping = 1.0f;   // how fast it settles
            const float maxVel  = 1.0f;   // max rad/sec
            const float dt = 1.0f / 60.0f;

            KAE.yawVel += status.stickX * handlingStat * accel * dt;
            KAE.yawVel = Clamp(KAE.yawVel, -maxVel, maxVel);
            KAE.yawVel -= KAE.yawVel * damping * dt;
            physics.rotVec0.y -= KAE.yawVel;

            // move kart with the new rotation

            Vec3 forward;
            forward.x = 2.0f * (physics.mainRot.x * physics.mainRot.z + physics.mainRot.w * physics.mainRot.y);
            forward.y = 0.0f;
            forward.z = 1.0f - 2.0f * (physics.mainRot.x * physics.mainRot.x + physics.mainRot.y * physics.mainRot.y);
            float len2D = EGG::Math::Sqrt(forward.x * forward.x + forward.z * forward.z);
            if (len2D != 0.0f) {
                forward.x /= len2D;
                forward.z /= len2D;
            }  
            movement.dir.x = forward.x;
            movement.dir.z = forward.z;
            movement.lastDir.x = forward.x;
            movement.lastDir.z = forward.z;
            movement.vel1Dir.x = forward.x;
            movement.vel1Dir.z = forward.z;
            movement.dirDiff.x = 0.0f;
            movement.dirDiff.z = 0.0f;

            // leaning
            const float maxLeanVel = 0.05f;    // max rotVec.z
            const float leanAccel  = 0.03f;   // how fast it ramps

            float targetLeanVel = status.stickX * maxLeanVel;
            KAE.leanVel += (targetLeanVel - KAE.leanVel) * leanAccel;                 
            physics.rotVec1.z = KAE.leanVel;
        }
    }
    KAE.prevUnderWater = KAE.underWater;
}

} // Race
} // MKWG
