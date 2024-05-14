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

void func_80B28B5C(EnFish2* this);
void func_80B28C14(EnFish2* this, PlayState* play);
void func_80B29128(EnFish2* this);
void func_80B2913C(EnFish2* this, PlayState* play);
void func_80B29194(EnFish2* this);
void func_80B29250(EnFish2* this, PlayState* play);
void func_80B2938C(EnFish2* this);
void func_80B293C4(EnFish2* this, PlayState* play);
void func_80B2951C(EnFish2* this);
void func_80B295A4(EnFish2* this, PlayState* play);
void func_80B29778(EnFish2* this);
void func_80B297FC(EnFish2* this, PlayState* play);
void func_80B29E5C(EnFish2* this, PlayState* play);
void func_80B29EE4(EnFish2* this, PlayState* play);
void func_80B2A01C(EnFish2* this, PlayState* play);
void func_80B2A094(EnFish2* this, PlayState* play);
void func_80B2A23C(EnFish2* this, PlayState* play);
void EnFish2_SetupSpitUpReward(EnFish2* this);
void EnFish2_SpitUpReward(EnFish2* this, PlayState* play);
void func_80B2ADB0(EnFish2* this, Vec3f* vec, s16 arg2);
void func_80B2AF80(EnFish2* this, PlayState* play);
void func_80B2B180(EnFish2* this, PlayState* play);

static s32 D_80B2B2E0 = 0;
static s32 D_80B2B2E4 = 0;
static s32 D_80B2B2E8 = false;
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
    /* 5 */ FISH2_ANIM_EAT,
    /* 6 */ FISH2_ANIM_MAX
} Fish2Animation;

static AnimationHeader* sAnimations[FISH2_ANIM_MAX] = {
    &gResearchLabFishSwimAnim,           // FISH2_ANIM_SWIM
    &gResearchLabFishFastSwimAnim,       // FISH2_ANIM_FAST_SWIM
    &gResearchLabFishWindUpAndChompAnim, // FISH2_ANIM_WIND_UP
    &gResearchLabFishWindUpAndChompAnim, // FISH2_ANIM_CHOMP
    &gResearchLabFishSpitUpAnim,         // FISH2_ANIM_SPIT_UP
    &gResearchLabFishEatAnim,            // FISH2_ANIM_EAT
};

static u8 sAnimationModes[FISH2_ANIM_MAX] = {
    ANIMMODE_LOOP, // FISH2_ANIM_SWIM
    ANIMMODE_LOOP, // FISH2_ANIM_FAST_SWIM
    ANIMMODE_ONCE, // FISH2_ANIM_WIND_UP
    ANIMMODE_ONCE, // FISH2_ANIM_CHOMP
    ANIMMODE_ONCE, // FISH2_ANIM_SPIT_UP
    ANIMMODE_ONCE, // FISH2_ANIM_EAT
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
    if (this->unk_2C8 == 0) {
        if ((D_80B2B2E4 != 0) && ((D_80B2B2E0 != 1) || (this->unk_350 == NULL) || (this->unk_350->update == NULL)) &&
            (this->unk_2B0 == 0)) {
            this->unk_2B4 = 0;
            this->unk_2C4 = 0;
            this->unk_2B6 = this->unk_2B4;
            func_80B28B5C(this);
            return true;
        }
    } else if ((this->unk_350 == NULL) || (this->unk_350->update == NULL)) {
        this->unk_2B4 = 0;
        this->unk_2C4 = 0;
        this->unk_2B6 = this->unk_2B4;
        func_80B28B5C(this);
        return true;
    }

    return false;
}

void EnFish2_Init(Actor* thisx, PlayState* play) {
    EnFish2* this = THIS;
    s32 i;
    s32 csId;

    Math_Vec3f_Copy(&this->unk_324, &this->actor.home.pos);
    this->index = sCurrentIndex;
    sCurrentIndex++;

    if (this->actor.params == 0) {
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
        func_80B28B5C(this);
    } else if (this->actor.params != 0) {
        this->unk_2B4 = 10;
        this->actor.draw = NULL;
        this->actor.flags |= ACTOR_FLAG_LOCK_ON_DISABLED;
        this->actionFunc = func_80B2A01C;
    }
}

void EnFish2_Destroy(Actor* thisx, PlayState* play) {
    EnFish2* this = THIS;

    if (this->actor.params != 1) {
        Collider_DestroyJntSph(play, &this->collider);
    }
}

