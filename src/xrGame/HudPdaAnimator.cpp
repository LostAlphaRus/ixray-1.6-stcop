#include "StdAfx.h"
#include "HudPdaAnimator.h"
#include "UIGameCustom.h"
#include "Inventory.h"

CHudPdaAnimator::CHudPdaAnimator(CActor* parent, const shared_str& section) : CHudAnimatorBase(parent)
{
	m_section = section;
	Load();
}

void CHudPdaAnimator::Load()
{
	CHudAnimatorBase::Load();

	if (pSettings->line_exist(m_section, "snd_draw"))
	{
		m_sounds.LoadSound(m_section.c_str(), "snd_draw", "sndDraw", true);
	}

	if (pSettings->line_exist(m_section, "snd_holster"))
	{
		m_sounds.LoadSound(m_section.c_str(), "snd_holster", "sndHide", true);
	}

	if (pSettings->line_exist(m_section, "snd_blowout"))
	{
		m_sounds.LoadSound(m_section.c_str(), "snd_blowout", "sndBlowout", true);
	}

	m_fBlowoutLevel = READ_IF_EXISTS(pSettings, r_float, m_section, "blowout_anim_level", 1000.0f);
	m_fZoomRotateTime = READ_IF_EXISTS(pSettings, r_float, m_section, "zoom_rotate_time", 0.25f);

	m_base_inertion = m_current_inertion;

	m_zoom_inertion.PitchOffsetR = READ_IF_EXISTS(pSettings, r_float, m_section, "inertion_aim_pitch_offset_r", 0.0f);
	m_zoom_inertion.PitchOffsetD = READ_IF_EXISTS(pSettings, r_float, m_section, "inertion_aim_pitch_offset_d", 0.0f);
	m_zoom_inertion.PitchOffsetN = READ_IF_EXISTS(pSettings, r_float, m_section, "inertion_aim_pitch_offset_n", 0.0f);

	m_zoom_inertion.OriginOffset = READ_IF_EXISTS(pSettings, r_float, m_section, "inertion_aim_origin_offset", ORIGIN_OFFSET * 0.5f);
	m_zoom_inertion.TendtoSpeed = READ_IF_EXISTS(pSettings, r_float, m_section, "inertion_aim_tendto_speed", TENDTO_SPEED);

	m_fHudFovZoomFactor = READ_IF_EXISTS(pSettings, r_float, m_section, "hud_fov_zoom_factor", m_fHudFovFactor);

	m_bDisableBore = READ_IF_EXISTS(pSettings, r_bool, m_section, "disable_bore", true);
}

void CHudPdaAnimator::Update()
{
	if (GetState() != eHidden)
	{
		psHUD_Flags.set(HUD_CROSSHAIR_RT2, false);
		psHUD_Flags.set(HUD_DRAW_RT, !m_bIsZoomed);

		m_current_inertion.lerp(m_base_inertion, m_zoom_inertion, m_fZoomRotationFactor);
	}

	if (m_bNeedActivated)
	{
		bool wpn_hide = !g_player_hud->attached_item(0) && !m_actor->inventory().ActiveItem() && !m_actor->inventory().GetNextActiveSlot() && !m_actor->inventory().GetActiveSlot();
		if (wpn_hide && !g_player_hud->attached_item(1))
		{
			m_bNeedActivated = false;
			SetState(eShowing);
		}
		else
		{
			CHudItem* active_item = m_actor->inventory().ActiveItem() ? m_actor->inventory().ActiveItem()->cast_hud_item() : nullptr;
			if (active_item && active_item->GetState() != CHUDState::eHiding)
			{
				u16 slot = m_actor->inventory().GetActiveSlot();
				m_iRestoreSlot = slot;
				m_actor->inventory().Activate(NO_ACTIVE_SLOT);
			}

			if (m_actor->GetDetector() && m_actor->GetDetector()->GetState() != CHUDState::eHiding)
			{
				m_bRestoreDetector = true;
				m_actor->GetDetector()->HideDetector(true, true);
			}
		}
	}
	else
	{
		if (GetState() == eIdle)
		{
			static const bool UseBlowoutAnim = g_player_hud->GetAnimator()->m_hand_motions.has_motion("anm_blowout");
			if (UseBlowoutAnim && !m_bSwitchSprint && m_bNeedBlowoutAnim && m_fBlowoutLevel <= m_actor->CurrentElectronicsProblemsCnt())
			{
				m_bNeedBlowoutAnim = false;
				SetState(eBlowout);
			}

			if (!m_bDisableBore && Device.dwTimeGlobal - m_dw_curr_substate_time > 20000)
			{
				SetState(eBore);
				ResetSubStateTime();
			}
		}
	}
	UpdateAnimation();
}

