#pragma once
#include "smart_zone.h"
#include "space_restrictor.h"

class CDynamicWallmarkZone : public CSmartZone {
private:
    typedef CSmartZone			inherited;

protected:

    shared_str shader = "effects\\wallmark";
    shared_str texture = "";
    float h = 1.0f, w = 1.0f, r = 0.0f;

    bool CurrentStatus = false;

public:

    virtual BOOL					net_Spawn(CSE_Abstract* DC) override;

    void SwitchWallmark(bool isOn);

};