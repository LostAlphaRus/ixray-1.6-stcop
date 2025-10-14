// EffectorShot.cpp: implementation of the CCameraShotEffector class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EffectorShot.h"
#include "Weapon.h"

//-----------------------------------------------------------------------------
// Weapon shot effector
//-----------------------------------------------------------------------------
CWeaponShotEffector::CWeaponShotEffector()
{
	Reset();
	//	m_first_shot_pos = 0.0f;
}

void CWeaponShotEffector::Initialize(const CameraRecoil& cam_recoil)
{
	m_cam_recoil.Clone(cam_recoil);
	Reset();
}

void CWeaponShotEffector::Reset()
{
	m_angle_vert = 0.0f;
	m_angle_horz = 0.0f;

	m_target_angle_vert = 0.0f;
	m_target_angle_horz = 0.0f;
	m_velocity_vert = 0.0f;
	m_velocity_horz = 0.0f;

	spring_stiffness = 0.0f;
	damping = 0.0f;
	impulse_strengt = 0.0f;


	m_prev_angle_vert = 0.0f;
	m_prev_angle_horz = 0.0f;

	m_delta_vert = 0.0f;
	m_delta_horz = 0.0f;

	m_LastSeed = 0;
	m_single_shot = false;
	m_first_shot = false;
	m_actived = false;
	m_using_pattern = false;
	m_shot_end = true;
}

void CWeaponShotEffector::Shot(CWeapon* weapon)
{
	R_ASSERT(weapon);
	m_shot_numer = weapon->ShotsFired() - 1;
	if (m_shot_numer <= 0)
	{
		m_shot_numer = 0;
		Reset();
	}
	m_single_shot = (weapon->GetCurrentFireMode() == 1);

	// Получаем паттерн отдачи от оружия
	float pattern_x = 0.0f;
	float pattern_y = 0.0f;

	if (weapon->GetCurrentRecoilPattern(pattern_x, pattern_y))
	{
		// Используем паттернную систему
		m_using_pattern = true;

		// Сохраняем параметры пружины
		spring_stiffness = weapon->m_spring_stiffness;
		damping = weapon->m_spring_damping;
		impulse_strengt = weapon->m_impulse_strength;

		// Получаем множители паттерна от оружия
		float pattern_factor = weapon->GetCurrentPatternFactor();

		// Применяем множитель и вес
		float final_x = pattern_x * pattern_factor;
		float final_y = pattern_y * pattern_factor;

		// Используем значения из паттерна
		ShotFromPattern(final_x, final_y);
	}
	else
	{
		// Используем стандартную систему - СТАРУЮ ЛОГИКУ
		m_using_pattern = false;
		float angle = m_cam_recoil.Dispersion * weapon->cur_silencer_koef.cam_dispersion;
		angle += m_cam_recoil.DispersionInc * weapon->cur_silencer_koef.cam_disper_inc * (float)m_shot_numer;

		// Восстанавливаем старую логику Shot2
		Shot2Legacy(angle);
	}
}

void CWeaponShotEffector::ShotFromPattern(float pattern_x, float pattern_y)
{
	// Добавляем мгновенную скорость для резкого начала отдачи
 
	m_velocity_vert += pattern_y * impulse_strengt;
	m_velocity_horz += pattern_x * impulse_strengt;

	// Также обновляем целевые углы
	m_target_angle_vert += pattern_y;
	m_target_angle_horz += pattern_x;

	// Ограничиваем целевые значения
	clamp(m_target_angle_vert, -m_cam_recoil.MaxAngleVert, m_cam_recoil.MaxAngleVert);
	clamp(m_target_angle_horz, -m_cam_recoil.MaxAngleHorz, m_cam_recoil.MaxAngleHorz);

	Msg("Recoil impulse: vert=%.3f (vel=%.3f), horz=%.3f (vel=%.3f)",
		pattern_y, pattern_y * impulse_strengt,
		pattern_x, pattern_x * impulse_strengt);

	m_first_shot = true;
	m_actived = true;
	m_shot_end = false;
}


