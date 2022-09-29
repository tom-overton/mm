/*
 * File: z_en_dno.c
 * Overlay: ovl_En_Dno
 * Description: Deku Butler
 *
 * This actor's name is probably short for "Dekunuts Ou (king)". This is likely a misnamed actor,
 * since it uses assets from object_dnj (probably short for "Dekunuts Jii (grandfather, elderly
 * person"). Note that the Deku Princess and King both call the Butler "Jii" in the Japanese text.
 */

#include "z_en_dno.h"
#include "overlays/actors/ovl_Bg_Crace_Movebg/z_bg_crace_movebg.h"
#include "objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_1 | ACTOR_FLAG_8 | ACTOR_FLAG_10 | ACTOR_FLAG_20)

#define THIS ((EnDno*)thisx)

void EnDno_Init(Actor* thisx, PlayState* play);
void EnDno_Destroy(Actor* thisx, PlayState* play);
void EnDno_Update(Actor* thisx, PlayState* play);
void EnDno_Draw(Actor* thisx, PlayState* play);

void EnDno_Credits_SetupGrieve(EnDno* this, PlayState* play);
void EnDno_Credits_DoNothing(EnDno* this, PlayState* play);
void EnDno_KingsChamber_SetupIdle(EnDno* this, PlayState* play);
void EnDno_KingsChamber_Idle(EnDno* this, PlayState* play);
void EnDno_KingsChamber_SetupTalk(EnDno* this, PlayState* play);
void EnDno_KingsChamber_Talk(EnDno* this, PlayState* play);
void EnDno_DekuShrine_SetupIdle(EnDno* this, PlayState* play);
void EnDno_DekuShrine_Idle(EnDno* this, PlayState* play);
void EnDno_DekuShrine_SetupTalk(EnDno* this, PlayState* play);
void EnDno_DekuShrine_Talk(EnDno* this, PlayState* play);
void EnDno_DekuShrine_SetupStartRace(EnDno* this, PlayState* play);
void EnDno_DekuShrine_StartRace(EnDno* this, PlayState* play);
void EnDno_DekuShrine_SetupGiveReward(EnDno* this, PlayState* play);
void EnDno_DekuShrine_GiveReward(EnDno* this, PlayState* play);
void EnDno_DekuShrine_SetupRace(EnDno* this, PlayState* play);
void EnDno_DekuShrine_SpawnBlueWarp(EnDno* this, PlayState* play);
void EnDno_DekuShrine_Race(EnDno* this, PlayState* play);
void EnDno_DekuShrine_SetupEndRace(EnDno* this, PlayState* play);
void EnDno_DekuShrine_EndRace(EnDno* this, PlayState* play);
s32 EnDno_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx);
void EnDno_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx);

typedef enum {
    /* 0 */ EN_DNO_RACE_STATE_NOT_FINISHED,
    /* 2 */ EN_DNO_RACE_STATE_FINISHED = 2,
    /* 3 */ EN_DNO_RACE_STATE_REWARD_GIVEN,
} EnDnoRaceState;

typedef enum {
    /*  0 */ EN_DNO_ANIM_START_RACE_START,
    /*  1 */ EN_DNO_ANIM_START_RACE_END,
    /*  2 */ EN_DNO_ANIM_FLY,
    /*  3 */ EN_DNO_ANIM_FAREWELL,
    /*  4 */ EN_DNO_ANIM_GREETING,
    /*  5 */ EN_DNO_ANIM_GREETING_WITH_CANDLE,
    /*  6 */ EN_DNO_ANIM_PRAYER_LOOP,
    /*  7 */ EN_DNO_ANIM_CLOSE_PARASOL,
    /*  8 */ EN_DNO_ANIM_OPEN_PARASOL,
    /*  9 */ EN_DNO_ANIM_IMPLORE_LOOP,
    /* 10 */ EN_DNO_ANIM_IMPLORE_END,
    /* 11 */ EN_DNO_ANIM_TALK,
    /* 12 */ EN_DNO_ANIM_TALK_WITH_PARSOL_AND_CANDLE,
    /* 13 */ EN_DNO_ANIM_IDLE,
    /* 14 */ EN_DNO_ANIM_IDLE_WITH_CANDLE,
    /* 15 */ EN_DNO_ANIM_PRAYER_START,
    /* 16 */ EN_DNO_ANIM_IMPLORE_START,
    /* 17 */ EN_DNO_ANIM_SHOCK_START,
    /* 18 */ EN_DNO_ANIM_SHOCK_LOOP,
    /* 19 */ EN_DNO_ANIM_GRIEVE,
} EnDnoAnimation;

static AnimationSpeedInfo sAnimations[] = {
    { &gDekuButlerStartRaceStartAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerStartRaceEndAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerFlyAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gDekuButlerFarewellAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerGreetingAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerGreetingWithCandleAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerPrayerLoopAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gDekuButlerCloseParasolAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerOpenParasolAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerImploreLoopAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gDekuButlerImploreEndAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerTalkAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gDekuButlerTalkWithParasolAndCandleAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gDekuButlerIdleAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gDekuButlerIdleWithCandleAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gDekuButlerPrayerStartAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerImploreStartAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerShockStartAnim, 1.0f, ANIMMODE_ONCE, 0.0f },
    { &gDekuButlerShockLoopAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gDekuButlerGrieveAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
};

const ActorInit En_Dno_InitVars = {
    ACTOR_EN_DNO,
    ACTORCAT_NPC,
    FLAGS,
    OBJECT_DNJ,
    sizeof(EnDno),
    (ActorFunc)EnDno_Init,
    (ActorFunc)EnDno_Destroy,
    (ActorFunc)EnDno_Update,
    (ActorFunc)EnDno_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COLTYPE_HIT0,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK1,
        { 0x00000000, 0x00, 0x00 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        TOUCH_NONE | TOUCH_SFX_NORMAL,
        BUMP_ON,
        OCELEM_ON,
    },
    { 17, 58, 0, { 0, 0, 0 } },
};

static Vec3f sUnitVecZ = { 0.0f, 0.0f, 1.0f };

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(uncullZoneDownward, 80, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneForward, 4000, ICHAIN_STOP),
};

