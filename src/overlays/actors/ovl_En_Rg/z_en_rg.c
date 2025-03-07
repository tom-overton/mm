/*
 * File: z_en_rg.c
 * Overlay: ovl_En_Rg
 * Description: Racing Goron
 */
#include "z_en_rg.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_MINIMAP_ICON_ENABLED)

void EnRg_Init(Actor* thisx, PlayState* play);
void EnRg_Destroy(Actor* thisx, PlayState* play);
void EnRg_Update(Actor* thisx, PlayState* play);
void EnRg_Draw(Actor* thisx, PlayState* play);

void func_80BF4FC4(EnRg* this, PlayState* play);

s32 D_80BF5C10;

ActorProfile En_Rg_Profile = {
    /**/ ACTOR_EN_RG,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_OF1D_MAP,
    /**/ sizeof(EnRg),
    /**/ EnRg_Init,
    /**/ EnRg_Destroy,
    /**/ EnRg_Update,
    /**/ EnRg_Draw,
};

static ColliderSphereInit sSphereInit = {
    {
        COL_MATERIAL_METAL,
        AT_ON | AT_TYPE_PLAYER | AT_TYPE_ENEMY,
        AC_ON | AC_HARD | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_SPHERE,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0xF7CFFFFF, 0x00, 0x00 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 0, { { 0, 0, 0 }, 20 }, 100 },
};

static ColliderCylinderInit sCylinderInit = {
    {
        COL_MATERIAL_HIT1,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK1,
        { 0x00000000, 0x00, 0x00 },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_NONE | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_ON,
    },
    { 20, 62, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit2 sColChkInfoInit = { 0, 0, 0, 0, MASS_IMMOVABLE };

static DamageTable sDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(1, 0x0),
    /* Deku Stick     */ DMG_ENTRY(1, 0x0),
    /* Horse trample  */ DMG_ENTRY(1, 0x0),
    /* Explosives     */ DMG_ENTRY(1, 0x0),
    /* Zora boomerang */ DMG_ENTRY(1, 0x0),
    /* Normal arrow   */ DMG_ENTRY(1, 0x0),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, 0x0),
    /* Hookshot       */ DMG_ENTRY(1, 0x0),
    /* Goron punch    */ DMG_ENTRY(1, 0xF),
    /* Sword          */ DMG_ENTRY(1, 0x0),
    /* Goron pound    */ DMG_ENTRY(1, 0xF),
    /* Fire arrow     */ DMG_ENTRY(1, 0x0),
    /* Ice arrow      */ DMG_ENTRY(1, 0x0),
    /* Light arrow    */ DMG_ENTRY(1, 0x0),
    /* Goron spikes   */ DMG_ENTRY(1, 0xF),
    /* Deku spin      */ DMG_ENTRY(1, 0x0),
    /* Deku bubble    */ DMG_ENTRY(1, 0x0),
    /* Deku launch    */ DMG_ENTRY(1, 0x0),
    /* UNK_DMG_0x12   */ DMG_ENTRY(1, 0x0),
    /* Zora barrier   */ DMG_ENTRY(1, 0x0),
    /* Normal shield  */ DMG_ENTRY(0, 0x0),
    /* Light ray      */ DMG_ENTRY(0, 0x0),
    /* Thrown object  */ DMG_ENTRY(1, 0x0),
    /* Zora punch     */ DMG_ENTRY(1, 0x0),
    /* Spin attack    */ DMG_ENTRY(1, 0x0),
    /* Sword beam     */ DMG_ENTRY(0, 0x0),
    /* Normal Roll    */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, 0x0),
    /* Unblockable    */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, 0x0),
    /* Powder Keg     */ DMG_ENTRY(1, 0x0),
};

s32 sStartingPointPerSceneExitIndex[][4] = {
    { 0, 0, 0, 0 },     { 0, 0, 0, 0 },     { 0, 0, 0, 0 },     { 0, 0, 0, 0 },     { 1, 1, 1, 1 },
    { 11, 11, 8, 10 },  { 16, 12, 12, 14 }, { 19, 15, 15, 18 }, { 25, 17, 18, 22 }, { 29, 20, 23, 28 },
    { 39, 24, 28, 33 }, { 43, 27, 33, 37 }, { 46, 28, 36, 37 }, { 51, 30, 38, 39 }, { 54, 33, 42, 40 },
    { 56, 34, 44, 41 }, { 60, 38, 50, 45 }, { 67, 42, 55, 49 }, { 74, 47, 61, 54 },
};

