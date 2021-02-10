#include "common.h"

#include "Camera.h"
#include "DMAudio.h"
#include "Clock.h"
#include "Darkel.h"
#include "Hud.h"
#include "Messages.h"
#include "Frontend.h"
#include "Font.h"
#include "Pad.h"
#include "Radar.h"
#include "Replay.h"
#include "Wanted.h"
#include "Sprite.h"
#include "Sprite2d.h"
#include "Text.h"
#include "Timer.h"
#include "Script.h"
#include "TxdStore.h"
#include "User.h"
#include "World.h"

#ifdef PS2_HUD
#define MONEY_X 100.0f
#define WEAPON_X 91.0f
#define AMMO_X 59.0f
#define HEALTH_X 100.0f
#define STARS_X 49.0f
#define ZONE_Y 61.0f
#define VEHICLE_Y 81.0f
#define CLOCK_X 101.0f
#define SUBS_Y 83.0f
#define WASTEDBUSTED_Y 122.0f
#define BIGMESSAGE_Y 80.0f
#else
#define MONEY_X 20.0f
#define WEAPON_X 99.0f
#define AMMO_X 72.5f
#define HEALTH_X 110.0f
#define STARS_X 24.0f
#define ZONE_Y 54.0f
#define VEHICLE_Y 78.0f
#define CLOCK_X 92.0f
#define SUBS_Y 68.0f
#define WASTEDBUSTED_Y 82.0f
#define BIGMESSAGE_Y 84.0f
#endif

#ifdef FIX_BUGS
#define TIMER_RIGHT_OFFSET 34.0f // Taken from VC frenzy timer
#define BIGMESSAGE_Y_OFFSET 18.0f
#else
#define TIMER_RIGHT_OFFSET 27.0f
#define BIGMESSAGE_Y_OFFSET 20.0f
#endif

#if defined(PS2_HUD) && !defined(FIX_BUGS)
	#define SCREEN_SCALE_X_PC(a) (a)
	#define SCREEN_SCALE_Y_PC(a) (a)
	#define SCALE_AND_CENTER_X_PC(a) (a)
#else
	#define SCREEN_SCALE_X_PC(a) SCREEN_SCALE_X(a)
	#define SCREEN_SCALE_Y_PC(a) SCREEN_SCALE_Y(a)
	#define SCALE_AND_CENTER_X_PC(a) SCALE_AND_CENTER_X(a)
#endif

#if defined(FIX_BUGS)
	#define SCREEN_SCALE_X_FIX(a) SCREEN_SCALE_X(a)
	#define SCREEN_SCALE_Y_FIX(a) SCREEN_SCALE_Y(a)
	#define SCALE_AND_CENTER_X_FIX(a) SCALE_AND_CENTER_X(a)
#else
	#define SCREEN_SCALE_X_FIX(a) (a)
	#define SCREEN_SCALE_Y_FIX(a) (a)
	#define SCALE_AND_CENTER_X_FIX(a) (a)
#endif

// Game has colors inlined in code.
// For easier modification we collect them here:
CRGBA ALPHAHUD_COLOR(255, 255, 1, 255);
CRGBA MONEY_COLOR(89, 115, 150, 255);
CRGBA AMMO_COLOR(0, 0, 0, 255);
CRGBA HEALTH_COLOR(186, 101, 50, 255);
CRGBA ARMOUR_COLOR(124, 140, 95, 255);
CRGBA WANTED_COLOR(193, 164, 120, 255);
CRGBA ZONE_COLOR(225, 225, 225, 255);
CRGBA VEHICLE_COLOR(255, 255, 255, 255);
CRGBA CLOCK_COLOR(194, 165, 120, 255);
CRGBA TIMER_COLOR(186, 101, 50, 255);
CRGBA COUNTER_COLOR(0, 106, 164, 255);
CRGBA PAGER_COLOR(0, 255, 25, 205);
CRGBA RADARDISC_COLOR(0, 0, 0, 255);
CRGBA BIGMESSAGE_COLOR(85, 119, 133, 255);
CRGBA WASTEDBUSTED_COLOR(170, 123, 87, 255);
CRGBA ODDJOB_COLOR(89, 115, 150, 255);
CRGBA ODDJOB2_COLOR(156, 91, 40, 255);
CRGBA MISSIONTITLE_COLOR(220, 172, 2, 255);


int16 CHud::m_ItemToFlash;
CSprite2d CHud::Sprites[NUM_HUD_SPRITES];
wchar *CHud::m_pZoneName;
wchar *CHud::m_pLastZoneName;
wchar *CHud::m_ZoneToPrint;
wchar CHud::m_Message[256];
wchar CHud::m_BigMessage[6][128];
wchar LastBigMessage[6][128];
wchar CHud::m_PagerMessage[256];
uint32 CHud::m_ZoneNameTimer;
int32 CHud::m_ZoneFadeTimer;
uint32 CHud::m_ZoneState;
wchar CHud::m_HelpMessage[HELP_MSG_LENGTH];
wchar CHud::m_LastHelpMessage[HELP_MSG_LENGTH];
wchar CHud::m_HelpMessageToPrint[HELP_MSG_LENGTH];
uint32 CHud::m_HelpMessageTimer;
int32 CHud::m_HelpMessageFadeTimer;
uint32 CHud::m_HelpMessageState;
bool CHud::m_HelpMessageQuick;
float CHud::m_HelpMessageDisplayTime;
int32 CHud::SpriteBrightness;
bool CHud::m_Wants_To_Draw_Hud;
bool CHud::m_Wants_To_Draw_3dMarkers;
wchar *CHud::m_pVehicleName;
wchar *CHud::m_pLastVehicleName;
uint32 CHud::m_VehicleNameTimer;
int32 CHud::m_VehicleFadeTimer;
uint32 CHud::m_VehicleState;
wchar *CHud::m_pVehicleNameToPrint;
bool Wants_To_Draw_Blips = true;

// These aren't really in CHud
float BigMessageInUse[6];
float BigMessageX[6];
float BigMessageAlpha[6];
int16 PagerOn;
int16 PagerTimer;
float PagerXOffset;
int16 PagerSoundPlayed;
int16 OddJob2On;
uint16 OddJob2Timer;
float OddJob2XOffset;
float OddJob2OffTimer;
bool CounterOnLastFrame;
uint16 CounterFlashTimer;
bool TimerOnLastFrame;
uint16 TimerFlashTimer;

RwTexture *gpSniperSightTex;
RwTexture *gpRocketSightTex;

struct
{
	const char *name;
	const char *mask;
} WeaponFilenames[] = {
	{"fist", "fistm"},
	{"bat", "batm"},
	{"pistol", "pistolm" },
	{"uzi", "uzim"},
	{"shotgun", "shotgunm"},
	{"ak47", "ak47m"},
	{"m16", "m16m"},
	{"sniper", "sniperm"},
	{"rocket", "rocketm"},
	{"flame", "flamem"},
	{"molotov", "molotovm"},
	{"grenade", "grenadem"},
	{"detonator", "detonator_mask"},
	{"radar_mask", ""},
	{"bar_filled", ""},
	{"pageron", ""},
	{"pageroff", ""},
	{"wanted", ""},
	{"health", ""},
	{"armour", ""},
	{"jailfree", ""},
	{"freecare", ""},
	{"sitesniper", "sitesniperm"},
	{"siteM16", "siteM16m"},
	{"siterocket", "siterocket"},
	{"arrow",""}
};

void CHud::Initialise()
{
	m_Wants_To_Draw_Hud = true;
	m_Wants_To_Draw_3dMarkers = true;

	int HudTXD = CTxdStore::AddTxdSlot("hud");
	CTxdStore::LoadTxd(HudTXD, "MODELS/HUD.TXD");
	CTxdStore::AddRef(HudTXD);
	CTxdStore::PopCurrentTxd();
	CTxdStore::SetCurrentTxd(HudTXD);

	for (int i = 0; i < NUM_HUD_SPRITES; i++) {
		Sprites[i].SetTexture(WeaponFilenames[i].name, WeaponFilenames[i].mask);
	}

	GetRidOfAllHudMessages();

	CounterOnLastFrame = false;
	m_ItemToFlash = ITEM_NONE;
	OddJob2Timer = 0;
	OddJob2OffTimer = 0.0f;
	OddJob2On = 0;
	OddJob2XOffset = 0.0f;
	CounterFlashTimer = 0;
	TimerOnLastFrame = false;
	TimerFlashTimer = 0;
	SpriteBrightness = 0;
	PagerOn = 0;
	PagerTimer = 0;
	PagerSoundPlayed = 0;
	PagerXOffset = 150.0f;

	CTxdStore::PopCurrentTxd();
}

