#ifndef KMPAREAEXPANDER_HPP
#define KMPAREAEXPANDER_HPP

#include <kamek.hpp>
#include <core/rvl/mtx/mtx.hpp>
#include <MarioKartWii/Math/Vector.hpp>
#include <KAE/KAEMath.hpp>

// This is just a storage room so that everything is connected basically.

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
    // AREA detection
    bool flagPermStay[11];
    bool flagKCLStay[11];

    s16 conditionalObject;
    s16 configurableGravity;
    s16 antiGravity;
    s16 pointGravity;
    s16 gliding;
    s16 wind;
    s16 airRing;
    s16 rail;
    s16 teleport;

    // AREA activation state
    bool condObj;
    bool configGrav;
    bool antiGrav;
    bool pointGrav;
    bool glider;
    bool windy;
    bool airSpeedUp;
    bool onRail;
    bool portal;
    bool camera;

    // Mechanic state
    bool inGlider;
    bool isRailRiding;
    bool lastPointReached;
    bool rotatedinAir;
    bool isRespawning;

    // Gravity and physics
    float upDown;
    float liftMemory;
    float gliderGravity;

    Vec3 configGravVec;
    Vec3 antiGravVec;
    Vec3 upVel;
    Vec3 sideVel;
    Vec3 flightDir;
    Vec3 cameraVec;
    Vec3 floorNormal;
    Vec3 floorNormal0;

    // Glider
    float gliderYaw;
    float gliderRoll;
    float gliderPitch;

    // Air boost
    float airBoosterDuration;

    // Rail riding
    u16 routeId;
    u8 nextPoint;
    bool direction;
    float jumpSpeed;
    bool jumped;

    // Teleportation
    bool teleported;
    u16 waitForTeleport;
    u16 waitingTime;

    // Player input
    float xInput;
    float yInput;
    float zInput;

    bool accelerate;
    bool brake;
    bool drift;
    bool hop;
    bool boost;
    bool justTricked;
    bool inATrick;
    bool mtBoost;
    bool stopped;
    bool wall;
    bool ground;
    bool air20;
    bool wheelie;

    // Item and player status
    bool mega;
    bool mushroom;
    bool star;
    bool shocked;
    bool feather;
    bool oob;
    bool respawn;
    bool hit;
    bool tc;
    bool bullet;

    // Collision
    u32 kclFlag;
    u32 wheelFlag;

    // Kart statistics
    float acceleration;
    float handling;
    float handlingSpeed;
    float baseSpeed;
    float hardSpeedLimit;
    float normalSoftSpeedLimit;

    bool statsFound;

    // Previous-frame state
    bool prevCondObj;
    bool prevInGlider;
    bool prevHop;
    bool prevMega;
    bool prevMushroom;
    bool prevOnRail;
    bool prevShocked;
    bool prevStar;
    bool prevOob;
    bool prevStopped;
    bool prevAirSpeedUp;
    bool prevAntiGrav;

    KMPAREAExpander();
    void Reset();
};

extern KMPAREAExpander KMPAREAExpand[12];

// In FrameRaceHook
// General functions
void KMPDetector(Kart::Physics* physics, u8 playerId);
void Effects(const Kart::Status& status, u8 playerId);
void Safe();
void KMP(Kart::Sub& sub, u8 playerId);
void KMPAREAs();

// Mechanics
void GliderState(Kart::Physics& physics, u8 playerId);
void GliderMovement(Kart::Movement& movement, u8 playerId);

void AntiGravity(Kart::Movement& movement, u8 playerId);

void PointGravity(Kart::Movement& movement, Kart::Physics& physics, u8 playerId);

void AirBoost(Kart::Movement& movement, u8 playerId);

void Wind(Kart::Physics& physics, u8 playerId);


// Conditional objects 
void ConditionalObjectFrameUpdate();
void ConditionalObjectReset();
bool ConditionalObjAction(u8 playerId);
void CondObjPrevState();


// Just before Physics::Update

// point gravity
Vec3 PointGravVec(Kart::Physics& physics, u8 playerId);

// configurable gravity
void RespawnInTiltedGravity(Kart::Physics& physics, u8 playerId);
void ConfigurableGravity(Kart::Physics& physics, u8 playerId);

// railride
void RailRideReset(Kart::Movement& movement, u8 playerId);
void DetermineNextPoint(Kart::Physics& physics, Kart::Movement& movement, u8 playerId);
void RailRideStart(Kart::Physics& physics, u8 playerId);
void RailRidePhysics(Kart::Physics& physics, Kart::Movement& movement, u8 playerId);
void RailRideRotation(Kart::Physics& physics, u8 playerId);
void RailRideJumpStart(Input::State* inputState, Kart::Physics& physics, Kart::Movement& movement, Kart::Status& status, u8 playerId);
void RailRideJumpPhysics(Kart::Movement& movement, Kart::Physics& physics, Kart::Status& status, u8 playerId);
void RailRideStop(Kart::Movement& movement, u8 playerId);

// anti gravity
void AntiGravPhysics(Kart::Physics& physics, u8 playerId);

// teleportation
void Teleportation(Kart::Physics& physics, u8 playerId);
void RotateSpeedAfterTP(Kart::Movement& movement, Kart::Physics& physics, u8 playerId);

// glider
void GliderGravity(Kart::Movement& movement, Kart::Physics& physics, u8 playerId);
void GliderRotation(Kart::Movement& movement, Kart::Physics& physics, u8 playerId);
//void UpdatePitch(Kart::Physics& physics, u8 playerId);
void UpdateYaw(Kart::Physics& physics, u8 playerId);
void UpdateRoll(Kart::Physics& physics, u8 playerId);
void UpdateFlightDirection(Kart::Movement& movement, Kart::Physics& physics, u8 playerId);
void UpdateLift(Kart::Movement& movement, Kart::Physics& physics, u8 playerId);
void ExitGlider(Kart::Physics& physics, u8 playerId);

} // Race
} // MKWG

#endif
