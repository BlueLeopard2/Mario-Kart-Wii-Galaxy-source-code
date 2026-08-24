#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <kamek.hpp>
#include <runtimeWrite.hpp>
#include <core/rvl/mtx/mtx.hpp>
#include <MarioKartWii/Math/Vector.hpp>
#include <core/egg/Math/Quat.hpp>
#include <KAE/KAEMath.hpp>

// UpdateCameraTargetImpl
// UpdateCameraRollWithGravity
// FinalizeCameraWithGravity

typedef unsigned char u8;
typedef unsigned int u32;

template <typename T>
inline T& Field(void* base, u32 offset) {
    return *reinterpret_cast<T*>(reinterpret_cast<u8*>(base) + offset);
}

template <typename T>
inline const T& Field(const void* base, u32 offset) {
    return *reinterpret_cast<const T*>(reinterpret_cast<const u8*>(base) + offset);
}

const float kCameraFieldInterpMission = 0.07f;
const float kCameraFieldInterpNormal = 0.05f;
const float kCameraFallStep = 10.0f;
const float kCameraFallMin = -50.0f;
const float kCameraZipperRise = 100.0f;
const float kCameraZipperFall = -100.0f;
const float kCameraRollDecay = 1.0f;
const float kCameraRollDecayAuto = 0.5f;
const float kCameraRollStep = -0.5f;
const float kCameraRollLimit = 15.0f;
const float kCameraRollLimitAuto = 10.0f;
const float kCameraPitchRecover = 0.03f;
const float kCameraGravityUpMinDot = 0.15f;
const float kCameraGravityUpMinBlend = 0.15f;
const float kRadToDeg = 57.29578f;
const float kVecEpsilon = 0.0001f;

const u32 kStatusBitfield0DriftManual = 0x8;
const u32 kStatusBitfield0Hop = 0x80000;
const u32 kStatusBitfield1OverZipper = 0x400;
const u32 kStatusBitfield4AutomaticDrift = 0x10;

const u16 kCameraFlagFalling = 0x1;
const u16 kCameraFlagFallingPastTarget = 0x2;
const u16 kCameraFlagScript = 0x4;
const u16 kCameraFlagFreezeMovement = 0x10;

const u32 kBasePositionOffset = 0x64;
const u32 kBaseTargetOffset = 0x70;
const u32 kBaseUpOffset = 0x7c;
const u32 kCameraHopOffset = 0xfc;
const u32 kCameraField104Offset = 0x104;
const u32 kCameraField108Offset = 0x108;
const u32 kCameraField10cOffset = 0x10c;
const u32 kCameraField110Offset = 0x110;
const u32 kCameraField118Offset = 0x118;
const u32 kCameraField124Offset = 0x124;
const u32 kCameraField128Offset = 0x128;
const u32 kCameraUpAxisOffset = 0x138;
const u32 kCameraBaseUpOffset = 0x150;
const u32 kCameraLastUpOffset = 0x15c;
const u32 kCameraSmoothedUpOffset = 0x168;
const u32 kCameraSmoothedUpBlendOffset = 0x174;
const u32 kCameraSmoothedQuatOffset = 0x178;
const u32 kCameraField344Offset = 0x344;

const u32 kGameCamUnknown20Offset = 0x20;

kmRuntimeUse(0x805a2cfc);
kmRuntimeUse(0x805a3070);
kmRuntimeUse(0x805a34b0);
kmRuntimeUse(0x805a40d0);
kmRuntimeUse(0x805a9a40);
kmRuntimeUse(0x805909c8);

typedef void (*CameraTargetFunc)(float f1, float f2, float f3, RaceCamera* camera, GameCamValues* values, Kart::Link* kartPlayer, Vec3* playerPos);
typedef void (*CameraRollFunc)(RaceCamera* camera, Kart::Link* kartPlayer);
typedef void (*CameraValuesFunc)(float f1, float f2, float f3, RaceCamera* camera, GameCamValues* values, u32 reverse, Kart::Link* kartPlayer, Vec3* playerPos);
typedef void (*CameraFinalizeFunc)(float f1, RaceCamera* camera, Kart::Link* kartPlayer, u32 mode);
typedef void (*CameraSub338Func)(void* sub);
typedef float (*LinkFloatFunc)(const Kart::Link* kartPlayer);

