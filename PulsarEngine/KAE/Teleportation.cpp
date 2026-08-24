#include <core/egg/Math/Math.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <MarioKartWii/KMP/CNPT.hpp>
#include <KAE/KMPAREAExpander.hpp>

// Teleportation [BlueLeopard]
// This one works really well in my opinion, if I could, I would make it possible to have an animation while teleporting / also making sure the camera angle teleports with the player, but looking at only physics it works really well.
namespace MKWG {
namespace Race {

void Teleportation(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.waitForTeleport >= kmp->areaSection->GetHolder(KAE.Teleport)->raw->setting1) {
        for (u16 i = 0; i < kmp->cnptSection->pointCount; i++) {
            KMP::Holder<CNPT>* holder = kmp->cnptSection->holdersArray[i];
            if (!holder || !holder->raw) continue;
            if (kmp->areaSection->GetHolder(KAE.Teleport)->raw->routeId == holder->raw->id) {
                physics.position = holder->raw->destPos;

                Vec3 rot = MultiplyVecByFloat(holder->raw->angle, 3.14159265358979323846f / 180.0f);
                physics.mainRot.SetRPY(rot.x, rot.y, rot.z);
                
                KAE.teleported = true;
                if (KAE.inGlider) ExitGlider(physics, playerId);
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
