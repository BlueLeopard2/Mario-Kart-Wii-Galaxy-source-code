#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KMPAREAExpander.hpp>

// AirBoost [BlueLeopard]
// This is simply a boost in the air.
namespace MKWG {
namespace Race {

void AirBoost(Kart::Movement& movement, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.airSpeedUp && !KAE.prevAirSpeedUp) {
        KAE.airBoosterDuration = kmp->areaSection->GetHolder(KAE.AirRing)->raw->setting1;
        movement.ActivateMushroom();
    }

    if (KAE.airBoosterDuration > 0) {
        movement.engineSpeed += 7.0f;
        KAE.airBoosterDuration -= 1;
    }
}

} // Race
} // MKWG