void EnDno_UpdateCandleLight(EnDno* this, PlayState* play, Vec3f* pos) {
    f32 rand = Rand_ZeroOne() * 0.5f;

    Lights_PointGlowSetInfo(&this->lightInfo, pos->x, pos->y, pos->z, (127.5f * rand) + 127.5f,
                            (100.0f * rand) + 100.0f, (40.0f * rand) + 40.0f, 320);
}

s32 func_80A71424(s16* arg0, s16 arg1, s16 yawToPlayer, s16 rotY, s16 arg4, s16 step) {
    s16 temp_v0 = yawToPlayer - rotY;
    s32 ret;

    if (arg4 >= ABS(temp_v0)) {
        ret = Math_ScaledStepToS(arg0, arg1 + temp_v0, step);
    } else {
        ret = Math_ScaledStepToS(arg0, arg1, step);
    }

    return ret;
}

void EnDno_OpenFirstSildingDoor(EnDno* this, PlayState* play) {
    Actor* actor = NULL;

    do {
        actor = SubS_FindActor(play, actor, ACTORCAT_BG, ACTOR_BG_CRACE_MOVEBG);
        if (actor != NULL) {
            if (BG_CRACE_MOVEBG_GET_TYPE(actor) == BG_CRACE_MOVEBG_TYPE_OPENING) {
                Flags_SetSwitch(play, BG_CRACE_MOVEBG_GET_SWITCH_FLAG(actor));
            }
            actor = actor->next;
        }
    } while (actor != NULL);
}

// Unused?
void func_80A7153C(EnDno* this, Vec3f* arg1, Vec3f* arg2) {
    f32 cos = Math_CosS(this->actor.home.rot.y);
    f32 sin = Math_SinS(this->actor.home.rot.y);
    f32 temp_f2 = arg2->x - this->actor.home.pos.x;
    f32 temp_f12 = arg2->z - this->actor.home.pos.z;

    arg1->x = (temp_f2 * cos) - (temp_f12 * sin);
    arg1->z = (temp_f12 * cos) + (temp_f2 * sin);
    arg1->y = arg2->y - this->actor.home.pos.y;
}

void EnDno_CheckIfButlerIsBeyondDoor(EnDno* this, PlayState* play) {
    BgCraceMovebg* crace = NULL;
    s32 pad[2];
    Vec3f intersect;
    Vec3f diff;
    Vec3f sp70;

    do {
        crace = (BgCraceMovebg*)SubS_FindActor(play, &crace->dyna.actor, ACTORCAT_BG, ACTOR_BG_CRACE_MOVEBG);
        if (crace != NULL) {
            if ((BG_CRACE_MOVEBG_GET_TYPE(&crace->dyna.actor) == BG_CRACE_MOVEBG_TYPE_CLOSING) &&
                !(crace->flags & BG_CRACE_MOVEBG_FLAG_BUTLER_IS_BEYOND_DOOR)) {
                if (SubS_LineSegVsPlane(&crace->dyna.actor.home.pos, &crace->dyna.actor.home.rot, &sUnitVecZ,
                                        &this->actor.prevPos, &this->actor.world.pos, &intersect)) {
                    Math_Vec3f_Diff(&this->actor.world.pos, &crace->dyna.actor.home.pos, &diff);
                    Matrix_RotateYS(-crace->dyna.actor.home.rot.y, MTXMODE_NEW);
                    Matrix_MultVec3f(&diff, &sp70);
                    if ((fabsf(sp70.x) < 100.0f) && (sp70.y >= -10.0f) && (sp70.y <= 180.0f) && (sp70.z < 0.0f)) {
                        crace->flags |= BG_CRACE_MOVEBG_FLAG_BUTLER_IS_BEYOND_DOOR;
                    }
                }
            }
            crace = (BgCraceMovebg*)crace->dyna.actor.next;
        }
    } while (crace != NULL);
}

void EnDno_InitializeSlidingDoors(EnDno* this, PlayState* play) {
    Actor* actor = NULL;

    do {
        actor = SubS_FindActor(play, actor, ACTORCAT_BG, ACTOR_BG_CRACE_MOVEBG);
        if (actor != NULL) {
            Flags_UnsetSwitch(play, BG_CRACE_MOVEBG_GET_SWITCH_FLAG(actor));
            actor = actor->next;
        }
    } while (actor != NULL);
}

