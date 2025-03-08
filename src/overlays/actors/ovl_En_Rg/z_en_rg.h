#ifndef Z_EN_RG_H
#define Z_EN_RG_H

#include "global.h"
#include "assets/objects/object_oF1d_map/object_oF1d_map.h"

struct EnRg;

typedef void (*EnRgActionFunc)(struct EnRg*, PlayState*);

#define ENRG_GET_PATH_INDEX(thisx) ((((thisx)->params & 0x7F80) >> 7) & 0xFF)

#define ENRG_PATH_INDEX_NONE 0xFF

#define ENRG_FIDGET_TABLE_LEN 3

#define ENRG_FLAG_3 (1 << 3)
#define ENRG_FLAG_ROLLED_UP (1 << 4)
#define ENRG_FLAG_AT_HIT (1 << 5)
#define ENRG_FLAG_AC_HIT (1 << 6)
#define ENRG_FLAG_OC1_HIT (1 << 7)
#define ENRG_FLAG_8 (1 << 8)
#define ENRG_FLAG_BOUNCING_IN_AIR (1 << 10)
#define ENRG_FLAG_HAS_TARGET_ACTOR (1 << 11)
#define ENRG_FLAG_CROSSED_FINISH_LINE (1 << 12)

typedef struct EnRgEffect {
    /* 0x00 */ u8 unk_00;
    /* 0x01 */ u8 unk_01;
    /* 0x02 */ u8 timer;
    /* 0x03 */ UNK_TYPE1 unk_03[0xD];
    /* 0x10 */ Vec3f pos;
    /* 0x1C */ Vec3f accel;
    /* 0x28 */ Vec3f velocity;
    /* 0x34 */ f32 scale;
    /* 0x38 */ f32 unk_38;
} EnRgEffect; // size = 0x3C

typedef struct EnRg {
    /* 0x000 */ Actor actor;
    /* 0x144 */ SkelAnime skelAnime;
    /* 0x188 */ EnRgActionFunc actionFunc;
    /* 0x18C */ Actor* targetActor;
    /* 0x190 */ ColliderCylinder colliderCylinder;
    /* 0x1DC */ ColliderSphere colliderSphere;
    /* 0x234 */ Path* path;
    /* 0x238 */ Vec3s jointTable[GORON_LIMB_MAX];
    /* 0x2A4 */ Vec3s morphTable[GORON_LIMB_MAX];
    /* 0x310 */ u16 flags;
    /* 0x314 */ f32 animPlaySpeed;
    /* 0x318 */ s16 timer;
    /* 0x31A */ UNK_TYPE1 unk31A[2];
    /* 0x31C */ s16 blinkTimer;
    /* 0x31E */ s16 eyeIndex;
    /* 0x320 */ s16 boostTimer;
    /* 0x322 */ s16 collisionTimer;
    /* 0x324 */ s16 treeCollisionTimer;
    /* 0x326 */ s16 playerCollisionTimer;
    /* 0x328 */ s16 fidgetTableZ[ENRG_FIDGET_TABLE_LEN];
    /* 0x32E */ s16 fidgetTableY[ENRG_FIDGET_TABLE_LEN];
    /* 0x334 */ s32 animIndex;
    /* 0x338 */ UNK_TYPE1 unk338[4];
    /* 0x33C */ s32 currentPoint;
    /* 0x340 */ s32 tireMarkEffectIndex;
    /* 0x344 */ s32 sceneExitIndex;
    /* 0x348 */ s32 numCheckpointsAheadOfPlayer;
    /* 0x34C */ EnRgEffect effects[32];
} EnRg; // size = 0xACC

#endif // Z_EN_RG_H