void CHud::Shutdown()
{
	for (int i = 0; i < NUM_HUD_SPRITES; ++i) {
		Sprites[i].Delete();
	}

	int HudTXD = CTxdStore::FindTxdSlot("hud");
	CTxdStore::RemoveTxdSlot(HudTXD);
}

void CHud::ReInitialise() {
	m_Wants_To_Draw_Hud = true;
	m_Wants_To_Draw_3dMarkers = true;

	GetRidOfAllHudMessages();

	CounterOnLastFrame = false;
	m_ItemToFlash = ITEM_NONE;
	OddJob2Timer = 0;
	OddJob2OffTimer = 0.0f;
	OddJob2On = 0;
	OddJob2XOffset = 0.0f;
	CounterFlashTimer = 0;
	TimerOnLastFrame = false;
	TimerFlashTimer = 0;
	SpriteBrightness = 0;
	PagerOn = 0;
	PagerTimer = 0;
	PagerSoundPlayed = 0;
	PagerXOffset = 150.0f;
}

void CHud::GetRidOfAllHudMessages()
{
	m_ZoneState = 0;
	m_pLastZoneName = nil;
	m_ZoneNameTimer = 0;
	m_pZoneName = nil;

	for (int i = 0; i < HELP_MSG_LENGTH; i++) {
		m_HelpMessage[i] = 0;
		m_LastHelpMessage[i] = 0;
		m_HelpMessageToPrint[i] = 0;
	}

	m_HelpMessageTimer = 0;
	m_HelpMessageFadeTimer = 0;
	m_HelpMessageState = 0;
	m_HelpMessageQuick = 0;
	m_HelpMessageDisplayTime = 1.0f;
	m_pVehicleName = nil;
	m_pLastVehicleName = nil;
	m_pVehicleNameToPrint = nil;
	m_VehicleNameTimer = 0;
	m_VehicleFadeTimer = 0;
	m_VehicleState = 0;

	for (int i = 0; i < ARRAY_SIZE(m_Message); i++)
		m_Message[i] = 0;

	for (int i = 0; i < 6; i++) {
		BigMessageInUse[i] = 0.0f;

		for (int j = 0; j < 128; j++)
			m_BigMessage[i][j] = 0;
	}
}

void CHud::SetZoneName(wchar *name)
{
	m_pZoneName = name;
}

void CHud::SetHelpMessage(wchar *message, bool quick)
{
	if (!CReplay::IsPlayingBack()) {
		CMessages::WideStringCopy(m_HelpMessage, message, HELP_MSG_LENGTH);
		CMessages::InsertPlayerControlKeysInString(m_HelpMessage);

		for (int i = 0; i < HELP_MSG_LENGTH; i++) {
			m_LastHelpMessage[i] = 0;
		}

		m_HelpMessageState = 0;
		m_HelpMessageQuick = quick;
	}
}

void CHud::SetVehicleName(wchar *name)
{
	m_pVehicleName = name;
}

void
CHud::DrawAlphaProgressBar(float x, float y, float w, float h, bool bArmour)
{
	CVector2D vecUV[4];
	float fOffset;

	float fValue;
	if(!bArmour) {
		fValue = clamp(1.0f - (Min(CWorld::Players[0].m_pPed->m_fHealth, 100) * 100) / 10000, 0.0f, 1.0f);
		fOffset = 0.70f;
		vecUV[0] = CVector2D(0.330f, 0.0f + fValue);
		vecUV[1] = CVector2D(0.660f, 0.0f + fValue);
		vecUV[2] = CVector2D(0.330f, 1.0f);
		vecUV[3] = CVector2D(0.660f, 1.0f);
	} else {
		fValue = 1.0f - (CWorld::Players[0].m_pPed->m_fArmour * 100) / 10000;
		fOffset = 0.75f;

		vecUV[0] = CVector2D(0.0f, 0.0f + fValue);
		vecUV[1] = CVector2D(0.330f, 0.0f + fValue);
		vecUV[2] = CVector2D(0.0f, 1.0f);
		vecUV[3] = CVector2D(0.330f, 1.0f);
	}

	if(fValue < 1.0f)
		Sprites[HUD_BARFILLED].Draw(CRect(SCREEN_SCALE_X(x), SCREEN_SCALE_FROM_BOTTOM(y * (1.0f - (fValue * fOffset))), SCREEN_SCALE_X(x) + SCREEN_SCALE_X(w), SCREEN_SCALE_FROM_BOTTOM(y) + SCREEN_SCALE_Y(h)),
	                                        CRGBA(255, 255, 255, 255), (vecUV[0].x), (vecUV[0].y), (vecUV[1].x), (vecUV[1].y), (vecUV[2].x), (vecUV[2].y),
	                                        (vecUV[3].x), (vecUV[3].y));
}

void
CHud::DrawGamePaused()
{
	if(CPad::GetPad(0)->GetFJustDown(5)) {
		CTimer::m_UserPause = CTimer::m_UserPause == false;
	}

	if(CTimer::GetIsUserPaused()) {
		CFont::SetPropOn();
		CFont::SetDropShadowPosition(5);
		CFont::SetFontStyle(FONT_HEADING);
		CFont::SetJustifyOff();
		CFont::SetRightJustifyOff();
		CFont::SetCentreOn();
		CFont::SetColor(CRGBA(128, 255, 255, 255));
		CFont::SetDropColor(CRGBA(0, 0, 0, 255));
		CFont::SetScale(SCREEN_SCALE_X(1.72f), SCREEN_SCALE_Y(1.86f));
		CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 25));
		CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_SCALE_Y(-10.0f) + SCREEN_HEIGHT / 2, TheText.Get("PAUSED"));
	}
}

void CHud::Draw()
{
	// disable hud via second controller
	if (CPad::GetPad(1)->GetStartJustDown())
		m_Wants_To_Draw_Hud = !m_Wants_To_Draw_Hud;

#ifdef GTA_PC
	if (CReplay::IsPlayingBack())
		return;
#endif

	if(m_Wants_To_Draw_Hud && !TheCamera.m_WideScreenOn) {
		bool DrawCrossHair = false;
#ifdef GTA_PC
		bool DrawCrossHairPC = false;
#endif

		int32 WeaponType = FindPlayerPed()->m_weapons[FindPlayerPed()->m_currentWeapon].m_eWeaponType;
		int32 Mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;

		if(Mode == CCam::MODE_SNIPER || Mode == CCam::MODE_ROCKETLAUNCHER || Mode == CCam::MODE_M16_1STPERSON
#ifdef GTA_PC
		   || Mode == CCam::MODE_HELICANNON_1STPERSON
#endif
		) {
			DrawCrossHair = true;
		}

#ifdef GTA_PC
		if(Mode == CCam::MODE_M16_1STPERSON_RUNABOUT || Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT || Mode == CCam::MODE_SNIPER_RUNABOUT)
			DrawCrossHairPC = true;

		/*
		        Draw Crosshairs
		*/
		if(TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam() && (!CPad::GetPad(0)->GetLookBehindForPed() || TheCamera.m_bPlayerIsInGarage) ||
		   Mode == CCam::MODE_1STPERSON_RUNABOUT) {
			if(FindPlayerPed() && !FindPlayerPed()->EnteringCar()) {
				if((WeaponType >= WEAPONTYPE_COLT45 && WeaponType <= WEAPONTYPE_M16) || WeaponType == WEAPONTYPE_FLAMETHROWER)
					DrawCrossHairPC = true;
			}
		}
#endif

		if(DrawCrossHair
#ifdef GTA_PC
		   || DrawCrossHairPC
#endif
		) {
			RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);

			SpriteBrightness = Min(SpriteBrightness + 1, 30);

			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)FALSE);

			float fStep = Sin((CTimer::GetTimeInMilliseconds() & 1023) / 1024.0f * 6.28f);
			float fMultBright = SpriteBrightness / 30.0f * (0.25f * fStep + 0.75f);
			CRect rect;