void EnDno_Init(Actor* thisx, PlayState* play) {
    EnDno* this = THIS;
    s32 pad;
    Actor* actor = NULL;

    while (true) {
        actor = SubS_FindActor(play, actor, ACTORCAT_NPC, ACTOR_EN_DNO);
        if (actor != NULL) {
            if (actor != thisx) {
                Actor_MarkForDeath(thisx);
                break;
            }
            actor = actor->next;
        }

        if (actor == NULL) {
            Actor_ProcessInitChain(thisx, sInitChain);
            ActorShape_Init(&thisx->shape, 0.0f, ActorShadow_DrawCircle, 21.0f);
            SkelAnime_InitFlex(play, &this->skelAnime, &gDekuButlerSkel, &gDekuButlerIdleAnim, this->jointTable,
                               this->morphTable, DEKU_BUTLER_LIMB_MAX);
            Collider_InitCylinder(play, &this->collider);
            Collider_SetCylinder(play, &this->collider, thisx, &sCylinderInit);
            Actor_UpdateBgCheckInfo(play, thisx, 0.0f, 0.0f, 0.0f, 4);
            Animation_Change(&this->skelAnime, sAnimations[EN_DNO_ANIM_IDLE_WITH_CANDLE].animation, 1.0f, 0.0f,
                             Animation_GetLastFrame(sAnimations[EN_DNO_ANIM_IDLE_WITH_CANDLE].animation),
                             sAnimations[EN_DNO_ANIM_IDLE_WITH_CANDLE].mode,
                             sAnimations[EN_DNO_ANIM_IDLE_WITH_CANDLE].morphFrames);

            this->unk_3BE = 0x3E93;
            this->unk_3C0 = 60.0f;
            this->unk_3B0 = 0;
            this->csAction = 99;
            this->skelAnime.playSpeed = 0.0f;

            switch (EN_DNO_GET_TYPE(thisx)) {
                case EN_DNO_TYPE_DEKU_SHRINE:
                    EnDno_InitializeSlidingDoors(this, play);
                    if (!(gSaveContext.save.weekEventReg[23] & 0x20) || (gSaveContext.save.weekEventReg[93] & 2)) {
                        Actor_MarkForDeath(thisx);
                    } else {
                        SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE_WITH_CANDLE,
                                                        &this->animIndex);
                        thisx->room = -1;
                        gSaveContext.timerStates[TIMER_ID_MINIGAME_1] = TIMER_STATE_STOP;
                        this->lightNode = LightContext_InsertLight(play, &play->lightCtx, &this->lightInfo);
                        this->unk_3B0 |= 1;
                        this->actionFunc = EnDno_DekuShrine_SetupIdle;
                        this->parasolScale = 0.0f;
                    }
                    break;

                case EN_DNO_TYPE_DEKU_KINGS_CHAMBER:
                    if (gSaveContext.save.weekEventReg[23] & 0x20) {
                        Actor_MarkForDeath(thisx);
                    } else {
                        SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE,
                                                        &this->animIndex);
                        this->dekuKing = SubS_FindActor(play, NULL, ACTORCAT_NPC, ACTOR_EN_DNQ);
                        if (this->dekuKing == NULL) {
                            Actor_MarkForDeath(thisx);
                        } else {
                            this->actionFunc = EnDno_KingsChamber_SetupIdle;
                        }
                    }
                    break;

                default:
                    this->actionFunc = EnDno_Credits_SetupGrieve;
                    break;
            }
            break;
        }
    }
}

void EnDno_Destroy(Actor* thisx, PlayState* play) {
    EnDno* this = THIS;

    Collider_DestroyCylinder(play, &this->collider);
    LightContext_RemoveLight(play, &play->lightCtx, this->lightNode);
}

void EnDno_Credits_SetupGrieve(EnDno* this, PlayState* play) {
    this->unk_452 = 0;
    SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_GRIEVE, &this->animIndex);
    this->actionFunc = EnDno_Credits_DoNothing;
}

void EnDno_Credits_DoNothing(EnDno* this, PlayState* play) {
}

void EnDno_KingsChamber_SetupIdle(EnDno* this, PlayState* play) {
    this->unk_452 = 0;
    this->actor.textId = 0;
    if (CHECK_QUEST_ITEM(QUEST_SONG_SONATA)) {
        if (gSaveContext.save.weekEventReg[27] & 1) {
            if (!(this->unk_3B0 & 0x20)) {
                SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_PRAYER_LOOP,
                                                &this->animIndex);
                this->actor.shape.rot.y = Actor_YawBetweenActors(&this->actor, this->dekuKing);
            }
        } else {
            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE, &this->animIndex);
        }
    } else {
        SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE, &this->animIndex);
    }
    this->actionFunc = EnDno_KingsChamber_Idle;
}

void EnDno_KingsChamber_Idle(EnDno* this, PlayState* play) {
    switch (this->animIndex) {
        case EN_DNO_ANIM_IMPLORE_LOOP:
        case EN_DNO_ANIM_IMPLORE_START:
            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IMPLORE_END, &this->animIndex);
            break;

        case EN_DNO_ANIM_FAREWELL:
        case EN_DNO_ANIM_IMPLORE_END:
            if (this->skelAnime.curFrame == this->skelAnime.endFrame) {
                SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE, &this->animIndex);
            }
            break;

        case EN_DNO_ANIM_TALK:
            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_PRAYER_START, &this->animIndex);
            break;

        case EN_DNO_ANIM_PRAYER_START:
            if (this->skelAnime.curFrame == this->skelAnime.endFrame) {
                SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_PRAYER_LOOP,
                                                &this->animIndex);
            }

        case EN_DNO_ANIM_PRAYER_LOOP:
            Math_SmoothStepToS(&this->actor.shape.rot.y, Actor_YawBetweenActors(&this->actor, this->dekuKing), 2, 0xE38,
                               0x222);
            break;
    }

    if ((this->animIndex == EN_DNO_ANIM_IDLE) && (this->actor.xzDistToPlayer <= 120.0f)) {
        func_80A71424(&this->upperBodyRot, 0, this->actor.yawTowardsPlayer, this->actor.shape.rot.y, 0x2000, 0x16C);
    } else {
        Math_ScaledStepToS(&this->upperBodyRot, 0, 0x16C);
    }

    if ((this->animIndex != EN_DNO_ANIM_FAREWELL) && (this->animIndex != EN_DNO_ANIM_PRAYER_START) &&
        (this->animIndex != EN_DNO_ANIM_PRAYER_LOOP)) {
        Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.home.rot.y, 0x222);
    }

    if (Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        play->msgCtx.msgMode = 0;
        play->msgCtx.msgLength = 0;
        EnDno_KingsChamber_SetupTalk(this, play);
    } else if (this->actor.xzDistToPlayer < 60.0f) {
        func_800B8614(&this->actor, play, 60.0f);
    }
}

