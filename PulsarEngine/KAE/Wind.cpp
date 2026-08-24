#include <core/egg/Math/Math.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KAEMath.hpp>
#include <KAE/KMPAREAExpander.hpp>


// Wind [BlueLeopard]
// No one will probably ever use this, it's a boring and niche mechanic and I wouldn't be surprised if it doesn't even work. Unimportant.

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
        
        physics.speed2 = AddVec(physics.speed2, MultiplyVecByFloat(windDir, windPower*dt));
    }
}

} // Race
} // MKWG
