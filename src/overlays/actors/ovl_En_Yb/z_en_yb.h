#ifndef Z_EN_YB_H
#define Z_EN_YB_H

#include "global.h"
#include "objects/object_yb/object_yb.h"

struct EnYb;

typedef void (*EnYbActionFunc)(struct EnYb*, PlayState*);

typedef struct EnYb {
    /* 0x000 */ Actor actor;
    /* 0x144 */ SkelAnime skelAnime;
    /* 0x188 */ Vec3s jointTable[YB_LIMB_MAX];
    /* 0x20C */ UNK_TYPE1 unkPadding20C[0x94];
    /* 0x2A0 */ Vec3s morphTable[YB_LIMB_MAX];
    /* 0x324 */ UNK_TYPE1 unkPadding324[0x94];
    /* 0x3B8 */ ColliderCylinder collider;
    /* 0x404 */ Vec3f shadowPos;
    /* 0x410 */ u16 playerOcarinaOut;
    /* 0x412 */ s16 animIndex;
    /* 0x414 */ s16 alpha;
    /* 0x416 */ s16 cutscenes[2];
    /* 0x41A */ s16 cutsceneIndex;
    /* 0x41C */ s16 teachingCutsceneTimer;
    /* 0x420 */ EnYbActionFunc actionFunc;
    /* 0x424 */ u8 holdTimer;
    /* 0x425 */ u8 scrollRate;
    /* 0x426 */ u8 longScrollTimer;
} EnYb; // size = 0x425

#endif // Z_EN_YB_H
