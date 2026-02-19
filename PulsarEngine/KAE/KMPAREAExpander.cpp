#include <core/rvl/mtx/mtx.hpp>
#include <core/egg/Math/Math.hpp>
#include <kamek.hpp>
#include <runtimeWrite.hpp>
#include <MarioKartWii/CourseMgr.hpp>
#include <MarioKartWii/Input/InputState.hpp>
#include <MarioKartWii/Input/InputManager.hpp>
#include <MarioKartWii/Input/ControllerHolder.hpp>
#include <MarioKartWii/Item/ItemPlayer.hpp>
#include <MarioKartWii/Kart/Hitbox.hpp>
#include <MarioKartWii/Kart/KartBody.hpp>
#include <MarioKartWii/Kart/KartLink.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartPlayer.hpp>
#include <MarioKartWii/Kart/KartStatus.hpp>
#include <MarioKartWii/KMP/AREA.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <MarioKartWii/KCL/Collision.hpp>
#include <MarioKartWii/KCL/KCLController.hpp>
#include <MarioKartWii/KCL/KCLManager.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <MarioKartWii/Race/Racedata.hpp>
#include <PulsarSystem.hpp>
#include <MarioKartWii/GlobalFunctions.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>

namespace Pulsar {
namespace Race {
void UseFeather(Item::Player& itemPlayer);
} //Race
} //Pulsar


static inline float FAbs(float x) {
    return x < 0.0f ? -x : x;
}

static inline float Pow(float x) {
    return 1.0f + x * (0.69314718f + x * (0.24022651f));
}

float clamp(float x, float minVal, float maxVal) {
    if (x < minVal) return minVal;
    if (x > maxVal) return maxVal;
    return x;
}

inline float fmaxf(float a, float b) {
    return (a > b) ? a : b;
}

namespace MKWG {
namespace Race {

// Per-player state KAE
struct KMPAREAExpander {
    // AREA indices (>= 0 means inside AREA)
    s16 Teleport;
    s16 Pole;
    s16 Gravity;
    s16 Rail;
    s16 AirRing;
    s16 Wind;
    s16 LaunchPad;
    s16 Speedboat;
    s16 Submarine;

    // Current AREA flags
    bool gravityZone;
    bool onRail;
    bool poleTouch;
    bool portal;
    bool airSpeedUp;
    bool windy;
    bool launch;
    bool onWater;
    bool underWater;

    // Gravity modes
    bool antiGrav;
    bool differentGrav;
    bool glider;
    bool SMGGrav;
    bool antiGravSMGGrav;
    bool gliderSMGGrav;
    bool inGlider;

    // Gravity values
    float AREAGravity;
    float upDown;
    float underWaterGravity;
    float yawVel;
    float leanVel;
    float zMemory;
    Vec3 antiGravVec;

    // Railride modes
    bool MKWorldRailRide;

    // Air boost / wind
    float airBooster;
    float airBoosterDuration;

    // Player input / state (current frame)
    bool accelerate;
    bool brake;
    bool drift;
    bool hop;
    bool inATrick;
    bool mtBoost;
    bool stopped;
    bool wall;
    bool ground;
    bool wheelie;

    bool mega;
    bool mushroom;
    bool star;
    bool shocked;
    bool feather;
    bool oob;

    float zInput;

    // Previous frame state
    bool prevInGlider;
    bool prevHop;
    bool prevMega;
    bool prevMushroom;
    bool prevOnRail;
    bool prevDest;
    bool prevShocked;
    bool prevStar;
    bool prevStopped;
    bool prevAirSpeedUp;
    bool prevAntiGrav;
    bool prevGliderSMGGrav;
    bool wasInGravityZone;

    // Timers / transition state
    u16 waitForLaunch;
    u16 waitForTeleport;
    u16 waitBeforeJump;
    u16 waitingTime;
    u16 height;

    bool jumpedSlow;
    bool jumpedFast;
    bool jumped;

    float normalSoftSpeedLimit;


    KMPAREAExpander()
        : Teleport(-1), Pole(-1), Gravity(-1), Rail(-1),
          AirRing(-1), Wind(-1), LaunchPad(-1),
          Speedboat(-1), Submarine(-1),

          gravityZone(false), onRail(false), poleTouch(false),
          portal(false), airSpeedUp(false), windy(false),
          launch(false), onWater(false), underWater(false),

          antiGrav(false), differentGrav(false), glider(false),
          SMGGrav(false), antiGravSMGGrav(false), gliderSMGGrav(false),
          inGlider(false),

