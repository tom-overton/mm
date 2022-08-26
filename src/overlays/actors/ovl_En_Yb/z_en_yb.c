/*
 * File: z_en_yb.c
 * Overlay: ovl_En_Yb
 * Description: Kamaro the Dancing Ghost
 */

#include "z_en_yb.h"
#include "objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_1 | ACTOR_FLAG_8 | ACTOR_FLAG_10 | ACTOR_FLAG_2000000)

#define THIS ((EnYb*)thisx)

void EnYb_Init(Actor* thisx, PlayState* play);
void EnYb_Destroy(Actor* thisx, PlayState* play);
void EnYb_Update(Actor* thisx, PlayState* play);
void EnYb_Draw(Actor* thisx, PlayState* play);

void EnYb_Idle(EnYb* this, PlayState* play);
void EnYb_TeachingDanceFinish(EnYb* this, PlayState* play);
void EnYb_SetupLeaving(EnYb* this, PlayState* play);

void EnYb_UpdateAnimation(EnYb* this, PlayState* play);
void EnYb_FinishTeachingCutscene(EnYb* this);
void EnYb_Disappear(EnYb* this, PlayState* play);
void EnYb_ReceiveMask(EnYb* this, PlayState* play);
void EnYb_Talk(EnYb* this, PlayState* play);
void EnYb_TeachingDance(EnYb* this, PlayState* play);

void EnYb_ChangeAnim(PlayState* play, EnYb* this, s16 animIndex, u8 animMode, f32 morphFrames);
s32 EnYb_CanTalk(EnYb* this, PlayState* play);

const ActorInit En_Yb_InitVars = {
    ACTOR_EN_YB,
    ACTORCAT_NPC,
    FLAGS,
    OBJECT_YB,
    sizeof(EnYb),
    (ActorFunc)EnYb_Init,
    (ActorFunc)EnYb_Destroy,
    (ActorFunc)EnYb_Update,
    (ActorFunc)EnYb_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COLTYPE_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_ENEMY,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        TOUCH_NONE | TOUCH_SFX_NORMAL,
        BUMP_ON,
        OCELEM_ON,
    },
    { 20, 40, 0, { 0, 0, 0 } },
};

// crashes if I try to mod it in to look at it
//  assumption: draw uses two different skeleton functions, might be incompatible
static AnimationHeader* gYbUnusedAnimations[] = { &object_yb_Anim_000200 };