void func_80B287F4(EnFish2* this, s32 arg1) {
    Vec3f sp2C;

    if ((this->unk_354 != 0) && (this->unk_2C8 == 0)) {
        if (this->unk_2C4 < 400) {
            this->unk_2C4++;
        }

        this->unk_338 = 440.0f - this->unk_2C4;

        if (!arg1) {
            this->unk_338 = 410.0f - this->unk_2C4;
        }

        Math_ApproachF(&this->unk_350->speed, (sScales[4] - this->scale) * this->unk_338, 0.1f, 0.4f);
    }

    Math_Vec3f_Copy(&sp2C, &this->unk_350->world.pos);
    this->unk_34A = Math_Vec3f_Yaw(&this->actor.world.pos, &sp2C);
    this->unk_348 = Math_Vec3f_Pitch(&this->actor.world.pos, &sp2C);
}

s32 func_80B288E8(EnFish2* this, Vec3f vec, s32 arg2) {
    f32 temp_f2 = this->unk_350->world.pos.x - vec.x;
    f32 temp_f12 = this->unk_350->world.pos.y - vec.y;
    f32 temp_f14 = this->unk_350->world.pos.z - vec.z;
    f32 dist = sqrtf(SQ(temp_f2) + SQ(temp_f12) + SQ(temp_f14));
    f32 phi_f2;

    if (!arg2) {
        phi_f2 = 40.0f;
    } else {
        phi_f2 = this->scale * 2000.0f;
        if (phi_f2 > 20.0f) {
            phi_f2 = 20.0f;
        }
    }

    if (dist < phi_f2) {
        return true;
    }

    return false;
}

s32 func_80B2899C(EnFish2* this, PlayState* play) {
    if (BgCheck_SphVsFirstWall(&play->colCtx, &this->wallCheckPos, this->wallCheckRadius)) {
        return true;
    }

    return false;
}

void func_80B289DC(EnFish2* this, PlayState* play) {
    WaterBox* waterBox;

    if (this->unk_2B4 != 0) {
        this->unk_348 = 0;
    }

    if (this->unk_348 != 0) {
        if (this->unk_348 > 0) {
            if (this->unk_2D4 < this->actor.world.pos.y) {
                this->actor.velocity.y = this->actor.world.rot.x * 0.001f * -0.1f;
                if (this->actionFunc == func_80B297FC) {
                    this->actor.velocity.y *= 2.0f;
                }
            } else {
                this->unk_348 = 0;
                this->actor.velocity.y = 0.0f;
                this->actor.gravity = 0.0f;
            }
        } else if (WaterBox_GetSurface1(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                                        &this->waterSurface, &waterBox)) {
            if ((this->waterSurface != BGCHECK_Y_MIN) &&
                (this->actor.world.pos.y < (this->waterSurface - this->unk_2D8))) {
                this->actor.velocity.y = this->actor.world.rot.x * 0.001f * -0.1f;
                if (this->actionFunc == func_80B297FC) {
                    this->actor.velocity.y *= 2.0f;
                }
            }
        } else {
            this->unk_348 = 0;
            this->actor.velocity.y = 0.0f;
            this->actor.gravity = 0.0f;
        }
    }
}

void func_80B28B5C(EnFish2* this) {
    EnFish2_ChangeAnim(this, FISH2_ANIM_SWIM);
    this->unk_2B4 = 0;
    this->unk_348 = 0;
    this->unk_2C4 = 0;
    this->unk_2C8 = 0;
    this->waterSurface = BGCHECK_Y_MIN;
    this->unk_2B6 = this->unk_2B4;
    this->unk_34C = this->unk_348;
    this->unk_34A = this->actor.world.rot.y;

    this->collider.elements[1].dim.modelSphere.radius = (s32)((this->scale - 0.01f) * 1000.0f) + 5;
    if (this->collider.elements[1].dim.modelSphere.radius > 15) {
        this->collider.elements[1].dim.modelSphere.radius = 15;
    }

    this->actionFunc = func_80B28C14;
}

