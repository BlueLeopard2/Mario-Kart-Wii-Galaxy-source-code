#include <kamek.hpp>
#include <runtimeWrite.hpp>
#include <core/egg/Math/Math.hpp>
#include <core/egg/Math/Quat.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartPlayer.hpp>
#include <MarioKartWii/Kart/KartSub.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KMPAREAExpander.hpp>

// anti-gravity [BlueLeopard] 

namespace MKWG {
namespace Race {

kmRuntimeUse(0x808B5AE8); // wheelie steepness changer [Gab]

static bool IsAntiGravPhysics(const Kart::Physics* physics) {
    Kart::Manager* kartManager = Kart::Manager::sInstance;
    if (!kartManager || physics == 0) return false;

    for (u8 i = 0; i < kartManager->playerCount; i++) {
        Kart::Player* player = kartManager->GetKartPlayer(i);
        if (player && &player->GetPhysics() == physics) {
            return KMPAREAExpand[i].antiGrav;
        }
    }
    return false;
}

// Vanilla only does totalForce.y += Flinear.y, which is correct for world-up gravity but
// collapses (or slides) under redirected gravity. Use the raw spring force (along the
// suspension axis / local down) as the linear contribution instead.
void ApplyWheelSuspensionAntiGrav(Kart::Physics* physics, const Vec3& suspTop, const Vec3& Flinear,
                                  const Vec3& Frot, bool ignoreX) {
    if (IsAntiGravPhysics(physics)) {
        physics->normalAcceleration = AddVec(physics->normalAcceleration, Frot);
    }
    else {
        physics->normalAcceleration.y += Flinear.y;
    }

    Vec3 fBody;
    physics->fullRot.RotateVectorConjugate(Frot, fBody);
    Vec3 r = SubVec(suspTop, physics->position);
    Vec3 rBody;
    physics->fullRot.RotateVectorConjugate(r, rBody);
    Vec3 torque = Cross(rBody, fBody);
    if (ignoreX) torque.x = 0.0f;
    torque.y = 0.0f;
    physics->normalRotVec = AddVec(physics->normalRotVec, torque);
}
kmBranch(0x805b6150, ApplyWheelSuspensionAntiGrav);

void AntiGravity(Kart::Movement& movement, u8 playerId) {
    // TODO: Make first rotation possible
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.antiGrav) {
        if (!KAE.prevAntiGrav) {
            KAE.antiGravVec = Vec3(0.0f, -1.3f, 0.0f);
            kmRuntimeWrite16A(0x808B5AE8, 0x4000); // steepness doesn't matter for wheelie [Gab], doesn't work completely, some weirder angles still don't work.
        }

        if (KAE.ground) {
            const float antiGravityStrength = 1.3f;
            KAE.antiGravVec = MultiplyVecByFloat(KAE.floorNormal, -antiGravityStrength);
        }
    }
    else if (!KAE.antiGrav && KAE.prevAntiGrav) {
        kmRuntimeWrite16A(0x808B5AE8, 0x3F00); // normal wheelie steepness [Gab] 
    }
}

void AntiGravPhysics(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    physics.gravity = 0.0f;
    if (KAE.ground && !KAE.hop) {
        // Keep suspension support, but only along anti-gravity up. Zeroing normalAcceleration
        // stopped slope slide, but also removed the force that holds the chassis up.
        Vec3 up = KAE.floorNormal;
        up.Normalize();
        float alongUp = Dot(physics.normalAcceleration, up);
        physics.normalAcceleration = MultiplyVecByFloat(up, alongUp);

        // Same gravity the airborne path uses, so ride height matches flat-road equilibrium.
        physics.speed0 = AddVec(physics.speed0, KAE.antiGravVec);
    }
    else if (!KAE.ground) {
        physics.speed0 = AddVec(physics.speed0, KAE.antiGravVec); // apply the Anti Gravity to speed0
        if (KAE.air20) physics.rotVec2.x = physics.rotVec2.y = physics.rotVec2.z = 0.0f; // Disables rotations in the air. Useful for now, but should be changed later to allow rotations while keeping the kart to Anti Gravity upright
    }

    if (!KAE.drift && !KAE.inATrick&& !KAE.hop) {
        MatchPitchAndRollToGround(physics, KAE.floorNormal);

        if (KAE.ground) physics.stabilizationFactor = 0.0f; // Set physics.stabilizationFactor to 0 so Mario Kart Wii doesn't change the rotation
    }
}

// fixes accelerating to worldDown [Melg]
void FixSlopes(Kart::Movement& movement, Vec3& dest) {
    movement.GetBodyMatCol2(dest);
    
    u8 playerId = movement.GetPlayerIdx();
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    
    if (KAE.antiGrav) {
        dest.y = 0;
        dest.Normalize();
    }
}
kmCall(0x8057b0a4, FixSlopes);

} // Race
} // MKWG