static LinkAnimationHeader* gLinkAnimations[] = {
    &gameplay_keep_Linkanim_00CF40, &gameplay_keep_Linkanim_00CF48, &gameplay_keep_Linkanim_00CF50,
    &gameplay_keep_Linkanim_00CF58, &gameplay_keep_Linkanim_00CF60, &gameplay_keep_Linkanim_00CF68,
    &gameplay_keep_Linkanim_00CF70, &gameplay_keep_Linkanim_00CF78, &gameplay_keep_Linkanim_00CF80,
    &gameplay_keep_Linkanim_00CF88, &gameplay_keep_Linkanim_00CF90, &gameplay_keep_Linkanim_00CF98,
    &gameplay_keep_Linkanim_00CFA0, &gameplay_keep_Linkanim_00CFA8, &gameplay_keep_Linkanim_00CFB0,
    &gameplay_keep_Linkanim_00CFB8, &gameplay_keep_Linkanim_00CFC0, &gameplay_keep_Linkanim_00CFC8,
    &gameplay_keep_Linkanim_00CFD0, &gameplay_keep_Linkanim_00CFD8, &gameplay_keep_Linkanim_00CFE0,
    &gameplay_keep_Linkanim_00CFE8, &gameplay_keep_Linkanim_00CFF0, &gameplay_keep_Linkanim_00CFF8,
    &gameplay_keep_Linkanim_00D000, &gameplay_keep_Linkanim_00D008, &gameplay_keep_Linkanim_00D010,
    &gameplay_keep_Linkanim_00D018, &gameplay_keep_Linkanim_00D020, &gameplay_keep_Linkanim_00D028,
    &gameplay_keep_Linkanim_00D030, &gameplay_keep_Linkanim_00D038, &gameplay_keep_Linkanim_00D040,
    &gameplay_keep_Linkanim_00D048, &gameplay_keep_Linkanim_00D050, &gameplay_keep_Linkanim_00D058,
    &gameplay_keep_Linkanim_00D060, &gameplay_keep_Linkanim_00D068, &gameplay_keep_Linkanim_00D070,
    &gameplay_keep_Linkanim_00D078, &gameplay_keep_Linkanim_00D080, &gameplay_keep_Linkanim_00D088,
    &gameplay_keep_Linkanim_00D090, &gameplay_keep_Linkanim_00D098, &gameplay_keep_Linkanim_00D0A0,
    &gameplay_keep_Linkanim_00D0A8, &gameplay_keep_Linkanim_00D0B0, &gameplay_keep_Linkanim_00D0B8,
    &gameplay_keep_Linkanim_00D0C0, &gameplay_keep_Linkanim_00D0C8, &gameplay_keep_Linkanim_00D0D0,
    &gameplay_keep_Linkanim_00D0D8, &gameplay_keep_Linkanim_00D0E0, &gameplay_keep_Linkanim_00D0E8,
    &gameplay_keep_Linkanim_00D0F0, &gameplay_keep_Linkanim_00D0F8, &gameplay_keep_Linkanim_00D100,
    &gameplay_keep_Linkanim_00D108, &gameplay_keep_Linkanim_00D110, &gameplay_keep_Linkanim_00D118,
    &gameplay_keep_Linkanim_00D120, &gameplay_keep_Linkanim_00D128, &gameplay_keep_Linkanim_00D130,
    &gameplay_keep_Linkanim_00D138, &gameplay_keep_Linkanim_00D140, &gameplay_keep_Linkanim_00D148,
    &gameplay_keep_Linkanim_00D150, &gameplay_keep_Linkanim_00D158, &gameplay_keep_Linkanim_00D160,
    &gameplay_keep_Linkanim_00D168, &gameplay_keep_Linkanim_00D170, &gameplay_keep_Linkanim_00D178,
    &gameplay_keep_Linkanim_00D180, &gameplay_keep_Linkanim_00D188, &gameplay_keep_Linkanim_00D190,
    &gameplay_keep_Linkanim_00D198, &gameplay_keep_Linkanim_00D1A0, &gameplay_keep_Linkanim_00D1A8,
    &gameplay_keep_Linkanim_00D1B0, &gameplay_keep_Linkanim_00D1B8, &gameplay_keep_Linkanim_00D1C0,
    &gameplay_keep_Linkanim_00D1C8, &gameplay_keep_Linkanim_00D1D0, &gameplay_keep_Linkanim_00D1D8,
    &gameplay_keep_Linkanim_00D1E0, &gameplay_keep_Linkanim_00D1E8, &gameplay_keep_Linkanim_00D1F0,
    &gameplay_keep_Linkanim_00D1F8, &gameplay_keep_Linkanim_00D200, &gameplay_keep_Linkanim_00D208,
    &gameplay_keep_Linkanim_00D210, &gameplay_keep_Linkanim_00D218, &gameplay_keep_Linkanim_00D220,
    &gameplay_keep_Linkanim_00D228, &gameplay_keep_Linkanim_00D230, &gameplay_keep_Linkanim_00D238,
    &gameplay_keep_Linkanim_00D240, &gameplay_keep_Linkanim_00D248, &gameplay_keep_Linkanim_00D250,
    &gameplay_keep_Linkanim_00D258, &gameplay_keep_Linkanim_00D260, &gameplay_keep_Linkanim_00D268,
    &gameplay_keep_Linkanim_00D270, &gameplay_keep_Linkanim_00D278, &gameplay_keep_Linkanim_00D280,
    &gameplay_keep_Linkanim_00D288, &gameplay_keep_Linkanim_00D290, &gameplay_keep_Linkanim_00D298,
    &gameplay_keep_Linkanim_00D2A0, &gameplay_keep_Linkanim_00D2A8, &gameplay_keep_Linkanim_00D2B0,
    &gameplay_keep_Linkanim_00D2B8, &gameplay_keep_Linkanim_00D2C0, &gameplay_keep_Linkanim_00D2C8,
    &gameplay_keep_Linkanim_00D2D0, &gameplay_keep_Linkanim_00D2D8, &gameplay_keep_Linkanim_00D2E0,
    &gameplay_keep_Linkanim_00D2E8, &gameplay_keep_Linkanim_00D2F0, &gameplay_keep_Linkanim_00D2F8,
    &gameplay_keep_Linkanim_00D300, &gameplay_keep_Linkanim_00D308, &gameplay_keep_Linkanim_00D310,
    &gameplay_keep_Linkanim_00D318, &gameplay_keep_Linkanim_00D320, &gameplay_keep_Linkanim_00D328,
    &gameplay_keep_Linkanim_00D330, &gameplay_keep_Linkanim_00D338, &gameplay_keep_Linkanim_00D340,
    &gameplay_keep_Linkanim_00D348, &gameplay_keep_Linkanim_00D350, &gameplay_keep_Linkanim_00D358,
    &gameplay_keep_Linkanim_00D360, &gameplay_keep_Linkanim_00D368, &gameplay_keep_Linkanim_00D370,
    &gameplay_keep_Linkanim_00D378, &gameplay_keep_Linkanim_00D380, &gameplay_keep_Linkanim_00D388,
    &gameplay_keep_Linkanim_00D390, &gameplay_keep_Linkanim_00D398, &gameplay_keep_Linkanim_00D3A0,
    &gameplay_keep_Linkanim_00D3A8, &gameplay_keep_Linkanim_00D3B0, &gameplay_keep_Linkanim_00D3B8,
    &gameplay_keep_Linkanim_00D3C0, &gameplay_keep_Linkanim_00D3C8, &gameplay_keep_Linkanim_00D3D0,
    &gameplay_keep_Linkanim_00D3D8, &gameplay_keep_Linkanim_00D3E0, &gameplay_keep_Linkanim_00D3E8,
    &gameplay_keep_Linkanim_00D3F0, &gameplay_keep_Linkanim_00D3F8, &gameplay_keep_Linkanim_00D400,
    &gameplay_keep_Linkanim_00D408, &gameplay_keep_Linkanim_00D410, &gameplay_keep_Linkanim_00D418,
    &gameplay_keep_Linkanim_00D420, &gameplay_keep_Linkanim_00D428, &gameplay_keep_Linkanim_00D430,
    &gameplay_keep_Linkanim_00D438, &gameplay_keep_Linkanim_00D440, &gameplay_keep_Linkanim_00D448,
    &gameplay_keep_Linkanim_00D450, &gameplay_keep_Linkanim_00D458, &gameplay_keep_Linkanim_00D460,
    &gameplay_keep_Linkanim_00D468, &gameplay_keep_Linkanim_00D470, &gameplay_keep_Linkanim_00D478,
    &gameplay_keep_Linkanim_00D480, &gameplay_keep_Linkanim_00D488, &gameplay_keep_Linkanim_00D490,
    &gameplay_keep_Linkanim_00D498, &gameplay_keep_Linkanim_00D4A0, &gameplay_keep_Linkanim_00D4A8,
    &gameplay_keep_Linkanim_00D4B0, &gameplay_keep_Linkanim_00D4B8, &gameplay_keep_Linkanim_00D4C0,
    &gameplay_keep_Linkanim_00D4C8, &gameplay_keep_Linkanim_00D4D0, &gameplay_keep_Linkanim_00D4D8,
    &gameplay_keep_Linkanim_00D4E0, &gameplay_keep_Linkanim_00D4E8, &gameplay_keep_Linkanim_00D4F0,
    &gameplay_keep_Linkanim_00D4F8, &gameplay_keep_Linkanim_00D500, &gameplay_keep_Linkanim_00D508,
    &gameplay_keep_Linkanim_00D510, &gameplay_keep_Linkanim_00D518, &gameplay_keep_Linkanim_00D520,
    &gameplay_keep_Linkanim_00D528, &gameplay_keep_Linkanim_00D530, &gameplay_keep_Linkanim_00D538,
    &gameplay_keep_Linkanim_00D540, &gameplay_keep_Linkanim_00D548, &gameplay_keep_Linkanim_00D550,
    &gameplay_keep_Linkanim_00D558, &gameplay_keep_Linkanim_00D560, &gameplay_keep_Linkanim_00D568,
    &gameplay_keep_Linkanim_00D570, &gameplay_keep_Linkanim_00D578, &gameplay_keep_Linkanim_00D580,
    &gameplay_keep_Linkanim_00D588, &gameplay_keep_Linkanim_00D590, &gameplay_keep_Linkanim_00D598,
    &gameplay_keep_Linkanim_00D5A0, &gameplay_keep_Linkanim_00D5A8, &gameplay_keep_Linkanim_00D5B0,
    &gameplay_keep_Linkanim_00D5B8, &gameplay_keep_Linkanim_00D5C0, &gameplay_keep_Linkanim_00D5C8,
    &gameplay_keep_Linkanim_00D5D0, &gameplay_keep_Linkanim_00D5D8, &gameplay_keep_Linkanim_00D5E0,
    &gameplay_keep_Linkanim_00D5E8, &gameplay_keep_Linkanim_00D5F0, &gameplay_keep_Linkanim_00D5F8,
    &gameplay_keep_Linkanim_00D600, &gameplay_keep_Linkanim_00D608, &gameplay_keep_Linkanim_00D610,
    &gameplay_keep_Linkanim_00D618, &gameplay_keep_Linkanim_00D620, &gameplay_keep_Linkanim_00D628,
    &gameplay_keep_Linkanim_00D630, &gameplay_keep_Linkanim_00D638, &gameplay_keep_Linkanim_00D640,
    &gameplay_keep_Linkanim_00D648, &gameplay_keep_Linkanim_00D650, &gameplay_keep_Linkanim_00D658,
    &gameplay_keep_Linkanim_00D660, &gameplay_keep_Linkanim_00D668, &gameplay_keep_Linkanim_00D670,
    &gameplay_keep_Linkanim_00D678, &gameplay_keep_Linkanim_00D680, &gameplay_keep_Linkanim_00D688,
    &gameplay_keep_Linkanim_00D690, &gameplay_keep_Linkanim_00D698, &gameplay_keep_Linkanim_00D6A0,
    &gameplay_keep_Linkanim_00D6A8, &gameplay_keep_Linkanim_00D6B0, &gameplay_keep_Linkanim_00D6B8,
    &gameplay_keep_Linkanim_00D6C0, &gameplay_keep_Linkanim_00D6C8, &gameplay_keep_Linkanim_00D6D0,
    &gameplay_keep_Linkanim_00D6D8, &gameplay_keep_Linkanim_00D6E0, &gameplay_keep_Linkanim_00D6E8,
    &gameplay_keep_Linkanim_00D6F0, &gameplay_keep_Linkanim_00D6F8, &gameplay_keep_Linkanim_00D700,
    &gameplay_keep_Linkanim_00D708, &gameplay_keep_Linkanim_00D710, &gameplay_keep_Linkanim_00D718,
    &gameplay_keep_Linkanim_00D720, &gameplay_keep_Linkanim_00D728, &gameplay_keep_Linkanim_00D730,
    &gameplay_keep_Linkanim_00D738, &gameplay_keep_Linkanim_00D740, &gameplay_keep_Linkanim_00D748,
    &gameplay_keep_Linkanim_00D750, &gameplay_keep_Linkanim_00D758, &gameplay_keep_Linkanim_00D760,
    &gameplay_keep_Linkanim_00D768, &gameplay_keep_Linkanim_00D770, &gameplay_keep_Linkanim_00D778,
    &gameplay_keep_Linkanim_00D780, &gameplay_keep_Linkanim_00D788, &gameplay_keep_Linkanim_00D790,
    &gameplay_keep_Linkanim_00D798, &gameplay_keep_Linkanim_00D7A0, &gameplay_keep_Linkanim_00D7A8,
    &gameplay_keep_Linkanim_00D7B0, &gameplay_keep_Linkanim_00D7B8, &gameplay_keep_Linkanim_00D7C0,
    &gameplay_keep_Linkanim_00D7C8, &gameplay_keep_Linkanim_00D7D0, &gameplay_keep_Linkanim_00D7D8,
    &gameplay_keep_Linkanim_00D7E0, &gameplay_keep_Linkanim_00D7E8, &gameplay_keep_Linkanim_00D7F0,
    &gameplay_keep_Linkanim_00D7F8, &gameplay_keep_Linkanim_00D800, &gameplay_keep_Linkanim_00D808,
    &gameplay_keep_Linkanim_00D810, &gameplay_keep_Linkanim_00D818, &gameplay_keep_Linkanim_00D820,
    &gameplay_keep_Linkanim_00D828, &gameplay_keep_Linkanim_00D830, &gameplay_keep_Linkanim_00D838,
    &gameplay_keep_Linkanim_00D840, &gameplay_keep_Linkanim_00D848, &gameplay_keep_Linkanim_00D850,
    &gameplay_keep_Linkanim_00D858, &gameplay_keep_Linkanim_00D860, &gameplay_keep_Linkanim_00D868,
    &gameplay_keep_Linkanim_00D870, &gameplay_keep_Linkanim_00D878, &gameplay_keep_Linkanim_00D880,
    &gameplay_keep_Linkanim_00D888, &gameplay_keep_Linkanim_00D890, &gameplay_keep_Linkanim_00D898,
    &gameplay_keep_Linkanim_00D8A0, &gameplay_keep_Linkanim_00D8A8, &gameplay_keep_Linkanim_00D8B0,
    &gameplay_keep_Linkanim_00D8B8, &gameplay_keep_Linkanim_00D8C0, &gameplay_keep_Linkanim_00D8C8,
    &gameplay_keep_Linkanim_00D8D0, &gameplay_keep_Linkanim_00D8D8, &gameplay_keep_Linkanim_00D8E0,
    &gameplay_keep_Linkanim_00D8E8, &gameplay_keep_Linkanim_00D8F0, &gameplay_keep_Linkanim_00D8F8,
    &gameplay_keep_Linkanim_00D900, &gameplay_keep_Linkanim_00D908, &gameplay_keep_Linkanim_00D910,
    &gameplay_keep_Linkanim_00D918, &gameplay_keep_Linkanim_00D920, &gameplay_keep_Linkanim_00D928,
    &gameplay_keep_Linkanim_00D930, &gameplay_keep_Linkanim_00D938, &gameplay_keep_Linkanim_00D940,
    &gameplay_keep_Linkanim_00D948, &gameplay_keep_Linkanim_00D950, &gameplay_keep_Linkanim_00D958,
    &gameplay_keep_Linkanim_00D960, &gameplay_keep_Linkanim_00D968, &gameplay_keep_Linkanim_00D970,
    &gameplay_keep_Linkanim_00D978, &gameplay_keep_Linkanim_00D980, &gameplay_keep_Linkanim_00D988,
    &gameplay_keep_Linkanim_00D990, &gameplay_keep_Linkanim_00D998, &gameplay_keep_Linkanim_00D9A0,
    &gameplay_keep_Linkanim_00D9A8, &gameplay_keep_Linkanim_00D9B0, &gameplay_keep_Linkanim_00D9B8,
    &gameplay_keep_Linkanim_00D9C0, &gameplay_keep_Linkanim_00D9C8, &gameplay_keep_Linkanim_00D9D0,
    &gameplay_keep_Linkanim_00D9D8, &gameplay_keep_Linkanim_00D9E0, &gameplay_keep_Linkanim_00D9E8,
    &gameplay_keep_Linkanim_00D9F0, &gameplay_keep_Linkanim_00D9F8, &gameplay_keep_Linkanim_00DA00,
    &gameplay_keep_Linkanim_00DA08, &gameplay_keep_Linkanim_00DA10, &gameplay_keep_Linkanim_00DA18,
    &gameplay_keep_Linkanim_00DA20, &gameplay_keep_Linkanim_00DA28, &gameplay_keep_Linkanim_00DA30,
    &gameplay_keep_Linkanim_00DA38, &gameplay_keep_Linkanim_00DA40, &gameplay_keep_Linkanim_00DA48,
    &gameplay_keep_Linkanim_00DA50, &gameplay_keep_Linkanim_00DA58, &gameplay_keep_Linkanim_00DA60,
    &gameplay_keep_Linkanim_00DA68, &gameplay_keep_Linkanim_00DA70, &gameplay_keep_Linkanim_00DA78,
    &gameplay_keep_Linkanim_00DA80, &gameplay_keep_Linkanim_00DA88, &gameplay_keep_Linkanim_00DA90,
    &gameplay_keep_Linkanim_00DA98, &gameplay_keep_Linkanim_00DAA0, &gameplay_keep_Linkanim_00DAA8,
    &gameplay_keep_Linkanim_00DAB0, &gameplay_keep_Linkanim_00DAB8, &gameplay_keep_Linkanim_00DAC0,
    &gameplay_keep_Linkanim_00DAC8, &gameplay_keep_Linkanim_00DAD0, &gameplay_keep_Linkanim_00DAD8,
    &gameplay_keep_Linkanim_00DAE0, &gameplay_keep_Linkanim_00DAE8, &gameplay_keep_Linkanim_00DAF0,
    &gameplay_keep_Linkanim_00DAF8, &gameplay_keep_Linkanim_00DB00, &gameplay_keep_Linkanim_00DB08,
    &gameplay_keep_Linkanim_00DB10, &gameplay_keep_Linkanim_00DB18, &gameplay_keep_Linkanim_00DB20,
    &gameplay_keep_Linkanim_00DB28, &gameplay_keep_Linkanim_00DB30, &gameplay_keep_Linkanim_00DB38,
    &gameplay_keep_Linkanim_00DB40, &gameplay_keep_Linkanim_00DB48, &gameplay_keep_Linkanim_00DB50,
    &gameplay_keep_Linkanim_00DB58, &gameplay_keep_Linkanim_00DB60, &gameplay_keep_Linkanim_00DB68,
    &gameplay_keep_Linkanim_00DB70, &gameplay_keep_Linkanim_00DB78, &gameplay_keep_Linkanim_00DB80,
    &gameplay_keep_Linkanim_00DB88, &gameplay_keep_Linkanim_00DB90, &gameplay_keep_Linkanim_00DB98,
    &gameplay_keep_Linkanim_00DBA0, &gameplay_keep_Linkanim_00DBA8, &gameplay_keep_Linkanim_00DBB0,
    &gameplay_keep_Linkanim_00DBB8, &gameplay_keep_Linkanim_00DBC0, &gameplay_keep_Linkanim_00DBC8,
    &gameplay_keep_Linkanim_00DBD0, &gameplay_keep_Linkanim_00DBD8, &gameplay_keep_Linkanim_00DBE0,
    &gameplay_keep_Linkanim_00DBE8, &gameplay_keep_Linkanim_00DBF0, &gameplay_keep_Linkanim_00DBF8,
    &gameplay_keep_Linkanim_00DC00, &gameplay_keep_Linkanim_00DC08, &gameplay_keep_Linkanim_00DC10,
    &gameplay_keep_Linkanim_00DC18, &gameplay_keep_Linkanim_00DC20, &gameplay_keep_Linkanim_00DC28,
    &gameplay_keep_Linkanim_00DC30, &gameplay_keep_Linkanim_00DC38, &gameplay_keep_Linkanim_00DC40,
    &gameplay_keep_Linkanim_00DC48, &gameplay_keep_Linkanim_00DC50, &gameplay_keep_Linkanim_00DC58,
    &gameplay_keep_Linkanim_00DC60, &gameplay_keep_Linkanim_00DC68, &gameplay_keep_Linkanim_00DC70,
    &gameplay_keep_Linkanim_00DC78, &gameplay_keep_Linkanim_00DC80, &gameplay_keep_Linkanim_00DC88,
    &gameplay_keep_Linkanim_00DC90, &gameplay_keep_Linkanim_00DC98, &gameplay_keep_Linkanim_00DCA0,
    &gameplay_keep_Linkanim_00DCA8, &gameplay_keep_Linkanim_00DCB0, &gameplay_keep_Linkanim_00DCB8,
    &gameplay_keep_Linkanim_00DCC0, &gameplay_keep_Linkanim_00DCC8, &gameplay_keep_Linkanim_00DCD0,
    &gameplay_keep_Linkanim_00DCD8, &gameplay_keep_Linkanim_00DCE0, &gameplay_keep_Linkanim_00DCE8,
    &gameplay_keep_Linkanim_00DCF0, &gameplay_keep_Linkanim_00DCF8, &gameplay_keep_Linkanim_00DD00,
    &gameplay_keep_Linkanim_00DD08, &gameplay_keep_Linkanim_00DD10, &gameplay_keep_Linkanim_00DD18,
    &gameplay_keep_Linkanim_00DD20, &gameplay_keep_Linkanim_00DD28, &gameplay_keep_Linkanim_00DD30,
    &gameplay_keep_Linkanim_00DD38, &gameplay_keep_Linkanim_00DD40, &gameplay_keep_Linkanim_00DD48,
    &gameplay_keep_Linkanim_00DD50, &gameplay_keep_Linkanim_00DD58, &gameplay_keep_Linkanim_00DD60,
    &gameplay_keep_Linkanim_00DD68, &gameplay_keep_Linkanim_00DD70, &gameplay_keep_Linkanim_00DD78,
    &gameplay_keep_Linkanim_00DD80, &gameplay_keep_Linkanim_00DD88, &gameplay_keep_Linkanim_00DD90,
    &gameplay_keep_Linkanim_00DD98, &gameplay_keep_Linkanim_00DDA0, &gameplay_keep_Linkanim_00DDA8,
    &gameplay_keep_Linkanim_00DDB0, &gameplay_keep_Linkanim_00DDB8, &gameplay_keep_Linkanim_00DDC0,
    &gameplay_keep_Linkanim_00DDC8, &gameplay_keep_Linkanim_00DDD0, &gameplay_keep_Linkanim_00DDD8,
    &gameplay_keep_Linkanim_00DDE0, &gameplay_keep_Linkanim_00DDE8, &gameplay_keep_Linkanim_00DDF0,
    &gameplay_keep_Linkanim_00DDF8, &gameplay_keep_Linkanim_00DE00, &gameplay_keep_Linkanim_00DE08,
    &gameplay_keep_Linkanim_00DE10, &gameplay_keep_Linkanim_00DE18, &gameplay_keep_Linkanim_00DE20,
    &gameplay_keep_Linkanim_00DE28, &gameplay_keep_Linkanim_00DE30, &gameplay_keep_Linkanim_00DE38,
    &gameplay_keep_Linkanim_00DE40, &gameplay_keep_Linkanim_00DE48, &gameplay_keep_Linkanim_00DE50,
    &gameplay_keep_Linkanim_00DE58, &gameplay_keep_Linkanim_00DE60, &gameplay_keep_Linkanim_00DE68,
    &gameplay_keep_Linkanim_00DE70, &gameplay_keep_Linkanim_00DE78, &gameplay_keep_Linkanim_00DE80,
    &gameplay_keep_Linkanim_00DE88, &gameplay_keep_Linkanim_00DE90, &gameplay_keep_Linkanim_00DE98,
    &gameplay_keep_Linkanim_00DEA0, &gameplay_keep_Linkanim_00DEA8, &gameplay_keep_Linkanim_00DEB0,
    &gameplay_keep_Linkanim_00DEB8, &gameplay_keep_Linkanim_00DEC0, &gameplay_keep_Linkanim_00DEC8,
    &gameplay_keep_Linkanim_00DED0, &gameplay_keep_Linkanim_00DED8, &gameplay_keep_Linkanim_00DEE0,
    &gameplay_keep_Linkanim_00DEE8, &gameplay_keep_Linkanim_00DEF0, &gameplay_keep_Linkanim_00DEF8,
    &gameplay_keep_Linkanim_00DF00, &gameplay_keep_Linkanim_00DF08, &gameplay_keep_Linkanim_00DF10,
    &gameplay_keep_Linkanim_00DF18, &gameplay_keep_Linkanim_00DF20, &gameplay_keep_Linkanim_00DF28,
    &gameplay_keep_Linkanim_00DF30, &gameplay_keep_Linkanim_00DF38, &gameplay_keep_Linkanim_00DF40,
    &gameplay_keep_Linkanim_00DF48, &gameplay_keep_Linkanim_00DF50, &gameplay_keep_Linkanim_00DF58,
    &gameplay_keep_Linkanim_00DF60, &gameplay_keep_Linkanim_00DF68, &gameplay_keep_Linkanim_00DF70,
    &gameplay_keep_Linkanim_00DF78, &gameplay_keep_Linkanim_00DF80, &gameplay_keep_Linkanim_00DF88,
    &gameplay_keep_Linkanim_00DF90, &gameplay_keep_Linkanim_00DF98, &gameplay_keep_Linkanim_00DFA0,
    &gameplay_keep_Linkanim_00DFA8, &gameplay_keep_Linkanim_00DFB0, &gameplay_keep_Linkanim_00DFB8,
    &gameplay_keep_Linkanim_00DFC0, &gameplay_keep_Linkanim_00DFC8, &gameplay_keep_Linkanim_00DFD0,
    &gameplay_keep_Linkanim_00DFD8, &gameplay_keep_Linkanim_00DFE0, &gameplay_keep_Linkanim_00DFE8,
    &gameplay_keep_Linkanim_00DFF0, &gameplay_keep_Linkanim_00DFF8, &gameplay_keep_Linkanim_00E000,
    &gameplay_keep_Linkanim_00E008, &gameplay_keep_Linkanim_00E010, &gameplay_keep_Linkanim_00E018,
    &gameplay_keep_Linkanim_00E020, &gameplay_keep_Linkanim_00E028, &gameplay_keep_Linkanim_00E030,
    &gameplay_keep_Linkanim_00E038, &gameplay_keep_Linkanim_00E040, &gameplay_keep_Linkanim_00E048,
    &gameplay_keep_Linkanim_00E050, &gameplay_keep_Linkanim_00E058, &gameplay_keep_Linkanim_00E060,
    &gameplay_keep_Linkanim_00E068, &gameplay_keep_Linkanim_00E070, &gameplay_keep_Linkanim_00E078,
    &gameplay_keep_Linkanim_00E080, &gameplay_keep_Linkanim_00E088, &gameplay_keep_Linkanim_00E090,
    &gameplay_keep_Linkanim_00E098, &gameplay_keep_Linkanim_00E0A0, &gameplay_keep_Linkanim_00E0A8,
    &gameplay_keep_Linkanim_00E0B0, &gameplay_keep_Linkanim_00E0B8, &gameplay_keep_Linkanim_00E0C0,
    &gameplay_keep_Linkanim_00E0C8, &gameplay_keep_Linkanim_00E0D0, &gameplay_keep_Linkanim_00E0D8,
    &gameplay_keep_Linkanim_00E0E0, &gameplay_keep_Linkanim_00E0E8, &gameplay_keep_Linkanim_00E0F0,
    &gameplay_keep_Linkanim_00E0F8, &gameplay_keep_Linkanim_00E100, &gameplay_keep_Linkanim_00E108,
    &gameplay_keep_Linkanim_00E110, &gameplay_keep_Linkanim_00E118, &gameplay_keep_Linkanim_00E120,
    &gameplay_keep_Linkanim_00E128, &gameplay_keep_Linkanim_00E130, &gameplay_keep_Linkanim_00E138,
    &gameplay_keep_Linkanim_00E140, &gameplay_keep_Linkanim_00E148, &gameplay_keep_Linkanim_00E150,
    &gameplay_keep_Linkanim_00E158, &gameplay_keep_Linkanim_00E160, &gameplay_keep_Linkanim_00E168,
    &gameplay_keep_Linkanim_00E170, &gameplay_keep_Linkanim_00E178, &gameplay_keep_Linkanim_00E180,
    &gameplay_keep_Linkanim_00E188, &gameplay_keep_Linkanim_00E190, &gameplay_keep_Linkanim_00E198,
    &gameplay_keep_Linkanim_00E1A0, &gameplay_keep_Linkanim_00E1A8, &gameplay_keep_Linkanim_00E1B0,
    &gameplay_keep_Linkanim_00E1B8, &gameplay_keep_Linkanim_00E1C0, &gameplay_keep_Linkanim_00E1C8,
    &gameplay_keep_Linkanim_00E1D0, &gameplay_keep_Linkanim_00E1D8, &gameplay_keep_Linkanim_00E1E0,
    &gameplay_keep_Linkanim_00E1E8, &gameplay_keep_Linkanim_00E1F0, &gameplay_keep_Linkanim_00E1F8,
    &gameplay_keep_Linkanim_00E200, &gameplay_keep_Linkanim_00E208, &gameplay_keep_Linkanim_00E210,
    &gameplay_keep_Linkanim_00E218, &gameplay_keep_Linkanim_00E220, &gameplay_keep_Linkanim_00E228,
    &gameplay_keep_Linkanim_00E230, &gameplay_keep_Linkanim_00E238, &gameplay_keep_Linkanim_00E240,
    &gameplay_keep_Linkanim_00E248, &gameplay_keep_Linkanim_00E250, &gameplay_keep_Linkanim_00E258,
    &gameplay_keep_Linkanim_00E260, &gameplay_keep_Linkanim_00E268, &gameplay_keep_Linkanim_00E270,
    &gameplay_keep_Linkanim_00E278, &gameplay_keep_Linkanim_00E280, &gameplay_keep_Linkanim_00E288,
    &gameplay_keep_Linkanim_00E290, &gameplay_keep_Linkanim_00E298, &gameplay_keep_Linkanim_00E2A0,
    &gameplay_keep_Linkanim_00E2A8, &gameplay_keep_Linkanim_00E2B0, &gameplay_keep_Linkanim_00E2B8,
    &gameplay_keep_Linkanim_00E2C0, &gameplay_keep_Linkanim_00E2C8, &gameplay_keep_Linkanim_00E2D0,
    &gameplay_keep_Linkanim_00E2D8, &gameplay_keep_Linkanim_00E2E0, &gameplay_keep_Linkanim_00E2E8,
    &gameplay_keep_Linkanim_00E2F0, &gameplay_keep_Linkanim_00E2F8, &gameplay_keep_Linkanim_00E300,
    &gameplay_keep_Linkanim_00E308, &gameplay_keep_Linkanim_00E310, &gameplay_keep_Linkanim_00E318,
    &gameplay_keep_Linkanim_00E320, &gameplay_keep_Linkanim_00E328, &gameplay_keep_Linkanim_00E330,
    &gameplay_keep_Linkanim_00E338, &gameplay_keep_Linkanim_00E340, &gameplay_keep_Linkanim_00E348,
    &gameplay_keep_Linkanim_00E350, &gameplay_keep_Linkanim_00E358, &gameplay_keep_Linkanim_00E360,
    &gameplay_keep_Linkanim_00E368, &gameplay_keep_Linkanim_00E370, &gameplay_keep_Linkanim_00E378,
    &gameplay_keep_Linkanim_00E380, &gameplay_keep_Linkanim_00E388, &gameplay_keep_Linkanim_00E390,
    &gameplay_keep_Linkanim_00E398, &gameplay_keep_Linkanim_00E3A0, &gameplay_keep_Linkanim_00E3A8,
    &gameplay_keep_Linkanim_00E3B0, &gameplay_keep_Linkanim_00E3B8, &gameplay_keep_Linkanim_00E3C0,
    &gameplay_keep_Linkanim_00E3C8, &gameplay_keep_Linkanim_00E3D0, &gameplay_keep_Linkanim_00E3D8,
    &gameplay_keep_Linkanim_00E3E0, &gameplay_keep_Linkanim_00E3E8, &gameplay_keep_Linkanim_00E3F0,
    &gameplay_keep_Linkanim_00E3F8, &gameplay_keep_Linkanim_00E400, &gameplay_keep_Linkanim_00E408,
    &gameplay_keep_Linkanim_00E410, &gameplay_keep_Linkanim_00E418, &gameplay_keep_Linkanim_00E420,
    &gameplay_keep_Linkanim_00E428, &gameplay_keep_Linkanim_00E430, &gameplay_keep_Linkanim_00E438,
    &gameplay_keep_Linkanim_00E440, &gameplay_keep_Linkanim_00E448, &gameplay_keep_Linkanim_00E450,
    &gameplay_keep_Linkanim_00E458, &gameplay_keep_Linkanim_00E460, &gameplay_keep_Linkanim_00E468,
    &gameplay_keep_Linkanim_00E470, &gameplay_keep_Linkanim_00E478, &gameplay_keep_Linkanim_00E480,
    &gameplay_keep_Linkanim_00E488, &gameplay_keep_Linkanim_00E490, &gameplay_keep_Linkanim_00E498,
    &gameplay_keep_Linkanim_00E4A0, &gameplay_keep_Linkanim_00E4A8, &gameplay_keep_Linkanim_00E4B0,
    &gameplay_keep_Linkanim_00E4B8, &gameplay_keep_Linkanim_00E4C0, &gameplay_keep_Linkanim_00E4C8,
    &gameplay_keep_Linkanim_00E4D0, &gameplay_keep_Linkanim_00E4D8, &gameplay_keep_Linkanim_00E4E0,
    &gameplay_keep_Linkanim_00E4E8, &gameplay_keep_Linkanim_00E4F0,
};

