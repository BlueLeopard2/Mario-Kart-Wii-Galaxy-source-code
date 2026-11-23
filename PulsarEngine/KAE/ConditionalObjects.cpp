/*#include <kamek.hpp>
#include <runtimeWrite.hpp>
#include <MarioKartWii/3D/Camera/CameraMgr.hpp>
#include <MarioKartWii/CourseMgr.hpp>
#include <MarioKartWii/Item/Obj/ItemObj.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartPlayer.hpp>
#include <MarioKartWii/Kart/KartStatus.hpp>
#include <MarioKartWii/KCL/Collision.hpp>
#include <MarioKartWii/KMP/GOBJ.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <MarioKartWii/Objects/Object.hpp>
#include <MarioKartWii/Objects/ObjectsMgr.hpp>
#include <MarioKartWii/Objects/KCL/ObjectKCLManager.hpp>
#include <MarioKartWii/Race/RaceData.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <KAE/KMPAREAExpander.hpp>

// Conditional Objects [Retro Rewind] + AREA-based detection [BlueLeopard]
// This works just fine. Might make some additions to it later, but for now it does what it's supposed to do.
*/
/*
GOBJ->padding (u16) -> routeId
action (2 bits) enemyRoute: bit 0-1
forEveryone (1 bit) enemyRoute: bit 2
lapCount (3 bits) enemyRoute: bits 3-5
visual (1 bit) enemyRoute: bit 6 still needs to be added
durationFrames (u16) -> setting2
delayFrames (u16) -> setting1
*/

/*
for the animations:
object->ModelDirector->scnMdlEx->ScnMdl->Material->Color->Alpha still needs to be added
*/