#ifdef GTA_PC
			if(DrawCrossHairPC && TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam()) {
				float f3rdX = SCREEN_WIDTH * TheCamera.m_f3rdPersonCHairMultX;
				float f3rdY = SCREEN_HEIGHT * TheCamera.m_f3rdPersonCHairMultY;
#ifdef ASPECT_RATIO_SCALE
				f3rdY -= SCREEN_SCALE_Y(2.0f);
#endif
				if(FindPlayerPed() && WeaponType == WEAPONTYPE_M16) {
					rect.left = f3rdX - SCREEN_SCALE_X(32.0f * 0.6f);
					rect.top = f3rdY - SCREEN_SCALE_Y(32.0f * 0.6f);
					rect.right = f3rdX + SCREEN_SCALE_X(32.0f * 0.6f);
					rect.bottom = f3rdY + SCREEN_SCALE_Y(32.0f * 0.6f);

					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
				} else {
					rect.left = f3rdX - SCREEN_SCALE_X(32.0f * 0.4f);
					rect.top = f3rdY - SCREEN_SCALE_Y(32.0f * 0.4f);
					rect.right = f3rdX + SCREEN_SCALE_X(32.0f * 0.4f);
					rect.bottom = f3rdY + SCREEN_SCALE_Y(32.0f * 0.4f);

					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
				}
			} else