static Vec3f D_80BFB2E8 = { 0.0f, 0.5f, 0.0f };

static Vec3f D_80BFB2F4 = { 500.0f, -500.0, 0.0f };

static Vec3f D_80BFB300 = { 500.0f, -500.0f, 0.0f };

void EnYb_Init(Actor* thisx, PlayState* play) {
    EnYb* this = THIS;
    s16 tempCutscene;
    s32 i;

    Actor_SetScale(&this->actor, 0.01f);
    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 20.0f);

    // @Bug this alignment is because of player animations, but should be using ALIGN16
    SkelAnime_InitFlex(play, &this->skelAnime, &gYbSkeleton, &object_yb_Anim_000200, (uintptr_t)this->jointTable & ~0xF,
                       (uintptr_t)this->morphTable & ~0xF, YB_LIMB_MAX);

    Animation_PlayLoop(&this->skelAnime, &object_yb_Anim_000200);

    Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    this->actionFunc = EnYb_Idle;
    this->animIndex = 3; // gets overwritten to 2 in EnYb_ChangeAnim later
    this->actor.terminalVelocity = -9.0f;
    this->actor.gravity = -1.0f;

    EnYb_ChangeAnim(play, this, 1, ANIMMODE_LOOP, 0.0f);

    tempCutscene = this->actor.cutscene;
    for (i = 0; i < ARRAY_COUNT(this->cutscenes); i++) {
        this->cutscenes[i] = tempCutscene;
        if (tempCutscene != -1) {
            this->actor.cutscene = tempCutscene;
            tempCutscene = ActorCutscene_GetAdditionalCutscene(this->actor.cutscene);
        }
    }

    this->cutsceneIndex = -1;
    this->actor.cutscene = this->cutscenes[0];

    this->alpha = 255;

    // check if already healed
    if (gSaveContext.save.weekEventReg[82] & 4) {
        Actor_MarkForDeath(&this->actor);
    }

    // Set daySpeed really low so that you don't get interrupted
    gSaveContext.save.daySpeed = -3;
    
    this->holdTimer = 0;
    this->scrollRate = 1;
    this->longScrollTimer = 0;
}