void EnDno_KingsChamber_SetupTalk(EnDno* this, PlayState* play) {
    if (CHECK_QUEST_ITEM(QUEST_SONG_SONATA)) {
        if (gSaveContext.save.weekEventReg[27] & 1) {
            this->textId = 0x811; // Lords, please save us!
        } else {
            this->textId = 0x80F; // This might be the end of the Kingdom!
            gSaveContext.save.weekEventReg[27] |= 1;
        }
    } else if (gSaveContext.save.weekEventReg[26] & 0x80) {
        this->textId = 0x80B; // Calm down, your highness!
    } else {
        this->textId = 0x80C; // The king cannot calm down...
        gSaveContext.save.weekEventReg[26] |= 0x80;
    }

    if (this->textId != 0x811) {
        this->unk_3B0 |= 0x10;
    } else {
        this->unk_3B0 &= ~0x10;
    }

    this->actionFunc = EnDno_KingsChamber_Talk;
}

void EnDno_KingsChamber_Talk(EnDno* this, PlayState* play) {
    Math_ScaledStepToS(&this->upperBodyRot, 0, 0x16C);
    switch (Message_GetState(&play->msgCtx)) {
        case TEXT_STATE_NONE:
            if (!(this->unk_3B0 & 0x10) ||
                Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0xE38)) {
                switch (this->textId) {
                    case 0x80B: // Calm down, your highness!
                        SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IMPLORE_START,
                                                        &this->animIndex);
                        // Fallthrough

                    case 0x811: // Lords, please save us!
                        Message_StartTextbox(play, this->textId, &this->actor);
                        break;

                    case 0x80C: // The king cannot calm down...
                    case 0x80F: // This might be the end of the Kingdom!
                        if (this->animIndex == EN_DNO_ANIM_IDLE) {
                            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_GREETING,
                                                            &this->animIndex);
                        } else if ((this->animIndex == EN_DNO_ANIM_GREETING) &&
                                   (this->skelAnime.curFrame == this->skelAnime.endFrame)) {
                            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_TALK,
                                                            &this->animIndex);
                            Message_StartTextbox(play, this->textId, &this->actor);
                        }
                        break;
                }
            }
            break;

        case TEXT_STATE_3:
            // Calm down, your highness!
            if (play->msgCtx.currentTextId == 0x80B) {
                switch (this->animIndex) {
                    case EN_DNO_ANIM_IMPLORE_START:
                        if (this->skelAnime.curFrame == this->skelAnime.endFrame) {
                            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IMPLORE_LOOP,
                                                            &this->animIndex);
                        }
                        Math_ScaledStepToS(&this->actor.shape.rot.y,
                                           Actor_YawBetweenActors(&this->actor, this->dekuKing), 0x71C);
                        break;

                    case EN_DNO_ANIM_IMPLORE_END:
                        if (this->skelAnime.curFrame == this->skelAnime.endFrame) {
                            func_801477B4(play);
                            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE,
                                                            &this->animIndex);
                            EnDno_KingsChamber_SetupIdle(this, play);
                        }
                        break;
                }
            }
            break;

        case TEXT_STATE_CHOICE:
        case TEXT_STATE_5:
        case TEXT_STATE_DONE:
            switch (play->msgCtx.currentTextId) {
                case 0x80B: // Calm down, your highness!
                    switch (this->animIndex) {
                        case EN_DNO_ANIM_IMPLORE_START:
                            if (this->skelAnime.curFrame == this->skelAnime.endFrame) {
                                SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IMPLORE_LOOP,
                                                                &this->animIndex);
                            }
                            break;

                        case EN_DNO_ANIM_IMPLORE_LOOP:
                            if (Message_ShouldAdvance(play)) {
                                SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IMPLORE_END,
                                                                &this->animIndex);
                                play->msgCtx.msgMode = 0x44;
                            }
                            break;

                        case EN_DNO_ANIM_IMPLORE_END:
                            if (this->skelAnime.curFrame == this->skelAnime.endFrame) {
                                func_801477B4(play);
                                SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE,
                                                                &this->animIndex);
                                EnDno_KingsChamber_SetupIdle(this, play);
                            }
                            break;
                    }
                    break;

                case 0x80C: // The king cannot calm down...
                    if (Message_ShouldAdvance(play)) {
                        func_80151938(play, 0x80D);
                    }
                    break;

                case 0x80D: // The princess may be in trouble...
                    if (Message_ShouldAdvance(play)) {
                        func_80151938(play, 0x80E);
                    }
                    break;

                case 0x80E: // The king won't send troops.
                    if (this->animIndex == EN_DNO_ANIM_TALK) {
                        SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_FAREWELL,
                                                        &this->animIndex);
                    } else if (this->animIndex == EN_DNO_ANIM_FAREWELL) {
                        if (this->skelAnime.curFrame == this->skelAnime.endFrame) {
                            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE,
                                                            &this->animIndex);
                        }
                    } else if ((this->animIndex == EN_DNO_ANIM_IDLE) && Message_ShouldAdvance(play)) {
                        func_801477B4(play);
                        EnDno_KingsChamber_SetupIdle(this, play);
                    }
                    break;

                case 0x80F: // This might be the end of the Kingdom!
                    if (Message_ShouldAdvance(play)) {
                        func_80151938(play, 0x810);
                    }
                    break;

                case 0x810: // Lords, save us!
                    if (Message_ShouldAdvance(play)) {
                        this->unk_3B0 |= 0x20;
                        func_801477B4(play);
                        EnDno_KingsChamber_SetupIdle(this, play);
                        break;
                    }

                    switch (this->animIndex) {
                        case EN_DNO_ANIM_TALK:
                            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_PRAYER_START,
                                                            &this->animIndex);
                            break;

                        case EN_DNO_ANIM_PRAYER_START:
                            if (this->skelAnime.curFrame == this->skelAnime.endFrame) {
                                SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_PRAYER_LOOP,
                                                                &this->animIndex);
                            }
                            break;
                    }
                    break;

                case 0x811: // Lords, please save us!
                    if (Message_ShouldAdvance(play)) {
                        func_801477B4(play);
                        EnDno_KingsChamber_SetupIdle(this, play);
                    }
                    break;
            }
            break;
    }
}

