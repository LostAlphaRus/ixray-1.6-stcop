#include "stdafx.h"
#include "DynamicWallmarkZone.h"

#include "../xrScripts/script_engine.h"
#include "xrServer_Objects_ALife.h"

BOOL CDynamicWallmarkZone::net_Spawn(CSE_Abstract* DC)
{
    auto result = inherited::net_Spawn(DC);

    auto Casted = smart_cast<CSE_ALifeDynamicWallmark*>(DC);

    shader = Casted->shader;
    texture = Casted->texture;
    w = Casted->w;
    h = Casted->h;
    r = Casted->r;
	
    return result;
}

void CDynamicWallmarkZone::SwitchWallmark(bool isOn)
{
    if (isOn == CurrentStatus)
    {
        return;
    }
    CurrentStatus = isOn;
    if (isOn)
    {
		
    } else
    {
		
    }
}