/*namespace MKWG {
namespace Race {


// --- AREA state (detection + delay/duration/forEveryone) ---
static const u32 MAX_PENDING = 256;
static const u32 MAX_EFFECTS = 2048;

struct PendingAction {
    u16 objMatch;
    u8 action;
    bool forEveryone;
    u8 triggerPlayerId;
    u16 delayFrames;
    u16 durationFrames;
};

struct Effect {
    u16 objMatch;
    u8 action;
    bool forEveryone;
    u8 triggerPlayerId;
    u16 durationFrames;
    u32 serial;
};

static PendingAction sPendingActions[MAX_PENDING];
static Effect sEffects[MAX_EFFECTS];
static u32 sPendingCount = 0;
static u32 sEffectCount = 0;
static u32 sEffectSerial = 0;

static void InitConditionalObjectState() {
    sPendingCount = 0;
    sEffectCount = 0;
    sEffectSerial = 0;
}

bool ConditionalObjAction(u8 playerId) {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    Kart::Manager* kartManager = Kart::Manager::sInstance;
    const Raceinfo* raceInfo = Raceinfo::sInstance;
    if (!raceInfo) return false;
    KMPAREAExpander& KAE = KMPAREAExpand[playerId];

    const RaceinfoPlayer* raceInfoPlayer = raceInfo->players[playerId];
    if (raceInfoPlayer == nullptr) return false;

    if (!KAE.condObj) return false;

    const u16 routeId = kmp->areaSection->GetHolder(KAE.conditionalObject)->raw->routeId;
    u16 currentLap = raceInfoPlayer->currentLap;

    for (u16 i = 0; i < kmp->gobjSection->pointCount; i++) {
        KMP::Holder<GOBJ>* holder = kmp->gobjSection->holdersArray[i];
        if (!holder || !holder->raw) continue;
        if (holder->raw->padding != routeId) continue;

        const u8 correctLap = (kmp->areaSection->GetHolder(KAE.conditionalObject)->raw->enemyRouteId >> 3) & 0x7;
        if (currentLap == 0) currentLap = 1;
        if (correctLap != 0 && correctLap != currentLap) continue;
        return true;
    }
    return false;
}

static void PushPending(u16 objMatch, u8 action, bool forEveryone, u8 triggerPlayerId, u16 delayFrames, u16 durationFrames) {
    if (sPendingCount >= MAX_PENDING) return;

    PendingAction& p = sPendingActions[sPendingCount++];
    p.objMatch = objMatch;
    p.action = action;
    p.forEveryone = forEveryone;
    p.triggerPlayerId = triggerPlayerId;
    p.delayFrames = delayFrames;
    p.durationFrames = durationFrames;
}

static void PushEffect(u16 objMatch, u8 action, bool forEveryone, u8 triggerPlayerId, u16 durationFrames) {
    if (sEffectCount >= MAX_EFFECTS) return;

    Effect& e = sEffects[sEffectCount++];
    e.objMatch = objMatch;
    e.action = action;
    e.forEveryone = forEveryone;
    e.triggerPlayerId = triggerPlayerId;
    e.durationFrames = durationFrames;
    e.serial = ++sEffectSerial;
}

static bool EffectAppliesToPlayer(const Effect& e, u8 playerId) {
    if (e.forEveryone) return true;
    return e.triggerPlayerId == playerId;
}

static void GetAREAObjectStateForPadding(u16 padding, u8 playerId, bool* outHasEffect, bool* outVisible) {
    if (!outHasEffect || !outVisible) return;
    *outHasEffect = false;
    *outVisible = true;

    const Effect* best = nullptr;
    for (u32 i = 0; i < sEffectCount; ++i) {
        if (sEffects[i].objMatch != padding) continue;
        if (!EffectAppliesToPlayer(sEffects[i], playerId)) continue;
        if (best == nullptr || sEffects[i].serial > best->serial) best = &sEffects[i];
    }

    if (best == nullptr) return;
    *outHasEffect = true;
    *outVisible = (best->action != 0);
}

void ConditionalObjectFrameUpdate() {
    KMP::Manager* kmp = KMP::Manager::sInstance;
    if (!kmp) return;
    Kart::Manager* kartManager = Kart::Manager::sInstance;

    for (u8 playerId = 0; playerId < kartManager->playerCount; playerId++) {
        KMPAREAExpander& KAE = KMPAREAExpand[playerId];

        if (KAE.condObj && !KAE.prevCondObj && ConditionalObjAction(playerId)) {
            u8 action = kmp->areaSection->GetHolder(KAE.conditionalObject)->raw->enemyRouteId & 0x3;
            bool forEveryone = (kmp->areaSection->GetHolder(KAE.conditionalObject)->raw->enemyRouteId & 0x4) != 0;
            u16 GOBJPadding = kmp->areaSection->GetHolder(KAE.conditionalObject)->raw->routeId;
            u16 delayFrames = kmp->areaSection->GetHolder(KAE.conditionalObject)->raw->setting1;
            u16 durationFrames = kmp->areaSection->GetHolder(KAE.conditionalObject)->raw->setting2;

            PushPending(GOBJPadding, action, forEveryone, playerId, delayFrames, durationFrames);
        }
    }

    for (u32 i = 0; i < sPendingCount;) {
        PendingAction& p = sPendingActions[i];

        if (p.delayFrames > 0) {
            p.delayFrames--;
            ++i;
            continue;
        }

        PushEffect(p.objMatch, p.action, p.forEveryone, p.triggerPlayerId, p.durationFrames);

        for (u32 j = i; j + 1 < sPendingCount; ++j) {
            sPendingActions[j] = sPendingActions[j + 1];
        }
        --sPendingCount;
    }

    for (u32 i = 0; i < sEffectCount; ++i) {
        Effect& e = sEffects[i];

        if (e.durationFrames > 0) {
            e.durationFrames--;

            if (e.durationFrames == 0) {
                e.action = e.action == 0 ? 1 : 0;
                e.serial = ++sEffectSerial;
            }
        }
    }
}

void CondObjPrevState() {
    Kart::Manager* kartManager = Kart::Manager::sInstance;
    for (u8 playerId = 0; playerId < kartManager->playerCount; playerId++) {
        KMPAREAExpander& KAE = KMPAREAExpand[playerId];

        KAE.prevCondObj = KAE.condObj && ConditionalObjAction(playerId);
    }
}

void ConditionalObjectReset() {
    InitConditionalObjectState();
}

struct ObjectConditionalView {
    u8 padding[0xa0];
    const void* gobjLink;
};

struct ConditionalState {
    bool isConditional;
    bool isActive;
    bool isCollisionActive;
    u8 localScreenCount;
    bool screenIsActive[4];
};

static void FillScreenState(bool (&screenIsActive)[4], bool value) {
    for (u8 i = 0; i < 4; ++i) screenIsActive[i] = value;
}

static void InitConditionalState(ConditionalState& state) {
    state.isConditional = false;
    state.isActive = true;
    state.isCollisionActive = true;
    state.localScreenCount = 1;
    FillScreenState(state.screenIsActive, true);
}

static const GOBJ* GetObjectGobj(const Object& object) {
    const ObjectConditionalView& view = reinterpret_cast<const ObjectConditionalView&>(object);
    if (view.gobjLink == nullptr) return nullptr;
    return *reinterpret_cast<GOBJ* const*>(view.gobjLink);
}

static bool IsConditionalReplayPlayer(const RacedataScenario& scenario, const Raceinfo& raceInfo, u8 playerId) {
    if (playerId >= 12) return false;
    const PlayerType type = scenario.players[playerId].playerType;
    if (type != PLAYER_GHOST && type != PLAYER_REAL_LOCAL) return false;
    return raceInfo.players[playerId] != nullptr;
}

static void EvaluateConditionalState(const Object& object, ConditionalState& state) {
    InitConditionalState(state);

    const GOBJ* gobj = GetObjectGobj(object);
    if (gobj == nullptr) return;

    const Racedata* raceData = Racedata::sInstance;
    const Raceinfo* raceInfo = Raceinfo::sInstance;
    if (raceData == nullptr || raceInfo == nullptr) return;

    const RacedataScenario& scenario = raceData->racesScenario;
    const u16 padding = gobj->padding;
    const GameMode mode = scenario.settings.gamemode;
    const bool isTTMode = (mode == MODE_TIME_TRIAL || mode == MODE_GHOST_RACE);

    if (isTTMode) {
        state.isCollisionActive = false;
        u8 watchedPlayerId = 0xFF;
        const RaceCameraMgr* cameraMgr = RaceCameraMgr::sInstance;
        if (cameraMgr != nullptr) {
            const u8 focusedPlayerId = cameraMgr->focusedPlayerIdx;
            if (IsConditionalReplayPlayer(scenario, *raceInfo, focusedPlayerId)) watchedPlayerId = focusedPlayerId;
        }
        if (watchedPlayerId == 0xFF) {
            const u8 hudPlayerId = scenario.settings.hudPlayerIds[0];
            if (IsConditionalReplayPlayer(scenario, *raceInfo, hudPlayerId)) watchedPlayerId = hudPlayerId;
        }
        if (watchedPlayerId != 0xFF) {
            bool hasEffect = false;
            bool visible = true;
            GetAREAObjectStateForPadding(padding, watchedPlayerId, &hasEffect, &visible);
            if (hasEffect) {
                state.isConditional = true;
                state.isActive = visible;
                state.isCollisionActive = visible;
                state.localScreenCount = 1;
                state.screenIsActive[0] = visible;
            }
        }
    } else {
        const u8 localScreenCount = (scenario.localPlayerCount > 4) ? 4 : scenario.localPlayerCount;
        if (localScreenCount == 0) return;

        state.localScreenCount = localScreenCount;
        state.isActive = false;
        state.isCollisionActive = false;
        bool anyAREAEffect = false;
        for (u8 i = 0; i < localScreenCount; ++i) {
            const u8 playerId = scenario.settings.hudPlayerIds[i];
            bool hasEffect = false;
            bool visible = true;
            GetAREAObjectStateForPadding(padding, playerId, &hasEffect, &visible);
            if (hasEffect) anyAREAEffect = true;
            state.screenIsActive[i] = visible;
            if (visible) {
                state.isActive = true;
                state.isCollisionActive = true;
            }
        }
        if (anyAREAEffect) state.isConditional = true;
    }
}

void AnimationViaColorChanges(const Object* object) {
    ModelDirector* director = object->mdlDirector;

    // Assume for now every ModelDirector is actually a MatModelDirector.
    MatModelDirector* matDirector = static_cast<MatModelDirector*>(director);

    if (matDirector == nullptr) return;

    // Make sure TEV copies exist.
    matDirector->CopyMaterialsTevColors();

    // Get the first material.
    g3d::ResTevColorDL* tev = matDirector->matsResTevColorCopy[0][0];

    if (tev == nullptr) return;

    // TODO: Change alpha here.

    // Push the modified copy back to the renderer.
    matDirector->ReplaceDefaultScnMdlTevColorDL(0);
}

// --- Per-screen visibility and apply ---
static bool IsModelDirectorReadyForPerScreenVisibility(const ModelDirector* director) {
    if (director == nullptr) return false;
    if ((director->bitfield & 0x100000) == 0) return false;
    return director->scnMdlEx[0] != nullptr && director->scnMdlEx[1] != nullptr;
}

static void ApplyModelDirectorScreenVisibility(ModelDirector* director, const ConditionalState& state) {
    if (!IsModelDirectorReadyForPerScreenVisibility(director)) return;
    for (u8 screenIdx = 0; screenIdx < state.localScreenCount; ++screenIdx) {
        if (state.screenIsActive[screenIdx])
            director->EnableScreen(screenIdx);
        else
            director->DisableScreen(screenIdx);
    }
}

static bool IsScreenSpecificModelRegistered(const ScnMgr& scnMgr, const ModelDirector* director) {
    void* current = nullptr;
    while (true) {
        current = nw4r::ut::List_GetNext(&scnMgr.screenSpecificModelDirectors, current);
        if (current == nullptr) return false;
        if (current == director) return true;
    }
}

static void EnsureScreenSpecificModelRegistration(ModelDirector* director) {
    if (!IsModelDirectorReadyForPerScreenVisibility(director)) return;
    ScnMgr* scnMgr = director->GetScnManager();
    if (scnMgr == nullptr) return;
    if (IsScreenSpecificModelRegistered(*scnMgr, director)) return;
    director->bitfield |= 0x8;
    scnMgr->AppendScreenSpecificModelDirector(director);
}

static void ApplyPerScreenVisibility(Object& object, const ConditionalState& state) {
    if (!state.isConditional || state.localScreenCount <= 1) return;
    EnsureScreenSpecificModelRegistration(object.mdlDirector);
    EnsureScreenSpecificModelRegistration(object.mdlLodDirector);
    EnsureScreenSpecificModelRegistration(object.shadowDirector);
    ApplyModelDirectorScreenVisibility(object.mdlDirector, state);
    ApplyModelDirectorScreenVisibility(object.mdlLodDirector, state);
    ApplyModelDirectorScreenVisibility(object.shadowDirector, state);
}

static void ApplyConditionalState(Object& object, const ConditionalState& state) {
    if (!state.isConditional) return;
    object.ToggleVisible(state.isActive);
    if (state.isCollisionActive)
        object.EnableCollision();
    else
        object.DisableCollision();
}

static void ApplyKCLConditionalState(Object& object, const ConditionalState& state) {
    if (!state.isConditional) return;
    object.ToggleVisible(state.isActive);
    if (state.isCollisionActive) {
        object.EnableCollision();
        if (object.entity != nullptr) object.entity->paramsBitfield |= 0x10;
    } else {
        object.DisableCollision();
        if (object.entity != nullptr) object.entity->paramsBitfield &= ~0x10;
    }
}

static bool IsObjectActiveForPlayer(const Object& object, u8 playerId) {
    const GOBJ* gobj = GetObjectGobj(object);
    if (gobj == nullptr) return true;
    bool hasEffect = false;
    bool visible = true;
    GetAREAObjectStateForPadding(gobj->padding, playerId, &hasEffect, &visible);
    if (!hasEffect) return true;
    return visible;
}

// --- Collision context ---
static u8 sCollisionContextPlayerId = 0xFF;
static u8 sCollisionContextStack[8];
static u32 sCollisionContextDepth = 0;

void PushConditionalCollisionPlayerContext(u8 playerId) {
    if (playerId >= 12) playerId = 0xFF;
    if (sCollisionContextDepth < 8) {
        sCollisionContextStack[sCollisionContextDepth] = sCollisionContextPlayerId;
    }
    ++sCollisionContextDepth;
    sCollisionContextPlayerId = playerId;
}

void PopConditionalCollisionPlayerContext() {
    if (sCollisionContextDepth == 0) {
        sCollisionContextPlayerId = 0xFF;
        return;
    }
    --sCollisionContextDepth;
    if (sCollisionContextDepth < 8) {
        sCollisionContextPlayerId = sCollisionContextStack[sCollisionContextDepth];
    } else if (sCollisionContextDepth == 0) {
        sCollisionContextPlayerId = 0xFF;
    }
}

static u8 GetConditionalCollisionPlayerContext() {
    return sCollisionContextPlayerId;
}

// --- Hooks ---
static const u32 BOXCOL_FLAG_DRIVER = 0x1;
static const u32 BOXCOL_FLAG_ITEM = 0x2;
static const u32 BOXCOL_FLAG_OBJECT = 0x4;
static const u32 BOXCOL_FLAG_OBJECT_OBSTACLE_ENEMY = 0x8;
static const u32 BOXCOL_FLAG_DRIVABLE = 0x10;

struct BoxColUnitView {
    u8 padding[0x0c];
    u32 unitType;
    void* userData;
};

static ObjectCollision* CallOriginalGetCollision(void* object) {
    typedef ObjectCollision* (*GetCollisionFn)(void*);
    const u32* vtable = *reinterpret_cast<const u32* const*>(object);
    GetCollisionFn getCollision = reinterpret_cast<GetCollisionFn>(vtable[0xb4 / 4]);
    return getCollision(object);
}

static ObjectCollision* ConditionalGetObjectCollision(void* object) {
    if (object == nullptr) return nullptr;
    ObjectCollision* collision = CallOriginalGetCollision(object);
    if (collision == nullptr) return nullptr;
    register const Kart::Player* kartPlayer;
    asm(mr kartPlayer, r25;);
    if (kartPlayer == nullptr) return collision;
    const u8 playerId = kartPlayer->GetPlayerIdx();
    const Object& mapObject = *reinterpret_cast<const Object*>(object);
    if (!IsObjectActiveForPlayer(mapObject, playerId)) return nullptr;
    return collision;
}
kmCall(0x8082ab8c, ConditionalGetObjectCollision);

static ObjectCollision* ConditionalGetObjectCollisionForItem(void* object) {
    if (object == nullptr) return nullptr;

    ObjectCollision* collision = CallOriginalGetCollision(object);
    if (collision == nullptr) return nullptr;

    register Item::Obj* itemObj;
    asm(mr itemObj, r27;);
    if (itemObj == nullptr) return collision;

    const u8 playerId = itemObj->playerUsedItemId;
    const Object& mapObject = *reinterpret_cast<const Object*>(object);
    if (!IsObjectActiveForPlayer(mapObject, playerId)) return nullptr;
    return collision;
}
kmCall(0x8082ae18, ConditionalGetObjectCollisionForItem);

static bool ConditionalCourseCollisionSetPlayerFromWheel(float radius, CourseMgr& mgr, const Vec3& position, const Vec3& prevPosition,
                                                         KCLBitfield acceptedFlags, CollisionInfo* info, KCLTypeHolder& kclFlags) {
    register u32 playerIdRaw;
    asm(mr playerIdRaw, r25;);
    PushConditionalCollisionPlayerContext(static_cast<u8>(playerIdRaw));
    const bool isColliding = mgr.IsCollidingAddEntry(position, prevPosition, acceptedFlags, info, &kclFlags, 0, radius);
    PopConditionalCollisionPlayerContext();
    return isColliding;
}
kmCall(0x805b7028, ConditionalCourseCollisionSetPlayerFromWheel);

static void FilterConditionalDriveablesForCurrentPlayer(void* boxColMgr) {
    u8 playerId = GetConditionalCollisionPlayerContext();

    if (playerId >= 12) {
        register Item::Obj* itemObj;
        asm(mr itemObj, r27;);
        if (itemObj != nullptr) {
            playerId = itemObj->playerUsedItemId;
        }
    }

    if (playerId >= 12 || boxColMgr == nullptr) return;

    u8* const mgr = reinterpret_cast<u8*>(boxColMgr);
    s32& maxId = *reinterpret_cast<s32*>(mgr + 0x438);
    if (maxId <= 0) return;

    BoxColUnitView** units = *reinterpret_cast<BoxColUnitView***>(mgr + 0x1c);
    if (units == nullptr) return;

    s32 writeIdx = 0;
    for (s32 readIdx = 0; readIdx < maxId; ++readIdx) {
        BoxColUnitView* unit = units[readIdx];
        if (unit == nullptr) continue;

        bool keep = true;
        if ((unit->unitType & BOXCOL_FLAG_DRIVABLE) != 0 && unit->userData != nullptr) {
            const Object& obj = *reinterpret_cast<const Object*>(unit->userData);
            keep = IsObjectActiveForPlayer(obj, playerId);
        }

        if (keep) {
            units[writeIdx] = unit;
            ++writeIdx;
        }
    }
    maxId = writeIdx;
}

static s32 FindFirstUnitOfType(BoxColUnitView* const* units, s32 maxId, u32 mask) {
    for (s32 i = 0; i < maxId; ++i) {
        const BoxColUnitView* unit = units[i];
        if (unit != nullptr && (unit->unitType & mask) != 0) return i;
    }
    return 0x100;
}

static void ConditionalResetIterators(void* boxColMgr) {
    if (boxColMgr == nullptr) return;

    FilterConditionalDriveablesForCurrentPlayer(boxColMgr);

    u8* const mgr = reinterpret_cast<u8*>(boxColMgr);
    const s32 maxId = *reinterpret_cast<const s32*>(mgr + 0x438);
    BoxColUnitView** units = *reinterpret_cast<BoxColUnitView***>(mgr + 0x1c);

    s32& nextPlayerId = *reinterpret_cast<s32*>(mgr + 0x428);
    s32& nextItemId = *reinterpret_cast<s32*>(mgr + 0x42c);
    s32& nextObjectId = *reinterpret_cast<s32*>(mgr + 0x430);
    s32& nextDrivableId = *reinterpret_cast<s32*>(mgr + 0x434);

    if (units == nullptr || maxId <= 0) {
        nextPlayerId = 0x100;
        nextItemId = 0x100;
        nextObjectId = 0x100;
        nextDrivableId = 0x100;
        return;
    }

    nextPlayerId = FindFirstUnitOfType(units, maxId, BOXCOL_FLAG_DRIVER);
    nextItemId = FindFirstUnitOfType(units, maxId, BOXCOL_FLAG_ITEM);
    nextObjectId = FindFirstUnitOfType(units, maxId, BOXCOL_FLAG_OBJECT | BOXCOL_FLAG_OBJECT_OBSTACLE_ENEMY);
    nextDrivableId = FindFirstUnitOfType(units, maxId, BOXCOL_FLAG_DRIVABLE);
}
kmBranch(0x80785f2c, ConditionalResetIterators);  // BoxColManager::resetIterators

static void ConditionalCalcCollisions(Kart::Status* status) {
    Kart::Link* link = status->link;
    u8 playerId = link->GetPlayerIdx();

    PushConditionalCollisionPlayerContext(playerId);
    status->UpdateCollisions();
    PopConditionalCollisionPlayerContext();
}
kmCall(0x80594858, ConditionalCalcCollisions);

static void ConditionalObjectUpdate(Object* object) {
    if (object == nullptr) return;

    ConditionalState state;
    EvaluateConditionalState(*object, state);
    ApplyConditionalState(*object, state);
    if (state.isActive || state.isCollisionActive) object->Update();
}
kmCall(0x8082a9e0, ConditionalObjectUpdate);

static void ConditionalObjectModelUpdate(Object* object) {
    if (object == nullptr) return;
    ConditionalState state;
    EvaluateConditionalState(*object, state);
    if (!state.isActive) return;
    object->UpdateModel();
    ApplyPerScreenVisibility(*object, state);
}
kmCall(0x8082aa20, ConditionalObjectModelUpdate);

kmRuntimeUse(0x8081b618);
static void ConditionalProcessAllAndCalcKCL(void* kclMgr, ObjectsMgr& objectsMgr) {
    if (kclMgr != nullptr) {
        const u16 kclCount = *reinterpret_cast<const u16*>(reinterpret_cast<const u8*>(kclMgr) + 0x4);
        Object** kclObjects = *reinterpret_cast<Object***>(reinterpret_cast<u8*>(kclMgr) + 0x8);
        for (u16 i = 0; i < kclCount; ++i) {
            Object* obj = kclObjects[i];
            if (obj == nullptr) continue;
            ConditionalState state;
            EvaluateConditionalState(*obj, state);
            ApplyKCLConditionalState(*obj, state);
            ApplyPerScreenVisibility(*obj, state);
        }
    }
    typedef void (*OriginalCalcFn)(void*);
    OriginalCalcFn originalCalc = reinterpret_cast<OriginalCalcFn>(kmRuntimeAddr(0x8081b618));
    originalCalc(kclMgr);
}
kmCall(0x8082aa40, ConditionalProcessAllAndCalcKCL);

static void ConditionalKCLObjectUpdate(Object* object) {
    if (object == nullptr) return;
    ConditionalState state;
    EvaluateConditionalState(*object, state);
    ApplyKCLConditionalState(*object, state);
    if (state.isActive || state.isCollisionActive) object->Update();
}
kmCall(0x8081b658, ConditionalKCLObjectUpdate);

static void ConditionalKCLObjectModelUpdate(Object* object) {
    if (object == nullptr) return;
    ConditionalState state;
    EvaluateConditionalState(*object, state);
    if (!state.isActive) return;
    object->UpdateModel();
    ApplyPerScreenVisibility(*object, state);
}
kmCall(0x8081b698, ConditionalKCLObjectModelUpdate);

} // namespace Race
} // namespace MKWG*/