#endif
			{
				if(Mode == CCam::MODE_M16_1STPERSON
#ifdef GTA_PC
				   || Mode == CCam::MODE_M16_1STPERSON_RUNABOUT || Mode == CCam::MODE_HELICANNON_1STPERSON
#endif
				) {
					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(32.0f);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(32.0f);
					rect.right = (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(32.0f);
					rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_SCALE_Y(32.0f);
					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
				}
#ifdef GTA_PC
				else if(Mode == CCam::MODE_1STPERSON_RUNABOUT) {
					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(32.0f);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(32.0f);
					rect.right = (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(32.0f);
					rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_SCALE_Y(32.0f);

					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
				}
#endif
				else {
					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(32.0f);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(32.0f);
					rect.right = (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(32.0f);
					rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_SCALE_Y(32.0f);

					Sprites[HUD_SITEM16].Draw(CRect(rect), CRGBA(255, 255, 255, 255), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
				}
			}
			RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
			RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
			RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
		} else {
			SpriteBrightness = 0;
		}

		RwRenderStateSet(rwRENDERSTATEFOGENABLE, reinterpret_cast<void *>(0));
		RwRenderStateSet(rwRENDERSTATESRCBLEND, reinterpret_cast<void *>(5));
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, reinterpret_cast<void *>(6));
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void *>(1));
		RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, reinterpret_cast<void *>(3));
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, reinterpret_cast<void *>(0));
		RwRenderStateSet(rwRENDERSTATESHADEMODE, reinterpret_cast<void *>(1));
		RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);

		/*
		        DrawMoneyCounter
		*/
		wchar sPrint[16];
		wchar sPrintIcon[16];
		char sTemp[16];

		sprintf(sTemp, "$%09d", CWorld::Players[CWorld::PlayerInFocus].m_nVisibleMoney);
		AsciiToUnicode(sTemp, sPrint);

		CFont::SetPropOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(0.54f), SCREEN_SCALE_Y(0.96f));
		CFont::SetCentreOn();
		CFont::SetRightJustifyOff();
		CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 25));
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetFontStyle(FONT_HEADING);
		CFont::SetPropOn();
		CFont::SetColor(ALPHAHUD_COLOR);
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(180.0f), SCREEN_SCALE_Y(12.0f), sPrint);

		/*
		        DrawAmmo
		*/
		int32 AmmoAmount = CWeaponInfo::GetWeaponInfo(FindPlayerPed()->GetWeapon()->m_eWeaponType)->m_nAmountofAmmunition;
		int32 AmmoInClip = FindPlayerPed()->m_weapons[FindPlayerPed()->m_currentWeapon].m_nAmmoInClip;
		int32 TotalAmmo = FindPlayerPed()->m_weapons[FindPlayerPed()->m_currentWeapon].m_nAmmoTotal;
		int32 Ammo, Clip;

		sprintf(sTemp, "%d", TotalAmmo);

		AsciiToUnicode(sTemp, sPrint);

		/*
		        DrawWeaponIcon
		*/
		Sprites[WeaponType].Draw(CRect(SCREEN_SCALE_X(33.5f), SCREEN_SCALE_FROM_BOTTOM(190.0f), SCREEN_SCALE_X(33.5f) + SCREEN_SCALE_X(36.0f),
		                               SCREEN_SCALE_FROM_BOTTOM(190.0f) + SCREEN_SCALE_Y(34.5f)),
		                         CRGBA(255, 255, 255, 255), 0.015f, 0.015f, 1.0f, 0.0f, 0.015f, 1.0f, 1.0f, 1.0f);

		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(0.5f), SCREEN_SCALE_Y(0.74f));
		CFont::SetJustifyOff();
		CFont::SetCentreOff();
		CFont::SetJustifyOn();
		CFont::SetPropOn();
		CFont::SetFontStyle(FONT_HEADING);

		if(!CDarkel::FrenzyOnGoing() && WeaponType != WEAPONTYPE_UNARMED && WeaponType != WEAPONTYPE_BASEBALLBAT) {
			CFont::SetColor(ALPHAHUD_COLOR);
			CFont::PrintString(SCREEN_SCALE_X(AMMO_X), SCREEN_SCALE_FROM_BOTTOM(178.0f), sPrint);
		}

		/*
		        DrawHealth
		*/

		if(m_ItemToFlash == ITEM_HEALTH && CTimer::GetFrameCounter() & 8 || m_ItemToFlash != ITEM_HEALTH ||
		   FindPlayerPed()->m_fHealth < 10 && CTimer::GetFrameCounter() & 8) {
			if(FindPlayerPed()->m_fHealth >= 10 || FindPlayerPed()->m_fHealth < 10 && CTimer::GetFrameCounter() & 8) {

				if(FindPlayerPed()->m_fHealth > 100.0f
				       ? 1
				       : (!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss) ||
				             (CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss + 2000) ||
				             CTimer::GetFrameCounter() & 4)
					DrawAlphaProgressBar((34.0f), (155.0f), (34.0f), (112.0f), false);
			}
		}

		/*
		        DrawArmour
		*/
		if(FindPlayerPed()->m_fArmour > 1.0f) {
			if(m_ItemToFlash == ITEM_ARMOUR && CTimer::GetFrameCounter() & 8 || m_ItemToFlash != ITEM_ARMOUR) {
				CFont::SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));

				if(!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss ||
				   CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastArmourLoss + 2000 ||
				   CTimer::GetFrameCounter() & 4) {
					Sprites[HUD_ARMOUR].Draw(SCREEN_SCALE_X(124.0f), SCREEN_SCALE_FROM_BOTTOM(130.0f), SCREEN_SCALE_X(22.0f),
					                         SCREEN_SCALE_Y(20.0f), CRGBA(255, 255, 255, 255));

					AsciiToUnicode("[", sPrintIcon);
#ifdef FIX_BUGS
					sprintf(sTemp, "%d", int32((FindPlayerPed()->m_fArmour + 0.5f)) / 10);
#else
					sprintf(sTemp, "%03d", (int32)FindPlayerPed()->m_fArmour);
#endif
					AsciiToUnicode(sTemp, sPrint);

					CFont::SetJustifyOff();
					CFont::SetRightJustifyOff();
					CFont::SetCentreOn();
					CFont::SetBackgroundOff();
					CFont::SetFontStyle(FONT_BANK);
					CFont::SetDropShadowPosition(0);
					CFont::SetScale(SCREEN_SCALE_X(0.4f), SCREEN_SCALE_Y(0.6f));
					CFont::SetColor(ARMOUR_COLOR);
					CFont::PrintString(SCREEN_SCALE_X(135.0f), SCREEN_SCALE_FROM_BOTTOM(127.0f), sPrint);
				}
			}
		}

		/*
				GetOutOfHospitalFree
		*/

		if (CWorld::Players[CWorld::PlayerInFocus].m_bGetOutOfHospitalFree)
			Sprites[HUD_FREECARE].Draw(SCREEN_SCALE_X(136.0f), SCREEN_SCALE_FROM_BOTTOM(76.0f), SCREEN_SCALE_X(22.0f), SCREEN_SCALE_Y(20.0f),
				CRGBA(255, 255, 255, 255));

		/*
				GetOutOfJailFree
		*/

		if(CWorld::Players[CWorld::PlayerInFocus].m_bGetOutOfJailFree)
			Sprites[HUD_JAILFREE].Draw(SCREEN_SCALE_X(138.0f), SCREEN_SCALE_FROM_BOTTOM(102.0f), SCREEN_SCALE_X(22.0f), SCREEN_SCALE_Y(20.0f),
			                           CRGBA(255, 255, 255, 255));

		/*
				BonusHealth25%
		*/

		if(FindPlayerPed()->m_fHealth >= 101.0f) {
			if(!CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss ||
			   CTimer::GetTimeInMilliseconds() > CWorld::Players[CWorld::PlayerInFocus].m_nTimeLastHealthLoss + 2000 ||
			   CTimer::GetFrameCounter() & 4) {
				Sprites[HUD_HEALTH].Draw(SCREEN_SCALE_X(96.0f), SCREEN_SCALE_FROM_BOTTOM(146.0f), SCREEN_SCALE_X(22.0f), SCREEN_SCALE_Y(20.0f),
				                         CRGBA(255, 255, 255, 255));

				sprintf(sTemp, "%d", (int32(FindPlayerPed()->m_fHealth * 125) / 125) - 100);
				AsciiToUnicode(sTemp, sPrint);

				CFont::SetJustifyOff();
				CFont::SetRightJustifyOff();
				CFont::SetCentreOn();
				CFont::SetBackgroundOff();
				CFont::SetFontStyle(FONT_BANK);
				CFont::SetDropShadowPosition(0);
				CFont::SetScale(SCREEN_SCALE_X(0.4f), SCREEN_SCALE_Y(0.6f));
				CFont::SetColor(HEALTH_COLOR);
				CFont::PrintString(SCREEN_SCALE_X(107.0f), SCREEN_SCALE_FROM_BOTTOM(144.0f), sPrint);
			}
		}

		/*
		                                DrawWantedLevel
		*/
		float fStarsX = STARS_X;
		for(int i = 0; i < 6; i++) {
			if(FindPlayerPed()->m_pWanted->GetWantedLevel() > i &&
			   (CTimer::GetTimeInMilliseconds() > FindPlayerPed()->m_pWanted->m_nLastWantedLevelChange + 2000 || CTimer::GetFrameCounter() & 4)) {

				Sprites[HUD_WANTED].Draw(SCREEN_SCALE_X(fStarsX), SCREEN_SCALE_Y(14.0f), SCREEN_SCALE_X(16.0f), SCREEN_SCALE_Y(14.0f),
				                         CRGBA(ALPHAHUD_COLOR));
			}

			fStarsX += (18.0f);
		}

		/*
		                DrawZoneName
		*/

		if(m_pZoneName) {
			float fZoneAlpha = 255.0f;

			if(m_pZoneName != m_pLastZoneName) {
				switch(m_ZoneState) {
				case 0:
					m_ZoneState = 2;
					m_ZoneToPrint = m_pZoneName;
					m_ZoneNameTimer = 0;
					m_ZoneFadeTimer = 0;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
					m_ZoneNameTimer = 5;
					m_ZoneState = 4;
					break;
				default: break;
				}
				m_pLastZoneName = m_pZoneName;
			}

			if(m_ZoneState) {
				switch(m_ZoneState) {
				case 1:
					m_ZoneFadeTimer = 1000;
					if(m_ZoneNameTimer > 10000) {
						m_ZoneFadeTimer = 1000;
						m_ZoneState = 3;
					}
					fZoneAlpha = 255.0f;
					break;
				case 2:
					m_ZoneFadeTimer += CTimer::GetTimeStepInMilliseconds();
					if(m_ZoneFadeTimer > 1000) {
						m_ZoneState = 1;
						m_ZoneFadeTimer = 1000;
					}
					fZoneAlpha = m_ZoneFadeTimer / 1000.0f * 255.0f;
					break;
				case 3:
					m_ZoneFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if(m_ZoneFadeTimer < 0) {
						m_ZoneState = 0;
						m_ZoneFadeTimer = 0;
					}
					fZoneAlpha = m_ZoneFadeTimer / 1000.0f * 255.0f;
					break;
				case 4:
					m_ZoneFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if(m_ZoneFadeTimer < 0) {
						m_ZoneFadeTimer = 0;
						m_ZoneToPrint = m_pLastZoneName;
						m_ZoneState = 2;
					}
					fZoneAlpha = m_ZoneFadeTimer / 1000.0f * 255.0f;
					break;
				default: break;
				}

#ifndef HUD_ENHANCEMENTS
				if(!m_Message[0])
#else
				if(!m_Message[0] && !m_BigMessage[2][0]) // Hide zone name if wasted/busted text is displaying
#endif
				{
					m_ZoneNameTimer += CTimer::GetTimeStepInMilliseconds();
					CFont::SetJustifyOff();
					CFont::SetPropOn();
					CFont::SetBackgroundOff();

					CFont::SetScale(SCREEN_SCALE_X(0.86f), SCREEN_SCALE_Y(0.96f));
					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetBackGroundOnlyTextOff();
					CFont::SetFontStyle(FONT_BANK);
					CFont::SetColor(CRGBA(ALPHAHUD_COLOR.r, ALPHAHUD_COLOR.g, ALPHAHUD_COLOR.b, fZoneAlpha));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(ZONE_Y), m_ZoneToPrint);
				}
			}
		}

		/*
		        DrawVehicleName
		*/
		if(m_pVehicleName) {
			float fVehicleAlpha = 0.0f;

			if(m_pVehicleName != m_pLastVehicleName) {
				switch(m_VehicleState) {
				case 0:
					m_VehicleState = 2;
					m_pVehicleNameToPrint = m_pVehicleName;
					m_VehicleNameTimer = 0;
					m_VehicleFadeTimer = 0;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
					m_VehicleNameTimer = 0;
					m_VehicleState = 4;
					break;
				default: break;
				}
				m_pLastVehicleName = m_pVehicleName;
			}

			if(m_VehicleState) {
				switch(m_VehicleState) {
				case 1:
					if(m_VehicleNameTimer > 10000) {
						m_VehicleFadeTimer = 1000;
						m_VehicleState = 3;
					}
					fVehicleAlpha = 255.0f;
					break;
				case 2:
					m_VehicleFadeTimer += CTimer::GetTimeStepInMilliseconds();
					if(m_VehicleFadeTimer > 1000) {
						m_VehicleState = 1;
						m_VehicleFadeTimer = 1000;
					}
					fVehicleAlpha = m_VehicleFadeTimer / 1000.0f * 255.0f;
					break;
				case 3:
					m_VehicleFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if(m_VehicleFadeTimer < 0) {
						m_VehicleState = 0;
						m_VehicleFadeTimer = 0;
					}
					fVehicleAlpha = m_VehicleFadeTimer / 1000.0f * 255.0f;
					break;
				case 4:
					m_VehicleFadeTimer -= CTimer::GetTimeStepInMilliseconds();
					if(m_VehicleFadeTimer < 0) {
						m_VehicleFadeTimer = 0;
						m_pVehicleNameToPrint = m_pLastVehicleName;
						m_VehicleNameTimer = 0;
						m_VehicleState = 2;
					}
					fVehicleAlpha = m_VehicleFadeTimer / 1000.0f * 255.0f;
					break;
				default: break;
				}

#ifndef HUD_ENHANCEMENTS
				if(!m_Message[0])
#else
				if(!m_Message[0] && !m_BigMessage[2][0]) // Hide vehicle name if wasted/busted text is displaying
#endif
				{
					m_VehicleNameTimer += CTimer::GetTimeStepInMilliseconds();
					CFont::SetJustifyOff();
					CFont::SetPropOn();
					CFont::SetBackgroundOff();
					CFont::SetScale(SCREEN_SCALE_X(0.60f), SCREEN_SCALE_Y(0.96f));
					CFont::SetRightJustifyOn();
					CFont::SetRightJustifyWrap(0.0f);
					CFont::SetBackGroundOnlyTextOff();
					CFont::SetFontStyle(FONT_HEADING);
					CFont::SetColor(CRGBA(VEHICLE_COLOR.r, VEHICLE_COLOR.g, VEHICLE_COLOR.b, fVehicleAlpha));
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(VEHICLE_Y), m_pVehicleNameToPrint);
				}
			}
		} else {
			m_pLastVehicleName = nil;
			m_VehicleState = 0;
			m_VehicleFadeTimer = 0;
			m_VehicleNameTimer = 0;
		}

		/*
		        DrawClock
		*/
		CFont::SetJustifyOff();
		CFont::SetCentreOn();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(0.54f), SCREEN_SCALE_Y(0.96));
		CFont::SetBackGroundOnlyTextOff();
		CFont::SetPropOn();
		CFont::SetFontStyle(FONT_HEADING);
		CFont::SetRightJustifyOff();
		CFont::SetCentreOn();

		sprintf(sTemp, "%02d:%02d", CClock::GetHours(), CClock::GetMinutes());
		AsciiToUnicode(sTemp, sPrint);

		CFont::SetColor(ALPHAHUD_COLOR);
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(92.0f), SCREEN_SCALE_Y(36.0f), sPrint);

		/*
		        DrawOnScreenTimer
		*/
		wchar sTimer[16];

		CFont::SetPropOn();
		CFont::SetBackgroundOff();
		CFont::SetRightJustifyOn();
		CFont::SetRightJustifyWrap(0.0f);
		CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
		CFont::SetBackGroundOnlyTextOn();
		CFont::SetScale(SCREEN_SCALE_X(0.6f), SCREEN_SCALE_Y(0.91f));
		CFont::SetColor(ALPHAHUD_COLOR);

		if(!CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerProcessed) TimerOnLastFrame = false;
		if(!CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterProcessed) CounterOnLastFrame = false;

		if(CUserDisplay::OnscnTimer.m_bProcessed) {
			if(CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerProcessed) {
				if(!TimerOnLastFrame) TimerFlashTimer = 1;

				TimerOnLastFrame = true;

				if(TimerFlashTimer) {
					if(++TimerFlashTimer > 50) TimerFlashTimer = 0;
				}

				if(CTimer::GetFrameCounter() & 4 || !TimerFlashTimer) {
					AsciiToUnicode(CUserDisplay::OnscnTimer.m_sEntries[0].m_bTimerBuffer, sTimer);
					CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_Y(120.0f), sTimer);

					if(CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText[0]) {
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_Y(120.0f),
						                   TheText.Get(CUserDisplay::OnscnTimer.m_sEntries[0].m_aTimerText));
					}
				}
			}
			if(CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterProcessed) {
				if(!CounterOnLastFrame) CounterFlashTimer = 1;

				CounterOnLastFrame = true;

				if(CounterFlashTimer) {
					if(++CounterFlashTimer > 50) CounterFlashTimer = 0;
				}

				float fRightStr;
				if(CTimer::GetFrameCounter() & 4 || !CounterFlashTimer) {
					if(CUserDisplay::OnscnTimer.m_sEntries[0].m_nType == COUNTER_DISPLAY_NUMBER) {
						AsciiToUnicode(CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterBuffer, sTimer);
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_Y(140.0f), sTimer);
						fRightStr = CFont::GetStringWidth(sTimer, true);
					} else {
						int counter = atoi(CUserDisplay::OnscnTimer.m_sEntries[0].m_bCounterBuffer);
#ifdef FIX_BUGS
						counter = Min(counter, 100);
#endif
						CSprite2d::DrawRect(
						    CRect(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(100.0f) / 2 + SCREEN_SCALE_X_FIX(4.0f),
						          SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(8.0f),
						          SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) + SCREEN_SCALE_X_FIX(4.0f),
						          SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y_PC(11.0f) + SCREEN_SCALE_Y(8.0f)),
						    CRGBA(0, 106, 164, 80));

						CSprite2d::DrawRect(
						    CRect(SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) - SCREEN_SCALE_X(100.0f) / 2 + SCREEN_SCALE_X_FIX(4.0f),
						          SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(8.0f),
						          SCREEN_SCALE_X_PC((float)counter) / 2.0f + SCREEN_SCALE_FROM_RIGHT(TIMER_RIGHT_OFFSET) -
						              SCREEN_SCALE_X(100.0f) / 2.0f + SCREEN_SCALE_X_FIX(4.0f),
						          SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y_PC(11.0f) + SCREEN_SCALE_Y(8.0f)),
						    CRGBA(0, 106, 164, 255));

						fRightStr = SCREEN_SCALE_X(50.0f);
					}

					if(CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText[0]) {
						CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(30.0f) - fRightStr, SCREEN_SCALE_Y(140.0f),
						                   TheText.Get(CUserDisplay::OnscnTimer.m_sEntries[0].m_aCounterText));
					}
				}
			}
		}

		/////////////////////////////////
		/*
		        DrawPager
		*/
		if(!m_PagerMessage[0] && PagerOn == 1) {
			PagerSoundPlayed = false;
			PagerOn = 2;
		}
		if(m_PagerMessage[0] || PagerOn == 2) {
			if(!PagerOn) {
				PagerOn = 1;
				PagerXOffset = 150.0f;
			}
			if(PagerOn == 1) {
				if(PagerXOffset > 0.0f) {
					float fStep = PagerXOffset * 0.1f;
					if(fStep > 10.0f) fStep = 10.0f;
					PagerXOffset -= fStep * CTimer::GetTimeStep();
				}
				if(!PagerSoundPlayed) {
					DMAudio.PlayFrontEndSound(SOUND_PAGER, 0);
					PagerSoundPlayed = 1;
				}
			} else if(PagerOn == 2) {
				float fStep = PagerXOffset * 0.1f;
				if(fStep < 2.0f) fStep = 2.0f;
				PagerXOffset += fStep;
				if(PagerXOffset > 150.0f) {
					PagerXOffset = 150.0f;
					PagerOn = 0;
				}
			}

			float fWantedOffset = 0.0f;

			if(FindPlayerPed()->m_pWanted->m_nWantedLevel > 0) fWantedOffset = 16.0f;

			if(CTimer::GetTimeInMilliseconds() % (1000 + 300) < 1000)
				Sprites[HUD_PAGERON].Draw(CRect(SCREEN_SCALE_X(22.0f) - SCREEN_SCALE_X_FIX(PagerXOffset),
												SCREEN_SCALE_Y(16.0f + fWantedOffset),
				                                SCREEN_SCALE_X(132.0f) + SCREEN_SCALE_X(22.0f) - SCREEN_SCALE_X_FIX(PagerXOffset),
				                                SCREEN_SCALE_Y(54.0f) + SCREEN_SCALE_Y(16.0f + fWantedOffset)),
				                          CRGBA(255, 255, 255, 255));
			else
				Sprites[HUD_PAGEROFF].Draw(CRect(SCREEN_SCALE_X(22.0f) - SCREEN_SCALE_X_FIX(PagerXOffset),
				                                 SCREEN_SCALE_Y(16.0f + fWantedOffset),
				                                 SCREEN_SCALE_X(132.0f) + SCREEN_SCALE_X(22.0f) - SCREEN_SCALE_X_FIX(PagerXOffset),
				                                 SCREEN_SCALE_Y(54.0f) + SCREEN_SCALE_Y(16.0f + fWantedOffset)),
				                           CRGBA(255, 255, 255, 255));

			CFont::SetBackgroundOff();
			CFont::SetScale(SCREEN_SCALE_X(0.58f), SCREEN_SCALE_Y(1.2f));
			CFont::SetColor(PAGER_COLOR);
			CFont::SetRightJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetCentreOff();
			CFont::SetWrapx(SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH));
			CFont::SetJustifyOff();
			CFont::SetPropOff();
			CFont::SetFontStyle(FONT_PAGER);
			CFont::PrintString(SCREEN_SCALE_X(38.5f) - SCREEN_SCALE_X_FIX(PagerXOffset), SCREEN_SCALE_Y(10.0f + 16.0f + fWantedOffset),
			                   m_PagerMessage);
		}

		/*
		        DrawRadar
		*/
		if(m_ItemToFlash == ITEM_RADAR && CTimer::GetFrameCounter() & 8 || m_ItemToFlash != ITEM_RADAR) {
			CRadar::UpdateRadar();
			CRect rect(0.0f, 0.0f, SCREEN_SCALE_X(RADAR_WIDTH), SCREEN_SCALE_Y(RADAR_HEIGHT));
			rect.Translate(SCREEN_SCALE_X_FIX(RADAR_LEFT), SCREEN_SCALE_FROM_BOTTOM(RADAR_BOTTOM + RADAR_HEIGHT));

#ifdef PS2_HUD
#ifdef FIX_BUGS
			rect.Grow(SCREEN_SCALE_X(2.0f), SCREEN_SCALE_X(4.0f), SCREEN_SCALE_Y(2.0f), SCREEN_SCALE_Y(4.0f));
#else
			rect.Grow(2.0f, 4.0f);
#endif
#else
#ifdef FIX_BUGS
			rect.Grow(SCREEN_SCALE_X(4.0f), SCREEN_SCALE_X(4.0f), SCREEN_SCALE_Y(4.0f), SCREEN_SCALE_Y(4.0f));
#else
			rect.Grow(4.0f);
#endif
#endif
			if(Wants_To_Draw_Blips)
				CRadar::DrawBlips();
		}
	}

	/*
		Draw3dMarkers
	*/
	if (m_Wants_To_Draw_3dMarkers && !TheCamera.m_WideScreenOn && !m_BigMessage[0][0] && !m_BigMessage[2][0]) {
		CRadar::Draw3dMarkers();
	}

	/*
		DrawScriptText
	*/
	if (!CTimer::GetIsUserPaused()) {
		for (int i = 0; i < ARRAY_SIZE(CTheScripts::IntroTextLines); i++) {
			if (CTheScripts::IntroTextLines[i].m_Text[0] && CTheScripts::IntroTextLines[i].m_bTextBeforeFade) {
				CFont::SetScale(SCREEN_SCALE_X_PC(CTheScripts::IntroTextLines[i].m_fScaleX), SCREEN_SCALE_Y_PC(CTheScripts::IntroTextLines[i].m_fScaleY) 
#if !defined(PS2_HUD) || defined(FIX_BUGS)
					* 0.5f
#endif
				);

				CFont::SetColor(CTheScripts::IntroTextLines[i].m_sColor);

				if (CTheScripts::IntroTextLines[i].m_bJustify)
					CFont::SetJustifyOn();
				else
					CFont::SetJustifyOff();

				if (CTheScripts::IntroTextLines[i].m_bRightJustify)
					CFont::SetRightJustifyOn();
				else
					CFont::SetRightJustifyOff();

				if (CTheScripts::IntroTextLines[i].m_bCentered)
					CFont::SetCentreOn();
				else
					CFont::SetCentreOff();

				CFont::SetWrapx(SCALE_AND_CENTER_X_PC(CTheScripts::IntroTextLines[i].m_fWrapX));
				
				CFont::SetCentreSize(SCREEN_SCALE_X_PC(CTheScripts::IntroTextLines[i].m_fCenterSize));
				
				if (CTheScripts::IntroTextLines[i].m_bBackground)
					CFont::SetBackgroundOn();
				else
					CFont::SetBackgroundOff();

				CFont::SetBackgroundColor(CTheScripts::IntroTextLines[i].m_sBackgroundColor);

				if (CTheScripts::IntroTextLines[i].m_bBackgroundOnly)
					CFont::SetBackGroundOnlyTextOn();
				else
					CFont::SetBackGroundOnlyTextOff();

				if (CTheScripts::IntroTextLines[i].m_bTextProportional)
					CFont::SetPropOn();
				else
					CFont::SetPropOff();

				CFont::SetFontStyle(FONT_LOCALE(CTheScripts::IntroTextLines[i].m_nFont));
				
#if defined(PS2_HUD) && !defined(FIX_BUGS)
				CFont::PrintString(CTheScripts::IntroTextLines[i].m_fAtX, CTheScripts::IntroTextLines[i].m_fAtY, CTheScripts::IntroTextLines[i].m_Text);
#else
				CFont::PrintString(SCALE_AND_CENTER_X(DEFAULT_SCREEN_WIDTH - CTheScripts::IntroTextLines[i].m_fAtX), SCREEN_SCALE_Y(DEFAULT_SCREEN_HEIGHT - CTheScripts::IntroTextLines[i].m_fAtY), CTheScripts::IntroTextLines[i].m_Text);
#endif
			}
		}
		for (int i = 0; i < ARRAY_SIZE(CTheScripts::IntroRectangles); i++) {
			intro_script_rectangle &IntroRect = CTheScripts::IntroRectangles[i];

			// Yeah, top and bottom changed place. R* vision
			if (IntroRect.m_bIsUsed && IntroRect.m_bBeforeFade) {
				if (IntroRect.m_nTextureId >= 0) {
					CRect rect (
						IntroRect.m_sRect.left,
						IntroRect.m_sRect.bottom,
						IntroRect.m_sRect.right,
						IntroRect.m_sRect.top );

					CTheScripts::ScriptSprites[IntroRect.m_nTextureId].Draw(rect, IntroRect.m_sColor);
				}
				else {
					CRect rect (
						IntroRect.m_sRect.left,
						IntroRect.m_sRect.bottom,
						IntroRect.m_sRect.right,
						IntroRect.m_sRect.top );

					CSprite2d::DrawRect(rect, IntroRect.m_sColor);
				}
			}
		}

		/*
			DrawSubtitles
		*/
		if (m_Message[0] && !m_BigMessage[2][0] && (FrontEndMenuManager.m_PrefsShowSubtitles == 1 || !TheCamera.m_WideScreenOn)) {
			CFont::SetJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetBackgroundColor(CRGBA(0, 0, 0, 128));
			CFont::SetScale(SCREEN_SCALE_X_PC(0.58f), SCREEN_SCALE_Y_PC(1.12f));
			CFont::SetCentreOn();
			CFont::SetPropOn();
			CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));

