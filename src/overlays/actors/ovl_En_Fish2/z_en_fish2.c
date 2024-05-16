/*
 * File: z_en_fish2.c
 * Overlay: ovl_En_Fish2
 * Description: Marine Research Lab Fish
 */

#include "z_en_fish2.h"
#include "attributes.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "overlays/actors/ovl_En_Col_Man/z_en_col_man.h"
#include "overlays/actors/ovl_En_Fish/z_en_fish.h"
#include "overlays/actors/ovl_En_Mushi2/z_en_mushi2.h"

#define FLAGS (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_10)

#define THIS ((EnFish2*)thisx)

void EnFish2_Init(Actor* thisx, PlayState* play);
void EnFish2_Destroy(Actor* thisx, PlayState* play);
void EnFish2_Update(Actor* thisx, PlayState* play2);
void EnFish2_Draw(Actor* thisx, PlayState* play);

void EnFish2_SetupSwim(EnFish2* this);
void EnFish2_Swim(EnFish2* this, PlayState* play);
void EnFish2_SetupChecked(EnFish2* this);
void EnFish2_Checked(EnFish2* this, PlayState* play);
void EnFish2_SetupChaseDroppedPrey(EnFish2* this);
void EnFish2_ChaseDroppedPrey(EnFish2* this, PlayState* play);
void EnFish2_SetupBite(EnFish2* this);
void EnFish2_Bite(EnFish2* this, PlayState* play);
void EnFish2_SetupSwallow(EnFish2* this);
void EnFish2_Swallow(EnFish2* this, PlayState* play);
void EnFish2_SetupScaleUp(EnFish2* this);
void EnFish2_ScaleUp(EnFish2* this, PlayState* play);
void EnFish2_SetupChaseOtherLabFish(EnFish2* this, PlayState* play);
void EnFish2_ChaseOtherLabFish(EnFish2* this, PlayState* play);
void EnFish2_CutsceneHandler_80B2A01C(EnFish2* this, PlayState* play);
void EnFish2_CutsceneHandler_80B2A094(EnFish2* this, PlayState* play);
void EnFish2_CutsceneHandler_80B2A23C(EnFish2* this, PlayState* play);
void EnFish2_SetupSpitUpReward(EnFish2* this);
void EnFish2_SpitUpReward(EnFish2* this, PlayState* play);
void EnFish2_AddEffect(EnFish2* this, Vec3f* pos, s16 timer);
void EnFish2_UpdateEffects(EnFish2* this, PlayState* play);
void EnFish2_DrawEffects(EnFish2* this, PlayState* play);

static s32 D_80B2B2E0 = 0;
static s32 D_80B2B2E4 = 0;
static s32 sIsOneLabFishChasingTheOther = false;
static s32 D_80B2B2EC = 0;
static s32 sCurrentIndex = 0;
static Actor* D_80B2B2F4 = NULL;

ActorProfile En_Fish2_Profile = {
    /**/ ACTOR_EN_FISH2,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_FB,
    /**/ sizeof(EnFish2),
    /**/ EnFish2_Init,
    /**/ EnFish2_Destroy,
    /**/ EnFish2_Update,
    /**/ EnFish2_Draw,
};

static ColliderJntSphElementInit sJntSphElementsInit[2] = {
    {
        {
            ELEM_MATERIAL_UNK2,
            { 0xF7CFFFFF, 0x00, 0x00 },
            { 0xF7CFFFFF, 0x00, 0x00 },
            ATELEM_NONE | ATELEM_SFX_NORMAL,
            ACELEM_NONE,
            OCELEM_ON,
        },
        { RESEARCH_LAB_FISH_LIMB_ROOT, { { 0, 0, 0 }, 0 }, 1 },
    },
    {
        {
            ELEM_MATERIAL_UNK2,
            { 0xF7CFFFFF, 0x00, 0x00 },
            { 0xF7CFFFFF, 0x00, 0x00 },
            ATELEM_NONE | ATELEM_SFX_NORMAL,
            ACELEM_NONE,
            OCELEM_ON,
        },
        { RESEARCH_LAB_FISH_LIMB_HEAD, { { 0, 0, 0 }, 0 }, 1 },
    },
};

static ColliderJntSphInit sJntSphInit = {
    {
        COL_MATERIAL_HARD,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_JNTSPH,
    },
    ARRAY_COUNT(sJntSphElementsInit),
    sJntSphElementsInit,
};

static f32 sScales[] = { 0.01f, 0.012f, 0.014f, 0.017f, 0.019f, 0.033f };

typedef enum {
    /* 0 */ FISH2_ANIM_SWIM,
    /* 1 */ FISH2_ANIM_FAST_SWIM,
    /* 2 */ FISH2_ANIM_WIND_UP, // unused
    /* 3 */ FISH2_ANIM_CHOMP,   // unused
    /* 4 */ FISH2_ANIM_SPIT_UP,
    /* 5 */ FISH2_ANIM_BITE,
    /* 6 */ FISH2_ANIM_MAX
} Fish2Animation;

static AnimationHeader* sAnimations[FISH2_ANIM_MAX] = {
    &gResearchLabFishSwimAnim,           // FISH2_ANIM_SWIM
    &gResearchLabFishFastSwimAnim,       // FISH2_ANIM_FAST_SWIM
    &gResearchLabFishWindUpAndChompAnim, // FISH2_ANIM_WIND_UP
    &gResearchLabFishWindUpAndChompAnim, // FISH2_ANIM_CHOMP
    &gResearchLabFishSpitUpAnim,         // FISH2_ANIM_SPIT_UP
    &gResearchLabFishBiteAnim,           // FISH2_ANIM_BITE
};

static u8 sAnimationModes[FISH2_ANIM_MAX] = {
    ANIMMODE_LOOP, // FISH2_ANIM_SWIM
    ANIMMODE_LOOP, // FISH2_ANIM_FAST_SWIM
    ANIMMODE_ONCE, // FISH2_ANIM_WIND_UP
    ANIMMODE_ONCE, // FISH2_ANIM_CHOMP
    ANIMMODE_ONCE, // FISH2_ANIM_SPIT_UP
    ANIMMODE_ONCE, // FISH2_ANIM_BITE
};

