#include <kamek.hpp>
#include <runtimeWrite.hpp>
#include <core/egg/Math/Math.hpp>
#include <core/egg/Math/Quat.hpp>
#include <MarioKartWii/3D/Camera/Camera.hpp>
#include <MarioKartWii/3D/Camera/RaceCamera.hpp>
#include <MarioKartWii/3D/Camera/CameraMgr.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartSub.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KMPAREAExpander.hpp>

// anti-gravity [BlueLeopard]
namespace MKWG {
namespace Race {

kmRuntimeUse(0x808B5AE8); // wheelie steepness changer [Gab]

void AntiGravity(Kart::Movement& movement, u8 playerId) {
    // TODO: Make first rotation possible
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.antiGrav) {
        if (!KAE.prevAntiGrav) {
            KAE.antiGravVec.x = 0.0f;
            KAE.antiGravVec.y = -1.3f;
            KAE.antiGravVec.z = 0.0f;
            kmRuntimeWrite16A(0x808B5AE8, 0x4000); // steepness doesn't matter for wheelie [Gab], doesn't work completely, some weirder angles still don't work.
        }
        movement.hopUp = movement.up;

        if (KAE.ground) {
            const float antiGravityStrength = 1.3f;
            Vec3 antiGravityDirection;
            antiGravityDirection.x = -KAE.floorNormal.x;
            antiGravityDirection.y = -KAE.floorNormal.y;
            antiGravityDirection.z = -KAE.floorNormal.z;

            KAE.antiGravVec.x = antiGravityDirection.x * antiGravityStrength;
            KAE.antiGravVec.y = antiGravityDirection.y * antiGravityStrength;
            KAE.antiGravVec.z = antiGravityDirection.z * antiGravityStrength;
        }
    }
    else if (!KAE.antiGrav && KAE.prevAntiGrav) {
        kmRuntimeWrite16A(0x808B5AE8, 0x3F00); // normal wheelie steepness [Gab] 
    }
}

void AntiGravPhysics(Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    physics.gravity = 0.0f;
    if (KAE.ground && !KAE.hop) { // fixes many speed issues
        physics.speed0.x = 0.0f;
        physics.speed0.y = 0.0f;
        physics.speed0.z = 0.0f;
        physics.normalAcceleration = physics.speed0;
    }
    else if (!KAE.ground) {
        physics.speed0.x += KAE.antiGravVec.x;
        physics.speed0.y += KAE.antiGravVec.y;
        physics.speed0.z += KAE.antiGravVec.z;
    }
    if (!KAE.ground) physics.rotVec2.x = physics.rotVec2.y = physics.rotVec2.z = 0.0f;
    if (!KAE.drift && !KAE.inATrick && !KAE.hop) { 
        MatchTiltAndRollToGround(physics, KAE.floorNormal);

        // Set physics.stabilizationFactor to 0 so Mario Kart Wii doesn't change the rotation
        if (KAE.ground) physics.stabilizationFactor = 0.0f; 
    }
    if (KAE.wheelie || KAE.hop) physics.stabilizationFactor = 0.0f; 
}

// fixes accelerating to worldDown [Melg]
void FixSlopes(Kart::Movement& movement, Vec3& dest) {
    movement.GetBodyMatCol2(dest);
    u8 playerId = movement.GetPlayerIdx();
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    
    if (KAE.antiGrav) dest.y = 0;
}
kmCall(0x8057b0a4, FixSlopes);


// Camera rotations for anti-gravity [BlueLeopard]
// TODO: make it work / use code from Patchzy when it releases

/*typedef void (*UpdateCamMain_t) (RaceCamera* cam, GameCamValues* dest, const Kart::Player* kartPlayer, const Vec3* playerPos, float f1, float f2, float f3);

static void UpdateCamMain_Hook(RaceCamera* cam, GameCamValues* dest, const Kart::Player* kartPlayer, const Vec3* playerPos, float f1, float f2, float f3) {
    Kart::Manager* kartManager = Kart::Manager::sInstance;
    if (!kartManager) return;
    u8 playerId = cam->playerId;
    Kart::Player* player = kartManager->GetKartPlayer(playerId);
    if (!player) return;
    Kart::Movement* movement = player->kartSub->kartMovement;
    if (!movement) return;
    Kart::Physics* physics = &player->kartSub->GetPhysics();
    if (!physics) return;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    u32 address = 0x805A2CFC;
    if(REGION == 'E') address = 0x805a34b0; // EDIT THESE
    if(REGION == 'J') address = 0x805a34b0;
    if(REGION == 'K') address = 0x805a34b0;

    static UpdateCamMain_t Real_UpdateCamMain = (UpdateCamMain_t)address;
    Real_UpdateCamMain(cam, dest, kartPlayer, playerPos, f1, f2, f3);

    if (KAE.antiGrav) {
        Vec3 up;
        up.x = -KAE.floorNormal.x;
        up.y = -KAE.floorNormal.y;
        up.z = -KAE.floorNormal.z;
        float zRot = EGG::Math::Atan2(2.0f * (physics->mainRot.w * physics->mainRot.z + physics->mainRot.x * physics->mainRot.y), 1.0f - 2.0f * (physics->mainRot.y * physics->mainRot.y + physics->mainRot.z * physics->mainRot.z));
    }
}
kmCall(0x805A2444, UpdateCamMain_Hook);*/

} // Race
} // MKWG