void CHudPdaAnimator::PlayMotion(const shared_str& name, bool blend, u32 state)
{
	m_on_animation_end_state = state;

	u32 ret = g_player_hud->GetAnimator()->anim_play(name, blend, m_current_motion_def);

	if (ret > 0)
	{
		m_dwMotionStartTm = Device.dwTimeGlobal;
		m_dwMotionCurrTm = m_dwMotionStartTm;
		m_dwMotionEndTm = m_dwMotionStartTm + ret;
		m_bStopAtEndAnimIsRunning = true;
	}
	else
		m_bStopAtEndAnimIsRunning = false;
}

void CHudPdaAnimator::UpdateAnimation()
{
	if (m_current_motion_def)
	{
		if (m_bStopAtEndAnimIsRunning)
		{
			auto& marks = m_current_motion_def->marks;
			if (!marks.empty())
			{
				float motion_prev_time = ((float)m_dwMotionCurrTm - (float)m_dwMotionStartTm) / 1000.0f;
				float motion_curr_time = ((float)Device.dwTimeGlobal - (float)m_dwMotionStartTm) / 1000.0f;
			
				for (auto& M : marks)
				{
					if (M.is_empty())
						continue;
			
					auto Iprev = M.pick_mark(motion_prev_time);
					auto Icurr = M.pick_mark(motion_curr_time);
					if (Iprev == nullptr && Icurr != nullptr)
					{
						OnMotionMark(M);
					}
				}
			
			}

			m_dwMotionCurrTm = Device.dwTimeGlobal;
			if (m_dwMotionCurrTm > m_dwMotionEndTm)
			{
				m_current_motion_def = nullptr;
				m_dwMotionStartTm = 0;
				m_dwMotionEndTm = 0;
				m_dwMotionCurrTm = 0;
				m_bStopAtEndAnimIsRunning = false;
				OnAnimationEnd(m_on_animation_end_state);
			}
		}
	}
}

void CHudPdaAnimator::OnAnimationEnd(u32 state)
{
	switch (state)
	{
	case eHiding:
	{
		SetState(eHidden);

		// Расскоментировать, когда будет готов рендер
		//if (auto ui = CurrentGameUI())
		//{
		//	ui->HidePdaMenu();
		//}

		if (m_iRestoreSlot > 0 && m_actor->inventory().ItemFromSlot(m_iRestoreSlot))
		{
			m_actor->inventory().Activate(m_iRestoreSlot);
			m_iRestoreSlot = 0;
		}

		if (m_bRestoreDetector && m_actor->GetDetector(true))
		{
			m_actor->GetDetector(true)->ToggleDetector(true, true);
			m_bRestoreDetector = false;
		}
	}break;
	case eBlowout:
	case eIdle:
	case eBore:
	case eShowing:
	case eSprintStart:
	case eSprintEnd:
	case eAimStart:
	case eAimEnd:
	{
		SetState(eIdle);
	}break;
	}
}

