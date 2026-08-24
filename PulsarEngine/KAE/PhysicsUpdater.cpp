#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartSub.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <KAE/KMPAREAExpander.hpp>

// This is where most of the mechanics get applied. EarlyPhysicsUpdate runs once per frame for every player (Also CPUs)
namespace MKWG {
namespace Race {

void PrevPhysicsState(u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    KAE.prevOnRail = (KAE.onRail && KAE.ground);
}

static void EarlyPhysicsUpdate(Kart::Physics& physics, float dt, float maxSpeed, int air) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    Kart::Manager* kartManager = Kart::Manager::sInstance;
    u8 playerId = 0;

    if (!kmp || !kartManager) {
        physics.Update(false, dt, maxSpeed);
        return;
    }

    for (u8 i = 0; i < kartManager->playerCount; i++) {
        Kart::Player* player = kartManager->GetKartPlayer(i);
        if (player && &player->GetPhysics() == &physics) {
            playerId = i;
            break;
        }
    }

    Kart::Player* player = kartManager->GetKartPlayer(playerId);
    Kart::Movement* movement = player->kartSub->kartMovement;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    ConfigurableGravity(physics, playerId);

    if (KAE.antiGrav) AntiGravPhysics(physics, playerId);
    if (KAE.pointGrav || KAE.antiGrav || KAE.configGrav) RespawnInTiltedGravity(physics, playerId);

    if (KAE.inGlider) {
        GliderMovement(*movement, playerId);
        UpdatePitch(physics, playerId);
        UpdateYaw(physics, playerId);
        UpdateRoll(physics, playerId);
        UpdateFlightDirection(*movement, physics, playerId);
        UpdateLift(*movement, physics, playerId);
    }

    /*if ((KAE.ground || KAE.hop) && !KAE.jumped && KAE.onRail) RailRidePhysics(physics);
    if (KAE.onRail && KAE.ground) {
        RailRideRotation(*movement, *physics, *status, playerId);
    }*/

    if (KAE.teleported) RotateSpeedAfterTP(*movement, physics, playerId);
    if (KAE.portal) Teleportation(physics, playerId);

    //if (KAE.onRail && !KAE.prevOnRail && KAE.ground) Stopper(physics);
    
    PrevPhysicsState(playerId);

    physics.Update(false, dt, maxSpeed);
}
kmCall(0x8059fb5c, EarlyPhysicsUpdate);

} // Race
} // MKWG