void EnDno_DekuShrine_SetupIdle(EnDno* this, PlayState* play) {
    this->unk_452 = 1;
    SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE_WITH_CANDLE, &this->animIndex);
    this->actor.textId = 0;
    if (Flags_GetSwitch(play, EN_DNO_GET_RACE_STARTED_SWITCH_FLAG(&this->actor))) {
        this->parasolScale = 1.0f;
    }
    this->actionFunc = EnDno_DekuShrine_Idle;
}

void EnDno_DekuShrine_Idle(EnDno* this, PlayState* play) {
    if (this->actor.xzDistToPlayer < 120.0f) {
        func_80A71424(&this->upperBodyRot, 0, this->actor.yawTowardsPlayer, this->actor.home.rot.y, 0x2000, 0x2D8);
    }

    if (Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        EnDno_DekuShrine_SetupTalk(this, play);
    } else if (this->actor.xzDistToPlayer < 60.0f) {
        func_800B8614(&this->actor, play, 60.0f);
    }
}

void EnDno_DekuShrine_GaveReward(EnDno* this, PlayState* play) {
    func_800B8500(&this->actor, play, this->actor.xzDistToPlayer, this->actor.playerHeightRel, PLAYER_AP_MINUS1);
}

void EnDno_DekuShrine_SetupGaveReward(EnDno* this, PlayState* play) {
    if (Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        EnDno_DekuShrine_SetupTalk(this, play);
    } else {
        EnDno_DekuShrine_GaveReward(this, play);
    }
}

void EnDno_DekuShrine_SetupTalk(EnDno* this, PlayState* play) {
    this->actionFunc = EnDno_DekuShrine_Talk;
}

void EnDno_DekuShrine_Talk(EnDno* this, PlayState* play) {
    s32 pad[2];

    func_80A71424(&this->upperBodyRot, 0, 0, 0, 0x2000, 0x16C);
    switch (Message_GetState(&play->msgCtx)) {
        case TEXT_STATE_NONE:
            switch (this->raceState) {
                case EN_DNO_RACE_STATE_NOT_FINISHED:
                    if (this->animIndex == EN_DNO_ANIM_IDLE_WITH_CANDLE) {
                        if (Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0x2D8)) {
                            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations,
                                                            EN_DNO_ANIM_GREETING_WITH_CANDLE, &this->animIndex);
                        }
                    } else if ((this->animIndex == EN_DNO_ANIM_GREETING_WITH_CANDLE) &&
                               Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                        if (Flags_GetSwitch(play, EN_DNO_GET_RACE_STARTED_SWITCH_FLAG(&this->actor))) {
                            // Don't get separated from me.
                            Message_StartTextbox(play, 0x801, &this->actor);
                        } else if (Player_GetMask(play) == PLAYER_MASK_SCENTS) {
                            // That looks like my son's mask.
                            Message_StartTextbox(play, 0x806, &this->actor);
                        } else {
                            // I will lead you to your reward.
                            Message_StartTextbox(play, 0x800, &this->actor);
                        }
                        SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE_WITH_CANDLE,
                                                        &this->animIndex);
                    }
                    break;

                case EN_DNO_RACE_STATE_FINISHED:
                    if (Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0x2D8)) {
                        gSaveContext.save.weekEventReg[93] |= 2;
                        // Please accept the item.
                        Message_StartTextbox(play, 0x802, &this->actor);
                    }
                    break;

                case EN_DNO_RACE_STATE_REWARD_GIVEN:
                    // I'm reminded of my son.
                    Message_StartTextbox(play, 0x804, &this->actor);
                    break;
            }
            break;

        case TEXT_STATE_1:
        case TEXT_STATE_CLOSING:
        case TEXT_STATE_3:
            if (((play->msgCtx.currentTextId == 0x800) || (play->msgCtx.currentTextId == 0x801)) &&
                (this->animIndex == EN_DNO_ANIM_OPEN_PARASOL)) {
                Math_SmoothStepToF(&this->parasolScale, 1.0f, 1.0f, 0.1f, 0.01f);
                if (this->skelAnime.curFrame <= 23.0f) {
                    this->unk_452 = 3;
                    if (Animation_OnFrame(&this->skelAnime, 23.0f)) {
                        Actor_PlaySfxAtPos(&this->actor, NA_SE_EV_OPEN_AMBRELLA);
                    }
                } else if (this->skelAnime.curFrame <= 24.0f) {
                    this->unk_452 = 4;
                } else if (this->skelAnime.curFrame >= 25.0f) {
                    this->unk_452 = 2;
                }

                if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                    SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations,
                                                    EN_DNO_ANIM_TALK_WITH_PARSOL_AND_CANDLE, &this->animIndex);
                    // Follow me.
                    Message_StartTextbox(play, 0x803, &this->actor);
                }
            }
            break;

        case TEXT_STATE_CHOICE:
        case TEXT_STATE_5:
        case TEXT_STATE_DONE:
            switch (play->msgCtx.currentTextId) {
                case 0x800: // I will lead you to your reward.
                case 0x801: // Don't get separated from me.
                    if (Message_ShouldAdvance(play)) {
                        play->msgCtx.msgMode = 0x44;
                        this->unk_452 = 1;
                        this->parasolScale = 0.0f;
                        SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_OPEN_PARASOL,
                                                        &this->animIndex);
                    }
                    break;

                case 0x802: // Please accept the item.
                    if (Message_ShouldAdvance(play)) {
                        if (INV_CONTENT(ITEM_MASK_SCENTS) == ITEM_MASK_SCENTS) {
                            this->getItemId = GI_RUPEE_RED;
                        } else {
                            this->getItemId = GI_MASK_SCENTS;
                        }
                        Actor_PickUp(&this->actor, play, this->getItemId, 60.0f, 60.0f);
                        func_801477B4(play);
                        EnDno_DekuShrine_SetupGiveReward(this, play);
                    }
                    break;

                case 0x803: // Follow me.
                    if (Message_ShouldAdvance(play)) {
                        EnDno_DekuShrine_SetupStartRace(this, play);
                    }
                    break;

                case 0x804: // I'm reminded of my son.
                    if (this->animIndex == EN_DNO_ANIM_IDLE_WITH_CANDLE) {
                        if (Message_ShouldAdvance(play)) {
                            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations,
                                                            EN_DNO_ANIM_GREETING_WITH_CANDLE, &this->animIndex);
                            if (!(this->unk_3B0 & 0x40)) {
                                EnDno_DekuShrine_SpawnBlueWarp(this, play);
                                this->unk_3B0 |= 0x40;
                            }
                        }
                    } else if ((this->animIndex == EN_DNO_ANIM_GREETING_WITH_CANDLE) &&
                               (this->skelAnime.curFrame == this->skelAnime.endFrame)) {
                        func_801477B4(play);
                        EnDno_DekuShrine_SetupIdle(this, play);
                    }
                    break;

                case 0x806: // That looks like my son's mask.
                    if (Message_ShouldAdvance(play)) {
                        func_80151938(play, 0x800);
                    }
                    break;

                default:
                    EnDno_DekuShrine_SetupIdle(this, play);
                    break;
            }
            break;
    }
}