typedef enum EnRgAnimation {
    /* -1 */ RG_ANIM_NONE = -1,
    /*  0 */ RG_ANIM_0,
    /*  1 */ RG_ANIM_1,
    /*  2 */ RG_ANIM_MAX
} EnRgAnimation;

static AnimationInfoS sAnimationInfo[RG_ANIM_MAX] = {
    { &gGoronUnrollAnim, 2.0f, 0, -1, ANIMMODE_ONCE, 0 },  // RG_ANIM_0
    { &gGoronUnrollAnim, -2.0f, 0, -1, ANIMMODE_ONCE, 0 }, // RG_ANIM_1
};

static TexturePtr sDustTextures[] = {
    gEffDust8Tex, gEffDust7Tex, gEffDust6Tex, gEffDust5Tex, gEffDust4Tex, gEffDust3Tex, gEffDust2Tex, gEffDust1Tex,
};

Color_RGBA8 D_80BF5954[] = {
    { 255, 255, 255, 0 },
    { 170, 130, 90, 0 },
    { 0, 0, 0, 0 },
};

Color_RGBA8 D_80BF5960[] = {
    { 255, 255, 255, 0 },
    { 100, 60, 20, 0 },
    { 0, 0, 0, 0 },
};

void EnRg_DrawDustEffects(EnRgEffect* effect, PlayState* play2) {
    PlayState* play = play2;
    f32 temp_f20;
    u8 phi_fp = false;
    s32 i;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    for (i = 0; i < 32; i++, effect++) {
        if ((effect->unk_00 < 4) || (effect->unk_00 >= 7)) {
            continue;
        }

        if (!phi_fp) {
            POLY_XLU_DISP = Gfx_SetupDL(POLY_XLU_DISP, SETUPDL_0);
            gSPDisplayList(POLY_XLU_DISP++, gGoronDustMaterialDL);
            phi_fp = true;
        }

        Matrix_Push();

        temp_f20 = (f32)effect->unk_02 / effect->unk_01;

        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, D_80BF5954[effect->unk_00 - 4].r, D_80BF5954[effect->unk_00 - 4].g,
                        D_80BF5954[effect->unk_00 - 4].b, (u8)(temp_f20 * 255.0f));
        gDPSetEnvColor(POLY_XLU_DISP++, D_80BF5960[effect->unk_00 - 4].r, D_80BF5960[effect->unk_00 - 4].g,
                       D_80BF5960[effect->unk_00 - 4].b, 0);

        Matrix_Translate(effect->pos.x, effect->pos.y, effect->pos.z, MTXMODE_NEW);
        Matrix_Scale(effect->scale, effect->scale, 1.0f, MTXMODE_APPLY);
        Matrix_ReplaceRotation(&play->billboardMtxF);

        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPSegment(POLY_XLU_DISP++, 0x08, Lib_SegmentedToVirtual(sDustTextures[(s32)(temp_f20 * 7.0f)]));
        gSPDisplayList(POLY_XLU_DISP++, gGoronDustModelDL);

        Matrix_Pop();
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void EnRg_UpdateEffects(EnRg* this) {
    EnRgEffect* effect = this->effects;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(this->effects); i++, effect++) {
        if (effect->unk_00) {
            if (!effect->unk_02) {
                effect->unk_00 = 0;
            } else {
                effect->pos.x += effect->velocity.x;
                effect->pos.y += effect->velocity.y;
                effect->pos.z += effect->velocity.z;
                effect->velocity.x += effect->accel.x;
                effect->velocity.y += effect->accel.y;
                effect->velocity.z += effect->accel.z;
                effect->scale += effect->unk_38;
                effect->unk_02--;
            }
        }
    }
}

void EnRg_DrawEffects(EnRg* this, PlayState* play) {
    EnRg_DrawDustEffects(this->effects, play);
}

void EnRg_UpdateRolledUpCollision(EnRg* this, PlayState* play) {
    this->colliderSphere.dim.worldSphere.center.x = this->actor.world.pos.x;
    this->colliderSphere.dim.worldSphere.center.y = this->actor.world.pos.y + this->actor.shape.yOffset;
    this->colliderSphere.dim.worldSphere.center.z = this->actor.world.pos.z;

    if (this->actor.speed >= 10.0f) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->colliderSphere.base);
    }

    CollisionCheck_SetAC(play, &play->colChkCtx, &this->colliderSphere.base);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->colliderSphere.base);
}

void EnRg_UpdateStandingCollision(EnRg* this, PlayState* play) {
    Math_Vec3f_ToVec3s(&this->colliderCylinder.dim.pos, &this->actor.world.pos);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->colliderCylinder.base);
}