          AREAGravity(0.0f), upDown(0.0f), underWaterGravity(0.0f),
          yawVel(0.0f), leanVel(0.0f), zMemory(0.0f),

          MKWorldRailRide(false),

          airBooster(0.0f), airBoosterDuration(0.0f),

          accelerate(false), brake(false), drift(false), hop(false),
          inATrick(false), mtBoost(false), stopped(false),
          wall(false), ground(false), wheelie(false),

          mega(false), mushroom(false), star(false),
          shocked(false), feather(false), oob(false),

          zInput(0.0f),

          prevInGlider(false), prevHop(false), prevMega(false),
          prevMushroom(false), prevOnRail(false), prevDest(false),
          prevShocked(false), prevStar(false), prevStopped(false),
          prevAirSpeedUp(false), prevAntiGrav(false), prevGliderSMGGrav(false),
          wasInGravityZone(false),

          waitForLaunch(0), waitForTeleport(0),
          waitBeforeJump(0), waitingTime(0), height(0),

          jumpedSlow(false), jumpedFast(false), jumped(false),

          normalSoftSpeedLimit(0.0f) {
          antiGravVec.x = 0.0f;
          antiGravVec.y = 0.0f;
          antiGravVec.z = 0.0f;
        }

    void Reset() {
        Teleport = -1;
        Pole = -1;
        Gravity = -1;
        Rail = -1;
        AirRing = -1;
        Wind = -1;
        LaunchPad = -1;
        Speedboat = -1;
        Submarine = -1;

        gravityZone = false;
        onRail = false;
        poleTouch = false;
        portal = false;
        airSpeedUp = false;
        windy = false;
        launch = false;
        onWater = false;
        underWater = false;

        antiGrav = false;
        differentGrav = false;
        glider = false;
        SMGGrav = false;
        antiGravSMGGrav = false;
        gliderSMGGrav = false;
        inGlider = false;

        AREAGravity = 0.0f;
        upDown = 0.0f;
        underWaterGravity = 0.0f;

        yawVel = 0.0f;
        leanVel = 0.0f;
        zMemory = 0.0f;

        antiGravVec.x = 0.0f;
        antiGravVec.y = 0.0f;
        antiGravVec.z = 0.0f;

        MKWorldRailRide = false;

        airBooster = 0.0f;
        airBoosterDuration = 0.0f;

        accelerate = false;
        brake = false;
        drift = false;
        hop = false;
        inATrick = false;
        mtBoost = false;
        stopped = false;
        wall = false;
        ground = false;
        wheelie = false;

        mega = false;
        mushroom = false;
        star = false;
        shocked = false;
        feather = false;
        oob = false;

        zInput = 0.0f;

        prevInGlider = false;
        prevHop = false;
        prevMega = false;
        prevMushroom = false;
        prevOnRail = false;
        prevDest = false;
        prevShocked = false;
        prevStar = false;
        prevStopped = false;
        prevAirSpeedUp = false;
        prevGliderSMGGrav = false;
        prevAntiGrav = false;
        wasInGravityZone = false;

        waitForLaunch = 0;
        waitForTeleport = 0;
        waitBeforeJump = 0;
        waitingTime = 0;
        height = 0;

        jumpedSlow = false;
        jumpedFast = false;
        jumped = false;

        normalSoftSpeedLimit = 0.0f;
    }
};
static KMPAREAExpander KMPAREAExpand[12];
static u16 raceFrameCount = 0;

// KMP AREA Detection, BlueLeopard
void KMPDetector(u8 playerId) {
    Kart::Manager* kartManager = Kart::Manager::sInstance;
    Kart::Player* player = kartManager->GetKartPlayer(playerId);
    const Vec3& pos = player->GetPhysics().position;
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    KAE.Rail      = kmp->FindAREA(pos, (u32)-1, (u8)0x0C);
    KAE.Pole      = kmp->FindAREA(pos, (u32)-1, (u8)0x0D);
    KAE.AirRing   = kmp->FindAREA(pos, (u32)-1, (u8)0x0E);
    KAE.Teleport  = kmp->FindAREA(pos, (u32)-1, (u8)0x0F);
    KAE.Gravity   = kmp->FindAREA(pos, (u32)-1, (u8)0x10);
    KAE.Wind      = kmp->FindAREA(pos, (u32)-1, (u8)0x11);
    KAE.LaunchPad = kmp->FindAREA(pos, (u32)-1, (u8)0x12);
    KAE.Speedboat = kmp->FindAREA(pos, (u32)-1, (u8)0x13);
    KAE.Submarine = kmp->FindAREA(pos, (u32)-1, (u8)0x14);
    
    KAE.gravityZone = (KAE.Gravity >= 0);
    KAE.onRail      = (KAE.Rail >= 0);
    KAE.poleTouch   = (KAE.Pole >= 0);
    KAE.airSpeedUp  = (KAE.AirRing >= 0);
    KAE.portal      = (KAE.Teleport >=0);
    KAE.windy       = (KAE.Wind >= 0);
    KAE.launch      = (KAE.LaunchPad >=0);
    KAE.onWater     = (KAE.Speedboat >=0);
    KAE.underWater  = (KAE.Submarine >=0);
}

void Effects(const Kart::Status& status, u8 playerId) {
    Input::ControllerHolder& controllerHolder = status.link->GetControllerHolder();
    u32 bitfield0 = status.bitfield0;
    u32 bitfield1 = status.bitfield1;
    u32 bitfield2 = status.bitfield2;
    
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    KAE.accelerate  = ((bitfield0 & 0x1) !=0);
    KAE.brake       = ((bitfield0 & 0x2) !=0);
    KAE.drift       = ((bitfield0 & 0x8) !=0);
    KAE.ground      = ((bitfield0 & (0x400 | 0x800 | 0x1000 | 0x40000)) != 0);
    KAE.mushroom    = ((bitfield0 & 0x2000000) != 0);
    KAE.star        = ((bitfield1 & 0x80000000) != 0);
    KAE.mega        = ((bitfield2 & 0x8000) != 0 || (bitfield2 & 0x20000000) != 0);
    KAE.stopped     = ((bitfield2 & 0x40000) != 0);
    KAE.shocked     = ((bitfield2 & 0x80) != 0 || (bitfield2 & 0x10000) != 0);
    KAE.inATrick    = ((bitfield1 & 0x40) != 0);
    KAE.wheelie     = ((bitfield0 & 0x20000000) !=0);
    KAE.feather     = ((bitfield1 & 0x4000) !=0);
    KAE.mtBoost     = ((bitfield1 & 0x100000) !=0);
    KAE.wall        = ((bitfield0 & (0x20 | 0x40)) != 0);
    KAE.hop         = ((bitfield0 & 0x80000) !=0);
    KAE.oob         = ((bitfield0 & 0x10) !=0);
    if ((status.airtime >= 2) && (!status.bool_0x96 || (status.airtime > 19))) KAE.zInput = controllerHolder.inputStates[0].stick.z;
    else KAE.zInput = 0;
}

void Safe() {
    for (u8 i = 0; i < 12; i++) {
        KMPAREAExpand[i].Reset();
    }
    raceFrameCount = 0;
}
RaceLoadHook Saver(Safe);

// Gravity and LaunchPad, BlueLeopard
void GravityVersion(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    float variantCG = 0;
    if (KAE.gravityZone) {
        float radiusX = kmp->areaSection->GetHolder(KAE.Gravity)->raw->scale.x; 
        float radiusY = kmp->areaSection->GetHolder(KAE.Gravity)->raw->scale.y;
        float radiusZ = kmp->areaSection->GetHolder(KAE.Gravity)->raw->scale.z;
        Vec3 diffPlayer = physics.position - kmp->areaSection->GetHolder(KAE.Gravity)->raw->position;
        float scaledDistSquared = (diffPlayer.x * diffPlayer.x) / (radiusX * radiusX) + (diffPlayer.y * diffPlayer.y) / (radiusY * radiusY) + (diffPlayer.z * diffPlayer.z) / (radiusZ * radiusZ);
        variantCG = kmp->areaSection->GetHolder(KAE.Gravity)->raw->unknown_0x2e;
        KAE.antiGrav        = variantCG == 0;
        KAE.differentGrav   = variantCG == 1;
        KAE.glider          = variantCG == 2;
        KAE.SMGGrav         = variantCG == 3;
        KAE.antiGravSMGGrav = variantCG == 4;
        KAE.gliderSMGGrav   = variantCG == 5;
    } else {
        variantCG = 0;
        KAE.antiGrav            = false;
        KAE.differentGrav       = false;
        KAE.glider              = false;
        KAE.SMGGrav             = false;
        KAE.antiGravSMGGrav     = false;
        KAE.gliderSMGGrav       = false;
    }
}

static void ApplyAREAGravity(Kart::Physics& physics, float dt, float maxSpeed, int air) {
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
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.gravityZone) {
        KAE.upDown = (kmp->areaSection->GetHolder(KAE.Gravity)->raw->setting1 == 0) ? 1.0f : -1.0f;
        if (KAE.glider || KAE.antiGrav || KAE.differentGrav) KAE.AREAGravity = kmp->areaSection->GetHolder(KAE.Gravity)->raw->setting2 / 100.0f;
        if (KAE.differentGrav) physics.gravity = KAE.upDown * KAE.AREAGravity;
        else if (KAE.antiGrav || KAE.SMGGrav || KAE.antiGravSMGGrav || KAE.gliderSMGGrav) physics.gravity = 0.0f;
    }
    if (KAE.inGlider) {
        KAE.zMemory = KAE.zMemory * 0.98f + KAE.zInput * 0.02f;
        if (KAE.zMemory > -0.8) physics.gravity = KAE.upDown * KAE.AREAGravity - KAE.zInput * (1.0f - fmaxf(0.0f, -KAE.zMemory - 0.3f));
        else physics.gravity = KAE.upDown * KAE.AREAGravity;
    }
    if (KAE.underWater) {
        KAE.underWaterGravity = kmp->areaSection->GetHolder(KAE.Submarine)->raw->setting2 / 100.0f;
        physics.gravity = KAE.underWaterGravity - KAE.zInput * 0.10f;
    }