void EnYb_Destroy(Actor* thisx, PlayState* play) {
    EnYb* this = THIS;

    Collider_DestroyCylinder(play, &this->collider);
    gSaveContext.save.daySpeed = 0;
}

void func_80BFA2FC(PlayState* play) {
    if (INV_CONTENT(ITEM_MASK_KAMARO) == ITEM_MASK_KAMARO) {
        func_80151BB4(play, 0x34);
    }
    func_80151BB4(play, 0xF);
}

void EnYb_ChangeAnim(PlayState* play, EnYb* this, s16 animIndex, u8 animMode, f32 morphFrames) {
    if (animIndex >= 0 && animIndex < ARRAY_COUNT(gLinkAnimations) + 1) {
        if (animIndex != this->animIndex || animMode != ANIMMODE_LOOP) {
            if (animIndex > 0) {
                if (animMode == ANIMMODE_LOOP) {
                    LinkAnimation_Change(play, &this->skelAnime, gLinkAnimations[animIndex - 1], 1.0f, 0.0f,
                                         Animation_GetLastFrame(gLinkAnimations[animIndex - 1]), ANIMMODE_LOOP,
                                         morphFrames);
                } else {
                    // unused case, (only called once with animMode = ANIMMODE_LOOP)
                    LinkAnimation_Change(play, &this->skelAnime, gLinkAnimations[animIndex - 1], 1.0f, 0.0f,
                                         Animation_GetLastFrame(gLinkAnimations[animIndex - 1]), ANIMMODE_LOOP,
                                         morphFrames);
                }
            } else {
                // unused case, (only called once with animIndex = 2)
                AnimationHeader* animationPtr = gYbUnusedAnimations[animIndex];

                if (1) {}

                Animation_Change(&this->skelAnime, gYbUnusedAnimations[animIndex], 1.0f, 0.0f,
                                 Animation_GetLastFrame(animationPtr), animMode, morphFrames);
            }
            this->animIndex = animIndex;
            gSaveContext.save.playerData.rupees = animIndex - 1;
        }
    }
}