void EnRg_UpdateCollision(EnRg* this, PlayState* play) {
    if (this->flags & ENRG_FLAG_ROLLED_UP) {
        EnRg_UpdateRolledUpCollision(this, play);
    } else {
        EnRg_UpdateStandingCollision(this, play);
    }
}

s32 EnRg_SetCollisionFlags(EnRg* this, PlayState* play) {
    this->flags &= ~ENRG_FLAG_AT_HIT;
    this->flags &= ~ENRG_FLAG_AC_HIT;
    this->flags &= ~ENRG_FLAG_OC1_HIT;

    if ((this->colliderCylinder.base.atFlags & AT_HIT) || (this->colliderSphere.base.atFlags & AT_HIT)) {
        this->colliderCylinder.base.atFlags &= ~AT_HIT;
        this->colliderSphere.base.atFlags &= ~AT_HIT;
        this->flags |= ENRG_FLAG_AT_HIT;
    }

    if ((this->colliderCylinder.base.acFlags & AC_HIT) || (this->colliderSphere.base.acFlags & AC_HIT)) {
        this->colliderCylinder.base.acFlags &= ~AC_HIT;
        this->colliderSphere.base.acFlags &= ~AC_HIT;
        this->flags |= ENRG_FLAG_AC_HIT;
    }

    if ((this->colliderCylinder.base.ocFlags1 & OC1_HIT) || (this->colliderSphere.base.ocFlags1 & OC1_HIT)) {
        this->colliderCylinder.base.ocFlags1 &= ~OC1_HIT;
        this->colliderSphere.base.ocFlags1 &= ~OC1_HIT;
        this->flags |= ENRG_FLAG_OC1_HIT;
    }

    return false;
}

void EnRg_UpdateSkelAnime(EnRg* this) {
    this->skelAnime.playSpeed = this->animPlaySpeed;
    SkelAnime_Update(&this->skelAnime);
}

s32 func_80BF4024(EnRg* this, PlayState* play) {
    if ((play->csCtx.state == CS_STATE_IDLE) && (this->animIndex == RG_ANIM_1)) {
        if (Animation_OnFrame(&this->skelAnime, 2.0f)) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_GOLON_CIRCLE);
        }

        if (Animation_OnFrame(&this->skelAnime, 22.0f)) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_GOLON_SIT_IMT);
        }
    }

    return false;
}

s32 EnRg_ChangeAnim(EnRg* this, s32 animIndex) {
    s32 didAnimChange = false;

    if (this->animIndex != animIndex) {
        this->animIndex = animIndex;
        didAnimChange = SubS_ChangeAnimationByInfoS(&this->skelAnime, sAnimationInfo, animIndex);
        this->animPlaySpeed = this->skelAnime.playSpeed;
    }

    return didAnimChange;
}

void EnRg_Blink(EnRg* this) {
    if (DECR(this->blinkTimer) == 0) {
        this->eyeIndex++;

        if (this->eyeIndex >= 4) {
            this->blinkTimer = Rand_S16Offset(30, 30);
            this->eyeIndex = 0;
        }
    }
}