void EnFish2_ChangeAnim(EnFish2* this, s32 animIndex) {
    f32 startFrame;

    this->animIndex = animIndex;
    this->animEndFrame = Animation_GetLastFrame(&sAnimations[animIndex]->common);

    startFrame = 0.0f;
    if (this->animIndex == FISH2_ANIM_CHOMP) {
        startFrame = Animation_GetLastFrame(&sAnimations[this->animIndex]->common) - 21.0f;
    }

    if (this->animIndex == FISH2_ANIM_WIND_UP) {
        this->animEndFrame = Animation_GetLastFrame(&sAnimations[this->animIndex]->common) - 21.0f;
    }

    Animation_Change(&this->skelAnime, sAnimations[this->animIndex], 1.0f, startFrame, this->animEndFrame,
                     sAnimationModes[this->animIndex], -10.0f);
}

s32 func_80B28478(EnFish2* this) {
    if (!this->wasFedBugs) {
        if ((D_80B2B2E4 != 0) &&
            ((D_80B2B2E0 != 1) || (this->targetActor == NULL) || (this->targetActor->update == NULL)) &&
            !this->isChasingOtherLabFish) {
            this->waitTimer = 0;
            this->frameCounter = 0;
            this->timer = this->waitTimer;
            EnFish2_SetupSwim(this);
            return true;
        }
    } else if ((this->targetActor == NULL) || (this->targetActor->update == NULL)) {
        this->waitTimer = 0;
        this->frameCounter = 0;
        this->timer = this->waitTimer;
        EnFish2_SetupSwim(this);
        return true;
    }

    return false;
}

void EnFish2_Init(Actor* thisx, PlayState* play) {
    EnFish2* this = THIS;
    s32 i;
    s32 csId;

    Math_Vec3f_Copy(&this->targetPos, &this->actor.home.pos);
    this->index = sCurrentIndex;
    sCurrentIndex++;

    if (ENFISH2_GET_TYPE(&this->actor) == ENFISH2_TYPE_FISH) {
        ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 20.0f);
        SkelAnime_InitFlex(play, &this->skelAnime, &gResearchLabFishSkel, &gResearchLabFishSwimAnim, this->jointTable,
                           this->morphTable, RESEARCH_LAB_FISH_LIMB_MAX);
        this->actor.colChkInfo.mass = MASS_IMMOVABLE;

        if (this->index == 0) {
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_81_10)) {
                this->scaleIndex = 1;
            }

            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_81_20)) {
                this->scaleIndex = 2;
            }

            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_81_40)) {
                this->scaleIndex = 3;
            }
        } else {
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_81_80)) {
                this->scaleIndex = 1;
            }

            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_82_01)) {
                this->scaleIndex = 2;
            }

            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_82_02)) {
                this->scaleIndex = 3;
            }
        }

        csId = this->actor.csId;
        this->scale = sScales[this->scaleIndex];
        i = 0;

        // clang-format off
        while (csId != CS_ID_NONE) { this->csIdList[i] = csId; csId = CutsceneManager_GetAdditionalCsId(csId); i++; }
        // clang-format on

        Collider_InitAndSetJntSph(play, &this->collider, &this->actor, &sJntSphInit, this->colliderElements);
        this->collider.elements[0].dim.modelSphere.radius = 5;
        this->collider.elements[0].dim.scale = 1.0f;
        this->collider.elements[0].dim.modelSphere.center.x = 1300;
        this->collider.elements[0].dim.modelSphere.center.y = 0;
        this->collider.elements[0].dim.modelSphere.center.z = 0;
        this->collider.elements[1].dim.modelSphere.radius = 5;
        this->collider.elements[1].dim.scale = 1.0f;
        this->collider.elements[1].dim.modelSphere.center.x = 500;
        this->collider.elements[1].dim.modelSphere.center.y = 0;
        this->collider.elements[1].dim.modelSphere.center.z = 0;
        this->actor.textId = 0x24C;
        EnFish2_SetupSwim(this);
    } else if (ENFISH2_GET_TYPE(&this->actor) != ENFISH2_TYPE_FISH) {
        this->waitTimer = 10;
        this->actor.draw = NULL;
        this->actor.flags |= ACTOR_FLAG_LOCK_ON_DISABLED;
        this->actionFunc = EnFish2_CutsceneHandler_80B2A01C;
    }
}

void EnFish2_Destroy(Actor* thisx, PlayState* play) {
    EnFish2* this = THIS;

    if (ENFISH2_GET_TYPE(&this->actor) != ENFISH2_TYPE_CUTSCENE_HANDLER) {
        Collider_DestroyJntSph(play, &this->collider);
    }
}

void EnFish2_UpdateTargetActorSpeed(EnFish2* this, s32 arg1) {
    Vec3f targetActorPos;

    if ((this->cutsceneHandler != NULL) && (!this->wasFedBugs)) {
        if (this->frameCounter < 400) {
            this->frameCounter++;
        }

        this->targetActorSpeedMultiplier = 440.0f - this->frameCounter;

        if (!arg1) {
            this->targetActorSpeedMultiplier = 410.0f - this->frameCounter;
        }

        Math_ApproachF(&this->targetActor->speed, (sScales[4] - this->scale) * this->targetActorSpeedMultiplier, 0.1f,
                       0.4f);
    }

    Math_Vec3f_Copy(&targetActorPos, &this->targetActor->world.pos);
    this->targetRotY = Math_Vec3f_Yaw(&this->actor.world.pos, &targetActorPos);
    this->targetRotX = Math_Vec3f_Pitch(&this->actor.world.pos, &targetActorPos);
}

/**
 * Returns true if the supplied `pos` is considered "close enough" to the fish's target actor. The target is considered
 * "close enough" if the distance between `pos` and the target actor is smaller than a certain threshold. If `useScale`
 * is true, then the value of that threshold will be determined by the scale of the fish. Otherwise, the threshold will
 * be 40; the target actor will be considered "close enough" if it is fewer than 40 units away from `pos`.
 */
s32 EnFish2_IsCloseEnoughToTargetActor(EnFish2* this, Vec3f pos, s32 useScale) {
    f32 diffX = this->targetActor->world.pos.x - pos.x;
    f32 diffY = this->targetActor->world.pos.y - pos.y;
    f32 diffZ = this->targetActor->world.pos.z - pos.z;
    f32 dist = sqrtf(SQ(diffX) + SQ(diffY) + SQ(diffZ));
    f32 distThreshold;

    if (!useScale) {
        distThreshold = 40.0f;
    } else {
        distThreshold = this->scale * 2000.0f;
        if (distThreshold > 20.0f) {
            distThreshold = 20.0f;
        }
    }

    if (dist < distThreshold) {
        return true;
    }

    return false;
}