void CHudPdaAnimator::OnStateSwitch(u32 state)
{
	m_current_state = state;

	if (state != eIdle)
	{
		m_dw_curr_state_time = Device.dwTimeGlobal;
		ResetSubStateTime();
	}

	switch (state)
	{
	case eShowing:
	{
		g_player_hud->create_animator_item(this, m_section);
		PlayMotion("anm_show", false, eShowing);

		m_bNeedBlowoutAnim = true;

		if (m_sounds.FindSoundItem("sndDraw", false))
		{
			m_sounds.PlaySound("sndDraw", zero_vel, m_actor, true);
		}

		// Расскоментировать, когда будет готов рендер
		//if (auto ui = CurrentGameUI())
		//{
		//	ui->ShowPdaMenu();
		//}
	}break;
	case eHiding:
	{
		if (m_sounds.FindSoundItem("sndHide", false))
		{
			m_sounds.PlaySound("sndHide", zero_vel, m_actor, true);
		}
		PlayMotion("anm_hide", true, eHiding);
	}break;
	case eIdle:
	{
		PlayAnimIdle();
	}break;
	case eHidden:
	{
		StopAnimator();
	}break;
	case eBlowout:
	{
		if (m_sounds.FindSoundItem("sndBlowout", false))
		{
			m_sounds.PlaySound("sndBlowout", zero_vel, m_actor, true);
		}
		PlayMotion("anm_blowout", true, eBlowout);
	}break;
	case eBore:
	{
		PlayMotion("anm_bore", true, eBore);
	}break;
	case eSprintStart:
	{
		m_bSwitchSprint = true;
		PlayMotion("anm_idle_sprint_start", true, eSprintStart);
		break;
	}
	case eSprintEnd:
	{
		m_bSwitchSprint = false;
		PlayMotion("anm_idle_sprint_end", true, eSprintEnd);
		break;
	}
	case eAimStart:
	{
		PlayMotion("anm_idle_aim_start", true, eAimStart);
		break;
	}
	case eAimEnd:
	{
		PlayMotion("anm_idle_aim_end", true, eAimEnd);
		break;
	}
	}

	if (state != eIdle && state != eSprintStart && state != eSprintEnd)
	{
		m_bSwitchSprint = false;
	}
}

void CHudPdaAnimator::StopAnimator()
{
	m_actor->set_inventory_disabled(false);
	m_bNeedBlowoutAnim = false;
	m_sounds.StopAllSounds();
	g_player_hud->delete_animator_item();
	m_current_state = eHidden;
	m_on_animation_end_state = eHidden;
	m_bIsZoomed = false;
}

ENGINE_API extern float psHUD_FOV_def;

float CHudPdaAnimator::GetHudFov() const
{
	bool wpn_hide = !g_player_hud->attached_item(0) && !g_player_hud->attached_item(1);
	if (IsActive() && wpn_hide)
	{
		float get = CHudAnimatorBase::GetHudFov() / m_fHudFovFactor;
		float hud_fov = m_fHudFovFactor;

		if (((IsZoomed() && m_fZoomRotationFactor <= 1.f) || (!IsZoomed() && m_fZoomRotationFactor > 0.f)))
		{
			hud_fov = hud_fov - (hud_fov - m_fHudFovZoomFactor) * m_fZoomRotationFactor;
		}

		return get * hud_fov;
	}

	return psHUD_FOV_def * m_fHudFovFactor;
}

void CHudPdaAnimator::SwitchPdaAnimator()
{
	if (GetState() == eIdle)
	{
		SetState(eHiding);
		m_actor->set_inventory_disabled(false);
	}
	else if (!m_bNeedActivated && GetState() == eHidden && g_player_hud->GetAnimator() == nullptr)
	{
		m_bNeedActivated = true;
		m_actor->set_inventory_disabled(true);
		if (auto ui = CurrentGameUI())
		{
			ui->HideActorMenu();
		}
	}
}

void CHudPdaAnimator::OnMovementChanged()
{
	if (GetState() == eIdle && !m_bStopAtEndAnimIsRunning)
	{
		PlayAnimIdle();
		ResetSubStateTime();
	}
}

void CHudPdaAnimator::PlayAnimIdle()
{
	if (TryPlayAnimIdle())
	{
		return;
	}

	if (m_bIsZoomed)
	{
		PlayMotion("anm_idle_aim", true, eIdle);
	}
	else
	{
		PlayMotion("anm_idle", true, eIdle);
	}
}