s32 EnRg_UpdateTireMark(EnRg* this, PlayState* play) {
    if ((this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && (this->actor.speed >= 0.01f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_GOLON_ROLLING - SFX_FLAG);
        func_800AE930(&play->colCtx, Effect_GetByIndex(this->tireMarkEffectIndex), &this->actor.world.pos, 18.0f,
                      this->actor.shape.rot.y, this->actor.floorPoly, this->actor.floorBgId);
    } else {
        func_800AEF44(Effect_GetByIndex(this->tireMarkEffectIndex));
    }

    return false;
}

s32 EnRg_IsValidTargetActor(EnRg* this, PlayState* play, Actor* targetActor) {
    CollisionPoly* wallPoly = NULL;
    s32 bgId;
    Vec3f wallPos;

    if (Actor_ActorAIsFacingAndNearActorB(&this->actor, targetActor, 400.0f, 0x2000) &&
        !BgCheck_EntityLineTest1(&play->colCtx, &this->actor.world.pos, &targetActor->world.pos, &wallPos, &wallPoly,
                                 true, false, false, true, &bgId)) {
        return true;
    }

    return false;
}

s32 func_80BF42BC(EnRg* this, f32 targetSpeed) {
    f32 sp2C;
    s32 sp24;

    Math_ApproachF(&this->actor.speed, targetSpeed, 0.3f, 0.5f);

    sp2C = this->actor.speed * (1.0f / 26);

    if (sp2C > 1.0f) {
        sp2C = 1.0f;
    }

    sp24 = 5460.0f * sp2C;

    this->actor.scale.x = 0.01f - (Math_SinS(sp24) * 0.01f);
    this->actor.scale.y = 0.01f + (Math_SinS(sp24) * 0.01f);
    this->actor.scale.z = 0.01f + (Math_SinS(sp24) * 0.01f);

    this->actor.shape.yOffset = this->actor.scale.y * 100.0f * 14.0f;

    if (!(this->flags & ENRG_FLAG_CROSSED_FINISH_LINE)) {
        this->actor.shape.rot.x += TRUNCF_BINANG(0x3552 * sp2C);
    }

    this->actor.world.rot.x = this->actor.shape.rot.x;
    return false;
}

s32 EnRg_GetCurrentPoint(EnRg* this) {
    Vec3f prevPointPos;
    Vec3f nextPointPos;
    f32 perpendicularPointX;
    f32 perpendicularPointZ;
    f32 lineLenSq;
    f32 minLineLengthSq = 0.0f;
    s32 pathIndex = ENRG_GET_PATH_INDEX(&this->actor);
    s32 currentPoint = -1;
    s32 sceneExitIndex = this->sceneExitIndex;
    s32 hasSetCurrentPointOnce = false;
    s32 pointIterator = sStartingPointPerSceneExitIndex[this->sceneExitIndex][pathIndex];

    do {
        SubS_CopyPointFromPathCheckBounds(this->path, pointIterator - 1, &prevPointPos);
        SubS_CopyPointFromPathCheckBounds(this->path, pointIterator + 1, &nextPointPos);

        if (Math3D_PointDistSqToLine2DImpl(this->actor.world.pos.x, this->actor.world.pos.z, prevPointPos.x,
                                           prevPointPos.z, nextPointPos.x, nextPointPos.z, &perpendicularPointX,
                                           &perpendicularPointZ, &lineLenSq) &&
            (!hasSetCurrentPointOnce || ((currentPoint + 1) == pointIterator) || (lineLenSq < minLineLengthSq))) {
            hasSetCurrentPointOnce = true;
            minLineLengthSq = lineLenSq;
            currentPoint = pointIterator;
        }

        pointIterator++;
    } while ((sceneExitIndex != 18) &&
             (pointIterator < sStartingPointPerSceneExitIndex[sceneExitIndex + 1][pathIndex]));

    return currentPoint;
}

s32 EnRg_GetSceneExitIndex(EnRg* this, PlayState* play) {
    // The Goron Racetrack is configured such that the sceneExitIndex for any given floor polygon gradually increases as
    // you move forward through the racetrack.
    s32 sceneExitIndex = SurfaceType_GetSceneExitIndex(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId);

    //! @bug This check does not protect against `sceneExitIndex` being 18, which will cause an out-of-bounds access to
    //! `sStartingPointPerSceneExitIndex` in `EnRg_GetCurrentPoint` due to the `sceneExitIndex + 1` access.
    if ((sceneExitIndex < 4) || (sceneExitIndex >= 19)) {
        sceneExitIndex = -1;
    }

    return sceneExitIndex;
}

s32 EnRg_CheckForWallOrAcCollisions(EnRg* this) {
    s32 sp24 = 0;
    s32 ret = false;
    s16 yawDiff;
    s16 yaw;

    if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) && (this->actor.speed >= 5.0f)) {
        yawDiff = this->actor.world.rot.y - BINANG_ROT180(this->actor.wallYaw);

        if (ABS_ALT(yawDiff) >= 0x3400) {
            this->actor.world.rot.y = BINANG_ROT180(BINANG_ROT180(this->actor.wallYaw) - yawDiff);
            this->collisionTimer = 10;
        } else if (ABS_ALT(yawDiff) >= 0x1000) {
            this->actor.world.rot.y = BINANG_ROT180(BINANG_ROT180(this->actor.wallYaw) - yawDiff);
            this->actor.speed *= 0.75f;
            this->collisionTimer = 10;
        } else {
            this->actor.world.rot.y = BINANG_ROT180(this->actor.world.rot.y);
            this->actor.speed *= 0.25f;
            ret = true;
        }

        sp24 = 1;
    } else if (this->flags & ENRG_FLAG_AC_HIT) {
        yaw = Actor_WorldYawTowardActor(&this->actor, ((void)0, this->colliderSphere.base.ac));
        sp24 = 2;

        if (this->actor.colorFilterTimer == 0) {
            this->actor.speed *= 0.5f;

            if ((s16)(yaw - this->actor.world.rot.y) > 0) {
                this->actor.world.rot.y -= 0x2000;
            } else {
                this->actor.world.rot.y += 0x2000;
            }
        }

        Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_OPA, 40);
        this->collisionTimer = 10;
    }

    if (sp24) {
        this->flags &= ~ENRG_FLAG_11;
        this->flags &= ~ENRG_FLAG_AC_HIT;

        if ((this->targetActor != NULL) && (this->targetActor->id == ACTOR_PLAYER)) {
            D_80BF5C10 = false;
        }

        this->targetActor = NULL;
        this->treeCollisionTimer = 0;
    }

    return ret;
}

