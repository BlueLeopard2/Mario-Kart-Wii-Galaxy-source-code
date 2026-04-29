#include <core/egg/Math/Math.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KAEMath.hpp>
#include <KAE/KMPAREAExpander.hpp>


// Wind [BlueLeopard]
namespace MKWG {
namespace Race {

void Wind(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    const float dt = 1.0f / 60.0f;
    if (KAE.windy) {
        float windPower = kmp->areaSection->GetHolder(KAE.Wind)->raw->setting2 / 100.0f;
        Vec3 windDir = (kmp->areaSection->GetHolder(KAE.Wind)->raw->rotation);
        windDir.Normalize();
        
        physics.speed2.x += windDir.x * windPower * dt;
        physics.speed2.y += windDir.y * windPower * dt;
        physics.speed2.z += windDir.z * windPower * dt;
    }
}

} // Race
} // MKWG