void CWeaponShotEffector::Shot2Legacy(float angle)
{
	// СТАРАЯ ЛОГИКА - работа напрямую с m_angle_vert и m_angle_horz
	m_angle_vert += angle * (m_cam_recoil.DispersionFrac + m_Random.randF(-1.0f, 1.0f) * (1.0f - m_cam_recoil.DispersionFrac));

	clamp(m_angle_vert, -m_cam_recoil.MaxAngleVert, m_cam_recoil.MaxAngleVert);
	if (fis_zero(m_angle_vert - m_cam_recoil.MaxAngleVert))
	{
		m_angle_vert *= m_Random.randF(0.96f, 1.04f);
	}

	float rdm = m_Random.randF(-1.0f, 1.0f);
	m_angle_horz += (m_angle_vert / m_cam_recoil.MaxAngleVert) * rdm * m_cam_recoil.StepAngleHorz;

	clamp(m_angle_horz, -m_cam_recoil.MaxAngleHorz, m_cam_recoil.MaxAngleHorz);

	m_first_shot = true;
	m_actived = true;
	m_shot_end = false;
}


void CWeaponShotEffector::UpdateSpringRecoil()
{
	if (!m_actived || !m_using_pattern) return;

	float dt = Device.fTimeDelta;

	// Вертикальная ось (пружинная физика)
	float acceleration_vert = (m_target_angle_vert - m_angle_vert) * spring_stiffness;
	acceleration_vert -= m_velocity_vert * damping;
	m_velocity_vert += acceleration_vert * dt;
	m_angle_vert += m_velocity_vert * dt;

	// Горизонтальная ось
	float acceleration_horz = (m_target_angle_horz - m_angle_horz) * spring_stiffness;
	acceleration_horz -= m_velocity_horz * damping;
	m_velocity_horz += acceleration_horz * dt;
	m_angle_horz += m_velocity_horz * dt;

	// Дополнительное ограничение текущих углов
	clamp(m_angle_vert, -m_cam_recoil.MaxAngleVert, m_cam_recoil.MaxAngleVert);
	clamp(m_angle_horz, -m_cam_recoil.MaxAngleHorz, m_cam_recoil.MaxAngleHorz);

	// Проверка на завершение движения
	bool is_vert_stable = _abs(m_velocity_vert) < 0.01f && _abs(m_angle_vert - m_target_angle_vert) < 0.001f;
	bool is_horz_stable = _abs(m_velocity_horz) < 0.01f && _abs(m_angle_horz - m_target_angle_horz) < 0.001f;

	if (is_vert_stable && is_horz_stable)
	{
		// Финализируем позиции
		m_angle_vert = m_target_angle_vert;
		m_angle_horz = m_target_angle_horz;
		m_velocity_vert = 0.0f;
		m_velocity_horz = 0.0f;

		// Отладочное сообщение
		if (m_shot_end)
		{
			Msg("Pattern recoil settled: vert=%.3f, horz=%.3f", m_angle_vert, m_angle_horz);
		}
	}
}

