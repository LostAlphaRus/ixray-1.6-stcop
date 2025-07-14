#pragma once

#include "weaponmagazinedwgrenade.h"
#include "../xrScripts/script_export_space.h"

class CWeaponGroza :
	public CWeaponMagazinedWGrenade
{
	typedef CWeaponMagazinedWGrenade inherited;
public:
	CWeaponGroza(void);
	virtual ~CWeaponGroza(void);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