s32 func_80BF47AC(EnRg* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 targetSpeed;
    f32 boostSpeed;
    f32 baseSpeed;

    if (player->speedXZ < 20.0f) {
        baseSpeed = 20.0f;
    } else {
        baseSpeed = player->speedXZ;
    }

    if ((this->flags & ENRG_FLAG_BOUNCING_IN_AIR) || (this->flags & ENRG_FLAG_CROSSED_FINISH_LINE)) {
        targetSpeed = 0.0f;
    } else if (this->numCheckpointsAheadOfPlayer >= 2) {
        targetSpeed = baseSpeed * 0.5f;
    } else if (this->numCheckpointsAheadOfPlayer == 1) {
        targetSpeed = baseSpeed * 0.75f;
    } else if (this->numCheckpointsAheadOfPlayer == 0) {
        s16 yawDiff = this->actor.yawTowardsPlayer - this->actor.world.rot.y;

        if ((ABS_ALT(yawDiff) > 0x4000) || (this->unk_326 > 0)) {
            targetSpeed = baseSpeed * 0.72f;
        } else {
            targetSpeed = baseSpeed * 0.94f;
        }
    } else if (this->numCheckpointsAheadOfPlayer == -1) {
        targetSpeed = baseSpeed * 1.6f;
    } else {
        targetSpeed = baseSpeed * 2.0f;
    }

    if (targetSpeed > 0.0f) {
        DECR(this->boostTimer);
        boostSpeed = this->boostTimer * 0.01f;
        targetSpeed += boostSpeed;
    }

    func_80BF42BC(this, targetSpeed);

    return false;
}

void func_80BF4934(EnRg* this) {
    if (this->timer == 1) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_GOLON_DASH);
    }
}

