#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartSub.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KMPAREAExpander.hpp>

// Configurable Gravity [ImZeraora, BlueLeopard]
namespace MKWG {
namespace Race {

void RespawnInTiltedGravity(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    static bool isRespawning = false;
    if (!KAE.oob && KAE.prevOob) {
        isRespawning = true;
        for (u16 i = 0; i < kmp->jgptSection->pointCount; i++) {
            KMP::Holder<JGPT>* holder = kmp->jgptSection->holdersArray[i];
            if (!holder || !holder->raw) continue;
            if (kmp->areaSection->GetHolder(KAE.PointGravity)->raw->enemyRouteId == holder->raw->id) {
                Vec3 rot = MultiplyVecByFloat(holder->raw->rotation, 3.14159265358979323846f / 180.0f);

                physics.mainRot.SetRPY(rot.x, rot.y, rot.z);

                Vec3 up = GetUp(physics.mainRot);
                Vec3 right = GetRight(physics.mainRot);
                Vec3 forward = GetRight(physics.mainRot);
                up.Normalize();

                Vec3 respawnOffset = MultiplyVecByFloat(up, 700.0f);
                physics.position = AddVec(holder->raw->position, respawnOffset);
                
                physics.position = AddVec(physics.position, MultiplyVecByFloat(right, playerId * 300.0f));
                physics.position = AddVec(physics.position, MultiplyVecByFloat(forward, playerId * -100.0f));

                Vec3 gravity = SubVec(holder->raw->position, physics.position);
                gravity.Normalize();
                gravity = MultiplyVecByFloat(gravity, 1.3);
                physics.speed0 = AddVec(physics.speed0, gravity);

            }
        }
    }
    if (KAE.ground) isRespawning = false;
    if (isRespawning) physics.stabilizationFactor = 0.0f;
}

void ConfigurableGravity(Kart::Physics& physics, u8 playerId) { // updates gravity for all gravity changing mechanics
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.inGlider) {
        if (KAE.glider) {
            KAE.upDown = (kmp->areaSection->GetHolder(KAE.Gliding)->raw->routeId == 1) ? 1.0f : -1.0f;
            KAE.gliderGravity = KAE.upDown * kmp->areaSection->GetHolder(KAE.Gliding)->raw->setting2 / 100.0f;
        }
        physics.gravity = KAE.gliderGravity;
    }
    if (KAE.configGrav) {
        KAE.upDown = (kmp->areaSection->GetHolder(KAE.ConfigurableGravity)->raw->routeId == 1) ? 1.0f : -1.0f;
        float configGravity = KAE.upDown * kmp->areaSection->GetHolder(KAE.ConfigurableGravity)->raw->setting2 / 100.0f;
        physics.gravity = configGravity;
    }
    if (KAE.pointGrav) {
        physics.gravity = 0.0f;
        physics.speed0 = AddVec(physics.speed0, PointGravVec(physics, playerId));
    }
}

} // Race
} // MKWG