    physics.Update(false, dt, maxSpeed);
}
kmCall(0x8059fb5c, ApplyAREAGravity);

kmRuntimeUse(0x808B5AE8);
void AntiGravity(Kart::Physics& physics, Kart::Movement& movement, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    if (KAE.antiGrav) {
        if (!KAE.prevAntiGrav) {
            KAE.antiGravVec.y = 1.3;
            kmRuntimeWrite16A(0x808B5AE8, 0x3F80);
        }
        if (KAE.ground) {
            KAE.AREAGravity = kmp->areaSection->GetHolder(KAE.Gravity)->raw->setting2 / 100.0f;

            KAE.antiGravVec.x = -movement.up.x * KAE.AREAGravity;
            KAE.antiGravVec.y = -movement.up.y * KAE.AREAGravity;
            KAE.antiGravVec.z = -movement.up.z * KAE.AREAGravity;

            physics.speed0.x = 0;
            physics.speed0.y = 0;
            physics.speed0.z = 0;
            if (movement.dir.y > 0.8 && !KAE.drift && !KAE.hop) {
                movement.smoothedUp.x = 0;
                movement.smoothedUp.y = 1;
                movement.smoothedUp.z = 0;
            }
        }
        else {
            physics.speed0.x += KAE.antiGravVec.x;
            physics.speed0.y += KAE.antiGravVec.y;
            physics.speed0.z += KAE.antiGravVec.z;
        }
        if (!KAE.antiGrav && KAE.prevAntiGrav) kmRuntimeWrite16A(0x808B5AE8, 0x3F00);
    }
    KAE.prevAntiGrav = KAE.antiGrav;
}


// Camera rotations for antiGrav, ImZeraora
/*typedef void (*CameraFallingFuncType)(int cameraData, float* cameraParams, void* kartProxy, int dirFlag);

static u8 GetPlayerIdFromKartProxy(void* kartProxy) {
    if (kartProxy == nullptr) return 0;
    
    Kart::Manager* kartManager = Kart::Manager::sInstance;
    if (kartManager == nullptr) return 0;

    for (u8 i = 0; i < kartManager->playerCount; i++) {
        Kart::Player* player = kartManager->players[i];
        if (player != nullptr) {
            if ((void*)player == kartProxy) {
                return i;
            }
        }
    }
    return 0;
}

void CameraFallingAdjustment_Hook(int cameraData, float* cameraParams, void* kartProxy, int dirFlag) {
    u8 playerId = GetPlayerIdFromKartProxy(kartProxy);
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    if (KAE.antiGrav || KAE.antiGravSMGGrav) {
        float interpRate = 0.1f;
        cameraParams[6] = cameraParams[6] * (1.0f - interpRate);
        return;
    }
    // Call original camera function otherwise
    CameraFallingFuncType originalFunc = (CameraFallingFuncType)0x805a463c;
    originalFunc(cameraData, cameraParams, kartProxy, dirFlag);
}
kmCall(0x805a3860, CameraFallingAdjustment_Hook);*/

void SuperMarioGalaxyGravity(Kart::Movement& movement, Kart::Physics& physics, u8 playerId) {
    // TODO make distanceDiffCenterSMG a range from 1 to 100
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    if (KAE.SMGGrav || KAE.antiGravSMGGrav || KAE.gliderSMGGrav) {
        Vec3 diffCenterSMG;
        diffCenterSMG.x = kmp->areaSection->GetHolder(KAE.Gravity)->raw->position.x - physics.position.x;
        diffCenterSMG.y = kmp->areaSection->GetHolder(KAE.Gravity)->raw->position.y - physics.position.y;
        diffCenterSMG.z = kmp->areaSection->GetHolder(KAE.Gravity)->raw->position.z - physics.position.z;

        float distanceDiffCenterSMG = EGG::Math::Sqrt(diffCenterSMG.x * diffCenterSMG.x + diffCenterSMG.y * diffCenterSMG.y + diffCenterSMG.z * diffCenterSMG.z);
        if (distanceDiffCenterSMG == 0) distanceDiffCenterSMG = 1;

        Vec3 dirDiffCenterSMG;
        dirDiffCenterSMG.x = diffCenterSMG.x / distanceDiffCenterSMG;
        dirDiffCenterSMG.y = diffCenterSMG.y / distanceDiffCenterSMG;
        dirDiffCenterSMG.z = diffCenterSMG.z / distanceDiffCenterSMG;

        float pushPull = 1; 
        if (kmp->areaSection->GetHolder(KAE.Gravity)->raw->setting1 == 0) pushPull = 1; 
        if (kmp->areaSection->GetHolder(KAE.Gravity)->raw->setting1 == 1) pushPull = -1;

        physics.speed2.x += dirDiffCenterSMG.x * pushPull * (kmp->areaSection->GetHolder(KAE.Gravity)->raw->setting2 / distanceDiffCenterSMG);
        physics.speed2.y += dirDiffCenterSMG.y * pushPull * (kmp->areaSection->GetHolder(KAE.Gravity)->raw->setting2 / distanceDiffCenterSMG);
        physics.speed2.z += dirDiffCenterSMG.z * pushPull * (kmp->areaSection->GetHolder(KAE.Gravity)->raw->setting2 / distanceDiffCenterSMG);

        if (pushPull == 1) movement.engineSpeed += (5 / distanceDiffCenterSMG) / 60;
    }
}

void GliderPhysics(Kart::Movement& movement, Kart::Status& status, Kart::Physics& physics, u8 playerId) {
    //TODO make gliders use real vehicle + character stats
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    float handlingStat = 0.02;
    float accelStat = 0.02;
    float handlingSpeedStat = 0.5;

    if (KAE.glider && !KAE.inGlider) {
        KAE.normalSoftSpeedLimit = movement.softSpeedLimit;
        KAE.inGlider = true;
    }

    if ((KAE.inGlider || KAE.gliderSMGGrav) && (KAE.ground || KAE.oob)) {
        KAE.inGlider = false;
        KAE.glider = false;
        movement.softSpeedLimit = KAE.normalSoftSpeedLimit;
        physics.rotVec1.z = 0.0f;
    }
    if (KAE.inGlider || KAE.gliderSMGGrav) {
        movement.hopDir.x = 0;
        movement.hopDir.y = 0;
        movement.hopDir.z = 0;
        movement.softSpeedLimit = 120.0f;

        // turning
        const float accel   = 5.0f;   // responsiveness
        const float damping = 3.0f;   // how fast it settles
        const float maxVel  = 2.0f;   // max rad/sec
        const float dt = 1.0f / 60.0f;

        KAE.yawVel += status.stickX * handlingStat * accel * dt;
        KAE.yawVel = clamp(KAE.yawVel, -maxVel, maxVel);
        KAE.yawVel -= KAE.yawVel * damping * dt;
        physics.rotVec0.y -= KAE.yawVel;

        // move kart with the new rotation
        Vec3 forward;
        forward.x = 2.0f * (physics.mainRot.x * physics.mainRot.z + physics.mainRot.w * physics.mainRot.y);
        forward.y = 0.0f;
        forward.z = 1.0f - 2.0f * (physics.mainRot.x * physics.mainRot.x + physics.mainRot.y * physics.mainRot.y);
        float len2D = EGG::Math::Sqrt(forward.x * forward.x + forward.z * forward.z);
        if (len2D != 0.0f) {
            forward.x /= len2D;
            forward.z /= len2D;
        }  
        movement.dir.x = forward.x;
        movement.dir.z = forward.z;
        movement.lastDir.x = forward.x;
        movement.lastDir.z = forward.z;
        movement.vel1Dir.x = forward.x;
        movement.vel1Dir.z = forward.z;
        movement.dirDiff.x = 0.0f;
        movement.dirDiff.z = 0.0f;

        // leaning
        const float maxLeanVel = 0.2f;    // max rotVec.z
        const float leanAccel  = 0.05f;   // how fast it ramps
        const float leanDamping= 0.03f;   // slows it naturally

        float targetLeanVel = status.stickX * maxLeanVel;
        KAE.leanVel += (targetLeanVel - KAE.leanVel) * leanAccel;
        KAE.leanVel -= KAE.leanVel * leanDamping;                  
        physics.rotVec1.z = KAE.leanVel;
    } 
    if (KAE.inGlider) {
        if (movement.engineSpeed <= (KAE.normalSoftSpeedLimit * 1.2) && movement.engineSpeed > ((KAE.normalSoftSpeedLimit * 1.2) - 20.0f)) movement.engineSpeed += 10.0f * accelStat;
        else if (movement.engineSpeed >= (KAE.normalSoftSpeedLimit * 1.2 - 0.2) && movement.engineSpeed <= ((KAE.normalSoftSpeedLimit * 1.2) + 0.2)) movement.engineSpeed += KAE.normalSoftSpeedLimit * 1.2 - movement.engineSpeed;
        else if (movement.engineSpeed <= 50.0f) movement.engineSpeed += 50.0f * accelStat;
        else if (movement.engineSpeed > 50.0f && movement.engineSpeed < KAE.normalSoftSpeedLimit * 1.2 - 20.0f) movement.engineSpeed += 30.0f * accelStat;
        movement.engineSpeed -= FAbs(status.stickX) * 16.0f * handlingSpeedStat / 60.0f;
        if (status.stickY < 0.0f) movement.engineSpeed -= (Pow(status.stickY) * 8.0f) / 60.0f;
        else if (status.stickY > 0.0f) movement.engineSpeed += (Pow(-status.stickY) * -1) / 60.0f;
    }
    KAE.prevInGlider = KAE.inGlider;
    KAE.prevMushroom = KAE.mushroom;
    KAE.prevStar = KAE.star;
    KAE.prevMega = KAE.mega;
}

void LaunchPadPhysics(Kart::Movement& movement, Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    if (KAE.launch) {
        KAE.waitForLaunch += 1;
        if (KAE.waitForLaunch > kmp->areaSection->GetHolder(KAE.LaunchPad)->raw->setting1) {
            for (u16 i = 0; i < kmp->cnptSection->pointCount; i++) {
                KMP::Holder<CNPT>* holder = kmp->cnptSection->holdersArray[i];
                if (kmp->areaSection->GetHolder(KAE.LaunchPad)->raw->setting1 == holder->raw->id) {
                    Vec3 diffCenterLP;
                    diffCenterLP.x = holder->raw->destPos.x - physics.position.x;
                    diffCenterLP.y = holder->raw->destPos.y - physics.position.y;
                    diffCenterLP.z = holder->raw->destPos.z - physics.position.z;

                    float distanceDiffCenterLP = EGG::Math::Sqrt(diffCenterLP.x * diffCenterLP.x + diffCenterLP.y * diffCenterLP.y + diffCenterLP.z * diffCenterLP.z);
                    if (distanceDiffCenterLP == 0) distanceDiffCenterLP = 1;

                    Vec3 dirDiffCenterLP;
                    dirDiffCenterLP.x = diffCenterLP.x / distanceDiffCenterLP;
                    dirDiffCenterLP.y = diffCenterLP.y / distanceDiffCenterLP;
                    dirDiffCenterLP.z = diffCenterLP.z / distanceDiffCenterLP;
                    if (dirDiffCenterLP.x == 0 && dirDiffCenterLP.y == 0 && dirDiffCenterLP.z == 0) return;

                    physics.speed2.x = dirDiffCenterLP.x * (holder->raw->destPos.x / 100);
                    physics.speed2.y = dirDiffCenterLP.y * (holder->raw->destPos.y / 100);
                    physics.speed2.z = dirDiffCenterLP.z * (holder->raw->destPos.z / 100);
                }
            }
        }
    }
    else KAE.waitForLaunch = 0;
}

// RailRide, BlueLeopard
void RailRide(Kart::Movement& movement, Kart::Status& status, Input::State& inputState, Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    Vec3 zero;
        zero.x = 0;
        zero.y = 0;
        zero.z = 0;
    if (KAE.onRail && KAE.ground) {
        physics.engineSpeed = zero;
        movement.acceleration = 0;
        if (KAE.accelerate && !KAE.brake) movement.engineSpeed = -1.0f;
        else movement.engineSpeed = 0.0f;

        KAE.MKWorldRailRide = kmp->areaSection->GetHolder(KAE.Rail)->raw->unknown_0x2e == 1;

        if (KAE.MKWorldRailRide) {
            Vec3 move;
            move.x = physics.speed.x;
            move.z = physics.speed.z;

            float len = EGG::Math::Sqrt(move.x * move.x + move.z * move.z);
            if (len > 20.0f) {
                move.x /= len;
                move.z /= len;
                float yaw = EGG::Math::Atan2(move.x, move.z);

                EGG::Vector3f up;
                up.x = 0.0f;
                up.y = 1.0f;
                up.z = 0.0f;

                physics.mainRot.SetAxisRotation(up, yaw);
            }
        }
        
        if ((KAE.mtBoost || KAE.mushroom) && inputState.motionControlFlick != 0 && KAE.waitBeforeJump == 0) {
            status.trickableTimer = 0x4;
            KAE.jumped = KAE.jumpedFast = true;
            KAE.onRail = false;
            KAE.waitBeforeJump = 120;
            KAE.height = 1;
        } else if (inputState.motionControlFlick != 0 && KAE.waitBeforeJump == 0) {
            status.trickableTimer = 0x4;
            KAE.jumped = KAE.jumpedSlow = true;
            KAE.onRail = false;
            KAE.waitBeforeJump = 120;
            KAE.height = 1;
        }
    }
    if (KAE.jumped && KAE.height >= 1) KAE.height += 1;
    if (KAE.height > 9) KAE.height = 0;
    if (KAE.waitBeforeJump > 0) KAE.waitBeforeJump -= 1;
    if (KAE.height > 0) physics.speed0.y += 9 - KAE.height;
    if (KAE.jumpedSlow) movement.engineSpeed = 85;
    if (KAE.jumpedFast) movement.engineSpeed = 120;
    if (KAE.MKWorldRailRide) physics.engineSpeed.x += status.stickX * 20.0f;
    if (KAE.ground && KAE.waitBeforeJump < 110) {
        KAE.jumped = KAE.jumpedSlow = KAE.jumpedFast = false;
        KAE.height = 0;
    }
    KAE.prevOnRail = KAE.onRail;
}

// PoleBounce, BlueLeopard
void PoleBounce(const Kart::Collision& collision, Input::State& inputState, Kart::Movement& movement, u8 playerId) {
    // TODO make bounce lower and add trick, or REMOVE BECAUSE MKWII PHYSICS ALREADY ALLOWS BETTER VARIANT
    Item::Player& itemPlayer = Item::Manager::sInstance->players[collision.GetPlayerIdx()];
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    if (KAE.poleTouch && inputState.motionControlFlick && KAE.wall) {
        itemPlayer.inventory.currentItemCount += 1;
        Pulsar::Race::UseFeather(itemPlayer);
        /*Vec3 left = Vec3::Cross(movement->dir, movement->up);
        left.Normalize(); // optional but safer
        trick->Start(left);*/
    }
}

// AirRing, BlueLeopard
void AirBoost(Kart::Movement& movement, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.airSpeedUp && !KAE.prevAirSpeedUp) {
        KAE.airBoosterDuration = kmp->areaSection->GetHolder(KAE.AirRing)->raw->setting1;
        movement.ActivateMushroom();
    }

