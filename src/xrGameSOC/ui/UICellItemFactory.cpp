#include "stdafx.h"
#include "UICellItemFactory.h"
#include "UICellCustomItems.h"



CUICellItem*	create_cell_item(CInventoryItem* itm)
{

	CWeaponAmmo* pAmmo					= smart_cast<CWeaponAmmo*>(itm);
	if(pAmmo)							return new CUIAmmoCellItem(pAmmo);

	CWeapon* pWeapon					= smart_cast<CWeapon*>(itm);
	if(pWeapon)							return new CUIWeaponCellItem(pWeapon);

	return new CUIInventoryCellItem(itm);
}