void EnDno_DekuShrine_SetupStartRace(EnDno* this, PlayState* play) {
    SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_START_RACE_START, &this->animIndex);
    EnDno_OpenFirstSildingDoor(this, play);
    this->actionFunc = EnDno_DekuShrine_StartRace;
}

void EnDno_DekuShrine_StartRace(EnDno* this, PlayState* play) {
    if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
        this->unk_44E = 0;
        EnDno_DekuShrine_SetupRace(this, play);
    }
}

void EnDno_DekuShrine_SetupGiveReward(EnDno* this, PlayState* play) {
    this->raceState = EN_DNO_RACE_STATE_REWARD_GIVEN;
    this->actionFunc = EnDno_DekuShrine_GiveReward;
}

void EnDno_DekuShrine_GiveReward(EnDno* this, PlayState* play) {
    if (Actor_HasParent(&this->actor, play)) {
        this->actor.parent = NULL;
        this->actionFunc = EnDno_DekuShrine_SetupGaveReward;
    } else {
        Actor_PickUp(&this->actor, play, this->getItemId, 60.0f, 60.0f);
    }
}

void EnDno_DekuShrine_SetupRace(EnDno* this, PlayState* play) {
    SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_START_RACE_START, &this->animIndex);
    this->actor.flags |= ACTOR_FLAG_8000000;
    this->actor.flags &= ~(ACTOR_FLAG_1 | ACTOR_FLAG_8);
    Math_Vec3f_Copy(&this->unk_334, &this->actor.world.pos);
    SubS_ActorPathing_Init(play, &this->unk_334, &this->actor, &this->actorPath, play->setupPathList,
                           EN_DNO_GET_PATH_INDEX(&this->actor), 1, 0, 1, 0);
    SubS_ActorPathing_ComputePointInfo(play, &this->actorPath);

    this->actor.world.rot.y = this->actorPath.rotToCurPoint.y;
    this->actor.world.rot.x = this->actorPath.rotToCurPoint.x;

    Flags_SetSwitch(play, EN_DNO_GET_RACE_STARTED_SWITCH_FLAG(&this->actor));
    this->actionFunc = EnDno_DekuShrine_Race;
}

void EnDno_DekuShrine_SpawnBlueWarp(EnDno* this, PlayState* play) {
    Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 4);
    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_DOOR_WARP1, this->actor.world.pos.x + 80.0f,
                       this->actor.floorHeight, this->actor.world.pos.z, 0, 0, 0, 0x201);
}

s32 EnDno_ActorPathing_UpdateActorInfo(PlayState* play, ActorPathing* actorPath) {
    Actor* thisx = actorPath->actor;
    s32 pad;
    s32 ret = false;
    f32 sp38;
    s16 yawDiff;
    s32 temp_v0_2;
    s32 sp2C;

    thisx->gravity = 0.0f;
    yawDiff = thisx->yawTowardsPlayer - thisx->world.rot.y;
    if ((yawDiff <= 0x4000) && (yawDiff >= -0x4000)) {
        Math_SmoothStepToF(&thisx->speedXZ, 15.0f, 0.8f, 1.0f, 0.01f);
    } else {
        if (thisx->xzDistToPlayer <= 80.0f) {
            Math_SmoothStepToF(&thisx->speedXZ, 8.0f, 0.5f, 0.5f, 0.01f);
        } else if (thisx->xzDistToPlayer <= 360.0f) {
            Math_SmoothStepToF(&thisx->speedXZ, 7.0f, 0.5f, 0.5f, 0.01f);
        } else {
            Math_SmoothStepToF(&thisx->speedXZ, 3.5f, 0.5f, 0.5f, 0.01f);
        }
    }

    if (actorPath->distSqToCurPoint < SQ(thisx->speedXZ)) {
        ret = true;
    } else {
        sp38 = thisx->speedXZ / sqrtf(actorPath->distSqToCurPointXZ);
        sp2C = ABS(actorPath->rotToCurPoint.x - thisx->world.rot.x);
        temp_v0_2 = sp2C;
        temp_v0_2 *= sp38;
        temp_v0_2 += 0x71C;
        sp2C = ABS(actorPath->rotToCurPoint.y - thisx->world.rot.y);

        Math_ScaledStepToS(&thisx->world.rot.x, actorPath->rotToCurPoint.x, temp_v0_2);
        Math_ScaledStepToS(&thisx->world.rot.y, actorPath->rotToCurPoint.y, (s32)(sp2C * sp38) + 0x71C);
    }

    return ret;
}

