#include <kamek.hpp>
#include <runtimeWrite.hpp>
#include <MarioKartWii/Input/ControllerHolder.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartPlayer.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartStatus.hpp>
#include <MarioKartWii/Kart/KartValues.hpp>
#include <MarioKartWii/Kart/Hitbox.hpp>
#include <MarioKartWii/KCL/Collision.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <PulsarSystem.hpp>
#include <KAE/KMPAREAExpander.hpp>
#include <core/rvl/OS/OS.hpp>

// KMP AREA Expander [BlueLeopard]
// This file holds the first code I ever wrote. There are definitely some huge mistakes I just need to eventually fix. The code does work though...
// At the bottom of this file is a function which is called once per frame per player (I believe).
namespace MKWG {
namespace Race {


kmRuntimeUse(0x808B5AE8); //wheelie steepness changer, used for anti-gravity [Gab]
KMPAREAExpander KMPAREAExpand[12];
u16 raceFrameCount = 0;

KMPAREAExpander::KMPAREAExpander() : 
    ConditionalObject(-1),
    ConfigurableGravity(-1), AntiGravity(-1), PointGravity(-1), Gliding(-1),
    Wind(-1), AirRing(-1),
    Rail(-1),
    Submarine(-1), Speedboat(-1),
    Teleport(-1),
    condObj(false), prevCondObj(false),
    antiGrav(false), configGrav(false), glider(false), pointGrav(false),
    inGlider(false),

    windy(false), airSpeedUp(false),
    onRail(false),
    underWater(false), onWater(false),
    portal(false),
    prevUnderWater(false),

    AREAGravity(0.0f), upDown(0.0f), underWaterGravity(0.0f),
    yawVel(0.0f), leanVel(0.0f), liftMemory(0.0f), 
    
    gliderGravity(0.0f), gliderBonusSpeed(0.0f), gliderPitch(0.0f),

    pitch(0.0f), roll(0.0f), previousPitch(0.0f),
    lift(0.0f), drag(0.0f),

    rotatedinAir(false),
    MKWorldRailRide(false),

    airBooster(0.0f), airBoosterDuration(0.0f),

    accelerate(false), brake(false), drift(false), hop(false),
    justTricked(false), inATrick(false), mtBoost(false), stopped(false),
    wall(false), ground(false), air20(false), wheelie(false),

    mega(false), mushroom(false), star(false),
    shocked(false), feather(false), oob(false), respawn(false),

    kclFlag(0), wheelFlag(0),

    xInput(0.0f), yInput(0.0f), zInput(0.0f), 

    railRideJumpDir(0.0f),

    acceleration(0.0f), handling(0.0f), handlingSpeed(0.0f),
    baseSpeed(0.0f),

    prevInGlider(false), prevHop(false), prevMega(false),
    prevMushroom(false), prevOnRail(false), prevShocked(false), 
    prevStar(false), prevOob(false), prevStopped(false), 
    prevAirSpeedUp(false), prevAntiGrav(false), teleported(false),

    waitForLaunch(0), waitForTeleport(0),
    waitBeforeJump(0), waitingTime(0), height(0),

    jumpedSlow(false), jumpedFast(false), jumped(false),

    statsFound(false),