/**
 * Returns true if the fish is touching a wall, false otherwise.
 */
s32 EnFish2_IsTouchingWall(EnFish2* this, PlayState* play) {
    if (BgCheck_SphVsFirstWall(&play->colCtx, &this->wallCheckPos, this->wallCheckRadius)) {
        return true;
    }

    return false;
}

void EnFish2_UpdateVelocityY(EnFish2* this, PlayState* play) {
    WaterBox* waterBox;

    if (this->waitTimer != 0) {
        this->targetRotX = 0;
    }

    if (this->targetRotX != 0) {
        if (this->targetRotX > 0) {
            if (this->minDistFromFloor < this->actor.world.pos.y) {
                this->actor.velocity.y = this->actor.world.rot.x * 0.001f * -0.1f;

                if (this->actionFunc == EnFish2_ScaleUp) {
                    this->actor.velocity.y *= 2.0f;
                }
            } else {
                this->targetRotX = 0;
                this->actor.velocity.y = 0.0f;
                this->actor.gravity = 0.0f;
            }
        } else if (WaterBox_GetSurface1(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                                        &this->waterSurface, &waterBox)) {
            if ((this->waterSurface != BGCHECK_Y_MIN) &&
                (this->actor.world.pos.y < (this->waterSurface - this->minDistFromWaterSurface))) {
                this->actor.velocity.y = this->actor.world.rot.x * 0.001f * -0.1f;

                if (this->actionFunc == EnFish2_ScaleUp) {
                    this->actor.velocity.y *= 2.0f;
                }
            }
        } else {
            this->targetRotX = 0;
            this->actor.velocity.y = 0.0f;
            this->actor.gravity = 0.0f;
        }
    }
}

void EnFish2_SetupSwim(EnFish2* this) {
    EnFish2_ChangeAnim(this, FISH2_ANIM_SWIM);
    this->waitTimer = 0;
    this->targetRotX = 0;
    this->frameCounter = 0;
    this->wasFedBugs = false;
    this->waterSurface = BGCHECK_Y_MIN;
    this->timer = this->waitTimer;
    this->angularVelocityModX = this->targetRotX;
    this->targetRotY = this->actor.world.rot.y;

    this->collider.elements[1].dim.modelSphere.radius = (s32)((this->scale - 0.01f) * 1000.0f) + 5;
    if (this->collider.elements[1].dim.modelSphere.radius > 15) {
        this->collider.elements[1].dim.modelSphere.radius = 15;
    }

    this->actionFunc = EnFish2_Swim;
}

void EnFish2_Swim(EnFish2* this, PlayState* play) {
    Actor* itemAction = play->actorCtx.actorLists[ACTORCAT_ITEMACTION].first;
    WaterBox* waterbox;

    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        EnFish2_SetupChecked(this);
        return;
    }

    if ((this->wallCheckTimer == 0) && (fabsf(this->actor.world.rot.y - this->targetRotY) < 100.0f)) {
        if (EnFish2_IsTouchingWall(this, play)) {
            if (this->direction == 0) {
                this->targetRotY += 0x4000;
            } else {
                this->targetRotY -= 0x4000;
            }

            this->wallCheckTimer = (s32)(this->scale * 1000.0f) - 10;
        }
    }

    EnFish2_UpdateVelocityY(this, play);

    if (this->timer == 0) {
        if (this->waitTimer == 0) {
            this->waitTimer = (s32)Rand_ZeroFloat(20.0f) + 10;
        } else if (this->waitTimer == 1) {
            if ((this->targetRotX == 0) || (Rand_ZeroOne() < 0.6f)) {
                this->targetRotX = Rand_CenteredFloat(0x2000);
            } else {
                this->targetRotX = Math_Vec3f_Pitch(&this->actor.world.pos, &this->targetPos);
                if (this->targetRotX < -0x1000) {
                    this->targetRotX = -0x1000;
                }

                if (this->targetRotX > 0x1000) {
                    this->targetRotX = 0x1000;
                }
            }

            this->timer = (s32)Rand_ZeroFloat(70.0f) + 30;
            if (this->scaleIndex >= 3) {
                this->timer -= (s32)Rand_ZeroFloat(20.0f);
            }
        }
    }

    if ((this->waterSurface == BGCHECK_Y_MIN) &&
        !WaterBox_GetSurface1(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                              &this->waterSurface, &waterbox)) {
        this->waterSurface = this->actor.world.pos.y;
    }

    if (this->waitTimer == 0) {
        Math_ApproachF(&this->actor.speed, (sScales[4] - this->scale) * 400.0f, 0.3f, 0.3f);
        if (this->actor.speed > 3.0f) {
            this->actor.speed = 3.0f;
        } else if (this->actor.speed < 1.5f) {
            this->actor.speed = 1.5f;
        }
    } else {
        Math_ApproachZeroF(&this->actor.speed, 0.3f, 0.3f);
    }

    if (!sIsOneLabFishChasingTheOther && (D_80B2B2E0 != 2)) {
        while (itemAction != NULL) {
            if ((itemAction->id != ACTOR_EN_FISH) && (itemAction->id != ACTOR_EN_MUSHI2)) {
                itemAction = itemAction->next;
                continue;
            }

            if (itemAction->id == ACTOR_EN_MUSHI2) {
                if ((itemAction->update != NULL) && (((EnMushi2*)itemAction)->unk_30C & 0x200)) {
                    this->targetActor = itemAction;
                    this->wasFedBugs = true;
                    EnFish2_SetupChaseDroppedPrey(this);
                    break;
                }
            } else if ((itemAction->update != NULL) && (itemAction->params == 0) &&
                       (fabsf(itemAction->world.pos.x - this->actor.world.pos.x) < 100.0f) &&
                       (fabsf(itemAction->world.pos.z - this->actor.world.pos.z) < 100.0f) &&
                       (itemAction->bgCheckFlags & BGCHECKFLAG_WATER)) {
                this->targetActor = itemAction;

                if (D_80B2B2E0 == 0) {
                    EnFish2* cutsceneHandler;

                    this->cutsceneHandler = NULL;
                    cutsceneHandler = (EnFish2*)Actor_SpawnAsChild(
                        &play->actorCtx, &this->actor, play, ACTOR_EN_FISH2, this->targetPos.x, this->targetPos.y,
                        this->targetPos.z, 0, 0, 0, ENFISH2_TYPE_CUTSCENE_HANDLER);
                    this->cutsceneHandler = cutsceneHandler;

                    if (this->cutsceneHandler != NULL) {
                        D_80B2B2E4 = 0;
                        D_80B2B2E0 = 1;
                        cutsceneHandler->csIdList[0] = this->csIdList[0];
                        cutsceneHandler->targetActor = this->targetActor;
                    }
                }

                EnFish2_SetupChaseDroppedPrey(this);
                break;
            }

            itemAction = itemAction->next;
        }

        Actor_OfferTalk(&this->actor, play, 100.0f);
    }
}