void func_80B28C14(EnFish2* this, PlayState* play) {
    Actor* itemAction = play->actorCtx.actorLists[ACTORCAT_ITEMACTION].first;
    WaterBox* waterbox;

    if (Actor_TalkOfferAccepted(&this->actor, &play->state)) {
        func_80B29128(this);
        return;
    }

    if ((this->unk_2B8 == 0) && (fabsf(this->actor.world.rot.y - this->unk_34A) < 100.0f)) {
        if (func_80B2899C(this, play)) {
            if (this->unk_340 == 0) {
                this->unk_34A += 0x4000;
            } else {
                this->unk_34A -= 0x4000;
            }

            this->unk_2B8 = (s32)(this->scale * 1000.0f) - 10;
        }
    }

    func_80B289DC(this, play);

    if (this->unk_2B6 == 0) {
        if (this->unk_2B4 == 0) {
            this->unk_2B4 = (s32)Rand_ZeroFloat(20.0f) + 10;
        } else if (this->unk_2B4 == 1) {
            if ((this->unk_348 == 0) || (Rand_ZeroOne() < 0.6f)) {
                this->unk_348 = Rand_CenteredFloat(0x2000);
            } else {
                this->unk_348 = Math_Vec3f_Pitch(&this->actor.world.pos, &this->unk_324);
                if (this->unk_348 < -0x1000) {
                    this->unk_348 = -0x1000;
                }

                if (this->unk_348 > 0x1000) {
                    this->unk_348 = 0x1000;
                }
            }

            this->unk_2B6 = (s32)Rand_ZeroFloat(70.0f) + 30;
            if (this->scaleIndex >= 3) {
                this->unk_2B6 -= (s32)Rand_ZeroFloat(20.0f);
            }
        }
    }

    if ((this->waterSurface == BGCHECK_Y_MIN) &&
        !WaterBox_GetSurface1(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                              &this->waterSurface, &waterbox)) {
        this->waterSurface = this->actor.world.pos.y;
    }

    if (this->unk_2B4 == 0) {
        Math_ApproachF(&this->actor.speed, (sScales[4] - this->scale) * 400.0f, 0.3f, 0.3f);
        if (this->actor.speed > 3.0f) {
            this->actor.speed = 3.0f;
        } else if (this->actor.speed < 1.5f) {
            this->actor.speed = 1.5f;
        }
    } else {
        Math_ApproachZeroF(&this->actor.speed, 0.3f, 0.3f);
    }

    if (!D_80B2B2E8 && (D_80B2B2E0 != 2)) {
        while (itemAction != NULL) {
            if ((itemAction->id != ACTOR_EN_FISH) && (itemAction->id != ACTOR_EN_MUSHI2)) {
                itemAction = itemAction->next;
                continue;
            }

            if (itemAction->id == ACTOR_EN_MUSHI2) {
                if ((itemAction->update != NULL) && (((EnMushi2*)itemAction)->unk_30C & 0x200)) {
                    this->unk_350 = itemAction;
                    this->unk_2C8 = 1;
                    func_80B29194(this);
                    break;
                }
            } else if ((itemAction->update != NULL) && (itemAction->params == 0) &&
                       (fabsf(itemAction->world.pos.x - this->actor.world.pos.x) < 100.0f) &&
                       (fabsf(itemAction->world.pos.z - this->actor.world.pos.z) < 100.0f) &&
                       (itemAction->bgCheckFlags & BGCHECKFLAG_WATER)) {
                this->unk_350 = itemAction;

                if (D_80B2B2E0 == 0) {
                    EnFish2* fish;

                    this->unk_354 = NULL;
                    fish = (EnFish2*)Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_FISH2,
                                                        this->unk_324.x, this->unk_324.y, this->unk_324.z, 0, 0, 0, 1);
                    this->unk_354 = fish;

                    if (this->unk_354 != NULL) {
                        D_80B2B2E4 = 0;
                        D_80B2B2E0 = 1;
                        fish->csIdList[0] = this->csIdList[0];
                        fish->unk_350 = this->unk_350;
                    }
                }

                func_80B29194(this);
                break;
            }

            itemAction = itemAction->next;
        }

        Actor_OfferTalk(&this->actor, play, 100.0f);
    }
}

void func_80B29128(EnFish2* this) {
    this->actionFunc = func_80B2913C;
}

void func_80B2913C(EnFish2* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_EVENT) && Message_ShouldAdvance(play)) {
        Message_CloseTextbox(play);
        func_80B28B5C(this);
    }
}

