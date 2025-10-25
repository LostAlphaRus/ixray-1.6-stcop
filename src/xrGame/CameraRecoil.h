////////////////////////////////////////////////////////////////////////////
//	Module 		: CameraRecoil.h
//	Created 	: 26.05.2008
//	Author		: Evgeniy Sokolov
//	Description : Camera Recoil struct
////////////////////////////////////////////////////////////////////////////
#pragma once

//отдача при стрельбе
struct CameraRecoil
{
	// Структура для пружинной системы паттернов отдачи
	struct PatternParams
	{
		float Factor;
		float Stiffness;
		float Damping;
		float Impulse;
		float ReturnSpeed;
		float ReturnFactor;
		bool  ReturnEnable;

		PatternParams() :
			Factor(0.0f),
			Stiffness(0.0f),
			Damping(0.0f),
			Impulse(0.0f),
			ReturnSpeed(0.0f),
			ReturnFactor(0.0f),
			ReturnEnable(true)
		{
		}

		// Добавляем Reset для PatternParams
		IC void Reset()
		{
			Factor = 0.0f;
			Stiffness = 0.0f;
			Damping = 0.0f;
			Impulse = 0.0f;
			ReturnSpeed = 0.0f;
			ReturnFactor = 0.0f;
			ReturnEnable = true;
		}
	};

	float		RelaxSpeed;
	float		RelaxSpeed_AI;
	float		Dispersion;
	float		DispersionInc;
	float		DispersionFrac;
	float		MaxAngleVert;
	float		MaxAngleHorz;
	float		StepAngleHorz;
	bool		ReturnMode;
	bool		StopReturn;

	// Параметры пружинной системы для паттернов отдачи
	PatternParams	Pattern;

	CameraRecoil() :
		MaxAngleVert(EPS),
		RelaxSpeed(EPS_L),
		RelaxSpeed_AI(EPS_L),
		Dispersion(EPS),
		DispersionInc(0.0f),
		DispersionFrac(1.0f),
		MaxAngleHorz(EPS),
		StepAngleHorz(0.0f),
		ReturnMode(false),
		StopReturn(false)
	{
	}

	CameraRecoil(const CameraRecoil& clone) { Clone(clone); }

	IC void Clone(const CameraRecoil& clone)
	{
		// *this = clone;
		RelaxSpeed = clone.RelaxSpeed;
		RelaxSpeed_AI = clone.RelaxSpeed_AI;
		Dispersion = clone.Dispersion;
		DispersionInc = clone.DispersionInc;
		DispersionFrac = clone.DispersionFrac;
		MaxAngleVert = clone.MaxAngleVert;
		MaxAngleHorz = clone.MaxAngleHorz;
		StepAngleHorz = clone.StepAngleHorz;

		ReturnMode = clone.ReturnMode;
		StopReturn = clone.StopReturn;

		// Копируем параметры пружинной системы
		Pattern = clone.Pattern;

		VERIFY(!fis_zero(RelaxSpeed));
		VERIFY(!fis_zero(RelaxSpeed_AI));
		VERIFY(!fis_zero(MaxAngleVert));
		VERIFY(!fis_zero(MaxAngleHorz));
	}

	// Добавляем функцию Reset для CameraRecoil
	IC void Reset()
	{
		RelaxSpeed = EPS_L;
		RelaxSpeed_AI = EPS_L;
		Dispersion = EPS;
		DispersionInc = 0.0f;
		DispersionFrac = 1.0f;
		MaxAngleVert = EPS;
		MaxAngleHorz = EPS;
		StepAngleHorz = 0.0f;
		ReturnMode = false;
		StopReturn = false;

		// Сбрасываем параметры паттерна
		Pattern.Reset();

		// Проверки
		VERIFY(!fis_zero(RelaxSpeed));
		VERIFY(!fis_zero(RelaxSpeed_AI));
		VERIFY(!fis_zero(MaxAngleVert));
		VERIFY(!fis_zero(MaxAngleHorz));
	}
};