    if (KAE.airBoosterDuration > 0) {
        KAE.airBoosterDuration -= 1;
        movement.engineSpeed += 10;
    }
    KAE.prevAirSpeedUp = KAE.airSpeedUp;
}

void Wind(Kart::Physics& physics, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    if (KAE.windy) {
        float windRotation = (kmp->areaSection->GetHolder(KAE.Wind)->raw->setting1 / 360) * (2.0f * 3.14159265358979323);
        float windPower = kmp->areaSection->GetHolder(KAE.Wind)->raw->setting2 / 100;
        Vec3 windDir;
        windDir.x = EGG::Math::Cos(windRotation);
        windDir.y = 0;
        windDir.z = EGG::Math::Sin(windRotation);
        physics.speed2.x += windDir.x * windPower;
        physics.speed2.z += windDir.z * windPower;
    }
}

// Teleportation, BlueLeopard
void Teleportation(Kart::Physics& physics, Kart::Movement& movement, u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];
    if (KAE.portal) {
        Vec3 forward;
        forward.x = 2.0f * (physics.mainRot.x * physics.mainRot.z + physics.mainRot.w * physics.mainRot.y);
        forward.y = 0.0f;
        forward.z = 1.0f - 2.0f * (physics.mainRot.x * physics.mainRot.x + physics.mainRot.y * physics.mainRot.y);
        float len2D = EGG::Math::Sqrt(forward.x * forward.x + forward.z * forward.z);
        if (len2D != 0.0f) {
            forward.x /= len2D;
            forward.z /= len2D;
        }

        KAE.waitForTeleport += 1;
        if (KAE.waitForTeleport > kmp->areaSection->GetHolder(KAE.Teleport)->raw->setting1) {
            for (u16 i = 0; i < kmp->cnptSection->pointCount; i++) {
                KMP::Holder<CNPT>* holder = kmp->cnptSection->holdersArray[i];
                if (!holder || !holder->raw) continue;
                if (kmp->areaSection->GetHolder(KAE.Teleport)->raw->setting2 == holder->raw->id) {
                    physics.position = holder->raw->destPos;

                    const float yawDeg = holder->raw->angle.y;
                    const float yawRad = yawDeg * (3.14159265f / 180.0f);

                    EGG::Vector3f up;
                    up.x = 0.0f;
                    up.y = 1.0f;
                    up.z = 0.0f;

                    physics.mainRot.SetAxisRotation(up, yawRad);

                    movement.dir.x = forward.x;
                    movement.dir.z = forward.z;
                    movement.lastDir.x = forward.x;
                    movement.lastDir.z = forward.z;
                    movement.vel1Dir.x = forward.x;
                    movement.vel1Dir.z = forward.z;
                    movement.dirDiff.x = 0.0f;
                    movement.dirDiff.z = 0.0f;

                    KAE.portal = false;
                    KAE.waitForTeleport = 0;
                }
            }
        }
    }
    else KAE.waitForTeleport = 0;
}

