#include <core/egg/Math/Math.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KMPAREAExpander.hpp>
#include <core/rvl/OS/OS.hpp>

// Super Mario Galaxy Gravity [BlueLeopard]
// Currently under construction
namespace MKWG {
namespace Race {

void PointGravity(Kart::Movement& movement, Kart::Physics& physics, u8 playerId) {
    // TODO: make distanceDiffCenterPoint a range from 10 to 1000
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    if (KAE.pointGrav) {
        Vec3 diffCenter;
        diffCenter.x = kmp->areaSection->GetHolder(KAE.PointGravity)->raw->position.x - physics.position.x;
        diffCenter.y = kmp->areaSection->GetHolder(KAE.PointGravity)->raw->position.y - physics.position.y;
        diffCenter.z = kmp->areaSection->GetHolder(KAE.PointGravity)->raw->position.z - physics.position.z;

        float distanceDiffCenter = EGG::Math::Sqrt(diffCenter.x * diffCenter.x + diffCenter.y * diffCenter.y + diffCenter.z * diffCenter.z);
        OS::Report("distanceDiffCenter = %f\n", distanceDiffCenter);
        if (distanceDiffCenter < 100.0f) distanceDiffCenter = 100.0f; // scale it correctly
        if (distanceDiffCenter > 10000.0f) distanceDiffCenter = 10000.0f;
        distanceDiffCenter *=0.1f;

        Vec3 dirDiffCenter;
        dirDiffCenter.x = diffCenter.x / distanceDiffCenter;
        dirDiffCenter.y = diffCenter.y / distanceDiffCenter;
        dirDiffCenter.z = diffCenter.z / distanceDiffCenter;

        float pushPull = 1; 
        if (kmp->areaSection->GetHolder(KAE.PointGravity)->raw->setting1 == 0) pushPull = 1.0f; 
        if (kmp->areaSection->GetHolder(KAE.PointGravity)->raw->setting1 == 1) pushPull = -1.0f;

        physics.speed0.x += dirDiffCenter.x * pushPull * (kmp->areaSection->GetHolder(KAE.PointGravity)->raw->setting2 / distanceDiffCenter);
        physics.speed0.y += dirDiffCenter.y * pushPull * (kmp->areaSection->GetHolder(KAE.PointGravity)->raw->setting2 / distanceDiffCenter);
        physics.speed0.z += dirDiffCenter.z * pushPull * (kmp->areaSection->GetHolder(KAE.PointGravity)->raw->setting2 / distanceDiffCenter);

        if (pushPull == 1) movement.engineSpeed += (5.0f / distanceDiffCenter) / 60.0f;
    }
}

} // Race
} // MKWG
