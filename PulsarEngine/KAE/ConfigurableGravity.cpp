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

    if (!KAE.oob && KAE.prevOob) {
        KAE.isRespawning = true;
        for (u16 i = 0; i < kmp->jgptSection->pointCount; i++) {
            KMP::Holder<JGPT>* holder = kmp->jgptSection->holdersArray[i];
            if (!holder || !holder->raw) continue;

            if (kmp->areaSection->GetHolder(KAE.pointGravity)->raw->enemyRouteId != holder->raw->id) continue;

            Vec3 rot = MultiplyVecByFloat(holder->raw->rotation, 3.14159265358979323846f / 180.0f);
            physics.mainRot.SetRPY(rot.x, rot.y, rot.z);

            Vec3 up = GetUp(physics.mainRot);
            Vec3 right = GetRight(physics.mainRot);
            Vec3 forward = GetForward(physics.mainRot);

            Vec3 respawnOffset = MultiplyVecByFloat(up, 700.0f);
            physics.position = AddVec(holder->raw->position, respawnOffset);

            u8 respawnPositionZ = playerId % 4;
            u8 respawnPositionX = playerId / 4;
            
            physics.position = AddVec(physics.position, MultiplyVecByFloat(right, respawnPositionZ * 200.0f));
            physics.position = AddVec(physics.position, MultiplyVecByFloat(forward, respawnPositionX * -200.0f));

            KAE.configGravVec = SubVec(holder->raw->position, physics.position);
            KAE.configGravVec.Normalize();
            KAE.configGravVec = MultiplyVecByFloat(KAE.configGravVec, 1.3f);
        }
    }
    if (!KAE.ground && KAE.isRespawning) {      
        physics.speed0 = KAE.configGravVec;
        physics.stabilizationFactor = 0.0f;
    }
    else KAE.isRespawning = false;
}

void ConfigurableGravity(Kart::Physics& physics, u8 playerId) { // updates gravity for all gravity changing mechanics
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.configGrav && !KAE.isRespawning) {
        u8 routeId = kmp->areaSection->GetHolder(KAE.teleport)->raw->routeId;
        KMP::Holder<CNPT>* holder = kmp->GetHolder<CNPT>(routeId);

        bool dynamicGrav = (kmp->areaSection->GetHolder(KAE.configurableGravity)->raw->enemyRouteId == 1);
        if (!dynamicGrav) {
            Vec3 direction;
            direction.x = EGG::Math::Cos(holder->raw->angle.y) * EGG::Math::Cos(holder->raw->angle.z);
            direction.y = EGG::Math::Sin(holder->raw->angle.y) * EGG::Math::Cos(holder->raw->angle.z);
            direction.z = -EGG::Math::Sin(holder->raw->angle.z);
            direction.Normalize();

            KAE.configGravVec = MultiplyVecByFloat(direction, kmp->areaSection->GetHolder(KAE.configurableGravity)->raw->setting2 / 100.0f);  
        } else {
            Vec3 direction = SubVec(holder->raw->destPos, physics.position);
            direction.Normalize();

            KAE.configGravVec = MultiplyVecByFloat(direction, kmp->areaSection->GetHolder(KAE.configurableGravity)->raw->setting2 / 100.0f);
        }
        if (!KAE.ground) physics.speed0 = AddVec(physics.speed0, KAE.configGravVec);

        KAE.camera = (kmp->areaSection->GetHolder(KAE.configurableGravity)->raw->camera == 1) ? true : false;
    }
}

} // Race
} // MKWG