s32 EnYb_CanTalk(EnYb* this, PlayState* play) {
    if (this->actor.xzDistToPlayer < 100.0f && Player_IsFacingActor(&this->actor, 0x3000, play) &&
        Actor_IsFacingPlayer(&this->actor, 0x3000)) {
        return true;
    } else {
        return false;
    }
}

void EnYb_UpdateAnimation(EnYb* this, PlayState* play) {
    if (this->animIndex <= 0) {
        SkelAnime_Update(&this->skelAnime);
    } else {
        LinkAnimation_Update(play, &this->skelAnime);
    }
}

void EnYb_FinishTeachingCutscene(EnYb* this) {
    if (this->cutsceneIndex != -1) {
        if (ActorCutscene_GetCurrentIndex() == this->cutscenes[this->cutsceneIndex]) {
            ActorCutscene_Stop(this->cutscenes[this->cutsceneIndex]);
        }
        this->cutsceneIndex = -1;
    }
}

void EnYb_ChangeCutscene(EnYb* this, s16 cutsceneId) {
    EnYb_FinishTeachingCutscene(this);
    this->cutsceneIndex = cutsceneId;
}

/**
 * Sets a flag that enables the Kamaro dancing proximity music at night.
 */
void EnYb_EnableProximityMusic(EnYb* this) {
}

