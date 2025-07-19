#include "StdAfx.h"
#include "HudAnimatorManager.h"
#include "player_hud.h"

CHudAnimatorBase::~CHudAnimatorBase()
{
	StopAnimator();
}

void CHudAnimatorBase::Load()
{
	m_fHudFov = READ_IF_EXISTS(pSettings, r_float, m_section, "hud_fov", 0.0f);
	m_fHudFovFactor = READ_IF_EXISTS(pSettings, r_float, m_section, "hud_fov_factor", 1.0f);

	m_bCanSprint = READ_IF_EXISTS(pSettings, r_bool, m_section, "can_sprint", false);

	m_current_inertion.PitchOffsetR = READ_IF_EXISTS(pSettings, r_float, m_section, "inertion_pitch_offset_r", PITCH_OFFSET_R);
	m_current_inertion.PitchOffsetD = READ_IF_EXISTS(pSettings, r_float, m_section, "inertion_pitch_offset_d", PITCH_OFFSET_D);
	m_current_inertion.PitchOffsetN = READ_IF_EXISTS(pSettings, r_float, m_section, "inertion_pitch_offset_n", PITCH_OFFSET_N);

	m_current_inertion.OriginOffset = READ_IF_EXISTS(pSettings, r_float, m_section, "inertion_origin_offset", ORIGIN_OFFSET);
	m_current_inertion.TendtoSpeed = READ_IF_EXISTS(pSettings, r_float, m_section, "inertion_tendto_speed", TENDTO_SPEED);
}

void CHudAnimatorBase::Update()
{
	UpdateAnimation();
}

void CHudAnimatorBase::UpdateAnimation()
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
			}
		}
	}
}

void CHudAnimatorBase::StopAnimator()
{
	m_sounds.StopAllSounds();
	g_player_hud->delete_animator_item();
}

ENGINE_API extern float psHUD_FOV_def;

float CHudAnimatorBase::GetHudFov() const
{
	if (!m_fHudFov)
	{
		return psHUD_FOV_def * m_fHudFovFactor;
	}

	return m_fHudFov * m_fHudFovFactor;
}

bool CHudAnimatorBase::HudAnimationExist(const shared_str& name)
{
	if (g_player_hud->GetAnimator() != nullptr)
	{
		return g_player_hud->GetAnimator()->m_hand_motions.has_motion(name);
	}

	return false;
}

CHudAnimatorManager::CHudAnimatorManager(CActor* actor) : m_actor(actor)
{
	m_item_animator = new CHudItemAnimator(actor);
	//m_pda_animator = new CHudPdaAnimator(actor, "pda_show_animator_hud");
}

CHudAnimatorManager::~CHudAnimatorManager()
{
	xr_delete(m_item_animator);
	//xr_delete(m_pda_animator);

	m_actor = nullptr;
	//m_pda_animator = nullptr;
	m_item_animator = nullptr;
	m_current_animator = nullptr;
}

void CHudAnimatorManager::Update()
{
	if (ItemAnimator() != nullptr)
	{
		ItemAnimator()->Update();
	}

	//if (PdaAnimator() != nullptr)
	//{
	//	PdaAnimator()->Update();
	//}
}

bool CHudAnimatorManager::AnyAnimatorActive()
{
	//auto pda = PdaAnimator();
	auto item = ItemAnimator();

	//if (pda != nullptr && pda->IsActive())
	//{
	//	return true;
	//}

	if (item != nullptr && item->IsActive())
	{
		return true;
	}

	return false;
}

CHudAnimatorBase* CHudAnimatorManager::GetCurrentAnimator()
{
	if (m_current_animator && m_current_animator->IsActive())
	{
		return m_current_animator;
	}

	//auto pda = PdaAnimator();
	//if (pda && pda->IsActive())
	//{
	//	return m_current_animator = pda;
	//}

	auto item = ItemAnimator();
	if (item && item->IsActive())
	{
		return m_current_animator = item;
	}

	return m_current_animator = nullptr;
}

bool CHudAnimatorManager::CanSprint()
{
	if (CHudAnimatorBase* animator = GetCurrentAnimator())
	{
		return animator->CanSprint();
	}

	return true;
}

void CHudAnimatorManager::OnMovementChanged()
{
	if (CHudAnimatorBase* animator = GetCurrentAnimator())
	{
		animator->OnMovementChanged();
	}
}

const float CHudAnimatorManager::GetHudFov()
{
	if (CHudAnimatorBase* animator = GetCurrentAnimator())
	{
		return animator->GetHudFov();
	}

	return psHUD_FOV_def;
}

bool CHudAnimatorManager::InputKeyPress(int cmd)
{
	if (CHudAnimatorBase* animator = GetCurrentAnimator())
	{
		return animator->InputKeyPress(cmd);
	}

	return false;
}