bool CHudPdaAnimator::TryPlayAnimIdle()
{
	if (m_bIsZoomed)
	{
		return false;
	}

	u32 state = m_actor->GetMovementState(eReal);
	if (state & ACTOR_DEFS::EMoveCommand::mcSprint && CanSprint() && HudAnimationExist("anm_idle_sprint"))
	{
		if (!m_bSwitchSprint && HudAnimationExist("anm_idle_sprint_start"))
		{
			SetState(eSprintStart);
			return true;
		}

		PlayAnimIdleSprint();
		return true;
	}
	else if (m_bSwitchSprint && HudAnimationExist("anm_idle_sprint_end"))
	{
		SetState(eSprintEnd);
		return true;
	}
	else if (state & ACTOR_DEFS::EMoveCommand::mcAnyMove)
	{
		if (state & ACTOR_DEFS::EMoveCommand::mcCrouch && (HudAnimationExist("anm_idle_moving_crouch_slow") || HudAnimationExist("anm_idle_moving_crouch")))
		{
			if (state & ACTOR_DEFS::EMoveCommand::mcAccel && HudAnimationExist("anm_idle_moving_crouch_slow"))
				PlayAnimIdleMovingCrouchSlow();
			else
				PlayAnimIdleMovingCrouch();

			return true;
		}
		else
		{
			if (state & ACTOR_DEFS::EMoveCommand::mcAccel && HudAnimationExist("anm_idle_moving_slow"))
				PlayAnimIdleMovingSlow();
			else
				PlayAnimIdleMoving();

			return true;
		}
	}

	return false;
}

void CHudPdaAnimator::PlayAnimIdleMoving()
{
	PlayMotion("anm_idle_moving", true, eIdle);
}

void CHudPdaAnimator::PlayAnimIdleMovingSlow()
{
	PlayMotion("anm_idle_moving_slow", true, eIdle);
}

void CHudPdaAnimator::PlayAnimIdleMovingCrouch()
{
	PlayMotion("anm_idle_moving_crouch", true, eIdle);
}

void CHudPdaAnimator::PlayAnimIdleMovingCrouchSlow()
{
	PlayMotion("anm_idle_moving_crouch_slow", true, eIdle);
}

void CHudPdaAnimator::PlayAnimIdleSprint()
{
	PlayMotion("anm_idle_sprint", true, eIdle);
}

bool CHudPdaAnimator::SwitchZoom()
{
	if (GetState() != eIdle)
	{
		return false;
	}

	if (m_bIsZoomed)
	{
		m_bIsZoomed = false;
		SetState(eAimEnd);
	}
	else
	{
		m_bIsZoomed = true;
		SetState(eAimStart);
	}

	return true;
}

u8 CHudPdaAnimator::GetCurrentHudOffsetIdx() const
{
	bool b_aiming = ((m_bIsZoomed && m_fZoomRotationFactor <= 1.0f) || (!m_bIsZoomed && m_fZoomRotationFactor > 0.0f));

	if (!b_aiming)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void CHudPdaAnimator::UpdateHudAdditonal(Fmatrix& trans)
{
	u8 idx = GetCurrentHudOffsetIdx();

	animator_item* animator = g_player_hud->GetAnimator();

	Fvector curr_offs = animator->m_hands_positions.hands_offsets[0][idx];//pos,aim
	Fvector curr_rot = animator->m_hands_positions.hands_offsets[1][idx];//rot,aim
	curr_offs.mul(m_fZoomRotationFactor);
	curr_rot.mul(m_fZoomRotationFactor);

	Fmatrix	hud_rotation;
	hud_rotation.identity();
	hud_rotation.rotateX(curr_rot.x);

	Fmatrix	hud_rotation_y;
	hud_rotation_y.identity();
	hud_rotation_y.rotateY(curr_rot.y);
	hud_rotation.mulA_43(hud_rotation_y);

	hud_rotation_y.identity();
	hud_rotation_y.rotateZ(curr_rot.z);
	hud_rotation.mulA_43(hud_rotation_y);

	hud_rotation.translate_over(curr_offs);
	trans.mulB_43(hud_rotation);

	if (m_bIsZoomed)
	{
		m_fZoomRotationFactor += Device.fTimeDelta / m_fZoomRotateTime;
	}
	else
	{
		m_fZoomRotationFactor -= Device.fTimeDelta / m_fZoomRotateTime;
	}

	clamp(m_fZoomRotationFactor, 0.f, 1.f);
}

bool CHudPdaAnimator::InputKeyPress(int cmd)
{
	if (cmd == kWPN_ZOOM)
	{
		return SwitchZoom();
	}

	return false;
}