#ifdef XBOX_SUBTITLES
			float radarBulge = SCREEN_SCALE_X(45.0f) + SCREEN_SCALE_X(16.0f);
			float rectWidth = SCREEN_WIDTH - SCREEN_SCALE_X(45.0f) - SCREEN_SCALE_X(16.0f) - radarBulge;
			CFont::SetCentreSize(rectWidth);
			CFont::SetColor(CRGBA(180, 180, 180, 255));

			CFont::PrintOutlinedString(rectWidth / 2.0f + radarBulge, SCREEN_SCALE_Y(4.0f) + SCREEN_SCALE_FROM_BOTTOM(48.0f) - SCREEN_SCALE_Y(1), m_Message,
				2.0f, true, CRGBA(0, 0, 0, 255));
#else
			float radarBulge = SCREEN_SCALE_X(40.0f) + SCREEN_SCALE_X(8.0f);
			float rectWidth = SCREEN_SCALE_FROM_RIGHT(50.0f) - SCREEN_SCALE_X(8.0f) - radarBulge;
			
			CFont::SetCentreSize(rectWidth);

			const int16 shadow = 1;
			CFont::SetDropShadowPosition(shadow);
			CFont::SetDropColor(CRGBA(0, 0, 0, 255));
			CFont::SetColor(CRGBA(235, 235, 235, 255));

			// I'm not sure shadow substaction was intentional here, might be a leftover if CFont::PrintString was used for a shadow draw call
			CFont::PrintString(rectWidth / 2.0f + radarBulge - SCREEN_SCALE_X_FIX(shadow), SCREEN_SCALE_Y_PC(4.0f) + SCREEN_SCALE_FROM_BOTTOM(SUBS_Y) - SCREEN_SCALE_Y_FIX(shadow), m_Message);
			CFont::SetDropShadowPosition(0);
