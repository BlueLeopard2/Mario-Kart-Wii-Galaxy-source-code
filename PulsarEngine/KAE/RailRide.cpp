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

// Rail Riding [BlueLeopard]
namespace MKWG {
namespace Race {

void RailRide(Input::State& inputState, Kart::Movement& movement, Kart::Physics& physics, Kart::Status& status, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    static float oldXInput = 0.0f;

    if (KAE.onRail && KAE.ground) {
        Vec3 speed = physics.speed;
        speed.Normalize();
        Vec3 forward = GetForward(physics.mainRot);

        Quat correction;
        correction.MakeVectorRotation(forward, speed);
        correction.Normalise();
        Quat targetRot = MultiplyQuat(correction, physics.mainRot);
        targetRot.Normalise();
        physics.mainRot.SlerpTo(targetRot, physics.mainRot, physics.stabilizationFactor);
        
        if ((KAE.mtBoost || KAE.mushroom) && inputState.motionControlFlick != 0 && KAE.waitBeforeJump == 0) {
            status.trickableTimer = 0x4;
            KAE.jumped = KAE.jumpedFast = true;
            KAE.onRail = false;
            KAE.waitBeforeJump = 100;
            KAE.height = 1;
            oldXInput = KAE.xInput;
        } 
        else if (inputState.motionControlFlick != 0 && KAE.waitBeforeJump == 0) {
            status.trickableTimer = 0x4;
            KAE.jumped = KAE.jumpedSlow = true;
            KAE.onRail = false;
            KAE.waitBeforeJump = 100;
            KAE.height = 1;
            oldXInput = KAE.xInput;
        }
    }
    if (KAE.jumped && KAE.height >= 1) KAE.height += 1;
    if (KAE.height > 9) KAE.height = 0;
    if (KAE.waitBeforeJump > 0) KAE.waitBeforeJump -= 1;
    if (KAE.height > 0) physics.speed0.y += 9 - KAE.height;
    if (KAE.jumpedSlow) movement.engineSpeed = 85;
    if (KAE.jumpedFast) movement.engineSpeed = 120;
    
    if (KAE.ground && KAE.waitBeforeJump < 90) {
        KAE.jumped = KAE.jumpedSlow = KAE.jumpedFast = false;
        KAE.height = 0;
    }

    if (KAE.jumped) {
        Vec3 right = GetRight(physics.mainRot);
        right.y = 0.0f;
        right.Normalize();

        static Vec3 sideVel;
        sideVel.x = right.x * -oldXInput * 50.0f;
        sideVel.z = right.z * -oldXInput * 50.0f;

        physics.speed2.x += sideVel.x;
        physics.speed2.z += sideVel.z;
    }
}

void RailRidePhysics(Kart::Physics& physics, u8 playerId) {
    physics.engineSpeed.x = 0.0f;
    physics.engineSpeed.y = 0.0f;
    physics.engineSpeed.z = 0.0f;
}

} // Race
} // MKWG