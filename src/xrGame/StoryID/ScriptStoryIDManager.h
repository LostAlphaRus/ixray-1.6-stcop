#pragma once
#include "script_game_object.h"

class CScriptStoryIDManager
{
    
    struct SContainer
    {
        ALife::_OBJECT_ID m_obj_id;
        shared_str m_script_story_id;

        bool operator==(const SContainer&) const = default;
    };

    struct SContainerObjIDPred
    {
        size_t operator()(SContainer* s) const
        {
            return std::hash<ALife::_OBJECT_ID>{}(s->m_obj_id);
        }
    };

    struct SContainerScriptStoryIDPred
    {
        size_t operator()(SContainer* s) const
        {
            return std::hash<shared_str>{}(s->m_script_story_id);
        }
    };

    xr_hash_set<SContainer*,SContainerObjIDPred> m_containers_by_id;
    xr_hash_set<SContainer*,SContainerScriptStoryIDPred> m_containers_by_script_story_id;

    CScriptStoryIDManager() = default;
    
public:

    CScriptStoryIDManager(const CScriptStoryIDManager&) = delete;
    CScriptStoryIDManager& operator=(const CScriptStoryIDManager&) = delete;
    CScriptStoryIDManager(CScriptStoryIDManager&&) = delete;
    CScriptStoryIDManager& operator=(CScriptStoryIDManager&&) = delete;

    void Register(ALife::_OBJECT_ID obj_id, shared_str script_story_id);
    void Unregister(ALife::_OBJECT_ID obj_id);
    void Unregister(LPCSTR script_story_id);
    ALife::_OBJECT_ID GetID(LPCSTR script_story_id) const;
    LPCSTR GetID(ALife::_OBJECT_ID obj_id) const;
    
    static CScriptStoryIDManager& GetInstance();
    static void VerifiedRegisterObject(CSE_Abstract* se_obj);
    static void script_register(lua_State *L);
};