#endif // #ifdef XBOX_SUBTITLES
		}

		/*
			DrawBigMessage
		*/
		// MissionCompleteFailedText
		if(m_BigMessage[0][0]) {
			if(BigMessageInUse[0] != 0.0f) {
				CFont::SetJustifyOff();
				CFont::SetBackgroundOff();
				CFont::SetBackGroundOnlyTextOff();

				CFont::SetScale(SCREEN_SCALE_X(1.72f), SCREEN_SCALE_Y(1.86f));

				CFont::SetPropOn();
				CFont::SetCentreOn();
				CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 25));
				CFont::SetDropShadowPosition(5);
				CFont::SetFontStyle(FONT_HEADING);

				// Appearently sliding text in here was abandoned very early, since this text is centered now.
#ifdef FIX_BUGS
				if(BigMessageX[0] >= SCALE_AND_CENTER_X(DEFAULT_SCREEN_WIDTH - 20))
#else
				if(BigMessageX[0] >= SCREEN_WIDTH - 20)
#endif
				{
					BigMessageInUse[0] += CTimer::GetTimeStep();

					if(BigMessageInUse[0] >= 120.0f) {
						BigMessageInUse[0] = 120.0f;
						BigMessageAlpha[0] -= (CTimer::GetTimeStepInMilliseconds() * 0.3f);
					}

					if(BigMessageAlpha[0] <= 0.0f) {
						m_BigMessage[0][0] = 0;
						BigMessageAlpha[0] = 0.0f;
					}
				} else {
					BigMessageX[0] += SCREEN_SCALE_X_FIX(CTimer::GetTimeStepInMilliseconds() * 0.3f);
					BigMessageAlpha[0] += (CTimer::GetTimeStepInMilliseconds() * 0.3f);

					if(BigMessageAlpha[0] > 255.0f) BigMessageAlpha[0] = 255.0f;
				}

				CFont::SetColor(CRGBA(128, 255, 255, BigMessageAlpha[0]));
				CFont::SetDropColor(CRGBA(0, 0, 0, BigMessageAlpha[0]));
				CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_SCALE_Y(-10.0f) + SCREEN_HEIGHT / 2, m_BigMessage[0]);
			} else {
				BigMessageAlpha[0] = 0.0f;
				BigMessageX[0] = SCALE_AND_CENTER_X_FIX(-60.0f);
				BigMessageInUse[0] = 1.0f;
			}
		} else {
			BigMessageInUse[0] = 0.0f;
		}


		// WastedBustedText
		if (m_BigMessage[2][0]) {
			if (BigMessageInUse[2] != 0.0f) {
				BigMessageAlpha[2] += (CTimer::GetTimeStepInMilliseconds() * 0.4f);

				if (BigMessageAlpha[2] > 255.0f)
					BigMessageAlpha[2] = 255.0f;

				CFont::SetBackgroundOff();

				CFont::SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.8f));

				CFont::SetPropOn();
				CFont::SetRightJustifyOff();
				CFont::SetCentreOn();
				CFont::SetFontStyle(FONT_HEADING);

				CFont::SetColor(CRGBA(ALPHAHUD_COLOR.r, ALPHAHUD_COLOR.g, ALPHAHUD_COLOR.b, BigMessageAlpha[2]));
				CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_SCALE_Y(-100.0f) + SCREEN_HEIGHT / 2, m_BigMessage[2]);
			}
			else {
				BigMessageAlpha[2] = 0.0f;
				BigMessageInUse[2] = 1.0f;
			}
		}
		else {
			BigMessageInUse[2] = 0.0f;
		}
	}

	DrawGamePaused();
}