void EnFish2_SetupChecked(EnFish2* this) {
    this->actionFunc = EnFish2_Checked;
}

void EnFish2_Checked(EnFish2* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);
        EnFish2_SetupSwim(this);
    }
}

void EnFish2_SetupChaseDroppedPrey(EnFish2* this) {
    if ((this->cutsceneHandler != NULL) && !this->wasFedBugs) {
        EnFish* fish = (EnFish*)this->targetActor;
        fish->unk_277 = 1;
    }

    this->frameCounter = 0;
    this->actor.speed = 0.0f;

    if (!this->wasFedBugs) {
        this->angularVelocityModX = 0x190;
        Math_Vec3f_Copy(&this->actor.world.pos, &this->targetPos);
        this->actor.world.pos.y = this->actor.floorHeight + (this->scale * 1200.0f);
    }

    this->targetRotY = 0;
    this->direction = (s32)Rand_ZeroOne() & 1;
    EnFish2_ChangeAnim(this, FISH2_ANIM_FAST_SWIM);
    this->actionFunc = EnFish2_ChaseDroppedPrey;
}

/**
 * Chases after the bugs or fish that the player dropped into the tank. Once it gets close enough to its prey, the fish
 * will attempt to bite it.
 */
void EnFish2_ChaseDroppedPrey(EnFish2* this, PlayState* play) {
    if (!func_80B28478(this)) {
        Math_ApproachF(&this->actor.speed, (sScales[4] - this->scale) * 1000.0f, 0.3f, 0.3f);

        if (this->actor.speed > 4.0f) {
            this->actor.speed = 4.0f;
        } else if (this->actor.speed < 2.0f) {
            this->actor.speed = 2.0f;
        }

        EnFish2_UpdateTargetActorSpeed(this, false);
        EnFish2_UpdateVelocityY(this, play);

        if (EnFish2_IsCloseEnoughToTargetActor(this, this->headPos, false) &&
            ((!this->wasFedBugs && (D_80B2B2E4 == 1)) || (this->wasFedBugs))) {
            Math_Vec3f_Copy(&this->targetActorPos, &this->targetActor->world.pos);
            EnFish2_SetupBite(this);
        }
    }
}

void EnFish2_SetupBite(EnFish2* this) {
    EnFish2_ChangeAnim(this, FISH2_ANIM_BITE);
    this->targetRotX = 0;
    this->actionFunc = EnFish2_Bite;
}

/**
 * Attempts to eat its prey by biting at it. If the fish's lower jaw gets close enough to its target during the bite
 * animation, then the fish will transition to a swallowing action where its prey's instance is killed. If the fish's
 * lower jaw does *not* get close enough to the target, then the fish will resume its chase.
 */
void EnFish2_Bite(EnFish2* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;

    if (!func_80B28478(this)) {
        EnFish2_UpdateTargetActorSpeed(this, true);
        Math_ApproachF(&this->actor.speed, (sScales[4] - this->scale) * 1000.0f, 0.3f, 0.3f);

        if (this->actor.speed > 3.0f) {
            this->actor.speed = 3.0f;
        } else if (this->actor.speed < 1.0f) {
            this->actor.speed = 1.0f;
        }

        if (curFrame >= this->animEndFrame) {
            EnFish2_ChangeAnim(this, FISH2_ANIM_FAST_SWIM);

            if (!this->isChasingOtherLabFish) {
                this->actionFunc = EnFish2_ChaseDroppedPrey;
            } else {
                this->actionFunc = EnFish2_ChaseOtherLabFish;
            }
        } else {
            EnFish2_UpdateVelocityY(this, play);

            if (EnFish2_IsCloseEnoughToTargetActor(this, this->lowerJawPos, true)) {
                EnFish2_SetupSwallow(this);
            }
        }
    }
}

void EnFish2_SetupSwallow(EnFish2* this) {
    this->actor.velocity.y = 0.0f;
    this->actor.gravity = 0.0f;
    this->targetRotX = 0;

    if (!this->wasFedBugs) {
        D_80B2B2E0 = 2;
    }

    this->waitTimer = 10;
    this->actor.speed = 3.0f;
    Actor_Kill(this->targetActor);
    this->targetActor = NULL;
    D_80B2B2F4 = &this->actor;
    Actor_PlaySfx(&this->actor, NA_SE_EN_DODO_M_EAT);
    this->actionFunc = EnFish2_Swallow;
}

void EnFish2_Swallow(EnFish2* this, PlayState* play) {
    s32 i;
    f32 curFrame = this->skelAnime.curFrame;
    s32 pad;
    Vec3f sp60;

    SkelAnime_Update(&this->skelAnime);
    Math_SmoothStepToS(&this->actor.world.rot.y, Math_Vec3f_Yaw(&this->actor.world.pos, &play->view.eye), 1, 0x1388, 0);
    Math_ApproachZeroF(&this->actor.speed, 0.3f, 0.3f);

    if (this->waitTimer != 0) {
        Math_Vec3f_Copy(&sp60, &this->lowerJawPos);
        sp60.x += Rand_CenteredFloat(100.0f);
        sp60.z += Rand_CenteredFloat(100.0f);

        for (i = 0; i < 2; i++) {
            EffectSsBubble_Spawn(play, &sp60, 0.0f, 5.0f, 5.0f, Rand_ZeroFloat(this->scale * 4.0f) + 0.1f);
        }
    } else if (curFrame >= this->animEndFrame) {
        if (this->animIndex != FISH2_ANIM_BITE) {
            EnFish2_ChangeAnim(this, FISH2_ANIM_BITE);
        } else if (!this->wasFedBugs) {
            EnFish2_SetupScaleUp(this);
        } else {
            this->waitTimer = 0;
            this->frameCounter = 0;
            this->timer = this->waitTimer;
            EnFish2_SetupSwim(this);
        }
    }
}

