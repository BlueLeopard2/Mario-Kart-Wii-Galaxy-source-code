#include <kamek.hpp>
#include <core/egg/Math/Math.hpp>
#include <MarioKartWii/3D/Camera/RaceCamera.hpp>
#include <MarioKartWii/Kart/KartLink.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartPointers.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <KAE/Camera.hpp>
#include <KAE/KAEMath.hpp>
#include <KAE/KMPAREAExpander.hpp>

// This is supposed to be the camera for anti gravity, which moves with the karts rotation. It does work, but it's very flickery and annoying in many ways, so it's commented out for now.

namespace MKWG {
namespace Race {

// Camera Rotations [Patchzy]
bool TryGetCameraUp(Kart::Link* kartPlayer, Vec3& cameraUp) {
    if (kartPlayer == 0) return false;
    const Kart::PhysicsHolder& holder = kartPlayer->GetPhysicsHolder();
    if (holder.physics == 0) return false;

    const u8 playerId = kartPlayer->GetPlayerIdx();
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (!KAE.antiGrav && !KAE.configGrav) return false;

    cameraUp = KAE.floorNormal;
    if (NormalizeSafe(cameraUp)) return true;

    cameraUp = WorldUp();
    return false;
}

bool BuildGravityContext(RaceCamera* camera, Kart::Link* kartPlayer, CameraGravityContext& context) {
    if (camera == 0 || kartPlayer == 0) return false;
    if (!TryGetCameraUp(kartPlayer, context.up)) return false;

    context.playerPos = kartPlayer->GetPosition();
    const float hopOffset = CameraFloat(*camera, kCameraHopOffset);
    context.anchorPos = SubVec(context.playerPos, MultiplyVecByFloat(context.up, hopOffset));
    return true;
}

void UpdateCameraTargetImpl(float positionInterp, float topDotInterp, float heightInterp, RaceCamera* camera, GameCamValues* values,
                            Kart::Link* kartPlayer, Vec3* playerPos) {
    if (camera == 0 || values == 0 || kartPlayer == 0 || playerPos == 0) return;

    CameraGravityContext context;
    if (!BuildGravityContext(camera, kartPlayer, context)) {
        sOriginalUpdateCameraTarget(positionInterp, topDotInterp, heightInterp, camera, values, kartPlayer, playerPos);
        return;
    }

    CameraVec(*camera, kCameraUpAxisOffset) = BuildPerpendicularInPlane(context.up, camera->angleOfRotAroundPlayer, context.up);
    *playerPos = context.anchorPos;

    if ((camera->bitfield & kCameraFlagFreezeMovement) != 0) return;

    float targetYPos = camera->camParams != 0 ? camera->camParams->yTargetPos : 0.0f;
    const Raceinfo* raceinfo = Raceinfo::sInstance;
    if (raceinfo == 0 || raceinfo->stage < RACESTAGE_COUNTDOWN) {
        float smooth = kCameraFieldInterpNormal;
        if (Racedata::sInstance != 0 &&
            Racedata::sInstance->racesScenario.settings.gamemode == MODE_MISSION_TOURNAMENT) {
            smooth = kCameraFieldInterpMission;
        }

        float& smoothedTargetYPos = CameraFloat(*camera, kCameraField344Offset);
        smoothedTargetYPos += (targetYPos - smoothedTargetYPos) * smooth;
        targetYPos = smoothedTargetYPos;
    }

    const float verticalDistance = sGetCameraVerticalDistance(kartPlayer);
    const float verticalTarget = targetYPos - verticalDistance;

    float topDot = 0.0f;
    if (kartPlayer->pointers != 0 && kartPlayer->pointers->kartMovement != 0) {
        topDot = topDotInterp * Abs(Dot(kartPlayer->pointers->kartMovement->unknown_0x80, context.up));
    }

    float zipperOffset = 0.0f;
    float targetLift = 0.0f;
    if (kartPlayer->pointers != 0 && kartPlayer->pointers->kartStatus != 0 &&
        (kartPlayer->pointers->kartStatus->bitfield1 & kStatusBitfield1OverZipper) != 0) {
        zipperOffset = kCameraZipperFall;
        const Kart::PhysicsHolder& physicsHolder = const_cast<Kart::Link*>(kartPlayer)->GetPhysicsHolder();
        const Vec3* speed = physicsHolder.physics != 0 ? &physicsHolder.physics->speed : 0;
        if (speed != 0 && Dot(*speed, context.up) < 0.0f) {
            targetLift = kCameraZipperRise * CameraFloat(*camera, kCameraField118Offset);
        }
    }

    float& field10c = CameraFloat(*camera, kCameraField10cOffset);
    float& field110 = CameraFloat(*camera, kCameraField110Offset);
    field10c += positionInterp * (zipperOffset - field10c);
    field110 += positionInterp * (targetLift - field110);

    float& field108 = CameraFloat(*camera, kCameraField108Offset);
    if ((camera->bitfield & kCameraFlagFalling) == 0) {
        float& field104 = CameraFloat(*camera, kCameraField104Offset);
        field104 += positionInterp * (topDot - field104);
        field108 += heightInterp * ((field10c + verticalTarget + field104) - field108);
    } else {
        field108 -= kCameraFallStep;
        if (field108 < kCameraFallMin) field108 = kCameraFallMin;

        if ((camera->bitfield & kCameraFlagFallingPastTarget) == 0 && GameCamFloat(*values, kGameCamUnknown20Offset) < context.playerPos.y) {
            camera->bitfield |= kCameraFlagFallingPastTarget;
        }
    }

    camera->unknown_0xAC = AddVec(context.anchorPos, MultiplyVecByFloat(CameraVec(*camera, kCameraUpAxisOffset), values->verticalOffset2));
    camera->unknown_0xAC = AddVec(camera->unknown_0xAC, MultiplyVecByFloat(context.up, field108));
}

void UpdateCameraTargetWithGravity(float positionInterp, float topDotInterp, float heightInterp, RaceCamera* camera,
                                   GameCamValues* values, Kart::Link* kartPlayer, Vec3* playerPos) {
    UpdateCameraTargetImpl(positionInterp, topDotInterp, heightInterp, camera, values, kartPlayer, playerPos);
}
kmCall(0x805a2444, UpdateCameraTargetWithGravity);

void UpdateCameraRollWithGravity(RaceCamera* camera, Kart::Link* kartPlayer) {
    if (camera == 0 || kartPlayer == 0) return;

    Vec3 cameraUp;
    if (!TryGetCameraUp(kartPlayer, cameraUp)) {
        sOriginalUpdateCameraRoll(camera, kartPlayer);
        return;
    }

    const Kart::Status* status = kartPlayer->pointers != 0 ? kartPlayer->pointers->kartStatus : 0;
    if (status == 0) return;

    float& roll = camera->unknown_0xf4[0];
    if ((status->bitfield0 & (kStatusBitfield0DriftManual | kStatusBitfield0Hop)) == 0) {
        const float step = (status->bitfield4 & kStatusBitfield4AutomaticDrift) != 0 ? kCameraRollDecayAuto : kCameraRollDecay;
        if (roll < 0.0f) {
            roll += step;
            if (roll > 0.0f) roll = 0.0f;
        } else if (roll > 0.0f) {
            roll -= step;
            if (roll < 0.0f) roll = 0.0f;
        }
        return;
    }

    const int hopStickX = kartPlayer->pointers != 0 && kartPlayer->pointers->kartMovement != 0
                              ? kartPlayer->pointers->kartMovement->hopStickX
                              : 0;
    float rollDelta = kCameraRollStep * static_cast<float>(hopStickX);
    if (kartPlayer->GetType() == INSIDE_BIKE) rollDelta = -rollDelta;
    roll += rollDelta;

    float maxRoll = (status->bitfield4 & kStatusBitfield4AutomaticDrift) != 0 ? kCameraRollLimitAuto : kCameraRollLimit;
    const float dot = Clamp(Dot(camera->angleOfRotAroundPlayer, cameraUp), -1.0f, 1.0f);
    const float tiltDegrees = 90.0f - (EGG::Math::Acos(dot) * kRadToDeg);
    if (tiltDegrees < 0.0f) maxRoll += kCameraPitchRecover * Abs(tiltDegrees);

    if (roll > maxRoll) roll = maxRoll;
    if (roll < -maxRoll) roll = -maxRoll;
}
kmCall(0x805a2450, UpdateCameraRollWithGravity);

void FinalizeCameraWithGravity(float upInterp, RaceCamera* camera, Kart::Link* kartPlayer, u32 mode) {
    const Kart::PhysicsHolder& holder = kartPlayer->GetPhysicsHolder();
    if (camera == 0 || kartPlayer == 0 || holder.physics == 0) return;

    const u8 playerId = kartPlayer->GetPlayerIdx();
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    CameraGravityContext context;
    if (!BuildGravityContext(camera, kartPlayer, context) || (camera->bitfield & kCameraFlagScript) != 0) {
        sOriginalFinalizeCamera(upInterp, camera, kartPlayer, mode);
        return;
    }

    if (Dot(context.up, WorldUp()) < -0.75f) {
        Vec3 positionOffset = SubVec(BasePosition(*camera), BaseTarget(*camera));
        BasePosition(*camera) = AddVec(BaseTarget(*camera), RemapWorldVerticalToGravityUp(positionOffset, context.up));
    }

    Vec3 lookDir = SubVec(BaseTarget(*camera), BasePosition(*camera));
    if (!NormalizeSafe(lookDir)) {
        BaseUp(*camera) = context.up;
        return;
    }

    Vec3 desiredUp = KAE.floorNormal;
    if (!NormalizeSafe(desiredUp)) desiredUp = context.up;

    Vec3& smoothedUp = CameraVec(*camera, kCameraSmoothedUpOffset);
    if (!NormalizeSafe(smoothedUp)) smoothedUp = desiredUp;

    float blend = Clamp(upInterp, 0.0f, 1.0f);
    if (blend < kCameraGravityUpMinBlend) blend = kCameraGravityUpMinBlend;

    smoothedUp = MultiplyVecByFloat(AddVec(smoothedUp, SubVec(desiredUp, smoothedUp)), blend);
    smoothedUp = NormalizedOrFallback(smoothedUp, desiredUp);
    CameraVec(*camera, kCameraBaseUpOffset) = smoothedUp;

    Vec3 right = Cross(smoothedUp, lookDir);
    Vec3 worldRight = MakeVec(1.0f, 0.0f, 0.0f);
    if (!NormalizeSafe(right)) {
        right = Cross(context.up, lookDir);
        if (!NormalizeSafe(right)) right = BuildPerpendicularInPlane(lookDir, context.up, worldRight);
    }

    BaseUp(*camera) = Cross(lookDir, right);
    BaseUp(*camera) = NormalizedOrFallback(BaseUp(*camera), smoothedUp);

    if (Dot(BaseUp(*camera), context.up) < kCameraGravityUpMinDot) {
        BaseUp(*camera) = NormalizedOrFallback(context.up, WorldUp());
    }

    CameraVec(*camera, kCameraLastUpOffset) = BaseUp(*camera);
    CameraVec(*camera, kCameraSmoothedUpOffset) = BaseUp(*camera);
    CameraFloat(*camera, kCameraSmoothedUpBlendOffset) = blend;
    CameraQuat(*camera, kCameraSmoothedQuatOffset) = IdentityQuat();

    if (mode == 0) {
        BaseTarget(*camera) = AddVec(BaseTarget(*camera), MultiplyVecByFloat(BaseUp(*camera), CameraFloat(*camera, kCameraField110Offset)));
    }

    if (camera->sub338 != 0) {
        sOriginalUpdateSub338(camera->sub338);
        sOriginalUpdateSub338(reinterpret_cast<u8*>(camera->sub338) + 0x1c);

        const float lateralOffset = Field<float>(camera->sub338, 0x10);
        const float verticalOffset = Field<float>(camera->sub338, 0x2c);
        BaseTarget(*camera) = AddVec(BaseTarget(*camera), MultiplyVecByFloat(right, lateralOffset));
        BaseTarget(*camera) = AddVec(BaseTarget(*camera), MultiplyVecByFloat(BaseUp(*camera), verticalOffset));
    }
}
kmCall(0x805a2ab8, FinalizeCameraWithGravity);

void RefreshCameraOnRespawn(const Kart::Link& link) {
    if (link.pointers == 0 || link.pointers->camera == 0) return;

    RaceCamera* camera = link.pointers->camera;
    Kart::Link* kartPlayer = const_cast<Kart::Link*>(&link);

    CameraGravityContext context;
    if (!BuildGravityContext(camera, kartPlayer, context)) return;

    CameraVec(*camera, kCameraUpAxisOffset) = BuildPerpendicularInPlane(context.up, camera->angleOfRotAroundPlayer, context.up);

    Vec3 playerPos = context.anchorPos;
    UpdateCameraTargetImpl(1.0f, 1.0f, 1.0f, camera, &camera->forwards, kartPlayer, &playerPos);
    sOriginalUpdateCameraValues(1.0f, 1.0f, 1.0f, camera, &camera->forwards, 0, kartPlayer, &playerPos);

    CameraVec(*camera, kCameraBaseUpOffset) = context.up;
    CameraVec(*camera, kCameraLastUpOffset) = context.up;
    CameraVec(*camera, kCameraSmoothedUpOffset) = context.up;
    CameraFloat(*camera, kCameraSmoothedUpBlendOffset) = 1.0f;
    CameraQuat(*camera, kCameraSmoothedQuatOffset) = IdentityQuat();

    BasePosition(*camera) = camera->forwards.position;
    BaseTarget(*camera) = camera->unknown_0xAC;
    BaseUp(*camera) = context.up;
    CameraFloat(*camera, kCameraField124Offset) = 0.0f;
    CameraFloat(*camera, kCameraField128Offset) = 0.0f;

    FinalizeCameraWithGravity(1.0f, camera, kartPlayer, 0);
    camera->playerPos = playerPos;
}

} // Race
} // MKWG