void EnRg_UpdatePath(EnRg* this) {
    Vec3s* points;
    Vec3f targetPos;

    if (this->path != NULL) {
        points = Lib_SegmentedToVirtual(this->path->points);

        if (SubS_HasReachedPoint(&this->actor, this->path, this->currentPoint)) {
            if ((this->path->count - 1) < (this->currentPoint + 1)) {
                this->currentPoint = this->path->count - 1;
            } else {
                this->currentPoint++;
            }
        }

        DECR(this->collisionTimer);
        DECR(this->treeCollisionTimer);
        DECR(this->unk_326);

        if (!(this->flags & ENRG_FLAG_BOUNCING_IN_AIR) && !(this->flags & ENRG_FLAG_CROSSED_FINISH_LINE) &&
            (this->collisionTimer == 0) && (this->treeCollisionTimer == 0) &&
            (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
            if (this->targetActor == NULL) {
                Math_Vec3s_ToVec3f(&targetPos, &points[this->currentPoint]);
            } else {
                Math_Vec3f_Copy(&targetPos, &this->targetActor->world.pos);
            }

            Math_ApproachS(&this->actor.world.rot.y, Math_Vec3f_Yaw(&this->actor.world.pos, &targetPos), 10, 0x71C);
        }
    }
}

void func_80BF4AB8(EnRg* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Actor* actorIter = NULL;

    if (!(this->flags & ENRG_FLAG_11)) {
        if (this->boostTimer == 0) {
            do {
                actorIter = SubS_FindActor(play, actorIter, ACTORCAT_PROP, ACTOR_OBJ_TSUBO);
                if (actorIter != NULL) {
                    if (EnRg_IsValidTargetActor(this, play, actorIter) && (actorIter->update != NULL)) {
                        this->targetActor = actorIter;
                        this->flags |= ENRG_FLAG_11;
                        break;
                    }
                    actorIter = actorIter->next;
                }
            } while (actorIter != NULL);
        }

        if ((actorIter == NULL) && !D_80BF5C10 && (this->unk_326 == 0) && (player->stateFlags3 & PLAYER_STATE3_80000) &&
            (player->invincibilityTimer == 0) && EnRg_IsValidTargetActor(this, play, &player->actor)) {
            this->targetActor = &player->actor;
            this->flags |= ENRG_FLAG_11;
            D_80BF5C10 = true;
        }
    } else if ((this->targetActor != NULL) && !EnRg_IsValidTargetActor(this, play, this->targetActor)) {
        if (this->targetActor->id == ACTOR_PLAYER) {
            D_80BF5C10 = false;
        }

        this->targetActor = NULL;
        this->flags &= ~ENRG_FLAG_11;
    }

    if (this->flags & ENRG_FLAG_AT_HIT) {
        if (this->colliderSphere.base.at != NULL) {
            if (this->colliderSphere.base.at->id == ACTOR_OBJ_TSUBO) {
                this->colliderSphere.base.at->params |= 0x3F;
                this->boostTimer += 400;
                this->boostTimer = CLAMP_MAX(this->boostTimer, 400);
            } else if (this->colliderSphere.base.at->id == ACTOR_PLAYER) {
                this->unk_326 = 40;

                if (player->stateFlags3 & PLAYER_STATE3_1000) {
                    player->speedXZ *= 0.5f;
                    player->unk_B08 = player->speedXZ;
                    player->unk_B0C += player->speedXZ * 0.05f;

                    if (BINANG_SUB(this->actor.yawTowardsPlayer, player->yaw) > 0) {
                        player->yaw += 0x2000;
                    } else {
                        player->yaw -= 0x2000;
                    }

                    player->unk_B8C = 4;
                    player->invincibilityTimer = 20;
                    player->actor.home.rot.y = player->actor.world.rot.y;
                    player->actor.shape.rot.y = player->actor.world.rot.y;
                }

                if ((this->targetActor != NULL) && (this->targetActor->id == ACTOR_PLAYER)) {
                    D_80BF5C10 = false;
                }
            }
        }

        this->targetActor = NULL;
        this->flags &= ~ENRG_FLAG_11;
        this->flags &= ~ENRG_FLAG_AT_HIT;
    }
}

/**
 * Returns true if the specified position is in the finish line.
 * The range extends a little bit beyond the finish line's in-game visual.
 */
s32 EnRg_IsInFinishLine(Vec3f* pos) {
    return Math3D_PointInSquare2D(-1261.0f, -901.0f, -1600.0f, -1520.0f, pos->x, pos->z);
}

Vec3f sGoronRacetrackTreePositions[] = {
    { -2473.0f, 39.0f, 7318.0f },
    { -2223.0f, 142.0f, 7184.0f },
    { -2281.0f, 41.0f, 7718.0f },
    //! @bug There is no tree present at this location in the final game, so the Goron can collide with thin air
    { -2136.0f, 96.0f, 7840.0f },
    { -2432.0f, 6.0f, 7857.0f },
    { -2412.0f, 139.0f, 6872.0f },
    { -2719.0f, 39.0f, 7110.0f },
    { -2289.0f, 67.0f, 7463.0f },
    { -2820.0f, 85.0f, 6605.0f },
    { -2088.0f, 160.0f, 7584.0f },
    { -2503.0f, 1.0f, 7643.0f },
};

s32 EnRg_CheckForTreeCollisions(EnRg* this) {
    s32 pad[4];
    s32 ret = false;
    f32 distToTreeXZ;
    s16 yawDiff;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sGoronRacetrackTreePositions); i++) {
        distToTreeXZ = Actor_WorldDistXZToPoint(&this->actor, &sGoronRacetrackTreePositions[i]);
        yawDiff = Actor_WorldYawTowardPoint(&this->actor, &sGoronRacetrackTreePositions[i]) - this->actor.world.rot.y;

        if ((distToTreeXZ < 100.0f) && (ABS_ALT(yawDiff) < 0xC00)) {
            if (yawDiff > 0) {
                this->actor.world.rot.y -= 0x1600;
            } else {
                this->actor.world.rot.y += 0x1600;
            }

            this->treeCollisionTimer = 20;
            this->flags &= ~ENRG_FLAG_11;

            if ((this->targetActor != NULL) && (this->targetActor->id == ACTOR_PLAYER)) {
                D_80BF5C10 = false;
            }

            this->targetActor = NULL;
            ret = true;
            break;
        }
    }

    return ret;
}