void EnYb_Disappear(EnYb* this, PlayState* play) {
    s32 pad;
    Vec3f sp60;
    s32 i;

    EnYb_UpdateAnimation(this, play);
    for (i = 3; i >= 0; i--) {
        sp60.x = randPlusMinusPoint5Scaled(60.0f) + this->actor.world.pos.x;
        sp60.z = randPlusMinusPoint5Scaled(60.0f) + this->actor.world.pos.z;
        sp60.y = randPlusMinusPoint5Scaled(50.0f) + (this->actor.world.pos.y + 20.0f);
        func_800B3030(play, &sp60, &D_80BFB2E8, &D_80BFB2E8, 100, 0, 2);
    }

    SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 20, NA_SE_EN_EXTINCT);
    if (this->alpha > 10) {
        this->alpha -= 10;
    } else {
        Actor_MarkForDeath(&this->actor);
    }
}

void EnYb_SetupLeaving(EnYb* this, PlayState* play) {
    EnYb_UpdateAnimation(this, play);
    if (Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        this->actor.flags &= ~ACTOR_FLAG_10000;
        this->actionFunc = EnYb_Talk;
        // I am counting on you
        Message_StartTextbox(play, 0x147D, &this->actor);
        func_80BFA2FC(play);
    } else {
        func_800B8500(&this->actor, play, 1000.0f, 1000.0f, PLAYER_AP_MINUS1);
    }
    EnYb_EnableProximityMusic(this);
}

