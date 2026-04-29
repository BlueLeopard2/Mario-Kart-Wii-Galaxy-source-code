#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartSub.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KMPAREAExpander.hpp>

// Configurable Gravity [ImZeraora, BlueLeopard]
namespace MKWG {
namespace Race {

void ConfigurableGravity(Kart::Physics& physics, u8 playerId) { // updates gravity for all gravity changing mechanics
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.configGrav) {
        KAE.upDown = (kmp->areaSection->GetHolder(KAE.ConfigurableGravity)->raw->setting1 == 0) ? -1.0f : 1.0f;
        KAE.AREAGravity = kmp->areaSection->GetHolder(KAE.ConfigurableGravity)->raw->setting2 / 100.0f;
        physics.gravity = KAE.upDown * KAE.AREAGravity;
    }
    if (KAE.glider) {
        KAE.upDown = (kmp->areaSection->GetHolder(KAE.Gliding)->raw->setting1 == 0) ? -1.0f : 1.0f;
        KAE.AREAGravity = kmp->areaSection->GetHolder(KAE.Gliding)->raw->setting2 / 100.0f;
    }
    
    if (KAE.underWater) {
        KAE.underWaterGravity = kmp->areaSection->GetHolder(KAE.Submarine)->raw->setting2 / 100.0f;
        physics.gravity = KAE.underWaterGravity - KAE.zInput * 0.10f;
    }
}

} // Race
} // MKWG