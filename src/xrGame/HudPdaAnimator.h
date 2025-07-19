#pragma once

#include "HudAnimatorManager.h"
#include "player_hud.h"
#include "InertionData.h"

class CHudPdaAnimator : public CHudAnimatorBase
{
public:
	CHudPdaAnimator(CActor* parent, const shared_str& pda_section);
	virtual ~CHudPdaAnimator() {}

	enum EPdaStates
	{
		eIdle = 0,
		eShowing,
		eHiding,
		eBore,
		eBlowout,
		eHidden,
		eDeviceSwitch,
		eSprintStart,
		eSprintEnd,
		eAimStart,
		eAimEnd,
	};

	virtual void Load();
	virtual void Update();
	virtual void StopAnimator();
	virtual float GetHudFov() const;
	void SetState(u32 state) { OnStateSwitch(state); }
	u32 GetState() const { return m_current_state; }
	virtual bool IsActive() const { return GetState() != eHidden || m_bNeedActivated; }
	void SwitchPdaAnimator();
	virtual void OnMovementChanged();
	bool SwitchZoom();
	virtual bool need_renderable() { return GetState() != eHidden; }
	virtual bool CanSprint() const { return m_bCanSprint && !m_bIsZoomed && (GetState() == eIdle || GetState() == eSprintStart); }
	bool IsZoomed() const {return m_bIsZoomed;}

	virtual bool InputKeyPress(int cmd);

	virtual u8 GetCurrentHudOffsetIdx() const;
	virtual void UpdateHudAdditonal(Fmatrix&);

	virtual CHudPdaAnimator* cast_pda_animator() { return this; }

protected:

	u32 m_current_state = eHidden;
	u32 m_on_animation_end_state = eHidden;

	u32 m_dw_curr_state_time;
	u32	m_dw_curr_substate_time;

	//void OnMotionMark(const motion_marks& mark);
	void OnAnimationEnd(u32 state);
	void OnStateSwitch(u32 state);
	virtual void UpdateAnimation();
	void PlayMotion(const shared_str& name, bool blend, u32 state);
	u32 CurrStateTime() const { return Device.dwTimeGlobal - m_dw_curr_state_time; }
	void ResetSubStateTime() { m_dw_curr_substate_time = Device.dwTimeGlobal; }
	void PlayAnimIdle();
	void PlayAnimIdleMoving();
	void PlayAnimIdleMovingSlow();
	void PlayAnimIdleMovingCrouch();
	void PlayAnimIdleMovingCrouchSlow();
	void PlayAnimIdleSprint();
	bool TryPlayAnimIdle();

	float m_fBlowoutLevel = 1000.0f;
	float m_fZoomRotateTime = 0.25f;
	float m_fZoomRotationFactor = 0.0f;
	float m_fHudFovZoomFactor = 1.0f;

	bool m_bNeedBlowoutAnim = false;
	bool m_bSwitchSprint = false;
	bool m_bIsZoomed = false;
	bool m_bDisableBore = true;

	InertionData m_base_inertion;
	InertionData m_zoom_inertion;
};