void EnYb_ReceiveMask(EnYb* this, PlayState* play) {
    EnYb_UpdateAnimation(this, play);
    // Player is parent: receiving the Kamaro mask
    if (Actor_HasParent(&this->actor, play)) {
        this->actor.parent = NULL;
        this->actionFunc = EnYb_SetupLeaving;
        this->actor.flags |= ACTOR_FLAG_10000;
        func_800B8500(&this->actor, play, 1000.0f, 1000.0f, PLAYER_AP_MINUS1);
    } else {
        Actor_PickUp(&this->actor, play, GI_MASK_KAMARO, 10000.0f, 100.0f);
    }
    EnYb_EnableProximityMusic(this);
}

void EnYb_Talk(EnYb* this, PlayState* play) {
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 2, 0x1000, 0x200);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    EnYb_UpdateAnimation(this, play);

    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_5) && Message_ShouldAdvance(play)) {
        switch (play->msgCtx.currentTextId) {
            case 0x147D: // I am counting on you
                func_801477B4(play);
                this->actionFunc = EnYb_Disappear;
                gSaveContext.save.weekEventReg[82] |= 0x4;
                break;
            case 0x147C: // Spread my dance across the world
                if (Player_GetMask(play) == PLAYER_MASK_KAMARO) {
                    func_801477B4(play);
                    this->actionFunc = EnYb_Idle;

                } else if (INV_CONTENT(ITEM_MASK_KAMARO) == ITEM_MASK_KAMARO) {
                    func_80151938(play, 0x147D); // I am counting on you
                    func_80BFA2FC(play);

                } else {
                    func_801477B4(play);
                    this->actionFunc = EnYb_ReceiveMask;
                    EnYb_ReceiveMask(this, play);
                }
                break;
            default:
                func_801477B4(play);
                this->actionFunc = EnYb_Idle;
                break;
        }
    }
    EnYb_EnableProximityMusic(this);
}

void EnYb_TeachingDanceFinish(EnYb* this, PlayState* play) {
    EnYb_UpdateAnimation(this, play);
    if (Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        this->actionFunc = EnYb_Talk;
        // Spread my dance across the world
        Message_StartTextbox(play, 0x147C, &this->actor);
        this->actor.flags &= ~ACTOR_FLAG_10000;
    } else {
        func_800B8500(&this->actor, play, 1000.0f, 1000.0f, PLAYER_AP_MINUS1);
    }
    EnYb_EnableProximityMusic(this);
}

// dancing countdown
void EnYb_TeachingDance(EnYb* this, PlayState* play) {
    EnYb_UpdateAnimation(this, play);

    if (this->teachingCutsceneTimer > 0) {
        this->teachingCutsceneTimer--;
    } else {
        EnYb_FinishTeachingCutscene(this);
        this->actionFunc = EnYb_TeachingDanceFinish;
        this->actor.flags |= ACTOR_FLAG_10000;
        func_800B8500(&this->actor, play, 1000.0f, 1000.0f, PLAYER_AP_MINUS1);
    }
    EnYb_EnableProximityMusic(this);
}

void EnYb_Idle(EnYb* this, PlayState* play) {
    s32 pad;
    Player* player = GET_PLAYER(play);

    EnYb_UpdateAnimation(this, play);
    if (this->actor.xzDistToPlayer < 180.0f && fabsf(this->actor.playerHeightRel) < 50.0f &&
        play->msgCtx.ocarinaMode == 3 && play->msgCtx.lastPlayedSong == OCARINA_SONG_HEALING &&
        gSaveContext.save.playerForm == PLAYER_FORM_HUMAN) {
        this->actionFunc = EnYb_TeachingDance;
        this->teachingCutsceneTimer = 200;
        EnYb_ChangeCutscene(this, 0);
    } else if (Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        func_80BFA2FC(play);
        this->actionFunc = EnYb_Talk;
        if (Player_GetMask(play) == PLAYER_MASK_KAMARO) {
            // I have taught you, go use it
            Message_StartTextbox(play, 0x147C, &this->actor);
        } else {
            // regular talk to him first dialogue
            Message_StartTextbox(play, 0x147B, &this->actor);
        }
    } else if (EnYb_CanTalk(this, play)) {
        func_800B8614(&this->actor, play, 120.0f);
    }

    if (this->playerOcarinaOut & 1) {
        if (!(player->stateFlags2 & 0x8000000)) {
            this->playerOcarinaOut &= ~1;
        }
    } else if ((player->stateFlags2 & 0x8000000) && this->actor.xzDistToPlayer < 180.0f &&
               fabsf(this->actor.playerHeightRel) < 50.0f) {
        this->playerOcarinaOut |= 1;
        Actor_PlaySfxAtPos(&this->actor, NA_SE_SY_TRE_BOX_APPEAR);
    }

    EnYb_EnableProximityMusic(this);
}