void EnFish2_SetupScaleUp(EnFish2* this) {
    EnFish2_ChangeAnim(this, FISH2_ANIM_SWIM);
    this->waitTimer = 0;
    this->scaleUpState = 0;
    this->timer = this->waitTimer;

    if (this->isChasingOtherLabFish) {
        this->targetPos.x = this->actor.home.pos.x - 14.0f;
        this->targetPos.z = this->actor.home.pos.z - 18.0f;
    }

    this->actionFunc = EnFish2_ScaleUp;
    this->targetPos.y = this->minDistFromFloor;
    this->actor.speed = 0.0f;
}

void EnFish2_ScaleUp(EnFish2* this, PlayState* play) {
    f32 scaleMultiplier = 0.0f;
    Vec3f sp48;

    switch (this->scaleUpState) {
        case 0:
            Math_Vec3f_Copy(&sp48, &this->targetPos);
            sp48.y = this->minDistFromFloor;
            this->targetRotX = Math_Vec3f_Pitch(&this->actor.world.pos, &sp48);

            if (this->targetRotX < -0x2000) {
                this->targetRotX = -0x2000;
            }

            if (this->targetRotX > 0x2000) {
                this->targetRotX = 0x2000;
            }

            this->targetRotY = Math_Vec3f_Yaw(&this->actor.world.pos, &this->targetPos);
            EnFish2_UpdateVelocityY(this, play);
            Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.3f, 3.0f);
            Math_ApproachF(&this->actor.world.pos.y, this->minDistFromFloor, 0.3f, 3.0f);
            Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.3f, 3.0f);

            if ((fabsf(this->actor.world.pos.x - this->targetPos.x) < 2.0f) &&
                (this->actor.world.pos.y < (this->minDistFromFloor + 3.0f)) &&
                (fabsf(this->actor.world.pos.z - this->targetPos.z) < 2.0f)) {
                this->actor.speed = 0.0f;
                this->targetRotY = BINANG_ROT180(this->actor.home.rot.y);
                this->scaleUpState++;
                this->actor.velocity.y = 0.0f;
                this->actor.gravity = 0.0f;
            }
            break;

        case 1:
            Math_SmoothStepToS(&this->targetRotX, 0, 1, 0x7D0, 0);

            if ((fabsf(this->actor.world.rot.y - this->targetRotY) < 100.0f) &&
                (fabsf(this->actor.world.rot.x) < 30.0f)) {
                this->scaleIndex++;

                if (this->index == 0) {
                    if (this->scaleIndex == 1) {
                        SET_WEEKEVENTREG(WEEKEVENTREG_81_10);
                    } else if (this->scaleIndex == 2) {
                        SET_WEEKEVENTREG(WEEKEVENTREG_81_20);
                    } else if (this->scaleIndex == 3) {
                        SET_WEEKEVENTREG(WEEKEVENTREG_81_40);
                    }
                } else if (this->scaleIndex == 1) {
                    SET_WEEKEVENTREG(WEEKEVENTREG_81_80);
                } else if (this->scaleIndex == 2) {
                    SET_WEEKEVENTREG(WEEKEVENTREG_82_01);
                } else if (this->scaleIndex == 3) {
                    SET_WEEKEVENTREG(WEEKEVENTREG_82_02);
                }

                if (this->isChasingOtherLabFish) {
                    this->scaleIndex = 5;
                }

                this->timer = 4;
                Actor_PlaySfx(&this->actor, NA_SE_EV_FISH_GROW_UP);
                this->scaleUpState++;
            }
            break;

        case 2:
            scaleMultiplier = 0.1f;
            FALLTHROUGH;
        case 4:
            if (scaleMultiplier == 0) {
                scaleMultiplier = 0.3f;
            }
            FALLTHROUGH;
        case 6:
            if (scaleMultiplier == 0) {
                scaleMultiplier = 0.5f;
            }

            Math_ApproachF(&this->scale, sScales[this->scaleIndex] * scaleMultiplier, 0.3f, 0.004f);

            if (this->timer == 0) {
                Vec3f effectPos;
                s32 i;
                s32 pad;

                Math_Vec3f_Copy(&effectPos, &this->actor.world.pos);
                effectPos.y += -10.0f;

                for (i = 0; i < 30; i++) {
                    EnFish2_AddEffect(this, &effectPos, 70);
                }

                this->scaleUpState++;
                this->timer = 2;
            }
            break;

        case 3:
            scaleMultiplier = 1.3f;
            FALLTHROUGH;
        case 5:
            if (scaleMultiplier == 0) {
                scaleMultiplier = 1.5f;
            }
            FALLTHROUGH;
        case 7:
            if (scaleMultiplier == 0) {
                scaleMultiplier = 1.7f;
            }

            Math_ApproachF(&this->scale, sScales[this->scaleIndex] * scaleMultiplier, 0.3f, 0.004f);

            if (this->timer == 0) {
                this->timer = 2;
                this->scaleUpState++;
            }
            break;

        case 8:
            Math_ApproachF(&this->scale, sScales[this->scaleIndex], 0.3f, 0.004f);
            if (this->timer == 0) {
                this->timer = 30;
                this->scaleUpState++;
            }
            break;

        case 9:
            if (this->timer == 0) {
                if (this->scaleIndex > 3) {
                    this->scaleIndex = 3;
                    this->scaleUpState = 0;

                    if (!this->isChasingOtherLabFish) {
                        this->actionFunc = EnFish2_SetupChaseOtherLabFish;
                    } else {
                        EnFish2_SetupSpitUpReward(this);
                    }
                } else {
                    this->timer = 0;
                    this->waitTimer = 0;
                    this->scaleUpState = 0;

                    if (D_80B2B2EC > 200) {
                        D_80B2B2E4 = 0;
                        D_80B2B2E0 = D_80B2B2EC = 0;
                    } else {
                        D_80B2B2E4 = 3;
                    }

                    this->collider.elements[0].dim.modelSphere.radius = (s32)((this->scale - 0.01f) * 1000.0f) + 5;
                    if (this->collider.elements[0].dim.modelSphere.radius > 15) {
                        this->collider.elements[0].dim.modelSphere.radius = 15;
                    }

                    this->collider.elements[1].dim.modelSphere.radius = (s32)((this->scale - 0.01f) * 1000.0f) + 5;
                    if (this->collider.elements[1].dim.modelSphere.radius > 15) {
                        this->collider.elements[1].dim.modelSphere.radius = 15;
                    }

                    EnFish2_SetupSwim(this);
                }
            }
            break;
    }

    Math_SmoothStepToS(&this->actor.world.rot.y, this->targetRotY, 1, 0xFA0, 0);
}

