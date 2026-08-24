#include <core/egg/Math/Math.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <MarioKartWii/KMP/CNPT.hpp>
#include <KAE/KMPAREAExpander.hpp>
#include <core/rvl/OS/OS.hpp>

// PointGravity [BlueLeopard]

// This file does absolutely nothing atm, it's a mechanic I will eventually create, but right now it's not working
namespace MKWG {
namespace Race {

Vec3 PointGravVec(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    for (u16 i = 0; i < kmp->cnptSection->pointCount; i++) {
        KMP::Holder<CNPT>* holder = kmp->cnptSection->holdersArray[i];
        if (!holder || !holder->raw) continue;
        if (kmp->areaSection->GetHolder(KAE.PointGravity)->raw->routeId == holder->raw->id) continue;

        Vec3 centerDiff = SubVec(physics.position, holder->raw->destPos);

        float gravStrength = (kmp->areaSection->GetHolder(KAE.PointGravity)->raw->setting2 / 100.0f);
        float pushPull = (kmp->areaSection->GetHolder(KAE.Gliding)->raw->routeId == 0) ? -1.0f : 1.0f;

        gravStrength *= pushPull;
        centerDiff.Normalize();

        return MultiplyVecByFloat(centerDiff, gravStrength);
    }
    return Vec3(0,0,0);
}

} // Race
} // MKWG
