#pragma once

#include "HudAnimatorManager.h"

class CHudItemAnimator : public CHudAnimatorBase
{
public:
	CHudItemAnimator(CActor* parent) : CHudAnimatorBase(parent) {}
	virtual ~CHudItemAnimator() {}

	virtual void Load();
	virtual void Update();
	virtual bool IsActive() const { return m_bIsPlaying || m_bNeedActivated; }
	void StartAnimator(const shared_str& section);
	virtual void StopAnimator();
	void SetLeftCallback(xr_delegate<void()> callback) { m_left_callback = callback; }
	void SetLeft2Callback(xr_delegate<void()> callback) { m_left2_callback = callback; }
	void SetRightCallback(xr_delegate<void()> callback) { m_right_callback = callback; }
	void SetRight2Callback(xr_delegate<void()> callback) { m_right2_callback = callback; }
	void SetStartCallback(xr_delegate<void()> callback) { m_start_callback = callback; }
	void SetEndCallback(xr_delegate<void()> callback) { m_end_callback = callback; }
	virtual float GetHudFov() const;
	virtual bool need_renderable() { return m_bIsPlaying; }

	virtual CHudItemAnimator* cast_item_animator() { return this; }

protected:

	virtual void OnMotionMark(const motion_marks& mark);
	void OnAnimationEnd();
	virtual void UpdateAnimation();
	void PlayMotion();

	void CallLeftCallback();
	void CallLeft2Callback();
	void CallRightCallback();
	void CallRight2Callback();
	void CallStartCallback();
	void CallEndCallback();

	bool m_bNeedActivated = false;
	bool m_bIsPlaying = false;
	bool m_bBlend = false;

	xr_delegate<void()> m_left_callback = nullptr;
	xr_delegate<void()> m_left2_callback = nullptr;
	xr_delegate<void()> m_right_callback = nullptr;
	xr_delegate<void()> m_right2_callback = nullptr;
	xr_delegate<void()> m_start_callback = nullptr;
	xr_delegate<void()> m_end_callback = nullptr;

	shared_str m_sLuaLeftCallback = "null";
	shared_str m_sLuaLeft2Callback = "null";
	shared_str m_sLuaRightCallback = "null";
	shared_str m_sLuaRight2Callback = "null";
	shared_str m_sLuaStartCallback = "null";
	shared_str m_sLuaEndCallback = "null";
	shared_str m_sLuaModifySect = "null";
	shared_str m_sLuaPrecondFunc = "null";
};