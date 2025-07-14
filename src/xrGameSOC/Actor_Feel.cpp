#include "stdafx.h"
#include "actor.h"
#include "weapon.h"
#include "mercuryball.h"
#include "inventory.h"
#include "hudmanager.h"
#include "character_info.h"
#include "../xrEngine/xr_level_controller.h"
#include "UsableScriptObject.h"
#include "customzone.h"
#include "../xrEngine/GameMtlLib.h"
#include "ui/UIMainIngameWnd.h"
#include "Grenade.h"
#include "clsid_game.h"
#include "UIFontDefines.h"
#include "game_cl_base.h"
#include "Level.h"
#include "UIGameCustom.h"
#include "PickupManager.h"

void CActor::feel_touch_new				(CObject* O)
{
}

void CActor::feel_touch_delete	(CObject* O)
{
	CPhysicsShellHolder* sh=smart_cast<CPhysicsShellHolder*>(O);
	if(sh&&sh->character_physics_support()) m_feel_touch_characters--;
}

BOOL CActor::feel_touch_contact		(CObject *O)
{
	CInventoryItem	*item = smart_cast<CInventoryItem*>(O);
	CInventoryOwner	*inventory_owner = smart_cast<CInventoryOwner*>(O);

	if (item && item->Useful() && !item->object().H_Parent()) 
		return TRUE;

	if(inventory_owner && inventory_owner != smart_cast<CInventoryOwner*>(this))
	{
		CPhysicsShellHolder* sh=smart_cast<CPhysicsShellHolder*>(O);
		if(sh&&sh->character_physics_support()) m_feel_touch_characters++;
		return TRUE;
	}

	return		(FALSE);
}

BOOL CActor::feel_touch_on_contact	(CObject *O)
{
	CCustomZone	*custom_zone = smart_cast<CCustomZone*>(O);
	if (!custom_zone)
		return	(TRUE);

	Fsphere		sphere;
	sphere.P	= Position();
	sphere.R	= EPS_L;
	if (custom_zone->inside(sphere))
		return	(TRUE);

	return		(FALSE);
}

void CActor::PickupModeUpdate()
{
	if (!pPickup->GetPickupMode())
		return; // kUSE key pressed

	if (!IsGameTypeSingle())
		return;

	//подбирание объекта
	if(inventory().m_pTarget && inventory().m_pTarget->Useful() &&
		m_pUsableObject && m_pUsableObject->nonscript_usable() &&
		!Level().m_feel_deny.is_object_denied(smart_cast<CGameObject*>(inventory().m_pTarget)) )
	{
		NET_Packet P;
		u_EventGen(P,GE_OWNERSHIP_TAKE, ID());
		P.w_u16(inventory().m_pTarget->object().ID());
		u_EventSend(P);
	}

	pPickup->RenderInfo();
}

#include "../xrEngine/CameraBase.h"
BOOL	g_b_COD_PickUpMode = TRUE;
void	CActor::PickupModeUpdate_COD	()
{
	if (Level().CurrentViewEntity() != this || !g_b_COD_PickUpMode) return;
		
	if (!g_Alive() || eacFreeLook == cam_active) 
	{
		CurrentGameUI()->UIMainIngameWnd->SetPickUpItem(nullptr);
		return;
	};
	
	CFrustum frustum;
	frustum.CreateFromMatrix(Device.mFullTransform,FRUSTUM_P_LRTB|FRUSTUM_P_FAR);

	//---------------------------------------------------------------------------
	ISpatialResult.resize(0);
	g_SpatialSpace->q_frustum(ISpatialResult, 0, STYPE_COLLIDEABLE, frustum);
	//---------------------------------------------------------------------------

	float maxlen = 1000.0f;
	CInventoryItem* pNearestItem = nullptr;
	for (u32 o_it=0; o_it<ISpatialResult.size(); o_it++)
	{
		ISpatialShared		spatial	= ISpatialResult[o_it];
		CInventoryItem*	pIItem	= smart_cast<CInventoryItem*> (spatial->dcast_CObject        ());
		if (0 == pIItem) continue;
		if (pIItem->object().H_Parent() != nullptr) continue;
		if (!pIItem->CanTake()) continue;
		if (pIItem->object().CLS_ID == CLSID_OBJECT_G_RPG7 || pIItem->object().CLS_ID == CLSID_OBJECT_G_FAKE)
			continue;

		CGrenade*	pGrenade	= smart_cast<CGrenade*> (spatial->dcast_CObject        ());
		if (pGrenade && !pGrenade->Useful()) continue;

		CMissile*	pMissile	= smart_cast<CMissile*> (spatial->dcast_CObject        ());
		if (pMissile && !pMissile->Useful()) continue;
		
		Fvector A, B, tmp; 
		pIItem->object().Center			(A);
		if (A.distance_to_sqr(Position())>4) continue;

		tmp.sub(A, cam_Active()->vPosition);
		B.mad(cam_Active()->vPosition, cam_Active()->vDirection, tmp.dotproduct(cam_Active()->vDirection));
		float len = B.distance_to_sqr(A);
		if (len > 1) continue;

		if (maxlen>len && !pIItem->object().getDestroy())
		{
			maxlen = len;
			pNearestItem = pIItem;
		};
	}

	if(pNearestItem)
	{
		CFrustum					frustum;
		frustum.CreateFromMatrix	(Device.mFullTransform,FRUSTUM_P_LRTB|FRUSTUM_P_FAR);
		if (!pPickup->CanPickItem(frustum,Device.vCameraPosition,&pNearestItem->object()))
			pNearestItem = nullptr;
	}

	if (pNearestItem && pNearestItem->cast_game_object())
	{
		if (Level().m_feel_deny.is_object_denied(pNearestItem->cast_game_object()))
				pNearestItem = nullptr;
	}

	CurrentGameUI()->UIMainIngameWnd->SetPickUpItem(pNearestItem);

	if (pNearestItem && pPickup->GetPickupMode())
	{
		//подбирание объекта
		Game().SendPickUpEvent(ID(), pNearestItem->object().ID());
	}
};

void CActor::feel_sound_new(CObject* who, int type, CSound_UserDataPtr user_data, const Fvector& Position, float power)
{
	if(who == this)
		m_snd_noise = _max(m_snd_noise,power);
}
