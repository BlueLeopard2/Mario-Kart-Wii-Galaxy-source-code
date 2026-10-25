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

// Camera Rotations [Patchzy, BlueLeopard]
bool BuildGravityContext(RaceCamera* camera, Kart::Link* kartPlayer, CameraGravityContext& context) {
    if (camera == 0 || kartPlayer == 0) return false;

    const u8 playerId = kartPlayer->GetPlayerIdx();
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    if (!KAE.antiGrav && !KAE.configGrav) return false;
    
    if (!KAE.camera) return false;

    context.playerPos = kartPlayer->GetPosition();

    context.up = KAE.floorNormal0;

    if (!NormalizeSafe(context.up)) {
        context.up = CameraVec(*camera, kCameraBaseUpOffset);

        if (!NormalizeSafe(context.up)) {
            context.up = WorldUp();
        }
    }

    const float hopOffset = CameraFloat(*camera, kCameraHopOffset);
    context.anchorPos = SubVec(
        context.playerPos,
        MultiplyVecByFloat(context.up, hopOffset)
    );

    return true;
}

void UpdateCameraTargetImpl(float positionInterp, float topDotInterp, float heightInterp, RaceCamera* camera, GameCamValues* values, Kart::Link* kartPlayer, Vec3* playerPos) {
    if (camera == 0 || values == 0 || kartPlayer == 0 || playerPos == 0) return;

    CameraGravityContext context;
    if (!BuildGravityContext(camera, kartPlayer, context)) {
        sOriginalUpdateCameraTarget(positionInterp, topDotInterp, heightInterp, camera, values, kartPlayer, playerPos);
        return;
    }

    //CameraVec(*camera, kCameraUpAxisOffset) = BuildPerpendicularInPlane(context.up, camera->angleOfRotAroundPlayer, context.up); it looks like this line doesn't do anything. Feels the same with and without it.
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

void FinalizeCameraWithGravity(float upInterp, RaceCamera* camera, Kart::Link* kartPlayer, u32 mode) {
    if (kartPlayer == 0) return;

    const Kart::PhysicsHolder& holder = kartPlayer->GetPhysicsHolder();
    if (camera == 0 || holder.physics == 0) return;

    const u8 playerId = kartPlayer->GetPlayerIdx();
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    CameraGravityContext context;
    if (!BuildGravityContext(camera, kartPlayer, context) || (camera->bitfield & kCameraFlagScript) != 0) {
        sOriginalFinalizeCamera(upInterp, camera, kartPlayer, mode);
        return;
    }

    Vec3 cameraOffset = SubVec(BasePosition(*camera), context.playerPos);
    Vec3 up = KAE.floorNormal0;
    if (NormalizeSafe(up)) {
        Vec3 forward = context.kartForwardPlanar;

        if (NormalizeSafe(forward)) {
            Vec3 right = Cross(forward, up);

            if (NormalizeSafe(right)) {
                const float lateralOffset = Dot(cameraOffset, right);

                BasePosition(*camera) = SubVec(BasePosition(*camera), MultiplyVecByFloat(right, lateralOffset));
            } 
        }
    }

    Vec3 lookDir = SubVec(BaseTarget(*camera), BasePosition(*camera));
    if (!NormalizeSafe(lookDir)) {
        BaseUp(*camera) = context.up;
        return;
    }

    Vec3 desiredUp = KAE.floorNormal0;
    if (!NormalizeSafe(desiredUp)) desiredUp = context.up;

    Vec3& smoothedUp = CameraVec(*camera, kCameraSmoothedUpOffset);
    if (!NormalizeSafe(smoothedUp)) smoothedUp = desiredUp;

    Vec3 delta = SubVec(desiredUp, smoothedUp);
    const float deltaLength = VecLength(delta);

    if (deltaLength > 0.05) {
        delta = MultiplyVecByFloat(delta, kCameraRollDecayAuto / deltaLength);
    }

    smoothedUp = NormalizedOrFallback(AddVec(smoothedUp, delta), desiredUp);

    NormalizeSafe(smoothedUp);
    float blend = Clamp(upInterp, 0.0f, 1.0f);
    if (blend < kCameraGravityUpMinBlend) blend = kCameraGravityUpMinBlend;

    CameraVec(*camera, kCameraBaseUpOffset) = smoothedUp;
    BaseUp(*camera) = smoothedUp;

    CameraVec(*camera, kCameraLastUpOffset) = BaseUp(*camera);
    CameraVec(*camera, kCameraSmoothedUpOffset) = BaseUp(*camera);
    CameraFloat(*camera, kCameraSmoothedUpBlendOffset) = blend;
    CameraQuat(*camera, kCameraSmoothedQuatOffset) = IdentityQuat();

    if (mode == 0) {
        BaseTarget(*camera) = AddVec(BaseTarget(*camera), MultiplyVecByFloat(BaseUp(*camera), CameraFloat(*camera, kCameraField110Offset)));
    }

    Vec3 right = Cross(smoothedUp, lookDir);
    right.Normalize();

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
    //BaseUp(*camera) = context.up;
    CameraFloat(*camera, kCameraField124Offset) = 0.0f;
    CameraFloat(*camera, kCameraField128Offset) = 0.0f;

    FinalizeCameraWithGravity(1.0f, camera, kartPlayer, 0);
    camera->playerPos = playerPos;
}

} // Race
} // MKWG