void EnYb_Update(Actor* thisx, PlayState* play) {
    s32 pad;
    EnYb* this = THIS;
    s32 animIndex;

    if (this->holdTimer == 1) {
        this->longScrollTimer = 22;
    }

    if (this->longScrollTimer == 1) {
        this->scrollRate = 10;
    }

    DECR(this->holdTimer);
    DECR(this->longScrollTimer);

    // The player made a new button press, so stop all hold-related things.
    if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_DRIGHT) || CHECK_BTN_ALL(play->state.input[0].press.button, BTN_DLEFT)) {
        this->holdTimer = 8;
        this->scrollRate = 1;
        this->longScrollTimer = 0;
    }

    if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_DRIGHT) ||
        (this->holdTimer == 0 && CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_DRIGHT))) {
        if (this->animIndex == ARRAY_COUNT(gLinkAnimations)) {
            play_sound(NA_SE_SY_ERROR);
        } else {
            play_sound(NA_SE_SY_CURSOR);
            animIndex = CLAMP_MAX(this->animIndex + this->scrollRate, ARRAY_COUNT(gLinkAnimations));
            EnYb_ChangeAnim(play, this, animIndex, ANIMMODE_LOOP, 0.0f);
        }
    }

    if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_DLEFT) ||
        (this->holdTimer == 0 && CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_DLEFT))) {
        if (this->animIndex == 1) {
            play_sound(NA_SE_SY_ERROR);
        } else {
            play_sound(NA_SE_SY_CURSOR);
            animIndex = CLAMP_MIN(this->animIndex - this->scrollRate, 1);
            EnYb_ChangeAnim(play, this, animIndex, ANIMMODE_LOOP, 0.0f);
        }
    }

    if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_DUP)) {
        this->actor.gravity = 0.0f;
        this->actor.world.pos.y += 20.0f;
    }

    if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_DDOWN)) {
        this->actor.gravity = -1.0f;
        this->actor.world.pos.y -= 20.0f;
    }

    if (CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_1)) {
        Collider_UpdateCylinder(&this->actor, &this->collider);
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    }
    if (CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_1)) {
        Actor_MoveWithGravity(&this->actor);
        Actor_UpdateBgCheckInfo(play, &this->actor, 40.0f, 25.0f, 40.0f, 5);
    }

    this->actionFunc(this, play);

    if (this->cutsceneIndex != -1 && ActorCutscene_GetCurrentIndex() != this->cutscenes[this->cutsceneIndex]) {
        if (ActorCutscene_GetCurrentIndex() == 0x7C) {
            ActorCutscene_Stop(0x7C);
            ActorCutscene_SetIntentToPlay(this->cutscenes[this->cutsceneIndex]);
        } else if (ActorCutscene_GetCanPlayNext(this->cutscenes[this->cutsceneIndex])) {
            if (this->cutsceneIndex == 0) {
                ActorCutscene_StartAndSetUnkLinkFields(this->cutscenes[this->cutsceneIndex], &this->actor);
            }
        } else {
            ActorCutscene_SetIntentToPlay(this->cutscenes[this->cutsceneIndex]);
        }
    }
}

void EnYb_PostLimbDrawOpa(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    EnYb* this = THIS;

    if (limbIndex == YB_LIMB_HEAD) {
        Matrix_MultVec3f(&D_80BFB2F4, &this->actor.focus.pos);
    }
    if (limbIndex == YB_LIMB_LEGS_ROOT) {
        Matrix_MultVec3f(&gZeroVec3f, &this->shadowPos);
    }
}

void EnYb_PostLimbDrawXlu(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx, Gfx** gfx) {
    EnYb* this = THIS;

    if (limbIndex == YB_LIMB_HEAD) {
        Matrix_MultVec3f(&D_80BFB300, &this->actor.focus.pos);
    }
    if (limbIndex == YB_LIMB_LEGS_ROOT) {
        Matrix_MultVec3f(&gZeroVec3f, &this->shadowPos);
    }
}

void EnYb_DrawAnimationName(EnYb* this, PlayState* play) {
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    GfxPrint printer;
    Gfx* gfxRef;
    Gfx* gfx;

    OPEN_DISPS(gfxCtx);

    func_8012C4C0(gfxCtx);

    GfxPrint_Init(&printer);

    gfxRef = POLY_OPA_DISP;
    gfx = Graph_GfxPlusOne(gfxRef);
    gSPDisplayList(OVERLAY_DISP++, gfx);

    GfxPrint_Open(&printer, gfx);

    GfxPrint_SetColor(&printer, 255, 255, 255, 255);

    GfxPrint_SetPos(&printer, 31, 28);
    GfxPrint_Printf(&printer, "%X", gLinkAnimations[this->animIndex - 1]);

    gfx = GfxPrint_Close(&printer);

    gSPEndDisplayList(gfx++);
    Graph_BranchDlist(gfxRef, gfx);
    POLY_OPA_DISP = gfx;

    GfxPrint_Destroy(&printer);

    CLOSE_DISPS(gfxCtx);
}

void EnYb_Draw(Actor* thisx, PlayState* play) {
    EnYb* this = THIS;

    OPEN_DISPS(play->state.gfxCtx);

    if (this->alpha != 0) {
        if (this->alpha < 255) {
            if (this->alpha > 128) {
                func_8012C2B4(POLY_XLU_DISP++);
                Scene_SetRenderModeXlu(play, 2, 2);
            } else {
                func_8012C304(POLY_XLU_DISP++);
                Scene_SetRenderModeXlu(play, 1, 2);
            }
            gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 0, this->alpha);

            if (1) {}

            POLY_XLU_DISP =
                SkelAnime_DrawFlex(play, this->skelAnime.skeleton, this->skelAnime.jointTable,
                                   this->skelAnime.dListCount, NULL, EnYb_PostLimbDrawXlu, &this->actor, POLY_XLU_DISP);

        } else {
            func_8012C28C(play->state.gfxCtx);
            Scene_SetRenderModeXlu(play, 0, 1);
            SkelAnime_DrawFlexOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable,
                                  this->skelAnime.dListCount, NULL, EnYb_PostLimbDrawOpa, &this->actor);
        }
    }

    EnYb_DrawAnimationName(this, play);

    CLOSE_DISPS(play->state.gfxCtx);
}