void CHud::DrawAfterFade()
{
	if (CTimer::GetIsUserPaused() || CReplay::IsPlayingBack())
		return;

	if (m_HelpMessage[0]) {
		if (!CMessages::WideStringCompare(m_HelpMessage, m_LastHelpMessage, HELP_MSG_LENGTH)) {
			switch (m_HelpMessageState) {
			case 0:
				m_HelpMessageFadeTimer = 0;
				m_HelpMessageState = 2;
				m_HelpMessageTimer = 0;
				CMessages::WideStringCopy(m_HelpMessageToPrint, m_HelpMessage, HELP_MSG_LENGTH);
				m_HelpMessageDisplayTime = CMessages::GetWideStringLength(m_HelpMessage) / 20.0f + 3.0f;

				if (TheCamera.m_ScreenReductionPercentage == 0.0f)
					DMAudio.PlayFrontEndSound(SOUND_HUD, 0);
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				m_HelpMessageTimer = 5;
				m_HelpMessageState = 4;
				break;
			default:
				break;
			}
			CMessages::WideStringCopy(m_LastHelpMessage, m_HelpMessage, HELP_MSG_LENGTH);
		}

		float fAlpha = 195.0f;

		if (m_HelpMessageState != 0) {
			switch (m_HelpMessageState) {
			case 1:
				fAlpha = 195.0f;
				m_HelpMessageFadeTimer = 600;
				if (m_HelpMessageTimer > m_HelpMessageDisplayTime * 1000.0f || m_HelpMessageQuick && m_HelpMessageTimer > 1500.0f) {
					m_HelpMessageFadeTimer = 600;
					m_HelpMessageState = 3;
				}
				break;
			case 2:
				m_HelpMessageFadeTimer += 2 * CTimer::GetTimeStepInMilliseconds();
				if (m_HelpMessageFadeTimer > 0) {
					m_HelpMessageState = 1;
					m_HelpMessageFadeTimer = 0;
				}
				fAlpha = m_HelpMessageFadeTimer / 1000.0f * 195.0f;
				break;
			case 3:
				m_HelpMessageFadeTimer -= 2 * CTimer::GetTimeStepInMilliseconds();
				if (m_HelpMessageFadeTimer < 0) {
					m_HelpMessageState = 0;
					m_HelpMessageFadeTimer = 0;
				}
				fAlpha = m_HelpMessageFadeTimer / 1000.0f * 195.0f;
				break;
			case 4:
				m_HelpMessageFadeTimer -= 2 * CTimer::GetTimeStepInMilliseconds();
				if (m_HelpMessageFadeTimer < 0) {
					m_HelpMessageState = 2;
					m_HelpMessageFadeTimer = 0;
					CMessages::WideStringCopy(m_HelpMessageToPrint, m_LastHelpMessage, HELP_MSG_LENGTH);
				}
				fAlpha = m_HelpMessageFadeTimer / 1000.0f * 195.0f;
				break;
			default:
				break;
			}

			m_HelpMessageTimer += CTimer::GetTimeStepInMilliseconds();

			float fWantedOffset = 0.0f;

			if(FindPlayerPed()->m_pWanted->m_nWantedLevel > 0)
				fWantedOffset = 24.0f;

			CFont::SetAlphaFade(fAlpha);
			CFont::SetCentreOff();
			CFont::SetPropOn();

			CFont::SetScale(SCREEN_SCALE_X(0.48f), SCREEN_SCALE_Y(0.9f));
			CFont::SetJustifyOff();			
			CFont::SetWrapx(SCREEN_SCALE_X(180.0f));
			CFont::SetFontStyle(FONT_LOCALE(FONT_BANK));
			CFont::SetBackgroundOn();
			CFont::SetBackGroundOnlyTextOff();
			CFont::SetBackgroundColor(CRGBA(127, 120, 161, fAlpha));
			CFont::SetColor(CRGBA(255, 255, 255, 255));
			CFont::SetDropShadowPosition(0);
			CFont::PrintString(SCREEN_SCALE_X(26.0f), SCREEN_SCALE_Y(16.0f + fWantedOffset) + SCREEN_SCALE_Y_FIX((150.0f - PagerXOffset) * 0.4f), m_HelpMessageToPrint);
			CFont::SetAlphaFade(255.0f);
		}
	}

	for (int i = 0; i < ARRAY_SIZE(CTheScripts::IntroTextLines); i++) {
		intro_text_line &line = CTheScripts::IntroTextLines[i];
		if (line.m_Text[0] != '\0' && !line.m_bTextBeforeFade) {

			CFont::SetScale(SCREEN_SCALE_X_PC(line.m_fScaleX), SCREEN_SCALE_Y_PC(line.m_fScaleY)
#if !defined(PS2_HUD) || defined(FIX_BUGS)
				/ 2
#endif
				);
			CFont::SetColor(line.m_sColor);
			if (line.m_bJustify)
				CFont::SetJustifyOn();
			else
				CFont::SetJustifyOff();

			if (line.m_bRightJustify)
				CFont::SetRightJustifyOn();
			else
				CFont::SetRightJustifyOff();

			if (line.m_bCentered)
				CFont::SetCentreOn();
			else
				CFont::SetCentreOff();

			CFont::SetWrapx(SCALE_AND_CENTER_X_PC(line.m_fWrapX));
			CFont::SetCentreSize(SCREEN_SCALE_X_PC(line.m_fCenterSize));
			
			if (line.m_bBackground)
				CFont::SetBackgroundOn();
			else
				CFont::SetBackgroundOff();

			CFont::SetBackgroundColor(line.m_sBackgroundColor);
			if (line.m_bBackgroundOnly)
				CFont::SetBackGroundOnlyTextOn();
			else
				CFont::SetBackGroundOnlyTextOff();

			if (line.m_bTextProportional)
				CFont::SetPropOn();
			else
				CFont::SetPropOff();

			CFont::SetFontStyle(line.m_nFont);
#if defined(PS2_HUD) && !defined(FIX_BUGS)
			CFont::PrintString(line.m_fAtX, line.m_fAtY, line.m_Text);
#else
			CFont::PrintString(SCALE_AND_CENTER_X(DEFAULT_SCREEN_WIDTH - line.m_fAtX), SCREEN_SCALE_Y(DEFAULT_SCREEN_HEIGHT - line.m_fAtY), line.m_Text);
#endif
		}
	}
	for (int i = 0; i < ARRAY_SIZE(CTheScripts::IntroRectangles); i++) {
		intro_script_rectangle &rectangle = CTheScripts::IntroRectangles[i];
		if (rectangle.m_bIsUsed && !rectangle.m_bBeforeFade) {

			// Yeah, top and bottom changed place. R* vision
			if (rectangle.m_nTextureId >= 0) {
				CTheScripts::ScriptSprites[rectangle.m_nTextureId].Draw(CRect(rectangle.m_sRect.left, rectangle.m_sRect.bottom,
					rectangle.m_sRect.right, rectangle.m_sRect.top), rectangle.m_sColor);
			} else {
				CSprite2d::DrawRect(CRect(rectangle.m_sRect.left, rectangle.m_sRect.bottom,
					rectangle.m_sRect.right, rectangle.m_sRect.top), rectangle.m_sColor);
			}
		}
	}

	/*
		DrawBigMessage2
	*/
	// Oddjob
	if (m_BigMessage[3][0]) {
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X_PC(1.2f), SCREEN_SCALE_Y_PC(1.5f));
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 40));
		CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::SetColor(ALPHAHUD_COLOR);
		CFont::PrintString((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(BIGMESSAGE_Y), m_BigMessage[3]);
	}

	if (!m_BigMessage[1][0] && m_BigMessage[4][0]) {
		CFont::SetJustifyOff();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X_PC(1.2f), SCREEN_SCALE_Y_PC(1.5f));
		CFont::SetCentreOn();
		CFont::SetPropOn();
		CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 20));
		CFont::SetColor(CRGBA(0, 0, 0, 255));
		CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
		CFont::SetColor(ALPHAHUD_COLOR);
		CFont::PrintString((SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(BIGMESSAGE_Y), m_BigMessage[4]);
	}

	// Oddjob result
	if (OddJob2OffTimer > 0)
		OddJob2OffTimer -= CTimer::GetTimeStepInMilliseconds();

	float fStep;
	if (m_BigMessage[5][0] && OddJob2OffTimer <= 0.0f) {
		switch (OddJob2On) {
			case 0:
				OddJob2On = 1;
				OddJob2XOffset = 380.0f;
				break;
			case 1:
				if (OddJob2XOffset <= 2.0f) {
					OddJob2Timer = 0;
					OddJob2On = 2;
				}
				else {
					fStep = Min(40.0f, OddJob2XOffset / 6.0f);
					OddJob2XOffset = OddJob2XOffset - fStep;
				}
				break;
			case 2:
				OddJob2Timer += CTimer::GetTimeStepInMilliseconds();
				if (OddJob2Timer > 1500) {
					OddJob2On = 3;
				}
				break;
			case 3:
				fStep = Max(30.0f, OddJob2XOffset / 5.0f);

				OddJob2XOffset = OddJob2XOffset - fStep;

				if (OddJob2XOffset < -380.0f) {
					OddJob2OffTimer = 5000.0f;
					OddJob2On = 0;
				}
				break;
			default:
				break;
		}

		if (!m_BigMessage[1][0]) {
			CFont::SetJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetScale(SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(1.2f));
			CFont::SetCentreOn();
			CFont::SetPropOn();
			// Not bug, we just want these kind of texts to be wrapped at the center.
#ifdef ASPECT_RATIO_SCALE
			CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 20.0f));