#include <kamek.hpp>
#include <runtimeWrite.hpp>
#include <MarioKartWii/Archive/ArchiveMgr.hpp>
#include <MarioKartWii/3D/Camera/CameraMgr.hpp>
#include <MarioKartWii/CourseMgr.hpp>
#include <MarioKartWii/Item/Obj/ItemObj.hpp>
#include <MarioKartWii/KCL/Collision.hpp>
#include <MarioKartWii/KMP/GOBJ.hpp>
#include <MarioKartWii/KMP/KMPManager.hpp>
#include <MarioKartWii/Kart/KartManager.hpp>
#include <MarioKartWii/Kart/KartPlayer.hpp>
#include <MarioKartWii/Kart/KartStatus.hpp>
#include <MarioKartWii/Objects/Object.hpp>
#include <MarioKartWii/Objects/ObjectsMgr.hpp>
#include <MarioKartWii/Objects/KCL/ObjectKCLManager.hpp>
#include <MarioKartWii/Race/RaceData.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <MarioKartWii/System/Random.hpp>
#include <Race/ConditionalTrackState.hpp>
#include <KAE/KMPAREAExpander.hpp>
//#include <GlobalFunction.hpp>

#ifndef CONDITIONAL_OBJECTS_NAMESPACE
#define CONDITIONAL_OBJECTS_NAMESPACE Pulsar
#endif