s32 EnDno_ActorPathing_Move(PlayState* play, ActorPathing* actorPath) {
    Actor* thisx = actorPath->actor;
    EnDno* this = THIS;
    f32 sp24 = Math_CosS(-thisx->world.rot.x) * thisx->speedXZ;
    f32 sp20 = gFramerateDivisorHalf;

    thisx->velocity.x = Math_SinS(thisx->world.rot.y) * sp24;
    thisx->velocity.y = Math_SinS(-thisx->world.rot.x) * thisx->speedXZ;
    thisx->velocity.z = Math_CosS(thisx->world.rot.y) * sp24;

    this->unk_334.x += (this->actor.velocity.x * sp20) + this->actor.colChkInfo.displacement.x;
    this->unk_334.y += (this->actor.velocity.y * sp20) + this->actor.colChkInfo.displacement.y;
    this->unk_334.z += (this->actor.velocity.z * sp20) + this->actor.colChkInfo.displacement.z;

    return false;
}

void EnDno_DekuShrine_Race(EnDno* this, PlayState* play) {
    s32 nextAnimIndex;

    if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
        nextAnimIndex = -1;
        switch (this->animIndex) {
            case EN_DNO_ANIM_START_RACE_START:
                if (this->unk_44E >= 20) {
                    nextAnimIndex = EN_DNO_ANIM_START_RACE_END;
                } else {
                    this->unk_44E = 20;
                }
                break;

            case EN_DNO_ANIM_START_RACE_END:
                nextAnimIndex = EN_DNO_ANIM_FLY;
                break;
        }

        if (nextAnimIndex >= 0) {
            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, nextAnimIndex, &this->animIndex);
        }
    }

    SubS_ActorPathing_Update(play, &this->actorPath, SubS_ActorPathing_ComputePointInfo,
                             EnDno_ActorPathing_UpdateActorInfo, EnDno_ActorPathing_Move,
                             SubS_ActorPathing_SetNextPoint);

    this->parasolRot += 0x1999;
    this->actorPath.pointOffset.x = 0.0f;
    this->actorPath.pointOffset.y = 0.0f;
    this->actorPath.pointOffset.z = 0.0f;
    Math_Vec3f_Copy(&this->actor.world.pos, &this->unk_334);
    this->actor.world.pos.y += Math_SinS(this->bobPhase1) * (4.0f + Math_SinS(this->bobPhase2));
    this->bobPhase1 += 0x1194;
    this->bobPhase2 += 0x3E8;
    this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
    EnDno_CheckIfButlerIsBeyondDoor(this, play);
    func_800B9010(&this->actor, NA_SE_EV_BUTLER_FRY - SFX_FLAG);
    if (this->actorPath.flags & ACTOR_PATHING_REACHED_END_PERMANENT) {
        Math_Vec3f_Copy(&this->actor.world.pos, &this->actorPath.curPoint);
        this->actor.speedXZ = 0.0f;
        this->actor.velocity.x = 0.0f;
        this->actor.velocity.y = 0.0f;
        this->actor.velocity.z = 0.0f;
        EnDno_DekuShrine_SetupEndRace(this, play);
    }
}

void EnDno_DekuShrine_SetupEndRace(EnDno* this, PlayState* play) {
    this->actor.flags &= ~ACTOR_FLAG_8000000;
    this->actor.flags |= (ACTOR_FLAG_1 | ACTOR_FLAG_8);
    this->raceState = EN_DNO_RACE_STATE_FINISHED;
    this->actor.speedXZ = 0.0f;
    Flags_UnsetSwitch(play, EN_DNO_GET_RACE_STARTED_SWITCH_FLAG(&this->actor));
    gSaveContext.timerStates[TIMER_ID_MINIGAME_1] = TIMER_STATE_STOP;
    this->unk_44E = 0;
    this->actionFunc = EnDno_DekuShrine_EndRace;
}

void EnDno_DekuShrine_EndRace(EnDno* this, PlayState* play) {
    s32 pad;

    if (this->unk_44E == 0) {
        if (Math_ScaledStepToS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 0x71C)) {
            this->unk_3B0 |= 4;
            this->unk_44E = 3;
            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_CLOSE_PARASOL, &this->animIndex);
        }
    } else if (this->unk_44E == 3) {
        if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
            SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_IDLE_WITH_CANDLE,
                                            &this->animIndex);
            EnDno_DekuShrine_SetupIdle(this, play);
        } else {
            if (this->skelAnime.curFrame >= 20.0f) {
                Math_SmoothStepToF(&this->parasolScale, 0.0f, 1.0f, 0.125f, 0.01f);
            }

            if (Animation_OnFrame(&this->skelAnime, 4.0f)) {
                Actor_PlaySfxAtPos(&this->actor, NA_SE_EV_CLOSE_AMBRELLA);
                this->unk_452 = 4;
            } else if (Animation_OnFrame(&this->skelAnime, 5.0f)) {
                this->unk_452 = 3;
            }
        }
    }
    Actor_MoveWithGravity(&this->actor);
}

void EnDno_PerformCutsceneActions(EnDno* this, PlayState* play) {
    s32 nextAnimIndex;
    u8 sp33 = true;
    s32 actionIndex;

    if (Cutscene_CheckActorAction(play, 475)) {
        actionIndex = Cutscene_GetActorActionIndex(play, 475);
        if (this->csAction != play->csCtx.actorActions[actionIndex]->action) {
            switch (play->csCtx.actorActions[actionIndex]->action) {
                case 1:
                    nextAnimIndex = EN_DNO_ANIM_IDLE;
                    break;

                case 2:
                    nextAnimIndex = EN_DNO_ANIM_SHOCK_START;
                    break;

                default:
                    nextAnimIndex = EN_DNO_ANIM_START_RACE_START;
                    sp33 = false;
                    break;
            }

            if (sp33) {
                SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, nextAnimIndex, &this->animIndex);
            }
        }
        Cutscene_ActorTranslateAndYaw(&this->actor, play, actionIndex);
    }

    if ((Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) &&
        (this->animIndex == EN_DNO_ANIM_SHOCK_START)) {
        if (0) {};
        SubS_ChangeAnimationBySpeedInfo(&this->skelAnime, sAnimations, EN_DNO_ANIM_SHOCK_LOOP, &this->animIndex);
    }
}