void func_80B29194(EnFish2* this) {
    if ((this->unk_354 != NULL) && (this->unk_2C8 == 0)) {
        EnFish* fish = (EnFish*)this->unk_350;
        fish->unk_277 = 1;
    }

    this->unk_2C4 = 0;
    this->actor.speed = 0.0f;
    if (this->unk_2C8 == 0) {
        this->unk_34C = 400;
        Math_Vec3f_Copy(&this->actor.world.pos, &this->unk_324);
        this->actor.world.pos.y = this->actor.floorHeight + (this->scale * 1200.0f);
    }

    this->unk_34A = 0;
    this->unk_340 = (s32)Rand_ZeroOne() & 1;
    EnFish2_ChangeAnim(this, FISH2_ANIM_FAST_SWIM);
    this->actionFunc = func_80B29250;
}

void func_80B29250(EnFish2* this, PlayState* play) {
    if (!func_80B28478(this)) {
        Math_ApproachF(&this->actor.speed, (sScales[4] - this->scale) * 1000.0f, 0.3f, 0.3f);

        if (this->actor.speed > 4.0f) {
            this->actor.speed = 4.0f;
        } else if (this->actor.speed < 2.0f) {
            this->actor.speed = 2.0f;
        }

        func_80B287F4(this, false);
        func_80B289DC(this, play);
        if (func_80B288E8(this, this->headPos, false) &&
            (((this->unk_2C8 == 0) && (D_80B2B2E4 == 1)) || (this->unk_2C8 != 0))) {
            Math_Vec3f_Copy(&this->unk_30C, &this->unk_350->world.pos);
            func_80B2938C(this);
        }
    }
}

void func_80B2938C(EnFish2* this) {
    EnFish2_ChangeAnim(this, FISH2_ANIM_EAT);
    this->unk_348 = 0;
    this->actionFunc = func_80B293C4;
}

void func_80B293C4(EnFish2* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;

    if (!func_80B28478(this)) {
        func_80B287F4(this, true);
        Math_ApproachF(&this->actor.speed, (sScales[4] - this->scale) * 1000.0f, 0.3f, 0.3f);

        if (this->actor.speed > 3.0f) {
            this->actor.speed = 3.0f;
        } else if (this->actor.speed < 1.0f) {
            this->actor.speed = 1.0f;
        }

        if (curFrame >= this->animEndFrame) {
            EnFish2_ChangeAnim(this, FISH2_ANIM_FAST_SWIM);
            if (this->unk_2B0 == 0) {
                this->actionFunc = func_80B29250;
            } else {
                this->actionFunc = func_80B29EE4;
            }
        } else {
            func_80B289DC(this, play);
            if (func_80B288E8(this, this->lowerJawPos, true)) {
                func_80B2951C(this);
            }
        }
    }
}

void func_80B2951C(EnFish2* this) {
    this->actor.velocity.y = 0.0f;
    this->actor.gravity = 0.0f;
    this->unk_348 = 0;

    if (this->unk_2C8 == 0) {
        D_80B2B2E0 = 2;
    }

    this->unk_2B4 = 10;
    this->actor.speed = 3.0f;
    Actor_Kill(this->unk_350);
    this->unk_350 = NULL;
    D_80B2B2F4 = &this->actor;
    Actor_PlaySfx(&this->actor, NA_SE_EN_DODO_M_EAT);
    this->actionFunc = func_80B295A4;
}

void func_80B295A4(EnFish2* this, PlayState* play) {
    s32 i;
    f32 curFrame = this->skelAnime.curFrame;
    s32 pad;
    Vec3f sp60;

    SkelAnime_Update(&this->skelAnime);
    Math_SmoothStepToS(&this->actor.world.rot.y, Math_Vec3f_Yaw(&this->actor.world.pos, &play->view.eye), 1, 0x1388, 0);
    Math_ApproachZeroF(&this->actor.speed, 0.3f, 0.3f);

    if (this->unk_2B4 != 0) {
        Math_Vec3f_Copy(&sp60, &this->lowerJawPos);
        sp60.x += Rand_CenteredFloat(100.0f);
        sp60.z += Rand_CenteredFloat(100.0f);

        for (i = 0; i < 2; i++) {
            EffectSsBubble_Spawn(play, &sp60, 0.0f, 5.0f, 5.0f, Rand_ZeroFloat(this->scale * 4.0f) + 0.1f);
        }
    } else if (curFrame >= this->animEndFrame) {
        if (this->animIndex != FISH2_ANIM_EAT) {
            EnFish2_ChangeAnim(this, FISH2_ANIM_EAT);
        } else if (this->unk_2C8 == 0) {
            func_80B29778(this);
        } else {
            this->unk_2B4 = 0;
            this->unk_2C4 = 0;
            this->unk_2B6 = this->unk_2B4;
            func_80B28B5C(this);
        }
    }
}