void func_80BF4EBC(EnRg* this, PlayState* play) {
    if (this->flags & ENRG_FLAG_8) {
        if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
            this->flags &= ~ENRG_FLAG_8;
            this->flags |= ENRG_FLAG_ROLLED_UP;
            this->boostTimer = 0;
            this->actor.shape.yOffset = 14.0f;
            this->actionFunc = func_80BF4FC4;
        }
    } else if (CHECK_WEEKEVENTREG(WEEKEVENTREG_GORON_RACE_INTRO_CS_WATCHED)) {
        if (DECR(this->timer) == 0) {
            EnRg_ChangeAnim(this, RG_ANIM_1);
            this->flags &= ~ENRG_FLAG_3;
            this->flags &= ~ENRG_FLAG_ROLLED_UP;
            this->flags |= ENRG_FLAG_8;
            this->actor.shape.yOffset = 0.0f;
            this->timer = Rand_S16Offset(0, 20);
        }
    }

    SubS_UpdateFidgetTables(play, this->fidgetTableY, this->fidgetTableZ, ENRG_FIDGET_TABLE_LEN);
}

void func_80BF4FC4(EnRg* this, PlayState* play) {
    this->sceneExitIndex = EnRg_GetSceneExitIndex(this, play);

    if (!Play_InCsMode(play)) {
        if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) {
            if (this->flags & ENRG_FLAG_BOUNCING_IN_AIR) {
                this->flags &= ~ENRG_FLAG_BOUNCING_IN_AIR;
                this->actor.speed = 0.0f;
            }

            if (this->sceneExitIndex != -1) {
                this->currentPoint = EnRg_GetCurrentPoint(this);

                if (this->currentPoint >= 0) {
                    this->currentPoint++;
                }
            }
        } else if ((this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && !(this->flags & ENRG_FLAG_CROSSED_FINISH_LINE) &&
                   EnRg_IsInFinishLine(&this->actor.world.pos)) {
            this->flags |= ENRG_FLAG_CROSSED_FINISH_LINE;
        }

        if ((this->targetActor != NULL) && (this->targetActor->update == NULL)) {
            this->targetActor = NULL;
            this->flags &= ~ENRG_FLAG_11;
        }

        if (CHECK_EVENTINF(EVENTINF_10)) {
            if (DECR(this->timer) == 0) {
                func_80BF47AC(this, play);

                if ((this->treeCollisionTimer == 0) && !EnRg_CheckForTreeCollisions(this)) {
                    func_80BF4AB8(this, play);
                }

                if (EnRg_CheckForWallOrAcCollisions(this)) {
                    this->flags |= ENRG_FLAG_BOUNCING_IN_AIR;
                    this->actor.velocity.y = 7.0f;
                }

                EnRg_UpdatePath(this);
                Actor_UpdateVelocityWithGravity(&this->actor);
                Actor_UpdatePos(&this->actor);
                EnRg_UpdateTireMark(this, play);
                return;
            }
        }

        func_80BF4934(this);
    }
}

void EnRg_Init(Actor* thisx, PlayState* play) {
    static EffectTireMarkInit sTireMarkInit = { 0, 62, { 0, 0, 15, 100 } };
    EnRg* this = (EnRg*)thisx;

    if (gSaveContext.save.entrance == ENTRANCE(GORON_RACETRACK, 1)) {
        ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 20.0f);
        SkelAnime_InitFlex(play, &this->skelAnime, &gGoronSkel, NULL, this->jointTable, this->morphTable,
                           GORON_LIMB_MAX);

        this->animIndex = RG_ANIM_NONE;
        EnRg_ChangeAnim(this, RG_ANIM_0);
        this->skelAnime.curFrame = this->skelAnime.endFrame;

        Collider_InitAndSetSphere(play, &this->colliderSphere, &this->actor, &sSphereInit);
        Collider_InitAndSetCylinder(play, &this->colliderCylinder, &this->actor, &sCylinderInit);
        this->colliderSphere.dim.worldSphere.radius = this->colliderSphere.dim.modelSphere.radius;
        CollisionCheck_SetInfo2(&this->actor.colChkInfo, &sDamageTable, &sColChkInfoInit);

        Effect_Add(play, &this->tireMarkEffectIndex, EFFECT_TIRE_MARK, 0, 0, &sTireMarkInit);

        this->path = SubS_GetDayDependentPath(play, ENRG_GET_PATH_INDEX(&this->actor), ENRG_PATH_INDEX_NONE,
                                              &this->currentPoint);
        if (this->path != NULL) {
            this->currentPoint = 1;
        }

        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
        this->flags = ENRG_FLAG_3;
        this->actor.gravity = -1.0f;

        // This is the only usage of this function whose actor does not use `SubS_Offer`.
        // Since these bits go unused, it seems like a copy paste that still used `SubSOfferMode`
        SubS_SetOfferMode(&this->flags, SUBS_OFFER_MODE_ONSCREEN, SUBS_OFFER_MODE_MASK);

        if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_GORON_RACE_INTRO_CS_WATCHED)) {
            this->timer = Rand_S16Offset(30, 30);
        }

        this->blinkTimer = 0;
        this->eyeIndex = 0;
        this->actionFunc = func_80BF4EBC;
    } else {
        Actor_Kill(&this->actor);
    }
}