void EnFish2_SetupChaseOtherLabFish(EnFish2* this, PlayState* play) {
    Actor* prop = play->actorCtx.actorLists[ACTORCAT_PROP].first;

    while (prop != NULL) {
        if (prop->id != ACTOR_EN_FISH2) {
            prop = prop->next;
            continue;
        }

        if (&this->actor != prop) {
            this->targetActor = prop;
            this->isChasingOtherLabFish = sIsOneLabFishChasingTheOther = true;
            EnFish2_ChangeAnim(this, FISH2_ANIM_SWIM);
            this->actionFunc = EnFish2_ChaseOtherLabFish;
            break;
        }

        prop = prop->next;
    }
}

/**
 * Chases after the other, smaller lab fish in the tank. In the final game, this only happens once one of the lab fishes
 * is fed three bottled fish. Once this fish gets close enough to the smaller lab fish, it will attempt to bite it.
 */
void EnFish2_ChaseOtherLabFish(EnFish2* this, PlayState* play) {
    Vec3f targetActorPos;

    if (this->frameCounter < 400) {
        this->frameCounter++;
    }

    this->targetActorSpeedMultiplier = 410.0f - this->frameCounter;
    Math_ApproachF(&this->actor.speed, 2.0f, 0.3f, 0.3f);
    Math_ApproachF(&this->targetActor->speed, (sScales[4] - this->scale) * this->targetActorSpeedMultiplier, 0.1f,
                   0.4f);
    EnFish2_UpdateVelocityY(this, play);
    Math_Vec3f_Copy(&targetActorPos, &this->targetActor->world.pos);
    this->targetRotY = Math_Vec3f_Yaw(&this->actor.world.pos, &targetActorPos);
    this->targetRotX = Math_Vec3f_Pitch(&this->actor.world.pos, &targetActorPos);

    if (EnFish2_IsCloseEnoughToTargetActor(this, this->headPos, false)) {
        Math_Vec3f_Copy(&this->targetActorPos, &this->targetActor->world.pos);
        EnFish2_SetupBite(this);
    }
}

void EnFish2_CutsceneHandler_80B2A01C(EnFish2* this, PlayState* play) {
    if (this->waitTimer == 0) {
        if (!CutsceneManager_IsNext(this->csIdList[0])) {
            CutsceneManager_Queue(this->csIdList[0]);
        } else {
            this->waitTimer = 15;
            CutsceneManager_StartWithPlayerCs(this->csIdList[0], &this->actor);
            this->actionFunc = EnFish2_CutsceneHandler_80B2A094;
        }
    }
}

void EnFish2_CutsceneHandler_80B2A094(EnFish2* this, PlayState* play) {
    Vec3f subCamEye;

    if (this->waitTimer == 0) {
        D_80B2B2E4 = 1;
    }

    this->subCamId = CutsceneManager_GetCurrentSubCamId(this->csIdList[0]);

    if (D_80B2B2EC != 0) {
        D_80B2B2EC++;

        if (D_80B2B2EC > 200) {
            Actor_Kill(&this->actor);
            CutsceneManager_Stop(this->csIdList[0]);
            return;
        }
    }

    if ((this->targetActor != NULL) && (this->targetActor->update != NULL)) {
        Math_Vec3f_Copy(&subCamEye, &this->targetActor->world.pos);
        subCamEye.x += Math_SinS(-0x3A98) * 110.0f;
        subCamEye.z += Math_CosS(-0x3A98) * 110.0f;
        Math_Vec3f_Copy(&this->subCamEye, &subCamEye);
        Math_Vec3f_Copy(&subCamEye, &this->targetActor->world.pos);
        subCamEye.x += Math_SinS(-0x3A98) * 10.0f;
        subCamEye.z += Math_CosS(-0x3A98) * 10.0f;
        Math_Vec3f_Copy(&this->subCamAt, &subCamEye);
    }

    Play_SetCameraAtEye(play, this->subCamId, &this->subCamAt, &this->subCamEye);

    if ((this->targetActor == NULL) || (this->targetActor->update == NULL)) {
        this->targetActor = NULL;
        this->cutsceneWaitTimer++;

        if (this->cutsceneWaitTimer > 10) {
            this->waitTimer = 20;
            this->actionFunc = EnFish2_CutsceneHandler_80B2A23C;
        }
    }
}

void EnFish2_CutsceneHandler_80B2A23C(EnFish2* this, PlayState* play) {
    Vec3f subCamAt;

    Math_Vec3f_Copy(&subCamAt, &this->actor.world.pos);

    if (D_80B2B2E4 == 2) {
        subCamAt.x += (Math_SinS(-0x3A98) * 180.0f);
        subCamAt.y += 90.0f;
        subCamAt.z += Math_CosS(-0x3A98) * 180.0f;
        Math_Vec3f_Copy(&this->subCamEye, &subCamAt);
        Math_Vec3f_Copy(&subCamAt, &this->actor.world.pos);
        subCamAt.y += 70.0f;
        Math_Vec3f_Copy(&this->subCamAt, &subCamAt);
    } else if (D_80B2B2F4 != NULL) {
        Math_Vec3f_Copy(&subCamAt, &D_80B2B2F4->world.pos);
        subCamAt.x += Math_SinS(-0x3A98) * 110.0f;
        subCamAt.z += Math_CosS(-0x3A98) * 110.0f;
        Math_Vec3f_Copy(&this->subCamEye, &subCamAt);
        Math_Vec3f_Copy(&subCamAt, &D_80B2B2F4->world.pos);
        subCamAt.x += Math_SinS(-0x3A98) * 10.0f;
        subCamAt.z += Math_CosS(-0x3A98) * 10.0f;
        Math_Vec3f_Copy(&this->subCamAt, &subCamAt);
    }

    Play_SetCameraAtEye(play, this->subCamId, &this->subCamAt, &this->subCamEye);

    if ((this->waitTimer == 0) && (D_80B2B2E4 == 3)) {
        D_80B2B2E0 = D_80B2B2EC = D_80B2B2E4 = 0;
        D_80B2B2F4 = NULL;
        CutsceneManager_Stop(this->csIdList[0]);
        Actor_Kill(&this->actor);
    }
}