#else
			CFont::SetCentreSize(SCREEN_SCALE_FROM_RIGHT(20.0f));
#endif
			CFont::SetColor(CRGBA(0, 0, 0, 255));
			CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));

#ifdef BETA_SLIDING_TEXT
			CFont::SetColor(ALPHAHUD_COLOR);
			CFont::PrintString(SCREEN_WIDTH / 2 - SCREEN_SCALE_X(OddJob2XOffset), SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(20.0f), m_BigMessage[5]);
#else
			CFont::PrintString(SCREEN_WIDTH / 2 + SCREEN_SCALE_X_PC(2.0f), SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(20.0f) + SCREEN_SCALE_Y_PC(2.0f), m_BigMessage[5]);
			CFont::SetColor(ODDJOB2_COLOR);
			CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - SCREEN_SCALE_Y(20.0f), m_BigMessage[5]);
#endif
		}
	}

	/*
		DrawMissionTitle
	*/
	if (m_BigMessage[1][0]) {
		if (BigMessageInUse[1] != 0.0f) {
			CFont::SetJustifyOff();
			CFont::SetBackgroundOff();
			CFont::SetScale(SCREEN_SCALE_X_PC(1.04f), SCREEN_SCALE_Y_PC(1.6f));

			CFont::SetPropOn();
#ifdef FIX_BUGS
			CFont::SetRightJustifyWrap(SCREEN_SCALE_FROM_RIGHT(DEFAULT_SCREEN_WIDTH + 500.0f));
#else
			CFont::SetRightJustifyWrap(-500.0f);
#endif
			CFont::SetRightJustifyOn();
			CFont::SetFontStyle(FONT_HEADING);
			
			if (BigMessageX[1] >= SCREEN_WIDTH - SCREEN_SCALE_X_FIX(20.0f))
			{
				BigMessageInUse[1] += CTimer::GetTimeStep();

				if (BigMessageInUse[1] >= 120.0f) {
					BigMessageInUse[1] = 120.0f;
					BigMessageAlpha[1] -= (CTimer::GetTimeStepInMilliseconds() * 0.3f);
				}
				if (BigMessageAlpha[1] <= 0) {
					m_BigMessage[1][0] = 0;
					BigMessageAlpha[1] = 0.0f;
				}
			} else {
				BigMessageX[1] += SCREEN_SCALE_X_FIX(CTimer::GetTimeStepInMilliseconds() * 0.3f);
				BigMessageAlpha[1] += (CTimer::GetTimeStepInMilliseconds() * 0.3f);

				if (BigMessageAlpha[1] > 255.0f)
					BigMessageAlpha[1] = 255.0f;
			}

#ifdef BETA_SLIDING_TEXT
			CFont::SetCentreOn();
			CFont::SetColor(CRGBA(ALPHAHUD_COLOR.r, ALPHAHUD_COLOR.g, ALPHAHUD_COLOR.b, BigMessageAlpha[1]));
			CFont::PrintString(SCREEN_WIDTH / 2, SCREEN_SCALE_Y(-20.0f) + SCREEN_HEIGHT / 2, m_BigMessage[1]);
#else
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f) + SCREEN_SCALE_X_FIX(2.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f) + SCREEN_SCALE_Y_PC(2.0f), m_BigMessage[1]);
			CFont::SetColor(CRGBA(MISSIONTITLE_COLOR.r, MISSIONTITLE_COLOR.g, MISSIONTITLE_COLOR.b, BigMessageAlpha[1]));
			CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(120.0f), m_BigMessage[1]);
#endif
		}
		else {
			BigMessageAlpha[1] = 0.0f;
#ifdef FIX_BUGS
			BigMessageX[1] = SCREEN_SCALE_FROM_RIGHT(DEFAULT_SCREEN_WIDTH + 60.0f);
#else
			BigMessageX[1] = -60.0f;
#endif
			BigMessageInUse[1] = 1.0f;
		}
	}
	else {
		BigMessageInUse[1] = 0.0f;
	}
}

void CHud::SetMessage(wchar *message)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(m_Message); i++) {
		if (message[i] == 0)
			break;

		m_Message[i] = message[i];
	}
	m_Message[i] = 0;
}

void CHud::SetBigMessage(wchar *message, uint16 style)
{
	int i = 0;

	if (style == 5) {
		for (i = 0; i < 128; i++) {
			if (message[i] == 0)
				break;

			if (message[i] != LastBigMessage[5][i]) {
				OddJob2On = 0;
				OddJob2OffTimer = 0.0f;
			}

			m_BigMessage[5][i] = message[i];
			LastBigMessage[5][i] = message[i];
		}
	} else {
		for (i = 0; i < 128; i++) {
			if (message[i] == 0)
				break;
			m_BigMessage[style][i] = message[i];
		}
	}
	LastBigMessage[style][i] = 0;
	m_BigMessage[style][i] = 0;
#ifndef FIX_BUGS
	m_BigMessage[style][i] = 0;
#endif
}

void CHud::SetPagerMessage(wchar *message)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(m_PagerMessage); i++) {
		if (message[i] == 0)
			break;

		m_PagerMessage[i] = message[i];
	}
	m_PagerMessage[i] = 0;
}