void func_80B29778(EnFish2* this) {
    EnFish2_ChangeAnim(this, FISH2_ANIM_SWIM);
    this->unk_2B4 = 0;
    this->unk_2C4 = 0;
    this->unk_2B6 = this->unk_2B4;

    if (this->unk_2B0 != 0) {
        this->unk_324.x = this->actor.home.pos.x - 14.0f;
        this->unk_324.z = this->actor.home.pos.z - 18.0f;
    }

    this->actionFunc = func_80B297FC;
    this->unk_324.y = this->unk_2D4;
    this->actor.speed = 0.0f;
}

void func_80B297FC(EnFish2* this, PlayState* play) {
    f32 phi_f0 = 0.0f;
    Vec3f sp48;

    switch (this->unk_2C4) {
        case 0:
            Math_Vec3f_Copy(&sp48, &this->unk_324);
            sp48.y = this->unk_2D4;
            this->unk_348 = Math_Vec3f_Pitch(&this->actor.world.pos, &sp48);

            if (this->unk_348 < -0x2000) {
                this->unk_348 = -0x2000;
            }

            if (this->unk_348 > 0x2000) {
                this->unk_348 = 0x2000;
            }

            this->unk_34A = Math_Vec3f_Yaw(&this->actor.world.pos, &this->unk_324);
            func_80B289DC(this, play);
            Math_ApproachF(&this->actor.world.pos.x, this->unk_324.x, 0.3f, 3.0f);
            Math_ApproachF(&this->actor.world.pos.y, this->unk_2D4, 0.3f, 3.0f);
            Math_ApproachF(&this->actor.world.pos.z, this->unk_324.z, 0.3f, 3.0f);
            if ((fabsf(this->actor.world.pos.x - this->unk_324.x) < 2.0f) &&
                (this->actor.world.pos.y < (this->unk_2D4 + 3.0f)) &&
                (fabsf(this->actor.world.pos.z - this->unk_324.z) < 2.0f)) {
                this->actor.speed = 0.0f;
                this->unk_34A = BINANG_ROT180(this->actor.home.rot.y);
                this->unk_2C4++;
                this->actor.velocity.y = 0.0f;
                this->actor.gravity = 0.0f;
            }
            break;

        case 1:
            Math_SmoothStepToS(&this->unk_348, 0, 1, 0x7D0, 0);

            if ((fabsf(this->actor.world.rot.y - this->unk_34A) < 100.0f) && (fabsf(this->actor.world.rot.x) < 30.0f)) {
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

                if (this->unk_2B0 != 0) {
                    this->scaleIndex = 5;
                }

                this->unk_2B6 = 4;
                Actor_PlaySfx(&this->actor, NA_SE_EV_FISH_GROW_UP);
                this->unk_2C4++;
            }
            break;

        case 2:
            phi_f0 = 0.1f;
            FALLTHROUGH;
        case 4:
            if (phi_f0 == 0) {
                phi_f0 = 0.3f;
            }
            FALLTHROUGH;
        case 6:
            if (phi_f0 == 0) {
                phi_f0 = 0.5f;
            }

            Math_ApproachF(&this->scale, sScales[this->scaleIndex] * phi_f0, 0.3f, 0.004f);

            if (this->unk_2B6 == 0) {
                Vec3f sp3C;
                s32 i;
                s32 pad2;

                Math_Vec3f_Copy(&sp3C, &this->actor.world.pos);
                sp3C.y += -10.0f;

                for (i = 0; i < 30; i++) {
                    func_80B2ADB0(this, &sp3C, 0x46);
                }

                this->unk_2C4++;
                this->unk_2B6 = 2;
            }
            break;

        case 3:
            phi_f0 = 1.3f;
            FALLTHROUGH;
        case 5:
            if (phi_f0 == 0) {
                phi_f0 = 1.5f;
            }
            FALLTHROUGH;
        case 7:
            if (phi_f0 == 0) {
                phi_f0 = 1.7f;
            }

            Math_ApproachF(&this->scale, sScales[this->scaleIndex] * phi_f0, 0.3f, 0.004f);

            if (this->unk_2B6 == 0) {
                this->unk_2B6 = 2;
                this->unk_2C4++;
            }
            break;

        case 8:
            Math_ApproachF(&this->scale, sScales[this->scaleIndex], 0.3f, 0.004f);
            if (this->unk_2B6 == 0) {
                this->unk_2B6 = 30;
                this->unk_2C4++;
            }
            break;

        case 9:
            if (this->unk_2B6 == 0) {
                if (this->scaleIndex > 3) {
                    this->scaleIndex = 3;
                    this->unk_2C4 = 0;

                    if (this->unk_2B0 == 0) {
                        this->actionFunc = func_80B29E5C;
                    } else {
                        EnFish2_SetupSpitUpReward(this);
                    }
                } else {
                    this->unk_2B6 = 0;
                    this->unk_2B4 = 0;
                    this->unk_2C4 = 0;

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

                    func_80B28B5C(this);
                }
            }
            break;
    }

    Math_SmoothStepToS(&this->actor.world.rot.y, this->unk_34A, 1, 0xFA0, 0);
}