void EnFish2_SetupSpitUpReward(EnFish2* this) {
    EnFish2_ChangeAnim(this, FISH2_ANIM_SPIT_UP);
    this->waitTimer = 0;
    this->frameCounter = 0;
    this->timer = this->waitTimer;
    D_80B2B2E4 = 2;
    this->actionFunc = EnFish2_SpitUpReward;
}

void EnFish2_SpitUpReward(EnFish2* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;
    Vec3f pos;

    if ((this->animIndex == FISH2_ANIM_SPIT_UP) && Animation_OnFrame(&this->skelAnime, 13.0f)) {
        Actor* heartPiece;

        Math_Vec3f_Copy(&pos, &this->lowerJawPos);
        heartPiece = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_COL_MAN, pos.x, pos.y, pos.z, 0,
                                 this->actor.world.rot.y, 0, EN_COL_MAN_TYPE_HEART_PIECE);

        if (heartPiece != NULL) {
            heartPiece->speed = 4.0f;
            heartPiece->velocity.y = 15.0f;
            Actor_PlaySfx(&this->actor, NA_SE_SY_PIECE_OF_HEART);
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_81_10);
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_81_20);
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_81_40);
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_81_80);
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_82_01);
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_82_02);
        }
    }

    if ((this->animIndex == FISH2_ANIM_SPIT_UP) &&
        (Animation_OnFrame(&this->skelAnime, 13.0f) || Animation_OnFrame(&this->skelAnime, 31.0f))) {
        WaterBox* waterBox;

        if (WaterBox_GetSurface1(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                                 &this->waterSurface, &waterBox)) {
            Vec3f splashPos;
            s32 i;

            SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 50, NA_SE_EV_BOMB_DROP_WATER);

            for (i = 0; i < 10; i++) {
                Math_Vec3f_Copy(&splashPos, &this->actor.world.pos);
                splashPos.x += Rand_CenteredFloat(70.0f);
                splashPos.y = this->waterSurface + 10.0f;
                splashPos.z += Rand_CenteredFloat(70.0f);
                EffectSsGSplash_Spawn(play, &splashPos, NULL, NULL, 0, (s32)Rand_CenteredFloat(50.0f) + 350);
            }
        }
    }

    if ((curFrame >= this->animEndFrame) && (this->animIndex == FISH2_ANIM_SPIT_UP)) {
        D_80B2B2E0 = 0;
        D_80B2B2E4 = 3;
        this->actor.world.pos.x = this->targetPos.x;
        this->actor.world.pos.z = this->targetPos.z;
        EnFish2_ChangeAnim(this, FISH2_ANIM_SWIM);
    }
}

void EnFish2_Update(Actor* thisx, PlayState* play2) {
    static f32 D_80B2B3A8[] = {
        0.0f,
        40.0f,
        -40.0f,
        0.0f,
    };
    PlayState* play = play2;
    EnFish2* this = THIS;

    if ((this->actionFunc != EnFish2_Swallow) && (ENFISH2_GET_TYPE(&this->actor) != ENFISH2_TYPE_CUTSCENE_HANDLER)) {
        SkelAnime_Update(&this->skelAnime);
    }

    DECR(this->wallCheckTimer);
    DECR(this->timer);
    DECR(this->waitTimer);

    this->actionFunc(this, play);
    Actor_SetFocus(&this->actor, 0.0f);

    if (ENFISH2_GET_TYPE(&this->actor) != ENFISH2_TYPE_CUTSCENE_HANDLER) {
        WaterBox* waterBox;
        s32 i;
        Vec3f bubblePos;

        if (ENFISH2_GET_TYPE(&this->actor) == ENFISH2_TYPE_FISH) {
            Math_SmoothStepToS(&this->actor.world.rot.x, this->targetRotX, 1, this->angularVelocityModX + 0xC8, 0);
            if (this->actionFunc != EnFish2_ScaleUp) {
                Math_SmoothStepToS(&this->actor.world.rot.y, this->targetRotY, 1, 0xBB8, 0);
            }
        }

        if ((play->gameplayFrames % 8) == 0) {
            Math_Vec3f_Copy(&bubblePos, &this->targetPos);
            bubblePos.x += Rand_CenteredFloat(100.0f);
            bubblePos.y = this->actor.floorHeight;
            bubblePos.z += Rand_CenteredFloat(100.0f);

            for (i = 0; i < (s32)Rand_CenteredFloat(5.0f) + 10; i++) {
                EffectSsBubble_Spawn(play, &bubblePos, 0.0f, 5.0f, 5.0f, Rand_ZeroFloat(this->scale * 4.0f) + 0.1f);
            }
        }

        EnFish2_UpdateEffects(this, play);
        Math_Vec3s_Copy(&this->actor.shape.rot, &this->actor.world.rot);
        Math_Vec3f_Copy(&this->wallCheckPos, &this->actor.world.pos);
        this->wallCheckPos.x += (Math_SinS(this->actor.world.rot.y) * 25.0f) - this->scale;
        this->wallCheckPos.z += (Math_CosS(this->actor.world.rot.y) * 25.0f) - this->scale;
        this->wallCheckRadius = 25.0f - ((this->scale - 0.01f) * 1000.0f);
        Actor_SetScale(&this->actor, this->scale);
        Actor_MoveWithGravity(&this->actor);
        Actor_UpdateBgCheckInfo(play, &this->actor, 0.0f, 15.0f, 10.0f,
                                UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_2 | UPDBGCHECKINFO_FLAG_4);

        if (ENFISH2_GET_TYPE(&this->actor) != ENFISH2_TYPE_UNK_2) {
            this->minDistFromFloor = this->actor.floorHeight + (this->scale * 1000.0f);
            this->minDistFromWaterSurface = this->scale * 600.0f;

            if (this->actor.world.pos.y < this->minDistFromFloor) {
                this->actor.world.pos.y = this->minDistFromFloor + 0.1f;
            }

            if (WaterBox_GetSurface1(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                                     &this->waterSurface, &waterBox)) {
                if ((this->waterSurface != BGCHECK_Y_MIN) &&
                    (this->waterSurface - this->minDistFromWaterSurface < this->actor.world.pos.y)) {
                    this->actor.world.pos.y = this->waterSurface - this->minDistFromWaterSurface;
                }
            }

            if (!sIsOneLabFishChasingTheOther && (this->actionFunc == EnFish2_Swim)) {
                s32 i = this->index * 2;
                //! FAKE: Can't find a match using 0.0f here. MM3D initializes this to 0.0f as expected
                f32 lowerBound = 0;
                f32 upperBound = 0.0f;
                WaterBox* waterBox;

                if (WaterBox_GetSurface1(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                                         &this->waterSurface, &waterBox)) {
                    upperBound = D_80B2B3A8[i] + (this->waterSurface - this->minDistFromWaterSurface);
                    lowerBound = D_80B2B3A8[i + 1] + this->minDistFromFloor;
                }

                if ((upperBound < this->actor.world.pos.y) && (this->targetRotX < 0)) {
                    this->targetRotX = 0;
                    this->actor.velocity.y = 0.0f;
                    this->actor.gravity = 0.0f;
                }

                if ((this->actor.world.pos.y < lowerBound) && (this->targetRotX > 0)) {
                    this->targetRotX = 0;
                    this->actor.velocity.y = 0.0f;
                    this->actor.gravity = 0.0f;
                }
            }

            if (!sIsOneLabFishChasingTheOther) {
                CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
            }
        }
    }
}