static const CameraTargetFunc sOriginalUpdateCameraTarget = reinterpret_cast<CameraTargetFunc>(kmRuntimeAddr(0x805a2cfc));
static const CameraRollFunc sOriginalUpdateCameraRoll = reinterpret_cast<CameraRollFunc>(kmRuntimeAddr(0x805a3070));
static const CameraValuesFunc sOriginalUpdateCameraValues = reinterpret_cast<CameraValuesFunc>(kmRuntimeAddr(0x805a34b0));
static const CameraFinalizeFunc sOriginalFinalizeCamera = reinterpret_cast<CameraFinalizeFunc>(kmRuntimeAddr(0x805a40d0));
static const CameraSub338Func sOriginalUpdateSub338 = reinterpret_cast<CameraSub338Func>(kmRuntimeAddr(0x805a9a40));
static const LinkFloatFunc sGetCameraVerticalDistance = reinterpret_cast<LinkFloatFunc>(kmRuntimeAddr(0x805909c8));

struct CameraGravityContext {
    Vec3 up;
    Vec3 playerPos;
    Vec3 anchorPos;
    /** Unit vector in the gravity horizontal plane, aligned with kart forward when possible. */
    Vec3 kartForwardPlanar;
};

Vec3 MakeVec(float x, float y, float z) {
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

bool NormalizeSafe(Vec3& vec) {
    const float length = VecLength(vec);
    if (length <= kVecEpsilon) return false;
    const float invLength = 1.0f / length;
    vec.x *= invLength;
    vec.y *= invLength;
    vec.z *= invLength;
    return true;
}

Vec3 NormalizedOrFallback(const Vec3& source, const Vec3& fallback) {
    Vec3 result = source;
    if (NormalizeSafe(result)) return result;
    result = fallback;
    NormalizeSafe(result);
    return result;
}

Vec3 BuildPerpendicularInPlane(const Vec3& planeNormal, const Vec3& vector, const Vec3& fallback) {
    Vec3 planeComponent = vector;
    if (!NormalizeSafe(planeComponent)) return NormalizedOrFallback(fallback, WorldUp());

    Vec3 result = SubVec(planeNormal, MultiplyVecByFloat(planeComponent, Dot(planeNormal, planeComponent)));
    if (NormalizeSafe(result)) return result;
    return NormalizedOrFallback(fallback, WorldUp());
}

Vec3 RemapWorldVerticalToGravityUp(const Vec3& offset, const Vec3& gravityUp) {
    const Vec3 worldUp = WorldUp();
    const float worldVertical = Dot(offset, worldUp);

    Vec3 remapped = SubVec(offset, MultiplyVecByFloat(worldUp, worldVertical));
    remapped = AddVec(remapped, MultiplyVecByFloat(gravityUp, worldVertical));
    return remapped;
}

Quat IdentityQuat() {
    Quat quat;
    quat.Set(1.0f, 0.0f, 0.0f, 0.0f);
    return quat;
}

Vec3& BasePosition(RaceCamera& camera) {
    return Field<Vec3>(&camera, kBasePositionOffset);
}

Vec3& BaseTarget(RaceCamera& camera) {
    return Field<Vec3>(&camera, kBaseTargetOffset);
}

Vec3& BaseUp(RaceCamera& camera) {
    return Field<Vec3>(&camera, kBaseUpOffset);
}

float& CameraFloat(RaceCamera& camera, u32 offset) {
    return Field<float>(&camera, offset);
}

Vec3& CameraVec(RaceCamera& camera, u32 offset) {
    return Field<Vec3>(&camera, offset);
}

Quat& CameraQuat(RaceCamera& camera, u32 offset) {
    return Field<Quat>(&camera, offset);
}

float& GameCamFloat(GameCamValues& values, u32 offset) {
    return Field<float>(&values, offset);
}

#endif