#pragma once

#include "HudSound.h"
#include "Actor.h"

class HUD_SOUND_COLLECTION;
class CActor;
class CMotionDef;
class CHudPdaAnimator;
class CHudItemAnimator;

class CHudAnimatorBase
{
public:

	CHudAnimatorBase(CActor* parent) : m_actor(parent) {}
	virtual ~CHudAnimatorBase();
	virtual void Load();
	void SetSection(const shared_str& new_section) { m_section = new_section; }
	shared_str GetSection() const { return m_section; }
	virtual void Update();
	virtual bool IsActive() const { return false; }
	virtual void StopAnimator();
	u8 GetSlotToRestore() const { return m_iRestoreSlot; }
	virtual float GetHudFov() const;
	virtual bool CanSprint() const { return m_bCanSprint; }
	void SetForceHideItems(bool value) { m_bForceHideItems = value; }
	bool IsForceHideItems() const { return m_bForceHideItems; }
	virtual void OnMovementChanged() {}
	virtual bool need_renderable() { return true; }

	virtual bool InputKeyPress(int cmd) { return false; }
	virtual bool InputKeyHold(int cmd) { return false; }
	virtual bool InputKeyRelease(int cmd) { return false; }
	virtual bool InputMouseMove(int x, int y) { return false; }
	virtual bool InputMouseWheel(int direction) { return false; }
	virtual bool InputGamepadUpdateStick(int id, Fvector2 value) { return false; }
	virtual bool InputGamepadKeyPress(int id) { return false; }

	InertionData& CurrentInertionData() { return m_current_inertion; }

	virtual u8 GetCurrentHudOffsetIdx() const { return 0; }
	virtual void UpdateHudAdditonal(Fmatrix&) {}

	virtual CHudPdaAnimator* cast_pda_animator() { return nullptr; }
	virtual CHudItemAnimator* cast_item_animator() { return nullptr; }

protected:

	virtual void OnMotionMark(const motion_marks& mark) {}
	virtual void UpdateAnimation();
	bool HudAnimationExist(const shared_str& name);

	shared_str m_section;
	HUD_SOUND_COLLECTION m_sounds;
	CActor* m_actor = nullptr;
	bool m_bRestoreDetector = false;
	u8 m_iRestoreSlot = 0;
	bool m_bNeedActivated = false;
	float m_fHudFov = 0.0f;
	float m_fHudFovFactor = 1.0f;
	bool m_bForceHideItems = false;
	bool m_bCanSprint = false;

	u32	m_dwMotionCurrTm = 0;
	u32	m_dwMotionStartTm = 0;
	u32	m_dwMotionEndTm = 0;
	bool m_bStopAtEndAnimIsRunning = true;
	const CMotionDef* m_current_motion_def = nullptr;

	InertionData m_current_inertion;
};

#include "HudPdaAnimator.h"
#include "HudItemAnimator.h"

class CHudAnimatorManager
{
public:
	CHudAnimatorManager(CActor* parent);
	~CHudAnimatorManager();

	void Update();
	bool InputKeyPress(int cmd);
	bool InputKeyHold(int cmd) { return false; }
	bool InputKeyRelease(int cmd) { return false; }
	bool InputMouseMove(int x, int y) { return false; }
	bool InputMouseWheel(int direction) { return false; }
	bool InputGamepadUpdateStick(int id, Fvector2 value) { return false; }
	bool InputGamepadKeyPress(int id) { return false; }
	bool AnyAnimatorActive();
	bool CanSprint();
	void OnMovementChanged();
	const float GetHudFov();

	CHudPdaAnimator* PdaAnimator() { return m_pda_animator; }
	CHudItemAnimator* ItemAnimator() { return m_item_animator; }
	CHudAnimatorBase* GetCurrentAnimator();

private:
	CActor* m_actor = nullptr;
	CHudPdaAnimator* m_pda_animator = nullptr;
	CHudItemAnimator* m_item_animator = nullptr;
	CHudAnimatorBase* m_current_animator = nullptr;
};