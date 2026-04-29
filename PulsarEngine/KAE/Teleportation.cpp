#include <core/egg/Math/Math.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <MarioKartWii/KMP/CNPT.hpp>
#include <KAE/KMPAREAExpander.hpp>

// Teleportation [BlueLeopard]
namespace MKWG {
namespace Race {

void Teleportation(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    if (KAE.waitForTeleport >= kmp->areaSection->GetHolder(KAE.Teleport)->raw->setting1) {
        for (u16 i = 0; i < kmp->cnptSection->pointCount; i++) {
            KMP::Holder<CNPT>* holder = kmp->cnptSection->holdersArray[i];
            if (!holder || !holder->raw) continue;
            if (kmp->areaSection->GetHolder(KAE.Teleport)->raw->setting2 == holder->raw->id) {
                physics.position = holder->raw->destPos;

                const float yawDeg = holder->raw->angle.y;
                const float yawRad = yawDeg * (3.14159265358979323846f / 180.0f);

                Vec3 up;
                up.x = 0.0f;
                up.y = 1.0f;
                up.z = 0.0f;

                physics.mainRot.SetAxisRotation(up, yawRad);
                KAE.teleported = true;
            }
        }
    }
    KAE.waitForTeleport += 1;
}

void RotateSpeedAfterTP(Kart::Movement& movement, Kart::Physics& physics, u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    
    SnapSpeedToRotation(movement, physics);

    KAE.waitForTeleport = 0;
    KAE.teleported = false;
}

} // Race
} // MKWG