void CWeaponShotEffector::Relax()
{
	if (m_using_pattern)
	{
		// Релаксация для паттернной системы
		float time_to_relax = _abs(m_angle_vert) / m_cam_recoil.RelaxSpeed;
		float relax_speed_horz = (fis_zero(time_to_relax)) ? 0.0f : _abs(m_angle_horz) / time_to_relax;

		float dt = Device.fTimeDelta;

		if (m_angle_horz >= 0.0f)
		{
			m_angle_horz -= relax_speed_horz * dt;
		}
		else
		{
			m_angle_horz += relax_speed_horz * dt;
		}

		if (m_angle_vert >= 0.0f)
		{
			m_angle_vert -= m_cam_recoil.RelaxSpeed * dt;
			if (m_angle_vert < 0.0f)
			{
				m_angle_vert = 0.0f;
				m_actived = false;
			}
		}
		else
		{
			m_angle_vert += m_cam_recoil.RelaxSpeed * dt;
			if (m_angle_vert > 0.0f)
			{
				m_angle_vert = 0.0f;
				m_actived = false;
			}
		}
	}
	else
	{
		// СТАРАЯ ЛОГИКА релаксации для непаттернной системы
		float time_to_relax = _abs(m_angle_vert) / m_cam_recoil.RelaxSpeed;
		float relax_speed_horz = (fis_zero(time_to_relax)) ? 0.0f : _abs(m_angle_horz) / time_to_relax;

		float dt = Device.fTimeDelta;

		if (m_angle_horz >= 0.0f)
		{
			m_angle_horz -= relax_speed_horz * dt;
		}
		else
		{
			m_angle_horz += relax_speed_horz * dt;
		}

		if (m_angle_vert >= 0.0f)
		{
			m_angle_vert -= m_cam_recoil.RelaxSpeed * dt;
			if (m_angle_vert < 0.0f)
			{
				m_angle_vert = 0.0f;
				m_actived = false;
			}
		}
		else
		{
			m_angle_vert += m_cam_recoil.RelaxSpeed * dt;
			if (m_angle_vert > 0.0f)
			{
				m_angle_vert = 0.0f;
				m_actived = false;
			}
		}
	}
}

void CWeaponShotEffector::Update()
{

	
	if (m_using_pattern)
	{
		UpdateSpringRecoil();
	}


	if (m_actived && m_cam_recoil.ReturnMode /*|| m_single_shot*/)
	{
		Relax();
	}

	if (!m_cam_recoil.ReturnMode && m_shot_end && !m_single_shot)
	{
		m_actived = false;
	}

	m_delta_vert = m_angle_vert - m_prev_angle_vert;
	m_delta_horz = m_angle_horz - m_prev_angle_horz;

	m_prev_angle_vert = m_angle_vert;
	m_prev_angle_horz = m_angle_horz;

	//	Msg( " <<[%d]  v=%.4f  dv=%.4f   a=%d s=%d  fr=%d", m_shot_numer, m_angle_vert, m_delta_vert, m_actived, m_first_shot, Device.dwFrame );
}

void CWeaponShotEffector::GetDeltaAngle(Fvector& angle)
{
	angle.x = -m_angle_vert;
	angle.y = -m_angle_horz;
	angle.z = 0.0f;
}

void CWeaponShotEffector::GetLastDelta(Fvector& delta_angle)
{
	delta_angle.x = -m_delta_vert;
	delta_angle.y = -m_delta_horz;
	delta_angle.z = 0.0f;
}

void CWeaponShotEffector::SetRndSeed(s32 Seed)
{
	if (m_LastSeed == 0)
	{
		m_LastSeed = Seed;
		//		m_Random.seed		(Seed);
		m_Random.seed(Device.dwFrame);
	}
}

void CWeaponShotEffector::ChangeHP(float* pitch, float* yaw)
{
	*pitch -= m_delta_vert; // y = pitch = p = vert
	*yaw -= m_delta_horz; // x = yaw   = h = horz

	//	if ( m_first_shot )
	//	{
	//		m_first_shot_pos = *pitch;
	//		m_first_shot = false;
	//	}

	//	if ( m_cam_recoil.ReturnMode && m_cam_recoil.StopReturn && (*pitch > m_first_shot_pos + 0.1f) )
	//	{
	//		m_actived = false;
	//	}
	//	Msg( "[%d]  pitch = %.4f   yaw = %.4f    fs=%d    a=%d  fr=%d", m_shot_numer, *pitch, *yaw, m_first_shot, m_actived, Device.dwFrame );

}

//-----------------------------------------------------------------------------
// Camera shot effector
//-----------------------------------------------------------------------------

CCameraShotEffector::CCameraShotEffector(const CameraRecoil& cam_recoil)
	: CEffectorCam(eCEShot, 100000.0f)
{
	CWeaponShotEffector::Initialize(cam_recoil);
	m_pActor = nullptr;
}

CCameraShotEffector::~CCameraShotEffector()
{
}

BOOL CCameraShotEffector::ProcessCam(SCamEffectorInfo& info)
{
	Update();
	return TRUE;
}