s32 EnFish2_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    EnFish2* this = THIS;

    if ((limbIndex == RESEARCH_LAB_FISH_LIMB_RIGHT_EYE) || (limbIndex == RESEARCH_LAB_FISH_LIMB_LEFT_EYE)) {
        *dList = NULL;
    }

    return false;
}

void EnFish2_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    EnFish2* this = THIS;
    s32 pad;

    if ((limbIndex == RESEARCH_LAB_FISH_LIMB_RIGHT_EYE) || (limbIndex == RESEARCH_LAB_FISH_LIMB_LEFT_EYE)) {
        OPEN_DISPS(play->state.gfxCtx);

        Matrix_Push();
        Matrix_ReplaceRotation(&play->billboardMtxF);

        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, *dList);

        Matrix_Pop();

        CLOSE_DISPS(play->state.gfxCtx);
    }

    if (limbIndex == RESEARCH_LAB_FISH_LIMB_LOWER_JAW) {
        Matrix_MultVec3f(&gZeroVec3f, &this->lowerJawPos);
    }

    if (limbIndex == RESEARCH_LAB_FISH_LIMB_HEAD) {
        Matrix_MultVec3f(&gZeroVec3f, &this->headPos);
    }

    Collider_UpdateSpheres(limbIndex, &this->collider);
}

void EnFish2_Draw(Actor* thisx, PlayState* play) {
    EnFish2* this = THIS;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    SkelAnime_DrawFlexOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount,
                          EnFish2_OverrideLimbDraw, EnFish2_PostLimbDraw, &this->actor);
    EnFish2_DrawEffects(this, play);
}

void EnFish2_AddEffect(EnFish2* this, Vec3f* pos, s16 timer) {
    s16 i;
    EnFish2Effect* effect = &this->effects[0];

    for (i = 0; i < ARRAY_COUNT(this->effects); i++, effect++) {
        if (!effect->isEnabled) {
            TexturePtr texture;

            if (Rand_ZeroOne() < 0.5f) {
                texture = gEffBubble2Tex;
            } else {
                texture = gEffBubble1Tex;
            }

            effect->texture = (TexturePtr)OS_K0_TO_PHYSICAL(SEGMENTED_TO_K0(texture));
            effect->isEnabled = true;
            effect->pos = *pos;
            effect->pos.x += Rand_CenteredFloat(effect->isEnabled + (this->scale * 4000.0f));
            effect->pos.y += Rand_CenteredFloat(20.0f);
            effect->pos.z += Rand_CenteredFloat(effect->isEnabled + (this->scale * 4000.0f));
            effect->targetScale = (this->scale * 20.0f) - (Rand_ZeroFloat(5.0f) * 0.01f);
            effect->unk_1C = 66;
            effect->timer = timer;
            break;
        }
    }
}

void EnFish2_UpdateEffects(EnFish2* this, PlayState* play) {
    EnFish2Effect* effect = &this->effects[0];
    WaterBox* waterBox;
    f32 waterSurface;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(this->effects); i++, effect++) {
        if (effect->isEnabled) {
            if ((effect->timer != 0) && (effect->unk_1C != 0)) {
                Math_ApproachF(&effect->scale, effect->targetScale, 0.4f, 0.5f);
                effect->timer--;
            } else {
                effect->isEnabled = false;
            }

            if (effect->isEnabled) {
                effect->pos.x += (0.3f + (Rand_ZeroOne() * 0.5f)) - 0.55f;
                effect->pos.y += 1.0f + ((Rand_ZeroOne() - 0.3f) * 1.2f);
                effect->pos.z += (0.3f + (Rand_ZeroOne() * 0.5f)) - 0.55f;
                waterSurface = effect->pos.y;

                if (!WaterBox_GetSurface1(play, &play->colCtx, effect->pos.x, effect->pos.z, &waterSurface,
                                          &waterBox)) {
                    effect->isEnabled = false;
                } else if (waterSurface < effect->pos.y) {
                    Vec3f ripplePos;

                    ripplePos.x = effect->pos.x;
                    ripplePos.y = waterSurface;
                    ripplePos.z = effect->pos.z;
                    EffectSsGRipple_Spawn(play, &ripplePos, 0, 80, 0);
                    effect->isEnabled = false;
                }
            }
        }
    }
}

void EnFish2_DrawEffects(EnFish2* this, PlayState* play) {
    s16 i;
    EnFish2Effect* effect;
    GraphicsContext* gfxCtx = play->state.gfxCtx;

    OPEN_DISPS(gfxCtx);

    effect = &this->effects[0];
    Gfx_SetupDL25_Opa(gfxCtx);

    for (i = 0; i < ARRAY_COUNT(this->effects); i++, effect++) {
        if (effect->isEnabled) {
            Matrix_Translate(effect->pos.x, effect->pos.y, effect->pos.z, MTXMODE_NEW);
            Matrix_Scale(effect->scale, effect->scale, effect->scale, MTXMODE_APPLY);

            MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gfxCtx);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 150, 150, 150, 0);
            gSPSegment(POLY_OPA_DISP++, 0x08, effect->texture);
            gSPDisplayList(POLY_OPA_DISP++, gEffBubbleDL);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
