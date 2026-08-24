#ifndef KAEMATH_HPP
#define KAEMATH_HPP

#include <kamek.hpp>
#include <core/rvl/mtx/mtx.hpp>
#include <MarioKartWii/Math/Vector.hpp>
#include <core/egg/Math/Quat.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <MarioKartWii/Kart/KartPhysics.hpp>
#include <MarioKartWii/Kart/KartStatus.hpp>
#include <KAE/KMPAREAExpander.hpp>

// This file holds recurring math stuff, not that interesting except for maybe the last couple

// general math
static inline float Clamp(float x, float minVal, float maxVal) {
    if (x < minVal) return minVal;
    if (x > maxVal) return maxVal;
    return x;
}

static inline float Abs(float x) {
    return x < 0.0f ? -x : x;
}

static inline float Pow(float x, int y) {
    float z = 1;

    if (y < 0) {
        x = 1.0f / x;
        y = -y;
    }

    for (int i = 0; i < y; i++) {
        z *= x;
    }
    return z;
}

static inline float Max(float a, float b) {
    return (a > b) ? a : b;
}

static inline float Min(float a, float b) {
    return (a < b) ? a : b;
}

// Vector math
static inline Vec3 WorldUp() {
    return Vec3(0.0f, 1.0f, 0.0f);
}

static inline Vec3 AddVec(const Vec3& lhs, const Vec3& rhs) {
    return Vec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

static inline Vec3 SubVec(const Vec3& lhs, const Vec3& rhs) {
    return Vec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

static inline float Dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float VecLength(const Vec3& vec) {
    return EGG::Math::Sqrt(Dot(vec, vec));
}

static inline Vec3 MultiplyVecByFloat(const Vec3 a, const float b) {
    return Vec3(
        a.x * b,
        a.y * b,
        a.z * b
    );
    
}

static inline Vec3 MultiplyVecByVec(const Vec3 a, const Vec3 b) {
    return Vec3(
        a.x * b.x,
        a.y * b.y,
        a.z * b.z
    );
}

static inline Vec3 Cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static inline float LengthSq(const Vec3& a) {
    return a.x * a.x + a.y * a.y + a.z * a.z;
}

// Quat math
static inline Vec3 GetUp(Quat q) {
    Vec3 up;
    up.x = 2.0f * (q.x * q.y - q.w * q.z);
    up.y = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
    up.z = 2.0f * (q.y * q.z + q.w * q.x);
    return up;
}

static inline Vec3 GetForward(Quat q) {
    Vec3 forward;
    forward.x = 2.0f * (q.x * q.z + q.w * q.y);
    forward.y = 2.0f * (q.y * q.z - q.w * q.x);
    forward.z = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    return forward;
}

static inline Vec3 GetRight(Quat q) {
    Vec3 right;
    right.x = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    right.y = 2.0f * (q.x * q.y + q.w * q.z);
    right.z = 2.0f * (q.x * q.z - q.w * q.y);
    return right;
}

static inline Quat MultiplyQuat(const Quat& a, const Quat& b) {
    Quat r;
    r.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    r.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    r.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    r.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    return r;
}

static inline float GetForwardSpeed(const Vec3 speed, Quat rot) {
    Vec3 localVel;
    rot.RotateVectorConjugate(speed, localVel);
    return localVel.z;
}

static inline float GetSidewaysSpeed(const Vec3 speed, Quat rot) {
    Vec3 localVel;
    rot.RotateVectorConjugate(speed, localVel);
    return localVel.x;
}

static inline float GetUpSpeed(const Vec3 speed, Quat rot) {
    Vec3 localVel;
    rot.RotateVectorConjugate(speed, localVel);
    return localVel.y;
}

// Different
static inline void SnapSpeedToRotation(Kart::Movement& movement, Kart::Physics& physics) {
    Vec3 forward = GetForward(physics.mainRot);
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
}

static inline Vec3 RotateYaw(const Vec3 v, float yaw) {
    float c = EGG::Math::Cos(yaw);
    float s = EGG::Math::Sin(yaw);

    Vec3 rotated;
    rotated.x = v.x * c - v.z * s; 
    rotated.y = v.y;
    rotated.z = v.x * s + v.z * c;

    return rotated;
}

static inline void MatchPitchAndRollToGround(Kart::Physics& physics, Vec3 groundNor) {
    Vec3 up = GetUp(physics.mainRot);

    groundNor.Normalize();
    
    Quat correction;
    correction.MakeVectorRotation(up, groundNor);
    Quat targetRot = MultiplyQuat(correction, physics.mainRot);

    physics.mainRot.SlerpTo(targetRot, physics.mainRot, physics.stabilizationFactor);
    physics.mainRot.Normalise();
}

static inline void MatchRollToGround(Kart::Physics& physics, Vec3 groundNor) {
    groundNor.Normalize();

    Vec3 forward = GetForward(physics.mainRot);
    Vec3 up = GetUp(physics.mainRot);

    float forwardComponent = Dot(groundNor, forward);

    Vec3 desiredUp = SubVec(
        groundNor,
        MultiplyVecByFloat(forward, forwardComponent)
    );

    desiredUp.Normalize();

    float sinAngle = Dot(Cross(up, desiredUp), forward);
    float cosAngle = Dot(up, desiredUp);

    float angle = EGG::Math::Atan2(sinAngle, cosAngle);

    Quat correction;
    correction.SetAxisRotation(forward, angle);

    Quat targetRot = MultiplyQuat(correction, physics.mainRot);

    physics.mainRot.SlerpTo(
        targetRot,
        physics.mainRot,
        physics.stabilizationFactor
    );

    physics.mainRot.Normalise();
}

static inline void Stopper(Kart::Physics& physics) {
    Vec3 zero = Vec3(0,0,0);
    physics.speed = physics.speed1Adj = physics.speed2 = physics.normalAcceleration = physics.speed3 = physics.acceleration0 = physics.engineSpeed = zero;
}

#endif