void EnRg_Destroy(Actor* thisx, PlayState* play) {
    EnRg* this = (EnRg*)thisx;

    Collider_DestroyCylinder(play, &this->colliderCylinder);
    Collider_DestroySphere(play, &this->colliderSphere);
    Effect_Destroy(play, this->tireMarkEffectIndex);
}

void EnRg_Update(Actor* thisx, PlayState* play) {
    EnRg* this = (EnRg*)thisx;

    EnRg_SetCollisionFlags(this, play);
    this->actionFunc(this, play);
    this->actor.shape.rot.y = this->actor.world.rot.y;

    if (!(this->flags & ENRG_FLAG_ROLLED_UP)) {
        EnRg_Blink(this);
        EnRg_UpdateSkelAnime(this);
        func_80BF4024(this, play);
    }

    Actor_UpdateBgCheckInfo(play, &this->actor, 30.0f, 20.0f, 0.0f,
                            UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4 | UPDBGCHECKINFO_FLAG_8 |
                                UPDBGCHECKINFO_FLAG_10);

    if (this->actor.floorHeight <= BGCHECK_Y_MIN) {
        Math_Vec3f_Copy(&this->actor.world.pos, &this->actor.prevPos);
    }

    EnRg_UpdateCollision(this, play);

    if (!Play_InCsMode(play)) {
        EnRg_UpdateEffects(this);
    }
}

void EnRg_DrawRolledUp(EnRg* this, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y + this->actor.shape.yOffset,
                     this->actor.world.pos.z, MTXMODE_NEW);
    Matrix_RotateYS(this->actor.shape.rot.y, MTXMODE_APPLY);
    Matrix_Translate(0.0f, -this->actor.shape.yOffset, 0.0f, MTXMODE_APPLY);
    Matrix_RotateZS(this->actor.shape.rot.z, MTXMODE_APPLY);
    Matrix_Translate(0.0f, this->actor.shape.yOffset, 0.0f, MTXMODE_APPLY);
    Matrix_Scale(this->actor.scale.x, this->actor.scale.y, this->actor.scale.z, MTXMODE_APPLY);
    Matrix_RotateXS(this->actor.shape.rot.x, MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gGoronRolledUpDL);

    CLOSE_DISPS(play->state.gfxCtx);
}

s32 EnRg_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    EnRg* this = (EnRg*)thisx;
    s32 fidgetIndex;

    switch (limbIndex) {
        case GORON_LIMB_BODY:
            fidgetIndex = 0;
            break;

        case GORON_LIMB_LEFT_UPPER_ARM:
            fidgetIndex = 1;
            break;

        case GORON_LIMB_RIGHT_UPPER_ARM:
            fidgetIndex = 2;
            break;

        default:
            fidgetIndex = 9;
            break;
    }

    if ((this->flags & ENRG_FLAG_3) && (fidgetIndex < 9)) {
        rot->y += TRUNCF_BINANG(Math_SinS(this->fidgetTableY[fidgetIndex]) * 200.0f);
        rot->z += TRUNCF_BINANG(Math_CosS(this->fidgetTableZ[fidgetIndex]) * 200.0f);
    }

    return false;
}

static TexturePtr sEyeTextures[] = {
    gGoronEyeOpenTex, gGoronEyeHalfTex, gGoronEyeClosedTex, gGoronEyeHalfTex, gGoronEyeClosed2Tex,
};

void EnRg_Draw(Actor* thisx, PlayState* play) {
    EnRg* this = (EnRg*)thisx;

    if (!(this->flags & ENRG_FLAG_ROLLED_UP)) {
        OPEN_DISPS(play->state.gfxCtx);

        Gfx_SetupDL25_Opa(play->state.gfxCtx);

        gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(sEyeTextures[this->eyeIndex]));

        SkelAnime_DrawFlexOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount,
                              EnRg_OverrideLimbDraw, NULL, &this->actor);

        CLOSE_DISPS(play->state.gfxCtx);
    } else {
        EnRg_DrawRolledUp(this, play);
    }

    EnRg_DrawEffects(this, play);
}