#ifndef CONDITIONAL_OBJECTS_ITEM_DRIVABLE_FALLBACK
#define CONDITIONAL_OBJECTS_ITEM_DRIVABLE_FALLBACK 1
#endif

namespace MKWG {
namespace Race {

inline bool IsTimeTrialOrGhostMode(GameMode mode) {
    return mode == MODE_TIME_TRIAL || mode == MODE_GHOST_RACE;
}


struct ObjectConditionalView {
    u8 padding[0xa0];
    const void *gobjLink;
};

struct ConditionalConfig {
    enum Mode {
        MODE_LAP_RANGE,
        MODE_CHECKPOINT_RANGE,
        MODE_LAP_PROGRESS_RANGE
    };

    u8 startIdx;
    u8 endIdx;
    u8 startProgressPercent;
    u8 endProgressPercent;
    Mode mode;
    bool invert;
};

struct ConditionalState {
    bool isConditional;
    bool isActive;
    bool isCollisionActive;
    u8 localScreenCount;
    bool screenIsActive[4];
};

struct ConditionalSource {
    bool hasRange;           
    ConditionalConfig config;
    bool hasAreaId;          
    u16 areaId;
};

struct BoxColUnitView {
    u8 padding[0x0c];
    u32 unitType;
    void *userData;
};

static const u32 BOXCOL_FLAG_DRIVER = 0x1;
static const u32 BOXCOL_FLAG_ITEM = 0x2;
static const u32 BOXCOL_FLAG_OBJECT = 0x4;
static const u32 BOXCOL_FLAG_OBJECT_OBSTACLE_ENEMY = 0x8;
static const u32 BOXCOL_FLAG_DRIVABLE = 0x10;

static const u8 MAX_CONDITIONAL_LAP_INDEX_COUNT = 8;
static const char *CONDITIONAL_OBJECTS_ENABLE_FILE = "enable.cobj";

enum ConditionalTrackFileState {
    CONDITIONAL_TRACK_FILE_UNKNOWN = -1,
    CONDITIONAL_TRACK_FILE_MISSING = 0,
    CONDITIONAL_TRACK_FILE_PRESENT = 1
};

static void FillScreenState(bool (&screenIsActive)[4], bool value) {
    for (u8 i = 0; i < 4; ++i) screenIsActive[i] = value;
}

static void InitConditionalState(ConditionalState &state) {
    state.isConditional = false;
    state.isActive = true;
    state.isCollisionActive = true;
    state.localScreenCount = 1;
    FillScreenState(state.screenIsActive, true);
}

static const GOBJ *GetObjectGobj(const Object &object) {
    const ObjectConditionalView &view = reinterpret_cast<const ObjectConditionalView &>(object);
    if (view.gobjLink == nullptr)
        return nullptr;
    return *reinterpret_cast<GOBJ *const *>(view.gobjLink);
}

static bool IsConditionalReplayPlayer(const RacedataScenario &scenario, const Raceinfo &raceInfo, u8 playerId) {
    if (playerId >= 12)
        return false;

    const PlayerType type = scenario.players[playerId].playerType;
    if (type != PLAYER_GHOST && type != PLAYER_REAL_LOCAL)
        return false;
    return raceInfo.players[playerId] != nullptr;
}

static u8 sCollisionContextPlayerId = 0xFF;
static u8 sCollisionContextStack[8];
static u32 sCollisionContextDepth = 0;

void PushConditionalCollisionPlayerContext(u8 playerId) {
    if (playerId >= 12)
        playerId = 0xFF;

    if (sCollisionContextDepth < 8) {
        sCollisionContextStack[sCollisionContextDepth] = sCollisionContextPlayerId;
    }
    ++sCollisionContextDepth;
    sCollisionContextPlayerId = playerId;
}

void PopConditionalCollisionPlayerContext() {
    if (sCollisionContextDepth == 0) {
        sCollisionContextPlayerId = 0xFF;
        return;
    }

    --sCollisionContextDepth;
    if (sCollisionContextDepth < 8) {
        sCollisionContextPlayerId = sCollisionContextStack[sCollisionContextDepth];
    } else if (sCollisionContextDepth == 0) {
        sCollisionContextPlayerId = 0xFF;
    }
}

static u8 GetConditionalCollisionPlayerContext() {
    return sCollisionContextPlayerId;
}

static const void *sCachedCourseArchive = nullptr;
static s8 sConditionalTrackFileState = CONDITIONAL_TRACK_FILE_UNKNOWN;

static void ResetRangeTrackState() {
    sCachedCourseArchive = nullptr;
    sConditionalTrackFileState = CONDITIONAL_TRACK_FILE_UNKNOWN;
}

static bool IsInWrappedRange(u8 value, u8 start, u8 end) {
    if (start <= end)
        return value >= start && value <= end;
    return value >= start || value <= end;
}

static bool IsInWrappedRange(u16 value, u16 start, u16 end, u16 wrapSize) {
    if (wrapSize == 0)
        return false;
    if (value >= wrapSize)
        value = static_cast<u16>(wrapSize - 1);
    if (start >= wrapSize)
        start = static_cast<u16>(wrapSize - 1);
    if (end >= wrapSize)
        end = static_cast<u16>(wrapSize - 1);

    if (start <= end)
        return value >= start && value <= end;
    return value >= start || value <= end;
}

static bool IsTrackConditionalObjectsEnabled() {
    const ArchiveMgr *archiveMgr = ArchiveMgr::sInstance;
    if (archiveMgr == nullptr) {
        ResetRangeTrackState();
        return false;
    }

    const void *courseArchive = archiveMgr->GetArchive(ARCHIVE_HOLDER_COURSE, 0);
    if (courseArchive != sCachedCourseArchive) {
        sCachedCourseArchive = courseArchive;
        sConditionalTrackFileState = CONDITIONAL_TRACK_FILE_UNKNOWN;
    }

    if (sConditionalTrackFileState == CONDITIONAL_TRACK_FILE_UNKNOWN) {
        if (courseArchive == nullptr)
            return false;

        const void *condFile = archiveMgr->GetFile(ARCHIVE_HOLDER_COURSE, CONDITIONAL_OBJECTS_ENABLE_FILE, nullptr);
        sConditionalTrackFileState = (condFile != nullptr) ? CONDITIONAL_TRACK_FILE_PRESENT : CONDITIONAL_TRACK_FILE_MISSING;
    }

    return sConditionalTrackFileState == CONDITIONAL_TRACK_FILE_PRESENT;
}

static bool TryGetTrackDefinedLapCount(u8 &lapCount) {
    const KMP::Manager *kmp = KMP::Manager::sInstance;
    if (kmp == nullptr || kmp->stgiSection == nullptr || kmp->stgiSection->holdersArray[0] == nullptr || kmp->stgiSection->holdersArray[0]->raw == nullptr) {
        return false;
    }

    lapCount = kmp->stgiSection->holdersArray[0]->raw->lapCount;
    if (lapCount == 0)
        return false;
    if (lapCount > MAX_CONDITIONAL_LAP_INDEX_COUNT)
        lapCount = MAX_CONDITIONAL_LAP_INDEX_COUNT;
    return true;
}

// Takes the GOBJ directly (the RR original took the Object and looked the GOBJ
// up itself) so the merged evaluator only resolves it once.
static bool TryGetConditionalConfig(const GOBJ *gobj, ConditionalConfig &config) {
    static const u16 LAP_PROGRESS_STEPS_PER_LAP = 100;
    if (!IsTrackConditionalObjectsEnabled())
        return false;

    if (gobj == nullptr)
        return false;

    const u16 flags = gobj->presenceFlags;
    const u8 mode = static_cast<u8>((flags >> 3) & 0x7);
    if (mode == 0 || mode > 6)
        return false;

    config.startIdx = static_cast<u8>((flags >> 6) & 0x7);
    config.endIdx = static_cast<u8>((flags >> 9) & 0x7);
    config.startProgressPercent = 0;
    config.endProgressPercent = 0;

    if (mode == 2 || mode == 4) {
        config.mode = ConditionalConfig::MODE_CHECKPOINT_RANGE;
    } else if (mode == 5 || mode == 6) {
        config.mode = ConditionalConfig::MODE_LAP_PROGRESS_RANGE;

        // For lap progression modes, GOBJ padding packs start/end lap percentage as [start, end] bytes.
        config.startProgressPercent = static_cast<u8>((gobj->padding >> 8) & 0xFF);
        config.endProgressPercent = static_cast<u8>(gobj->padding & 0xFF);
        if (config.startProgressPercent > LAP_PROGRESS_STEPS_PER_LAP)
            config.startProgressPercent = LAP_PROGRESS_STEPS_PER_LAP;
        if (config.endProgressPercent > LAP_PROGRESS_STEPS_PER_LAP)
            config.endProgressPercent = LAP_PROGRESS_STEPS_PER_LAP;
    } else {
        config.mode = ConditionalConfig::MODE_LAP_RANGE;
    }

    config.invert = (mode == 3 || mode == 4 || mode == 6);
    return true;
}

static u8 GetPlayerLapRangeIdx(const RaceinfoPlayer &player, u8 trackLapCount) {
    u16 currentLap = player.currentLap;
    if (currentLap == 0)
        currentLap = 1;

    if (trackLapCount > 0) {
        currentLap = static_cast<u16>(((currentLap - 1) % trackLapCount) + 1);
    } else if (currentLap > MAX_CONDITIONAL_LAP_INDEX_COUNT) {
        currentLap = MAX_CONDITIONAL_LAP_INDEX_COUNT;
    }

    if (currentLap > MAX_CONDITIONAL_LAP_INDEX_COUNT)
        currentLap = MAX_CONDITIONAL_LAP_INDEX_COUNT;
    return static_cast<u8>(currentLap - 1);
}

static u8 GetPlayerCheckpointRangeIdx(const RaceinfoPlayer &player, u16 ckptCount) {
    u16 checkpoint = player.checkpoint;
    if (checkpoint >= ckptCount)
        checkpoint = static_cast<u16>(ckptCount - 1);

    u8 currentIdx = static_cast<u8>((checkpoint * 8u) / ckptCount);
    if (currentIdx > 7)
        currentIdx = 7;
    return currentIdx;
}

static u16 GetLapProgressValue(u8 lapIdx, u8 progressPercent) {
    static const u16 LAP_PROGRESS_STEPS_PER_LAP = 100;
    static const u16 LAP_PROGRESS_WRAP = 8 * LAP_PROGRESS_STEPS_PER_LAP;

    if (lapIdx > 7)
        lapIdx = 7;
    if (progressPercent > LAP_PROGRESS_STEPS_PER_LAP)
        progressPercent = LAP_PROGRESS_STEPS_PER_LAP;

    u16 value = static_cast<u16>(lapIdx) * LAP_PROGRESS_STEPS_PER_LAP + progressPercent;
    if (value >= LAP_PROGRESS_WRAP)
        value = static_cast<u16>(LAP_PROGRESS_WRAP - 1);
    return value;
}

static u16 GetPlayerLapProgressRangeValue(const RaceinfoPlayer &player, u8 trackLapCount) {
    static const float LAP_PROGRESS_STEPS_PER_LAP_FLOAT = 100.0f;
    static const s32 LAP_PROGRESS_STEPS_PER_LAP_INT = 100;
    static const u16 LAP_PROGRESS_WRAP = 800;

    float raceCompletion = player.raceCompletion;
    if (raceCompletion < 1.0f)
        raceCompletion = 1.0f;
    if (trackLapCount == 0 && raceCompletion > 9.0f)
        raceCompletion = 9.0f;

    s32 lapProgress = static_cast<s32>((raceCompletion - 1.0f) * LAP_PROGRESS_STEPS_PER_LAP_FLOAT);
    if (lapProgress < 0)
        lapProgress = 0;
    if (trackLapCount > 0) {
        const s32 lapProgressCycleWrap = static_cast<s32>(trackLapCount) * LAP_PROGRESS_STEPS_PER_LAP_INT;
        if (lapProgressCycleWrap > 0)
            lapProgress %= lapProgressCycleWrap;
    }
    if (lapProgress >= LAP_PROGRESS_WRAP)
        lapProgress = LAP_PROGRESS_WRAP - 1;
    return static_cast<u16>(lapProgress);
}

static bool IsPlayerInConditionalRange(const RaceinfoPlayer &player, const ConditionalConfig &config, u16 ckptCount, u8 trackLapCount) {
    switch (config.mode) {
        case ConditionalConfig::MODE_CHECKPOINT_RANGE: {
            const u8 checkpointIdx = GetPlayerCheckpointRangeIdx(player, ckptCount);
            return IsInWrappedRange(checkpointIdx, config.startIdx, config.endIdx);
        }
        case ConditionalConfig::MODE_LAP_PROGRESS_RANGE: {
            static const u16 LAP_PROGRESS_WRAP = 800;

            const u16 currentProgress = GetPlayerLapProgressRangeValue(player, trackLapCount);
            const u16 startProgress = GetLapProgressValue(config.startIdx, config.startProgressPercent);
            const u16 endProgress = GetLapProgressValue(config.endIdx, config.endProgressPercent);
            return IsInWrappedRange(currentProgress, startProgress, endProgress, LAP_PROGRESS_WRAP);
        }
        default:
            break;
    }

    const u8 lapIdx = GetPlayerLapRangeIdx(player, trackLapCount);
    return IsInWrappedRange(lapIdx, config.startIdx, config.endIdx);
}

// Fails open (returns true) whenever the race state needed to decide is missing.
static bool EvaluateConditionalForPlayer(const ConditionalConfig &config, u8 playerId) {
    const Raceinfo *raceInfo = Raceinfo::sInstance;
    if (raceInfo == nullptr)
        return true;

    if (playerId >= 12)
        return true;
    const RaceinfoPlayer *player = raceInfo->players[playerId];
    if (player == nullptr)
        return true;

    u16 ckptCount = 0;
    u8 trackLapCount = 0;
    if (config.mode == ConditionalConfig::MODE_CHECKPOINT_RANGE) {
        const KMP::Manager *kmp = KMP::Manager::sInstance;
        if (kmp == nullptr || kmp->ckptSection == nullptr || kmp->ckptSection->pointCount == 0)
            return true;
        ckptCount = kmp->ckptSection->pointCount;
    } else {
        TryGetTrackDefinedLapCount(trackLapCount);
    }

    const bool inRange = IsPlayerInConditionalRange(*player, config, ckptCount, trackLapCount);
    return config.invert ? !inRange : inRange;
}

static const u32 MAX_PENDING = 256;
static const u32 MAX_EFFECTS = 2048;

// Trigger seen, waiting out its delay.
struct PendingAction {
    u16 objMatch;
    u8 action;
    bool forEveryone;
    u8 triggerPlayerId;
    u16 delayFrames;
    u16 durationFrames;
};

// Delay finished; this is what objects actually look at.
struct Effect {
    u16 objMatch;
    u8 action;
    bool forEveryone;
    u8 triggerPlayerId;
    u16 durationFrames;
    u32 serial;  // higher = newer; newest applicable effect wins
};

// Decoded AREA parameters.
struct AreaConditionalParams {
    u16 matchId;
    u8 action;
    bool forEveryone;
    u8 lapFilter;
    u16 delayFrames;
    u16 durationFrames;
};

static PendingAction sPendingActions[MAX_PENDING];
static Effect sEffects[MAX_EFFECTS];
static u32 sPendingCount = 0;
static u32 sEffectCount = 0;
static u32 sEffectSerial = 0;

static void InitConditionalObjectState() {
    sPendingCount = 0;
    sEffectCount = 0;
    sEffectSerial = 0;
}

void ConditionalObjectReset() {
    InitConditionalObjectState();
    ResetRangeTrackState();
}

void ResetConditionalObjectsTrackState() {
    ConditionalObjectReset();
}

static KMPAREAExpander &GetKAE(u8 playerId) {
    return KMPAREAExpand[playerId];
}

static bool ReadAreaConditionalParams(u32 areaIdx, AreaConditionalParams &out) {
    const KMP::Manager *kmp = KMP::Manager::sInstance;
    if (kmp == nullptr || kmp->areaSection == nullptr)
        return false;
    if (kmp->areaSection->GetHolder(areaIdx) == nullptr || kmp->areaSection->GetHolder(areaIdx)->raw == nullptr)
        return false;

    const u16 enemyRouteId = kmp->areaSection->GetHolder(areaIdx)->raw->enemyRouteId;
    out.matchId = kmp->areaSection->GetHolder(areaIdx)->raw->routeId;
    out.action = static_cast<u8>(enemyRouteId & 0x3);
    out.forEveryone = (enemyRouteId & 0x4) != 0;
    out.lapFilter = static_cast<u8>((enemyRouteId >> 3) & 0x7);
    out.delayFrames = kmp->areaSection->GetHolder(areaIdx)->raw->setting1;
    out.durationFrames = kmp->areaSection->GetHolder(areaIdx)->raw->setting2;
    return true;
}

// True while player `playerId` is inside its AREA, on a matching lap, and at
// least one GOBJ carries the AREA's match id.
bool ConditionalObjAction(u8 playerId) {
    const Raceinfo *raceInfo = Raceinfo::sInstance;
    if (!raceInfo)
        return false;
    if (playerId >= 12)
        return false;

    KMPAREAExpander &KAE = GetKAE(playerId);

    const RaceinfoPlayer *raceInfoPlayer = raceInfo->players[playerId];
    if (raceInfoPlayer == nullptr)
        return false;

    if (!KAE.condObj)
        return false;

    const KMP::Manager *kmp = KMP::Manager::sInstance;
    if (kmp == nullptr || kmp->gobjSection == nullptr)
        return false;

    AreaConditionalParams params;
    if (!ReadAreaConditionalParams(KAE.conditionalObject, params))
        return false;

    u16 currentLap = raceInfoPlayer->currentLap;
    if (currentLap == 0)
        currentLap = 1;
    if (params.lapFilter != 0 && params.lapFilter != currentLap)
        return false;

    for (u16 i = 0; i < kmp->gobjSection->pointCount; i++) {
        KMP::Holder<GOBJ> *holder = kmp->gobjSection->holdersArray[i];
        if (!holder || !holder->raw)
            continue;
        if (holder->raw->padding != params.matchId)
            continue;
        return true;
    }
    return false;
}

static void PushPending(u16 objMatch, u8 action, bool forEveryone, u8 triggerPlayerId, u16 delayFrames, u16 durationFrames) {
    if (sPendingCount >= MAX_PENDING)
        return;

    PendingAction &p = sPendingActions[sPendingCount++];
    p.objMatch = objMatch;
    p.action = action;
    p.forEveryone = forEveryone;
    p.triggerPlayerId = triggerPlayerId;
    p.delayFrames = delayFrames;
    p.durationFrames = durationFrames;
}

// Two effects with the same key apply to exactly the same players for the same objects.
static bool IsSameEffectKey(const Effect &e, u16 objMatch, bool forEveryone, u8 triggerPlayerId) {
    if (e.objMatch != objMatch || e.forEveryone != forEveryone)
        return false;
    return forEveryone || e.triggerPlayerId == triggerPlayerId;
}

static void PruneSettledEffects(u16 objMatch, bool forEveryone, u8 triggerPlayerId) {
    u32 writeIdx = 0;
    for (u32 readIdx = 0; readIdx < sEffectCount; ++readIdx) {
        if (sEffects[readIdx].durationFrames == 0 && IsSameEffectKey(sEffects[readIdx], objMatch, forEveryone, triggerPlayerId))
            continue;
        if (writeIdx != readIdx)
            sEffects[writeIdx] = sEffects[readIdx];
        ++writeIdx;
    }
    sEffectCount = writeIdx;
}

static void PushEffect(u16 objMatch, u8 action, bool forEveryone, u8 triggerPlayerId, u16 durationFrames) {
    PruneSettledEffects(objMatch, forEveryone, triggerPlayerId);
    if (sEffectCount >= MAX_EFFECTS)
        return;

    Effect &e = sEffects[sEffectCount++];
    e.objMatch = objMatch;
    e.action = action;
    e.forEveryone = forEveryone;
    e.triggerPlayerId = triggerPlayerId;
    e.durationFrames = durationFrames;
    e.serial = ++sEffectSerial;
}

static bool EffectAppliesToPlayer(const Effect &e, u8 playerId) {
    if (e.forEveryone)
        return true;
    return e.triggerPlayerId == playerId;
}

static void GetAREAObjectStateForPadding(u16 padding, u8 playerId, bool *outHasEffect, bool *outVisible) {
    if (!outHasEffect || !outVisible)
        return;
    *outHasEffect = false;
    *outVisible = true;

    const Effect *best = nullptr;
    for (u32 i = 0; i < sEffectCount; ++i) {
        if (sEffects[i].objMatch != padding)
            continue;
        if (!EffectAppliesToPlayer(sEffects[i], playerId))
            continue;
        if (best == nullptr || sEffects[i].serial > best->serial)
            best = &sEffects[i];
    }

    if (best == nullptr)
        return;
    *outHasEffect = true;
    *outVisible = (best->action != 0);
}

void ConditionalObjectFrameUpdate() {
    const KMP::Manager *kmp = KMP::Manager::sInstance;
    const Kart::Manager *kartManager = Kart::Manager::sInstance;
    if (!kmp || !kartManager)
        return;

    // 1) rising edge into an AREA -> queue its action
    for (u8 playerId = 0; playerId < kartManager->playerCount; playerId++) {
        KMPAREAExpander &KAE = GetKAE(playerId);

        if (KAE.condObj && !KAE.prevCondObj && ConditionalObjAction(playerId)) {
            AreaConditionalParams params;
            if (!ReadAreaConditionalParams(KAE.conditionalObject, params))
                continue;

            PushPending(params.matchId, params.action, params.forEveryone, playerId, params.delayFrames, params.durationFrames);
        }
    }

    // 2) count down delays; when finished, turn the pending action into an effect
    for (u32 i = 0; i < sPendingCount;) {
        PendingAction &p = sPendingActions[i];

        if (p.delayFrames > 0) {
            p.delayFrames--;
            ++i;
            continue;
        }

        PushEffect(p.objMatch, p.action, p.forEveryone, p.triggerPlayerId, p.durationFrames);

        for (u32 j = i; j + 1 < sPendingCount; ++j) {
            sPendingActions[j] = sPendingActions[j + 1];
        }
        --sPendingCount;
    }

    // 3) count down durations; when one runs out the effect flips (hide <-> show)
    for (u32 i = 0; i < sEffectCount; ++i) {
        Effect &e = sEffects[i];

        if (e.durationFrames > 0) {
            e.durationFrames--;

            if (e.durationFrames == 0) {
                e.action = e.action == 0 ? 1 : 0;
                e.serial = ++sEffectSerial;
            }
        }
    }
}

void CondObjPrevState() {
    const Kart::Manager *kartManager = Kart::Manager::sInstance;
    if (!kartManager)
        return;

    for (u8 playerId = 0; playerId < kartManager->playerCount; playerId++) {
        KMPAREAExpander &KAE = GetKAE(playerId);

        KAE.prevCondObj = KAE.condObj && ConditionalObjAction(playerId);
    }
}

static void GatherConditionalSource(const Object &object, ConditionalSource &src) {
    src.hasRange = false;
    src.hasAreaId = false;
    src.areaId = 0;

    const GOBJ *gobj = GetObjectGobj(object);
    if (gobj == nullptr)
        return;

    src.hasRange = TryGetConditionalConfig(gobj, src.config);

    // Range modes 5/6 store percentages in padding, so it cannot double as an AREA match id.
    src.hasAreaId = !(src.hasRange && src.config.mode == ConditionalConfig::MODE_LAP_PROGRESS_RANGE);
    if (src.hasAreaId)
        src.areaId = gobj->padding;
}

static bool EvaluateSourceForPlayer(const ConditionalSource &src, u8 playerId, bool &active) {
    bool hasRule = false;
    active = true;

    if (src.hasRange) {
        hasRule = true;
        active = EvaluateConditionalForPlayer(src.config, playerId);
    }

    if (src.hasAreaId) {
        bool hasEffect = false;
        bool visible = true;
        GetAREAObjectStateForPadding(src.areaId, playerId, &hasEffect, &visible);
        if (hasEffect) {
            hasRule = true;
            active = visible;
        }
    }
    return hasRule;
}

// True if the object is active for at least one replay-relevant player
// (or if there is no such player). `anyRule` reports whether any of them has a rule.
static bool EvaluateSourceForAnyReplayPlayer(const ConditionalSource &src, const RacedataScenario &scenario, const Raceinfo &raceInfo, bool &anyRule) {
    bool hasReplayPlayer = false;
    bool anyActive = false;
    anyRule = false;

    for (u8 playerId = 0; playerId < 12; ++playerId) {
        if (!IsConditionalReplayPlayer(scenario, raceInfo, playerId))
            continue;

        hasReplayPlayer = true;
        bool active = true;
        if (EvaluateSourceForPlayer(src, playerId, active))
            anyRule = true;
        if (active)
            anyActive = true;
    }
    return anyActive || !hasReplayPlayer;
}

static void EvaluateConditionalState(const Object &object, ConditionalState &state) {
    InitConditionalState(state);

    ConditionalSource src;
    GatherConditionalSource(object, src);

    if (!src.hasRange && sEffectCount == 0)
        return;

    state.isConditional = src.hasRange;

    const Racedata *raceData = Racedata::sInstance;
    const Raceinfo *raceInfo = Raceinfo::sInstance;
    if (raceData == nullptr || raceInfo == nullptr)
        return;

    const RacedataScenario &scenario = raceData->racesScenario;
    const GameMode mode = scenario.settings.gamemode;
    const bool isTTMode = IsTimeTrialOrGhostMode(mode);

    if (isTTMode) {
        // Keep collision/update active whenever any replay-relevant player can interact with this object.
        bool anyRule = false;
        state.isCollisionActive = EvaluateSourceForAnyReplayPlayer(src, scenario, *raceInfo, anyRule);

        u8 watchedPlayerId = 0xFF;
        const RaceCameraMgr *cameraMgr = RaceCameraMgr::sInstance;
        if (cameraMgr != nullptr) {
            const u8 focusedPlayerId = cameraMgr->focusedPlayerIdx;
            if (IsConditionalReplayPlayer(scenario, *raceInfo, focusedPlayerId))
                watchedPlayerId = focusedPlayerId;
        }

        if (watchedPlayerId == 0xFF) {
            const u8 hudPlayerId = scenario.settings.hudPlayerIds[0];
            if (IsConditionalReplayPlayer(scenario, *raceInfo, hudPlayerId))
                watchedPlayerId = hudPlayerId;
        }

        if (watchedPlayerId != 0xFF) {
            bool watchedActive = true;
            if (EvaluateSourceForPlayer(src, watchedPlayerId, watchedActive))
                anyRule = true;
            state.isActive = watchedActive;
        } else {
            state.isActive = state.isCollisionActive;
        }

        if (anyRule)
            state.isConditional = true;

        state.localScreenCount = 1;
        FillScreenState(state.screenIsActive, state.isActive);
        return;
    }

    const u8 localScreenCount = (scenario.localPlayerCount > 4) ? 4 : scenario.localPlayerCount;
    if (localScreenCount == 0)
        return;

    state.localScreenCount = localScreenCount;
    FillScreenState(state.screenIsActive, true);
    state.isActive = false;
    state.isCollisionActive = false;
    for (u8 i = 0; i < localScreenCount; ++i) {
        const u8 playerId = scenario.settings.hudPlayerIds[i];
        bool playerActive = true;
        if (EvaluateSourceForPlayer(src, playerId, playerActive))
            state.isConditional = true;
        state.screenIsActive[i] = playerActive;
        if (playerActive) {
            state.isActive = true;
            state.isCollisionActive = true;
        }
    }
}

static bool IsObjectActiveForPlayer(const Object &object, u8 playerId) {
    ConditionalSource src;
    GatherConditionalSource(object, src);

    bool active = true;
    EvaluateSourceForPlayer(src, playerId, active);
    return active;
}

static bool IsModelDirectorReadyForPerScreenVisibility(const ModelDirector *director) {
    if (director == nullptr)
        return false;
    if ((director->bitfield & 0x100000) == 0)
        return false;
    return director->scnMdlEx[0] != nullptr && director->scnMdlEx[1] != nullptr;
}

static void ApplyModelDirectorScreenVisibility(ModelDirector *director, const ConditionalState &state) {
    // ScnMgr::UpdateVisibility dereferences both scnMdlEx slots for screen-specific directors.
    if (!IsModelDirectorReadyForPerScreenVisibility(director))
        return;

    for (u8 screenIdx = 0; screenIdx < state.localScreenCount; ++screenIdx) {
        if (state.screenIsActive[screenIdx])
            director->EnableScreen(screenIdx);
        else
            director->DisableScreen(screenIdx);
    }
}

static bool IsScreenSpecificModelRegistered(const ScnMgr &scnMgr, const ModelDirector *director) {
    void *current = nullptr;
    while (true) {
        current = nw4r::ut::List_GetNext(&scnMgr.screenSpecificModelDirectors, current);
        if (current == nullptr)
            return false;
        if (current == director)
            return true;
    }
}

static void EnsureScreenSpecificModelRegistration(ModelDirector *director) {
    // Only register directors that are safe for ScnMgr::UpdateVisibility.
    if (!IsModelDirectorReadyForPerScreenVisibility(director))
        return;

    ScnMgr *scnMgr = director->GetScnManager();
    if (scnMgr == nullptr)
        return;
    if (IsScreenSpecificModelRegistered(*scnMgr, director))
        return;

    // Mark as screen-specific and register so ScnMgr::UpdateVisibility updates this model per local screen.
    director->bitfield |= 0x8;
    scnMgr->AppendScreenSpecificModelDirector(director);
}

static void ApplyPerScreenVisibility(Object &object, const ConditionalState &state) {
    if (!state.isConditional || state.localScreenCount <= 1)
        return;

    EnsureScreenSpecificModelRegistration(object.mdlDirector);
    EnsureScreenSpecificModelRegistration(object.mdlLodDirector);
    EnsureScreenSpecificModelRegistration(object.shadowDirector);

    ApplyModelDirectorScreenVisibility(object.mdlDirector, state);
    ApplyModelDirectorScreenVisibility(object.mdlLodDirector, state);
    ApplyModelDirectorScreenVisibility(object.shadowDirector, state);
}

static void ApplyConditionalState(Object &object, const ConditionalState &state) {
    if (!state.isConditional)
        return;

    object.ToggleVisible(state.isActive);
    if (state.isCollisionActive)
        object.EnableCollision();
    else
        object.DisableCollision();
}

static void ApplyKCLConditionalState(Object &object, const ConditionalState &state) {
    if (!state.isConditional)
        return;

    object.ToggleVisible(state.isActive);
    if (state.isCollisionActive) {
        object.EnableCollision();
        if (object.entity != nullptr)
            object.entity->paramsBitfield |= 0x10;
    } else {
        object.DisableCollision();
        if (object.entity != nullptr)
            object.entity->paramsBitfield &= ~0x10;
    }
}

static ObjectCollision *CallOriginalGetCollision(void *object) {
    typedef ObjectCollision *(*GetCollisionFn)(void *);
    const u32 *vtable = *reinterpret_cast<const u32 *const *>(object);
    GetCollisionFn getCollision = reinterpret_cast<GetCollisionFn>(vtable[0xb4 / 4]);
    return getCollision(object);
}

static ObjectCollision *ConditionalGetObjectCollision(void *object) {
    if (object == nullptr)
        return nullptr;

    ObjectCollision *collision = CallOriginalGetCollision(object);
    if (collision == nullptr)
        return nullptr;

    register const Kart::Player *kartPlayer;
    asm(mr kartPlayer, r25;);
    if (kartPlayer == nullptr)
        return collision;

    const u8 playerId = kartPlayer->GetPlayerIdx();
    const Object &mapObject = *reinterpret_cast<const Object *>(object);
    if (!IsObjectActiveForPlayer(mapObject, playerId))
        return nullptr;
    return collision;
}
kmCall(0x8082ab8c, ConditionalGetObjectCollision);

static ObjectCollision *ConditionalGetObjectCollisionForItem(void *object) {
    if (object == nullptr)
        return nullptr;

    ObjectCollision *collision = CallOriginalGetCollision(object);
    if (collision == nullptr)
        return nullptr;

    register Item::Obj *itemObj;
    asm(mr itemObj, r27;);
    if (itemObj == nullptr)
        return collision;

    const u8 playerId = itemObj->playerUsedItemId;
    const Object &mapObject = *reinterpret_cast<const Object *>(object);
    if (!IsObjectActiveForPlayer(mapObject, playerId))
        return nullptr;
    return collision;
}
kmCall(0x8082ae18, ConditionalGetObjectCollisionForItem);

static bool ConditionalCourseCollisionSetPlayerFromWheel(
  float radius, CourseMgr &mgr, const Vec3 &position, const Vec3 &prevPosition, KCLBitfield acceptedFlags, CollisionInfo *info, KCLTypeHolder &kclFlags) {
    register u32 playerIdRaw;
    asm(mr playerIdRaw, r25;);

    PushConditionalCollisionPlayerContext(static_cast<u8>(playerIdRaw));
    const bool isColliding = mgr.IsCollidingAddEntry(position, prevPosition, acceptedFlags, info, &kclFlags, 0, radius);
    PopConditionalCollisionPlayerContext();
    return isColliding;
}
kmCall(0x805b7028, ConditionalCourseCollisionSetPlayerFromWheel);  // CourseMgr::IsCollidingAddEntry call in KartCollide::calcWheelCollision

static void FilterConditionalDriveablesForCurrentPlayer(void *boxColMgr) {
    u8 playerId = GetConditionalCollisionPlayerContext();

#if CONDITIONAL_OBJECTS_ITEM_DRIVABLE_FALLBACK
    if (playerId >= 12) {
        register Item::Obj *itemObj;
        asm(mr itemObj, r27;);
        if (itemObj != nullptr) {
            playerId = itemObj->playerUsedItemId;
        }
    }
#endif

    if (playerId >= 12 || boxColMgr == nullptr)
        return;

    u8 *const mgr = reinterpret_cast<u8 *>(boxColMgr);
    s32 &maxId = *reinterpret_cast<s32 *>(mgr + 0x438);
    if (maxId <= 0)
        return;

    BoxColUnitView **units = *reinterpret_cast<BoxColUnitView ***>(mgr + 0x1c);
    if (units == nullptr)
        return;

    s32 writeIdx = 0;
    for (s32 readIdx = 0; readIdx < maxId; ++readIdx) {
        BoxColUnitView *unit = units[readIdx];
        if (unit == nullptr)
            continue;

        bool keep = true;
        if ((unit->unitType & BOXCOL_FLAG_DRIVABLE) != 0 && unit->userData != nullptr) {
            const Object &object = *reinterpret_cast<const Object *>(unit->userData);
            keep = IsObjectActiveForPlayer(object, playerId);
        }

        if (keep) {
            units[writeIdx] = unit;
            ++writeIdx;
        }
    }

    maxId = writeIdx;
}

static s32 FindFirstUnitOfType(BoxColUnitView *const *units, s32 maxId, u32 mask) {
    for (s32 i = 0; i < maxId; ++i) {
        const BoxColUnitView *unit = units[i];
        if (unit != nullptr && (unit->unitType & mask) != 0)
            return i;
    }
    return 0x100;
}

static void ConditionalResetIterators(void *boxColMgr) {
    if (boxColMgr == nullptr)
        return;

    FilterConditionalDriveablesForCurrentPlayer(boxColMgr);

    u8 *const mgr = reinterpret_cast<u8 *>(boxColMgr);
    const s32 maxId = *reinterpret_cast<const s32 *>(mgr + 0x438);
    BoxColUnitView **units = *reinterpret_cast<BoxColUnitView ***>(mgr + 0x1c);

    s32 &nextPlayerId = *reinterpret_cast<s32 *>(mgr + 0x428);
    s32 &nextItemId = *reinterpret_cast<s32 *>(mgr + 0x42c);
    s32 &nextObjectId = *reinterpret_cast<s32 *>(mgr + 0x430);
    s32 &nextDrivableId = *reinterpret_cast<s32 *>(mgr + 0x434);

    if (units == nullptr || maxId <= 0) {
        nextPlayerId = 0x100;
        nextItemId = 0x100;
        nextObjectId = 0x100;
        nextDrivableId = 0x100;
        return;
    }

    nextPlayerId = FindFirstUnitOfType(units, maxId, BOXCOL_FLAG_DRIVER);
    nextItemId = FindFirstUnitOfType(units, maxId, BOXCOL_FLAG_ITEM);
    nextObjectId = FindFirstUnitOfType(units, maxId, BOXCOL_FLAG_OBJECT | BOXCOL_FLAG_OBJECT_OBSTACLE_ENEMY);
    nextDrivableId = FindFirstUnitOfType(units, maxId, BOXCOL_FLAG_DRIVABLE);
}
kmBranch(0x80785f2c, ConditionalResetIterators);  // BoxColManager::resetIterators

static void ConditionalCalcCollisions(Kart::Status *status) {
    Kart::Link *link = status->link;
    u8 playerId = link->GetPlayerIdx();
    PushConditionalCollisionPlayerContext(playerId);
    status->UpdateCollisions();
    PopConditionalCollisionPlayerContext();
}
kmCall(0x80594858, ConditionalCalcCollisions);  // KartStatus::UpdateCollisions call in KartStatus::calc

static void ConditionalObjectUpdate(Object *object) {
    if (object == nullptr)
        return;

    ConditionalState state;
    EvaluateConditionalState(*object, state);
    ApplyConditionalState(*object, state);
    if (state.isActive || state.isCollisionActive)
        object->Update();
}
kmCall(0x8082a9e0, ConditionalObjectUpdate);  // Object::Update call in ObjectsMgr::Update

static void ConditionalObjectModelUpdate(Object *object) {
    if (object == nullptr)
        return;

    ConditionalState state;
    EvaluateConditionalState(*object, state);
    if (!state.isActive)
        return;

    object->UpdateModel();
    ApplyPerScreenVisibility(*object, state);
}
kmCall(0x8082aa20, ConditionalObjectModelUpdate);  // Object::UpdateModel call in ObjectsMgr::Update

kmRuntimeUse(0x8081b618);
static void ConditionalProcessAllAndCalcKCL(void *kclMgr, ObjectsMgr &objectsMgr) {
    if (kclMgr == nullptr)
        return;

    const u16 kclCount = *reinterpret_cast<const u16 *>(reinterpret_cast<const u8 *>(kclMgr) + 0x4);
    Object **kclObjects = *reinterpret_cast<Object ***>(reinterpret_cast<u8 *>(kclMgr) + 0x8);
    for (u16 i = 0; i < kclCount; ++i) {
        Object *obj = kclObjects[i];
        if (obj == nullptr)
            continue;
        ConditionalState state;
        EvaluateConditionalState(*obj, state);

        ApplyKCLConditionalState(*obj, state);
        ApplyPerScreenVisibility(*obj, state);
    }

    typedef void (*OriginalCalcFn)(void *);
    OriginalCalcFn originalCalc = reinterpret_cast<OriginalCalcFn>(kmRuntimeAddr(0x8081b618));
    originalCalc(kclMgr);
}
kmCall(0x8082aa40, ConditionalProcessAllAndCalcKCL);  // ObjectDriveableDirector::calc call in ObjectsMgr::Update

static void ConditionalKCLObjectUpdate(Object *object) {
    if (object == nullptr)
        return;

    ConditionalState state;
    EvaluateConditionalState(*object, state);
    ApplyKCLConditionalState(*object, state);
    if (state.isActive || state.isCollisionActive)
        object->Update();
}
kmCall(0x8081b658, ConditionalKCLObjectUpdate);  // ObjectKCL::Update call in ObjectDriveableDirector::calc

static void ConditionalKCLObjectModelUpdate(Object *object) {
    if (object == nullptr)
        return;

    ConditionalState state;
    EvaluateConditionalState(*object, state);
    if (!state.isActive)
        return;

    object->UpdateModel();
    ApplyPerScreenVisibility(*object, state);
}
kmCall(0x8081b698, ConditionalKCLObjectModelUpdate);  // ObjectKCL::UpdateModel call in ObjectDriveableDirector::calc

}  // namespace Race
}  // namespace MKWG