// Under Water, BlueLeopard
/*
void UnderWaterPhysics(Kart::Physics& physics, Kart::Movement& movement, const Kart::Status& status) {
    float handlingStat = 0.02;
    if (underWater) {
        if (!prevUnderWater) {
            movement.engineSpeed *=0.8;
            normalSoftSpeedLimit = movement.softSpeedLimit;
        }       
        if (!ground) {
            physics.rotVec0.y -= status.stickX * handlingStat * 0.1;
            if (physics.rotVec1.z = 0) physics.rotVec1.z = status.stickX * 0.05f;
            else physics.rotVec1.z = status.stickX * 0.1f;
            movement.dir.x = forward.x;
            movement.dir.z = forward.z;
            movement.lastDir.x = forward.x;
            movement.lastDir.z = forward.z;
            movement.vel1Dir.x = forward.x;
            movement.vel1Dir.z = forward.z;
            movement.dirDiff.x = 0.0f;
            movement.dirDiff.z = 0.0f;
        }
    }
}
// On Water, BlueLeopard
static float PseudoRandomWaveFactor() {
    float phase = raceFrameCount + (waterVariant * 100);
    float slowWave = sin(phase * 0.013f) * 0.5f;
    float fastWave = sin(phase * 0.031f) * 0.3f;
    float waveFactor = 1.0f + slowWave + fastWave;
    return waveFactor;
}


void OnWaterPhysics() {
    sin(kmp->areaSection->GetHolder(Teleport)->raw->setting2 / 100)}
}

// Dynamic Objects, BlueLeopad
void SoftObject(Kart::Physics& physics, )
*/