    normalSoftSpeedLimit(0.0f) {

    memset(flagPermStay, 0, sizeof(flagPermStay));
    memset(flagKCLStay, 0, sizeof(flagKCLStay));

    antiGravVec.x = antiGravVec.y = antiGravVec.z =
    flightDir.x = flightDir.y = flightDir.z =
    cameraVec.x = cameraVec.y = cameraVec.z =
    floorNormal.x = floorNormal.y = floorNormal.z = 0.0f;
}

void KMPAREAExpander::Reset() {
    memset(flagPermStay, 0, sizeof(flagPermStay));
    memset(flagKCLStay, 0, sizeof(flagKCLStay));

    ConditionalObject = -1;
    ConfigurableGravity = -1;
    AntiGravity = -1;
    PointGravity = -1;
    Gliding = -1;
    Wind = -1;
    AirRing = -1;
    Rail = -1;
    Submarine = -1;
    Speedboat = -1;
    Teleport = -1;

    condObj = false;

    antiGrav = false;
    configGrav = false;
    glider = false;
    pointGrav = false;
    inGlider = false;

    windy = false;
    airSpeedUp = false;

    onRail = false;

    underWater = false;
    onWater = false;

    portal = false;

    AREAGravity = 0.0f;
    upDown = 0.0f;
    underWaterGravity = 0.0f;

    yawVel = 0.0f;
    leanVel = 0.0f;
    liftMemory = 0.0f; // not used atm

    gliderGravity = 0.0f;
    gliderBonusSpeed = 0.0f;
    gliderPitch = 0.0f;

    pitch = 0.0f;
    roll = 0.0f;
    previousPitch = 0.0f;
    lift = 0.0f;
    drag = 0.0f;

    antiGravVec.x = antiGravVec.y = antiGravVec.z =
    flightDir.x = flightDir.y = flightDir.z =
    cameraVec.x = cameraVec.y = cameraVec.z =
    floorNormal.x = floorNormal.y = floorNormal.z = 0.0f;

    rotatedinAir = false;

    MKWorldRailRide = false;

    airBooster = 0.0f;
    airBoosterDuration = 0.0f;

    accelerate = false;
    brake = false;
    drift = false;
    hop = false;
    justTricked = false;
    inATrick = false;
    mtBoost = false;
    stopped = false;
    wall = false;
    ground = false;
    air20 = false;
    wheelie = false;

    mega = false;
    mushroom = false;
    star = false;
    shocked = false;
    feather = false;
    oob = false;
    respawn = false;

    kclFlag = 0;
    wheelFlag = 0;

    acceleration = 0.0f;
    handling = 0.0f;
    handlingSpeed = 0.0f;
    baseSpeed = 0.0f;

    xInput = 0.0f;
    yInput = 0.0f;
    zInput = 0.0f;

    railRideJumpDir = 0.0f;

    prevInGlider = false;
    prevHop = false;
    prevMega = false;
    prevMushroom = false;
    prevOnRail = false;
    prevShocked = false;
    prevStar = false;
    prevOob = false;
    prevStopped = false;
    
    prevAirSpeedUp = false;
    prevAntiGrav = false;
    prevCondObj = false;
    prevUnderWater = false;
    teleported = false;

    waitForLaunch = 0;
    waitForTeleport = 0;
    waitBeforeJump = 0;
    waitingTime = 0;
    height = 0;

    jumpedSlow = false;
    jumpedFast = false;
    jumped = false;

    normalSoftSpeedLimit = 0.0f;

    statsFound = false;

    kmRuntimeWrite16A(0x808B5AE8, 0x3F00);
}

bool UpdateAREAFlag(s16 areaIndex, bool prevFlag, u8 playerId, u8 mechNum) {
    const Raceinfo* raceInfo = Raceinfo::sInstance;
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (areaIndex < 0) {
        KAE.flagPermStay[mechNum] = false;
        KAE.flagKCLStay[mechNum] = false;
        return false;
    }
    if (KAE.flagPermStay[mechNum]) return true;

    u8 checkpointCheck = kmp->areaSection->GetHolder(areaIndex)->raw->camera;
    u16 padding = kmp->areaSection->GetHolder(areaIndex)->raw->unknown_0x2e;
    u8 targetKCL = padding & 0xFF;
    u32 targetKCLFlag = 1u << targetKCL;

    if (padding & 0x100 && !KAE.ground) return false; // only when on ground
    if (padding & 0x200 && (KAE.ground || !KAE.air20)) return false; // only when in air
    if (padding & 0x800 && KAE.justTricked) return false;ed
    //if (checkpointCheck < raceInfo->checkpoint && checkpointCheck != 0) return false;
    if (targetKCL < 0x1F) {
        if ((padding & 0x4000) && KAE.kclFlag != 0) KAE.flagKCLStay[mechNum] = ((KAE.kclFlag & targetKCLFlag) != 0); // if 14th bit is true, mechanic stays activated until different KCL type is touched
        if ((KAE.kclFlag & targetKCLFlag) != 0) {
            if (padding & 0x8000) KAE.flagPermStay[mechNum] = true;
            return true;
        }    
        if (KAE.flagKCLStay[mechNum]) return true;
        
        return false;
    }

    if (padding & 0x8000) KAE.flagPermStay[mechNum] = true;

    return true;
}

void KMPDetector(Kart::Physics* physics, u8 playerId) {
    const Vec3& pos = physics->position;
    KMP::Manager* kmp = KMP::Manager::sInstance;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    KAE.ConditionalObject   = kmp->FindAREA(pos, (u32)-1, (u8)0x0C);
    KAE.ConfigurableGravity = kmp->FindAREA(pos, (u32)-1, (u8)0x0D);
    KAE.AntiGravity         = kmp->FindAREA(pos, (u32)-1, (u8)0x0E);
    KAE.PointGravity        = kmp->FindAREA(pos, (u32)-1, (u8)0x0F);
    KAE.Gliding             = kmp->FindAREA(pos, (u32)-1, (u8)0x10);
    KAE.Wind                = kmp->FindAREA(pos, (u32)-1, (u8)0x11);
    KAE.AirRing             = kmp->FindAREA(pos, (u32)-1, (u8)0x12);
    KAE.Rail                = kmp->FindAREA(pos, (u32)-1, (u8)0x13);
    KAE.Teleport            = kmp->FindAREA(pos, (u32)-1, (u8)0x14);
    KAE.Submarine           = kmp->FindAREA(pos, (u32)-1, (u8)0x15);
    KAE.Speedboat           = kmp->FindAREA(pos, (u32)-1, (u8)0x16);

    KAE.condObj    = UpdateAREAFlag(KAE.ConditionalObject, KAE.condObj, playerId, 0);
    KAE.configGrav = UpdateAREAFlag(KAE.ConfigurableGravity, KAE.configGrav, playerId, 1);
    KAE.antiGrav   = UpdateAREAFlag(KAE.AntiGravity, KAE.antiGrav, playerId, 2);
    KAE.pointGrav  = UpdateAREAFlag(KAE.PointGravity, KAE.pointGrav, playerId, 3);
    KAE.glider     = UpdateAREAFlag(KAE.Gliding, KAE.glider, playerId, 4);
    KAE.windy      = UpdateAREAFlag(KAE.Wind, KAE.windy, playerId, 5);
    KAE.airSpeedUp = UpdateAREAFlag(KAE.AirRing, KAE.airSpeedUp, playerId, 6);
    KAE.onRail     = UpdateAREAFlag(KAE.Rail, KAE.onRail, playerId, 7);
    KAE.portal     = UpdateAREAFlag(KAE.Teleport, KAE.portal, playerId, 8);
    KAE.underWater = UpdateAREAFlag(KAE.Submarine, KAE.underWater, playerId, 9);
    KAE.onWater    = UpdateAREAFlag(KAE.Speedboat, KAE.onWater, playerId, 10);
}

void Effects(const Kart::Status& status, u8 playerId) {
    Input::ControllerHolder& controllerHolder = status.link->GetControllerHolder();
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    u32 bitfield0 = status.bitfield0;
    u32 bitfield1 = status.bitfield1;
    u32 bitfield2 = status.bitfield2;

    KAE.accelerate  = (bitfield0 & 0x1) != 0;
    KAE.brake       = (bitfield0 & 0x2) != 0;
    KAE.drift       = (bitfield0 & (0x8 | 0x4)) != 0;
    KAE.ground      = (bitfield0 & (0x400 | 0x800 | 0x1000 | 0x40000)) != 0;
    KAE.mushroom    = (bitfield0 & 0x2000000) != 0;
    KAE.star        = (bitfield1 & 0x80000000) != 0;
    KAE.mega        = (bitfield2 & 0x8000) != 0 || (bitfield2 & 0x20000000) != 0;
    KAE.stopped     = (bitfield2 & 0x40000) != 0;
    KAE.shocked     = (bitfield2 & 0x80) != 0 || (bitfield2 & 0x10000) != 0;
    KAE.justTricked = (bitfield1 & 0x20) != 0;
    KAE.inATrick    = (bitfield1 & 0x40) != 0;
    KAE.wheelie     = (bitfield0 & 0x20000000) != 0;
    KAE.feather     = (bitfield1 & 0x4000) != 0;
    KAE.mtBoost     = (bitfield1 & 0x100000) != 0;
    KAE.wall        = (bitfield0 & (0x20 | 0x40)) != 0;
    KAE.hop         = (bitfield0 & 0x80000) != 0;
    KAE.oob         = (bitfield0 & 0x10) != 0;
    KAE.air20       = (status.airtime > 19);
    KAE.respawn     = (bitfield1 & 0x2000) != 0;
    
    KAE.xInput = status.stickX;
    KAE.yInput = status.stickY;
    if (status.airtime >= 2 && (!status.bool_0x96 || status.airtime > 19)) KAE.zInput = controllerHolder.inputStates[0].stick.z;
    else KAE.zInput = 0;

    KAE.kclFlag = 0;
    u16 wheelCount = status.link->GetWheelCount0();
    for (u8 i = 0; i < wheelCount; i++) {
        Kart::CollisionData& floorData = status.link->GetWheelPhysicsCollisionData(i);
        KAE.kclFlag |= floorData.closestFloorFlags;
        if (floorData.floorNor.x != 0.0f || floorData.floorNor.y != 0.0f || floorData.floorNor.z != 0.0f) KAE.floorNormal = floorData.floorNor;
    }
}

void FindStats(const Kart::Status& status, u8 playerId) {
    const Kart::Stats& stats = status.link->GetStats();
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (KAE.statsFound) return;

    KAE.handling = stats.manualHandling;
    KAE.handlingSpeed = stats.handlingSpeedMultiplier;
    KAE.acceleration = (stats.standard_acceleration_as[0] + stats.standard_acceleration_as[1] + stats.standard_acceleration_as[2] + stats.standard_acceleration_as[3]) * 0.25f;
    KAE.baseSpeed = stats.baseSpeed;
    KAE.statsFound = true;
}

void PrevState(u8 playerId) {
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    KAE.prevHop = KAE.hop;
    KAE.prevMega = KAE.mega;
    KAE.prevMushroom = KAE.mushroom;
    KAE.prevShocked = KAE.shocked;
    KAE.prevStar = KAE.star;
    KAE.prevOob = KAE.oob;
    KAE.prevStopped = KAE.stopped;
    KAE.prevAirSpeedUp = KAE.airSpeedUp;
    KAE.prevAntiGrav = KAE.antiGrav;
}

void Safe() {
    raceFrameCount = 0;
    for (u8 i = 0; i < 12; i++) {
        KMPAREAExpand[i].Reset();
    }
    ConditionalObjectReset();
}
RaceLoadHook Saver(Safe);

void KMP(Kart::Sub& sub, u8 playerId) {
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
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    Effects(*status, playerId);
    FindStats(*status, playerId);

    KMPDetector(physics, playerId);

    AntiGravity(*movement, playerId);
    // PointGravity(*movement, *physics, playerId);
    // UnderWaterPhysics(*movement, *physics, *status, playerId)
    GliderState(*physics, playerId);
    
    //if (KAE.onrail && KAE.ground) RailRideEject(*inputState);

    AirBoost(*movement, playerId);
    Wind(*physics, playerId);

    PrevState(playerId);
}

void KMPAREAs() {
    if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_VS_WW || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_BT_WW) return;
    Kart::Manager* kartManager = Kart::Manager::sInstance;
    if (!kartManager) return;

    ConditionalObjectFrameUpdate();
    CondObjPrevState();

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