void func_80B29E5C(EnFish2* this, PlayState* play) {
    Actor* prop = play->actorCtx.actorLists[ACTORCAT_PROP].first;

    while (prop != NULL) {
        if (prop->id != ACTOR_EN_FISH2) {
            prop = prop->next;
            continue;
        }

        if (&this->actor != prop) {
            this->unk_350 = prop;
            this->unk_2B0 = D_80B2B2E8 = 1;
            EnFish2_ChangeAnim(this, FISH2_ANIM_SWIM);
            this->actionFunc = func_80B29EE4;
            break;
        }

        prop = prop->next;
    }
}

void func_80B29EE4(EnFish2* this, PlayState* play) {
    Vec3f sp2C;

    if (this->unk_2C4 < 400) {
        this->unk_2C4++;
    }

    this->unk_338 = 410.0f - this->unk_2C4;
    Math_ApproachF(&this->actor.speed, 2.0f, 0.3f, 0.3f);
    Math_ApproachF(&this->unk_350->speed, (sScales[4] - this->scale) * this->unk_338, 0.1f, 0.4f);
    func_80B289DC(this, play);
    Math_Vec3f_Copy(&sp2C, &this->unk_350->world.pos);
    this->unk_34A = Math_Vec3f_Yaw(&this->actor.world.pos, &sp2C);
    this->unk_348 = Math_Vec3f_Pitch(&this->actor.world.pos, &sp2C);

    if (func_80B288E8(this, this->headPos, false)) {
        Math_Vec3f_Copy(&this->unk_30C, &this->unk_350->world.pos);
        func_80B2938C(this);
    }
}

void func_80B2A01C(EnFish2* this, PlayState* play) {
    if (this->unk_2B4 == 0) {
        if (!CutsceneManager_IsNext(this->csIdList[0])) {
            CutsceneManager_Queue(this->csIdList[0]);
        } else {
            this->unk_2B4 = 15;
            CutsceneManager_StartWithPlayerCs(this->csIdList[0], &this->actor);
            this->actionFunc = func_80B2A094;
        }
    }
}

void func_80B2A094(EnFish2* this, PlayState* play) {
    Vec3f subCamEye;

    if (this->unk_2B4 == 0) {
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

    if ((this->unk_350 != NULL) && (this->unk_350->update != NULL)) {
        Math_Vec3f_Copy(&subCamEye, &this->unk_350->world.pos);
        subCamEye.x += Math_SinS(-0x3A98) * 110.0f;
        subCamEye.z += Math_CosS(-0x3A98) * 110.0f;
        Math_Vec3f_Copy(&this->subCamEye, &subCamEye);
        Math_Vec3f_Copy(&subCamEye, &this->unk_350->world.pos);
        subCamEye.x += Math_SinS(-0x3A98) * 10.0f;
        subCamEye.z += Math_CosS(-0x3A98) * 10.0f;
        Math_Vec3f_Copy(&this->subCamAt, &subCamEye);
    }

    Play_SetCameraAtEye(play, this->subCamId, &this->subCamAt, &this->subCamEye);
    if ((this->unk_350 == NULL) || (this->unk_350->update == NULL)) {
        this->unk_350 = NULL;
        this->unk_2B0++;
        
        if (this->unk_2B0 > 10) {
            this->unk_2B4 = 20;
            this->actionFunc = func_80B2A23C;
        }
    }
}

void func_80B2A23C(EnFish2* this, PlayState* play) {
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

    if ((this->unk_2B4 == 0) && (D_80B2B2E4 == 3)) {
        D_80B2B2E0 = D_80B2B2EC = D_80B2B2E4 = 0;
        D_80B2B2F4 = NULL;
        CutsceneManager_Stop(this->csIdList[0]);
        Actor_Kill(&this->actor);
    }
}

void EnFish2_SetupSpitUpReward(EnFish2* this) {
    EnFish2_ChangeAnim(this, FISH2_ANIM_SPIT_UP);
    this->unk_2B4 = 0;
    this->unk_2C4 = 0;
    this->unk_2B6 = this->unk_2B4;
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
        this->actor.world.pos.x = this->unk_324.x;
        this->actor.world.pos.z = this->unk_324.z;
        EnFish2_ChangeAnim(this, FISH2_ANIM_SWIM);
    }
}

