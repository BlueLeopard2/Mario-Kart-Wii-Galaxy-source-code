#include <kamek.hpp>
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

// Conditional Objects [Retro Rewind] + AREA-based detection only [BlueLeopard]

/*
GOBJ->padding (u16) -> setting1
action (2 bits) routeId: bit 0-1
forEveryone (1 bit) routeId: bit 2
lapCount (3 bits) routeId: bits 3-5
durationFrames (u16) -> setting2
delayFrames (u8) -> enemyrouteId
*/

namespace MKWG {
namespace Race {

// --- AREA state (detection + delay/duration/forEveryone) ---
static const u32 MAX_PENDING = 32;
static const u32 MAX_EFFECTS = 64;

struct PendingAction {
    u16 objMatch;
    u8 action;       // 0 = remove (hide), 1 = add (show)
    bool forEveryone;  // true = for everyone, false = only for triggerPlayerId
    u8 triggerPlayerId;
    u16 delayFrames;
    u16 durationFrames;
    u16 delayCounter;
    bool valid;
};

struct Effect {
    u16 objMatch;
    u8 action;
    bool forEveryone;
    u8 triggerPlayerId;
    u16 durationRemaining;
    u32 serial;
    bool valid;
};

static PendingAction sPendingActions[MAX_PENDING];
static Effect sEffects[MAX_EFFECTS];
static u32 sEffectSerial = 0;

static void InitConditionalObjectState() {
    for (u32 i = 0; i < MAX_PENDING; ++i) sPendingActions[i].valid = false;
    for (u32 i = 0; i < MAX_EFFECTS; ++i) sEffects[i].valid = false;
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

    const u16 areaSetting1 = kmp->areaSection->GetHolder(KAE.ConditionalObject)->raw->setting1;
    u16 currentLap = raceInfoPlayer->currentLap;

    for (u16 i = 0; i < kmp->gobjSection->pointCount; i++) {
        KMP::Holder<GOBJ>* holder = kmp->gobjSection->holdersArray[i];
        if (!holder || !holder->raw) continue;
        if (holder->raw->padding != areaSetting1) continue;

        const u8 correctLap = (kmp->areaSection->GetHolder(KAE.ConditionalObject)->raw->routeId >> 3) & 0x7;
        if (currentLap == 0 || currentLap > 50) currentLap = 1;
        if (correctLap != 0 && correctLap != currentLap) continue;
        return true;
    }
    return false;
}

static void PushPending(u16 objMatch, u8 action, bool forEveryone, u8 triggerPlayerId, u16 delayFrames, u16 durationFrames) {
    for (u32 i = 0; i < MAX_PENDING; ++i) {
        if (!sPendingActions[i].valid) {
            sPendingActions[i].objMatch = objMatch;
            sPendingActions[i].action = action;
            sPendingActions[i].forEveryone = forEveryone;
            sPendingActions[i].triggerPlayerId = triggerPlayerId;
            sPendingActions[i].delayFrames = delayFrames;
            sPendingActions[i].durationFrames = durationFrames;
            sPendingActions[i].delayCounter = delayFrames;
            sPendingActions[i].valid = true;
            return;
        }
    }
}

static void PushEffect(u16 objMatch, u8 action, bool forEveryone, u8 triggerPlayerId, u16 durationFrames) {
    for (u32 i = 0; i < MAX_EFFECTS; ++i) {
        if (!sEffects[i].valid) {
            sEffects[i].objMatch = objMatch;
            sEffects[i].action = action;
            sEffects[i].forEveryone = forEveryone;
            sEffects[i].triggerPlayerId = triggerPlayerId;
            sEffects[i].durationRemaining = durationFrames;
            sEffects[i].serial = ++sEffectSerial;
            sEffects[i].valid = true;
            return;
        }
    }
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
    for (u32 i = 0; i < MAX_EFFECTS; ++i) {
        if (!sEffects[i].valid || sEffects[i].objMatch != padding) continue;
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
            u8 action = kmp->areaSection->GetHolder(KAE.ConditionalObject)->raw->routeId & 0x3; // appear / disappear. May add more actions later if possible
            bool forEveryone = (kmp->areaSection->GetHolder(KAE.ConditionalObject)->raw->routeId & 0x4) != 0; // only for activator or for everyone
            u16 areaSetting1 = kmp->areaSection->GetHolder(KAE.ConditionalObject)->raw->setting1; // just so it can get transported to PushPending
            u8 delayFrames = kmp->areaSection->GetHolder(KAE.ConditionalObject)->raw->enemyRouteId; // frames it takes to enable / disable. good for animations later.
            u16 durationFrames = kmp->areaSection->GetHolder(KAE.ConditionalObject)->raw->setting2; // frames it takes to return on previous action.

            PushPending(areaSetting1, action, forEveryone, playerId, delayFrames, durationFrames);
        }
    }
    

    for (u32 i = 0; i < MAX_PENDING; ++i) {
        if (!sPendingActions[i].valid) continue;
        PendingAction& p = sPendingActions[i];
        if (p.delayCounter > 0) {
            p.delayCounter--;
            continue;
        }
        PushEffect(p.objMatch, p.action, p.forEveryone, p.triggerPlayerId, p.durationFrames);
        p.valid = false;
    }

    for (u32 i = 0; i < MAX_EFFECTS; ++i) {
        if (!sEffects[i].valid) continue;
        Effect& e = sEffects[i];
        if (e.durationRemaining > 0) {
            e.durationRemaining--;
            if (e.durationRemaining == 0) {
                u8 oppositeAction = e.action == 0 ? 1 : 0;
                PushEffect(e.objMatch, oppositeAction, e.forEveryone, e.triggerPlayerId, 0);
                e.valid = false;
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

// from here

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
} // namespace MKWG