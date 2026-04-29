#ifndef KMPAREAEXPANDER_HPP
#define KMPAREAEXPANDER_HPP

#include <kamek.hpp>
#include <core/rvl/mtx/mtx.hpp>
#include <MarioKartWii/Math/Vector.hpp>
#include <KAE/KAEMath.hpp>

namespace Kart {
class Status;
class Physics;
class Movement;
class Sub;
class Collision;
} // Kart                              

namespace Input {
class State;
} // Input

namespace MKWG {
namespace Race {

// Per-player state KAE
struct KMPAREAExpander {
    bool flagPermStay;
    bool flagKCLStay;
    u32 prevWheelFlags;
    // AREA indices (>=0 means player is inside the AREA)
    s16 ConditionalObject;
    s16 ConfigurableGravity;
    s16 AntiGravity;
    s16 PointGravity;
    s16 Gliding;
    s16 Wind;
    s16 AirRing;
    s16 Rail;
    s16 Submarine;
    s16 Speedboat;
    s16 Teleport;

    // Current AREA flags
    bool condObj;
    bool antiGrav;
    bool configGrav;
    bool glider;
    bool pointGrav;
    bool inGlider;

    // Air
    bool windy;
    bool airSpeedUp;

    // Ground
    bool onRail;

    // Water
    bool onWater;
    bool underWater;

    // Other
    bool portal;
    bool launch;


    // Gravity values
    float AREAGravity;
    float upDown;
    float underWaterGravity;
    float yawVel;
    float leanVel;
    float zMemory;
    float xSideSpeed;
    float zSideSpeed;

    Vec3 antiGravVec;
    Vec3 cameraVec;
    Vec3 floorNormal;

    bool rotatedinAir;

    // RailRide modes
    bool MKWorldRailRide;


    // Air boost
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
    bool air20;
    bool wheelie;

    bool mega;
    bool mushroom;
    bool star;
    bool shocked;
    bool feather;
    bool oob;

    u16 kclFlag;
    u32 wheelFlag;

    float xInput;
    float yInput;
    float zInput;

    float acceleration;
    float handling;
    float handlingSpeed;
    float baseSpeed;


    // Previous frame state

    bool prevCondObj;

    bool prevHop;
    bool prevMega;
    bool prevMushroom;

    bool prevOnRail;

    bool prevUnderWater;

    bool prevDest;
    bool prevShocked;
    bool prevStar;
    bool prevStopped;

    bool prevAirSpeedUp;

    bool prevAntiGrav;
    bool prevGliderPointGrav;
    bool prevInGlider;

    bool teleported;


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

    bool statsFound;


    KMPAREAExpander();
    void Reset();
};

extern KMPAREAExpander KMPAREAExpand[12];
extern u16 raceFrameCount;

// In FrameRaceHook {
    // General functions
    void KMPDetector(Kart::Status& status, u8 playerId);
    void Effects(const Kart::Status& status, u8 playerId);
    void Safe();
    void KMP(Kart::Sub& sub, u8 playerId);
    void KMPAREAs();

    // Mechanics
    void GliderState(Kart::Physics& physics, u8 playerId);
    void GliderMovement(Kart::Movement& movement, u8 playerId);
    void AntiGravity(Kart::Movement& movement, u8 playerId);
    void SuperMarioGalaxyGravity(Kart::Movement& movement, Kart::Physics& physics, u8 playerId);
    void UnderWaterPhysics(Kart::Movement& movement, Kart::Physics& physics, const Kart::Status& status, u8 playerId);
    void RailRide(Input::State& inputState, Kart::Movement& movement, Kart::Physics& physics, Kart::Status& status, u8 playerId);
    void AirBoost(Kart::Movement& movement, u8 playerId);
    void Wind(Kart::Physics& physics, u8 playerId);
    

    // Conditional objects 
    void ConditionalObjectFrameUpdate();
    void ConditionalObjectReset();
    bool ConditionalObjAction(u8 playerId);
    void CondObjPrevState();
// }

// Just before Physics::Update
void ConfigurableGravity(Kart::Physics& physics, u8 playerId);
void RailRidePhysics(Kart::Physics& physics, u8 playerId); 
void AntiGravPhysics(Kart::Physics& physics, u8 playerId);
void GliderGravity(Kart::Physics& physics, u8 playerId);
void GliderRotation(Kart::Movement& movement, Kart::Physics& physics, u8 playerId);
void Teleportation(Kart::Physics& physics, u8 playerId);
void RotateSpeedAfterTP(Kart::Movement& movement, Kart::Physics& physics, u8 playerId);

} // Race
} // MKWG

#endif
