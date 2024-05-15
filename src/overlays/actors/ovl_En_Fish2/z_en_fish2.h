#ifndef Z_EN_FISH2_H
#define Z_EN_FISH2_H

#include "global.h"
#include "assets/objects/object_fb/object_fb.h"

struct EnFish2;

#define ENFISH2_GET_TYPE(thisx) ((thisx)->params)

typedef void (*EnFish2ActionFunc)(struct EnFish2*, PlayState*);

typedef enum EnFish2Type {
    /* 0 */ ENFISH2_TYPE_FISH,
    /* 1 */ ENFISH2_TYPE_CUTSCENE_HANDLER,
    /* 2 */ ENFISH2_TYPE_UNK_2 // unused and only ever checked once in `EnFish2_Update`; crashes the game if spawned
} EnFish2Type;

typedef struct EnFish2Effect {
    /* 0x00 */ u8 isEnabled;
    /* 0x04 */ Vec3f pos;
    /* 0x10 */ s16 timer;
    /* 0x14 */ f32 scale;
    /* 0x18 */ f32 targetScale;
    /* 0x1C */ s16 unk_1C; // always 66 in the final game; if it is set to 0, the effect is immediately disabled
    /* 0x20 */ TexturePtr texture;
} EnFish2Effect; // size = 0x24

#define ENFISH2_EFFECT_COUNT 200

typedef struct EnFish2 {
    /* 0x000 */ Actor actor;
    /* 0x144 */ SkelAnime skelAnime;
    /* 0x188 */ Vec3s jointTable[RESEARCH_LAB_FISH_LIMB_MAX];
    /* 0x218 */ Vec3s morphTable[RESEARCH_LAB_FISH_LIMB_MAX];
    /* 0x2A8 */ EnFish2ActionFunc actionFunc;
    /* 0x2AC */ s32 animIndex;
    /* 0x2B0 */ union {
                    s32 isChasingOtherLabFish;
                    s32 cutsceneWaitTimer;
                };
    /* 0x2B4 */ s16 waitTimer;
    /* 0x2B6 */ s16 timer;
    /* 0x2B8 */ s16 wallCheckTimer;
    /* 0x2BA */ s16 csIdList[3];
    /* 0x2C0 */ s32 scaleIndex;
    /* 0x2C4 */ union {
                    s32 scaleUpState;
                    s32 frameCounter;
                };
    /* 0x2C8 */ u8 wasFedBugs;
    /* 0x2CA */ s16 subCamId;
    /* 0x2CC */ f32 animEndFrame;
    /* 0x2D0 */ UNK_TYPE1 unk_2D0[0x4];
    /* 0x2D4 */ f32 minDistFromFloor;
    /* 0x2D8 */ f32 minDistFromWaterSurface;
    /* 0x2DC */ Vec3f subCamEye;
    /* 0x2E8 */ Vec3f subCamAt;
    /* 0x2F4 */ Vec3f wallCheckPos;
    /* 0x300 */ Vec3f headPos;
    /* 0x30C */ Vec3f targetActorPos; // set but never used
    /* 0x318 */ Vec3f lowerJawPos;
    /* 0x324 */ Vec3f targetPos;
    /* 0x330 */ f32 scale;
    /* 0x334 */ f32 waterSurface;
    /* 0x338 */ f32 targetActorSpeedMultiplier;
    /* 0x33C */ f32 wallCheckRadius;
    /* 0x340 */ s32 direction; // determines which direction the fish should turn when it hits a wall
    /* 0x344 */ s32 index;
    /* 0x348 */ s16 targetRotX;
    /* 0x34A */ s16 targetRotY;
    /* 0x34C */ s16 angularVelocityModX; // the fish's X-rotation will step by 100 plus this value on every frame
    /* 0x350 */ Actor* targetActor;
    /* 0x354 */ struct EnFish2* cutsceneHandler;
    /* 0x358 */ ColliderJntSph collider;
    /* 0x378 */ ColliderJntSphElement colliderElements[2];
    /* 0x3F8 */ EnFish2Effect effects[ENFISH2_EFFECT_COUNT];
} EnFish2; // size = 0x2018

#endif // Z_EN_FISH2_H