void EnDno_Update(Actor* thisx, PlayState* play) {
    EnDno* this = THIS;
    s32 pad;

    SkelAnime_Update(&this->skelAnime);
    EnDno_PerformCutsceneActions(this, play);
    this->actionFunc(this, play);
    if (this->unk_3B0 & 4) {
        Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 0.0f, 0.0f, 4);
    }
    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    Actor_SetFocus(&this->actor, 51.0f);
}

void EnDno_Draw(Actor* thisx, PlayState* play) {
    EnDno* this = THIS;

    SkelAnime_DrawFlexOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount,
                          EnDno_OverrideLimbDraw, EnDno_PostLimbDraw, &this->actor);
}

s32 EnDno_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    EnDno* this = THIS;

    *dList = NULL;

    // The eye limb is actually the root limb of the entire upper body (specifically, everything above the pelvis).
    if (limbIndex == DEKU_BUTLER_LIMB_EYES) {
        rot->x += this->upperBodyRot;
    }

    return false;
}

void EnDno_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    static Vec3f sCandleLightOffset = { 0.0f, 0.0f, 0.0f };
    Gfx* gfxOpa;
    Gfx* gfxXlu;
    Vec3f candleLightPos;
    EnDno* this = THIS;
    s32 pad;
    s32 shouldDrawLimb = false;

    if (*dList != NULL) {
        switch (this->unk_452) {
            case 0:
                if ((limbIndex != DEKU_BUTLER_LIMB_CANDLE) && (limbIndex != DEKU_BUTLER_LIMB_CANDLE_WICK) &&
                    (limbIndex != DEKU_BUTLER_LIMB_PARASOL_HANDLE) &&
                    (limbIndex != DEKU_BUTLER_LIMB_PARASOL_LOWER_PETALS) &&
                    (limbIndex != DEKU_BUTLER_LIMB_PARASOL_MIDDLE_PETALS) &&
                    (limbIndex != DEKU_BUTLER_LIMB_PARASOL_UPPER_PETALS)) {
                    shouldDrawLimb = true;
                }
                break;

            case 1:
                if ((limbIndex != DEKU_BUTLER_LIMB_CANDLE_WICK) && (limbIndex != DEKU_BUTLER_LIMB_PARASOL_HANDLE) &&
                    (limbIndex != DEKU_BUTLER_LIMB_PARASOL_LOWER_PETALS) &&
                    (limbIndex != DEKU_BUTLER_LIMB_PARASOL_MIDDLE_PETALS) &&
                    (limbIndex != DEKU_BUTLER_LIMB_PARASOL_UPPER_PETALS)) {
                    shouldDrawLimb = true;
                }
                break;

            case 2:
                if ((limbIndex != DEKU_BUTLER_LIMB_PARASOL_LOWER_PETALS) &&
                    (limbIndex != DEKU_BUTLER_LIMB_PARASOL_MIDDLE_PETALS)) {
                    shouldDrawLimb = true;
                }
                break;

            case 3:
                if ((limbIndex != DEKU_BUTLER_LIMB_PARASOL_MIDDLE_PETALS) &&
                    (limbIndex != DEKU_BUTLER_LIMB_PARASOL_UPPER_PETALS)) {
                    shouldDrawLimb = true;
                }
                break;

            case 4:
                if ((limbIndex != DEKU_BUTLER_LIMB_PARASOL_LOWER_PETALS) &&
                    (limbIndex != DEKU_BUTLER_LIMB_PARASOL_UPPER_PETALS)) {
                    shouldDrawLimb = true;
                }
                break;
        }
    }

    if (shouldDrawLimb == true) {
        OPEN_DISPS(play->state.gfxCtx);

        func_8012C28C(play->state.gfxCtx);
        if (limbIndex == DEKU_BUTLER_LIMB_PARASOL_HANDLE) {
            Matrix_Scale(this->parasolScale, this->parasolScale, this->parasolScale, MTXMODE_APPLY);
            Matrix_RotateXS(this->parasolRot, MTXMODE_APPLY);
        }

        gfxOpa = POLY_OPA_DISP;
        gSPMatrix(gfxOpa, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(&gfxOpa[1], *dList);

        POLY_OPA_DISP = &gfxOpa[2];
        CLOSE_DISPS(play->state.gfxCtx);
    }

    if ((this->unk_3B0 & 1) && (limbIndex == DEKU_BUTLER_LIMB_CANDLE_WICK)) {
        u32 frames;

        OPEN_DISPS(play->state.gfxCtx);

        Matrix_Push();
        frames = play->gameplayFrames;
        Matrix_MultVec3f(&sCandleLightOffset, &candleLightPos);
        EnDno_UpdateCandleLight(this, play, &candleLightPos);
        Matrix_ReplaceRotation(&play->billboardMtxF);
        Matrix_Scale(0.15f, 0.15f, 1.0f, MTXMODE_APPLY);
        Matrix_Translate(0.0f, -3200.0f, 0.0f, MTXMODE_APPLY);
        gfxXlu = func_8012C2B4(POLY_XLU_DISP);

        gSPMatrix(gfxXlu, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPSegment(&gfxXlu[1], 0x08,
                   Gfx_TwoTexScroll(play->state.gfxCtx, 0, 0, 0, 0x20, 0x40, 1, 0, -frames * 20, 0x20, 0x80));
        gDPSetPrimColor(&gfxXlu[2], 0x80, 0x80, 255, 255, 0, 255);
        gDPSetEnvColor(&gfxXlu[3], 255, 0, 0, 0);
        gSPDisplayList(&gfxXlu[4], gEffFire1DL);

        POLY_XLU_DISP = &gfxXlu[5];

        Matrix_Pop();

        CLOSE_DISPS(play->state.gfxCtx);
    }
}
