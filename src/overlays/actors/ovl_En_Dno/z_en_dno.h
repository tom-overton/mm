#ifndef Z_EN_DNO_H
#define Z_EN_DNO_H

#include "global.h"
#include "objects/object_dnj/object_dnj.h"

struct EnDno;

typedef void (*EnDnoActionFunc)(struct EnDno*, PlayState*);

#define EN_DNO_GET_PATH_INDEX(thisx) ((thisx)->params & 0x7F)
#define EN_DNO_GET_RACE_STARTED_SWITCH_FLAG(thisx) (((thisx)->params >> 7) & 0x7F)
#define EN_DNO_GET_TYPE(thisx) (((thisx)->params >> 0xE) & 0x3)

typedef enum {
    /* 0x0 */ EN_DNO_TYPE_DEKU_SHRINE,
    /* 0x1 */ EN_DNO_TYPE_DEKU_KINGS_CHAMBER,
} EnDnoType;

typedef struct EnDno {
    /* 0x000 */ Actor actor;
    /* 0x144 */ EnDnoActionFunc actionFunc;
    /* 0x148 */ SkelAnime skelAnime;
    /* 0x18C */ ColliderCylinder collider;
    /* 0x1D8 */ Vec3s jointTable[DEKU_BUTLER_LIMB_MAX];
    /* 0x280 */ Vec3s morphTable[DEKU_BUTLER_LIMB_MAX];
    /* 0x328 */ s32 raceState;
    /* 0x32C */ s32 animIndex;
    /* 0x330 */ UNK_TYPE1 unk_330[0x4];
    /* 0x334 */ Vec3f unk_334;
    /* 0x340 */ ActorPathing actorPath;
    /* 0x3AC */ s16 bobPhase1;
    /* 0x3AE */ s16 bobPhase2;
    /* 0x3B0 */ u16 unk_3B0; // some kind of flags
    /* 0x3B2 */ UNK_TYPE1 unk_3B2[0xC];
    /* 0x3BE */ s16 unk_3BE; // Initialized, but never used
    /* 0x3C0 */ f32 unk_3C0; // Initialized, but never used
    /* 0x3C4 */ UNK_TYPE1 unk_3C4[0x78];
    /* 0x43C */ LightNode* lightNode;
    /* 0x440 */ LightInfo lightInfo;
    /* 0x44E */ u8 unk_44E;
    /* 0x44F */ UNK_TYPE1 unk_44F[0x3];
    /* 0x452 */ s16 unk_452; // Determines which parts of the model to draw
    /* 0x454 */ f32 parasolScale;
    /* 0x458 */ s32 getItemId;
    /* 0x45C */ s16 parasolRot;
    /* 0x460 */ Actor* dekuKing;
    /* 0x464 */ u16 textId;
    /* 0x466 */ s16 upperBodyRot;
    /* 0x468 */ u8 csAction;
} EnDno; // size = 0x46C

extern const ActorInit En_Dno_InitVars;

#endif // Z_EN_DNO_H