void EnFish2_Update(Actor* thisx, PlayState* play2) {
    static f32 D_80B2B3A8[] = {
        0.0f, 40.0f, -40.0f, 0.0f, 0.0f, 0.0f,
    };
    PlayState* play = play2;
    EnFish2* this = THIS;

    if ((this->actionFunc != func_80B295A4) && (this->actor.params != 1)) {
        SkelAnime_Update(&this->skelAnime);
    }

    DECR(this->unk_2B8);

    if (this->unk_2B6 != 0) {
        this->unk_2B6--;
    }

    if (this->unk_2B4 == 0) {
    } else {
        this->unk_2B4--;
    }

    this->actionFunc(this, play);
    Actor_SetFocus(&this->actor, 0);

    if (this->actor.params != 1) {
        WaterBox* sp6C;
        s32 i;
        Vec3f sp5C;

        if (this->actor.params == 0) {
            Math_SmoothStepToS(&this->actor.world.rot.x, this->unk_348, 1, this->unk_34C + 0xC8, 0);
            if (this->actionFunc != func_80B297FC) {
                Math_SmoothStepToS(&this->actor.world.rot.y, this->unk_34A, 1, 0xBB8, 0);
            }
        }

        if ((play->gameplayFrames % 8) == 0) {
            Math_Vec3f_Copy(&sp5C, &this->unk_324);
            sp5C.x += Rand_CenteredFloat(100.0f);
            sp5C.y = this->actor.floorHeight;
            sp5C.z += Rand_CenteredFloat(100.0f);

            for (i = 0; i < (s32)Rand_CenteredFloat(5.0f) + 10; i++) {
                EffectSsBubble_Spawn(play, &sp5C, 0, 5.0f, 5.0f, Rand_ZeroFloat(this->scale * 4.0f) + 0.1f);
            }
        }

        func_80B2AF80(this, play);
        Math_Vec3s_Copy(&this->actor.shape.rot, &this->actor.world.rot);
        Math_Vec3f_Copy(&this->wallCheckPos, &this->actor.world.pos);
        this->wallCheckPos.x += (Math_SinS(this->actor.world.rot.y) * 25.0f) - this->scale;
        this->wallCheckPos.z += (Math_CosS(this->actor.world.rot.y) * 25.0f) - this->scale;
        this->wallCheckRadius = 25.0f - ((this->scale - 0.01f) * 1000.0f);
        Actor_SetScale(&this->actor, this->scale);
        Actor_MoveWithGravity(&this->actor);
        Actor_UpdateBgCheckInfo(play, &this->actor, 0, 15.0f, 10.0f,
                                UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_2 | UPDBGCHECKINFO_FLAG_4);

        if (this->actor.params != 2) {
            this->unk_2D4 = this->actor.floorHeight + (this->scale * 1000.0f);
            this->unk_2D8 = this->scale * 600.0f;
            if (this->actor.world.pos.y < this->unk_2D4) {
                this->actor.world.pos.y = this->unk_2D4 + 0.1f;
            }

            if (WaterBox_GetSurface1(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                                     &this->waterSurface, &sp6C)) {
                if ((this->waterSurface != BGCHECK_Y_MIN) &&
                    (this->waterSurface - this->unk_2D8 < this->actor.world.pos.y)) {
                    this->actor.world.pos.y = this->waterSurface - this->unk_2D8;
                }
            }

            if (!D_80B2B2E8 && (this->actionFunc == func_80B28C14)) {
                s32 temp_s0_2 = this->index * 2;
                f32 phi_f2 = 0.0f;
                f32 phi_f20 = 0;
                WaterBox* sp4C;

                if (WaterBox_GetSurface1(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z,
                                         &this->waterSurface, &sp4C)) {
                    phi_f20 = D_80B2B3A8[temp_s0_2] + (this->waterSurface - this->unk_2D8);
                    phi_f2 = D_80B2B3A8[temp_s0_2 + 1] + this->unk_2D4;
                }

                if ((phi_f20 < this->actor.world.pos.y) && (this->unk_348 < 0)) {
                    this->unk_348 = 0;
                    this->actor.velocity.y = 0;
                    this->actor.gravity = 0;
                }

                if ((this->actor.world.pos.y < phi_f2) && (this->unk_348 > 0)) {
                    this->unk_348 = 0;
                    this->actor.velocity.y = 0;
                    this->actor.gravity = 0;
                }
            }

            if (!D_80B2B2E8) {
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
    func_80B2B180(this, play);
}

void func_80B2ADB0(EnFish2* this, Vec3f* vec, s16 arg2) {
    s16 i;
    EnFish2UnkStruct* ptr = &this->unk_3F8[0];

    for (i = 0; i < ARRAY_COUNT(this->unk_3F8); i++, ptr++) {
        if (!ptr->unk_00) {
            TexturePtr texture;

            if (Rand_ZeroOne() < 0.5f) {
                texture = gEffBubble2Tex;
            } else {
                texture = gEffBubble1Tex;
            }

            ptr->unk_20 = (TexturePtr)OS_K0_TO_PHYSICAL(SEGMENTED_TO_K0(texture));
            ptr->unk_00 = true;
            ptr->unk_04 = *vec;
            ptr->unk_04.x += Rand_CenteredFloat(ptr->unk_00 + (this->scale * 4000.0f));
            ptr->unk_04.y += Rand_CenteredFloat(20.0f);
            ptr->unk_04.z += Rand_CenteredFloat(ptr->unk_00 + (this->scale * 4000.0f));
            ptr->unk_18 = (this->scale * 20.0f) - (Rand_ZeroFloat(5.0f) * 0.01f);
            ptr->unk_1C = 0x42;
            ptr->unk_10 = arg2;
            break;
        }
    }
}

void func_80B2AF80(EnFish2* this, PlayState* play) {
    EnFish2UnkStruct* ptr = &this->unk_3F8[0];
    WaterBox* waterBox;
    f32 sp8C;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(this->unk_3F8); i++, ptr++) {
        if (ptr->unk_00) {
            if ((ptr->unk_10 != 0) && (ptr->unk_1C != 0)) {
                Math_ApproachF(&ptr->unk_14, ptr->unk_18, 0.4f, 0.5f);
                ptr->unk_10--;
            } else {
                ptr->unk_00 = 0;
            }

            if (ptr->unk_00) {
                ptr->unk_04.x += (0.3f + (Rand_ZeroOne() * 0.5f)) - 0.55f;
                ptr->unk_04.y += 1.0f + ((Rand_ZeroOne() - 0.3f) * 1.2f);
                ptr->unk_04.z += (0.3f + (Rand_ZeroOne() * 0.5f)) - 0.55f;
                sp8C = ptr->unk_04.y;
                if (!WaterBox_GetSurface1(play, &play->colCtx, ptr->unk_04.x, ptr->unk_04.z, &sp8C, &waterBox)) {
                    ptr->unk_00 = 0;
                } else if (sp8C < ptr->unk_04.y) {
                    Vec3f sp7C;

                    sp7C.x = ptr->unk_04.x;
                    sp7C.y = sp8C;
                    sp7C.z = ptr->unk_04.z;
                    EffectSsGRipple_Spawn(play, &sp7C, 0, 80, 0);
                    ptr->unk_00 = 0;
                }
            }
        }
    }
}

void func_80B2B180(EnFish2* this, PlayState* play) {
    s16 i;
    EnFish2UnkStruct* ptr;
    GraphicsContext* gfxCtx = play->state.gfxCtx;

    OPEN_DISPS(gfxCtx);

    ptr = &this->unk_3F8[0];
    Gfx_SetupDL25_Opa(gfxCtx);

    for (i = 0; i < ARRAY_COUNT(this->unk_3F8); i++, ptr++) {
        if (ptr->unk_00) {
            Matrix_Translate(ptr->unk_04.x, ptr->unk_04.y, ptr->unk_04.z, MTXMODE_NEW);
            Matrix_Scale(ptr->unk_14, ptr->unk_14, ptr->unk_14, MTXMODE_APPLY);

            MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gfxCtx);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 150, 150, 150, 0);
            gSPSegment(POLY_OPA_DISP++, 0x08, ptr->unk_20);
            gSPDisplayList(POLY_OPA_DISP++, gEffBubbleDL);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