void KMP(Kart::Sub& sub, u8 playerId) {
    if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_WW || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_WW) return;
    Kart::Status* status = sub.kartStatus;
    if (!status) return;
    Kart::Physics* physics = &sub.GetPhysics();
    if (!physics) return;
    Kart::Movement* movement = sub.kartMovement;
    if (!movement) return;
    Kart::Collision* collision = sub.kartCollision;
    if (!collision) return;
    KMP::Manager* kmp = KMP::Manager::sInstance;
    if (!kmp) return;
    Input::State* inputState = &sub.GetControllerHolder().inputStates[0];
    if (!inputState) return;
    KMPDetector(playerId);
    Effects(*status, playerId);
    GravityVersion(*physics, playerId);
    AntiGravity(*physics, *movement, playerId);
    //SuperMarioGalaxyGravity(*movement, *physics, playerId);
    //LaunchPadPhysics(*movement, *physics, playerId);
    GliderPhysics(*movement, *status, *physics, playerId);
    RailRide(*movement, *status, *inputState, *physics, playerId);
    Teleportation(*physics, *movement, playerId);
    AirBoost(*movement, playerId);
    Wind(*physics, playerId);
    PoleBounce(*collision, *inputState, *movement, playerId);
}

void KMPAREAs() {
    Kart::Manager* kartManager = Kart::Manager::sInstance;
    if (!kartManager) return;
    for (u8 i = 0; i < kartManager->playerCount; i++) {
        Kart::Player* player = kartManager->GetKartPlayer(i);
        if (player && player->kartSub) {
            KMP(*player->kartSub, i);
        }
    }
}
RaceFrameHook KMPExpander(KMPAREAs);

} // Race
} // MKWG