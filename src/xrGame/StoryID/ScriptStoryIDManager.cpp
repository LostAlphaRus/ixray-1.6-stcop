#include "StdAfx.h"
#include "ScriptStoryIDManager.h"

#include "alife_object_registry.h"
#include "alife_simulator.h"

CScriptStoryIDManager& CScriptStoryIDManager::GetInstance()
{
    static CScriptStoryIDManager instance;
    return instance;
}

void CScriptStoryIDManager::VerifiedRegisterObject(CSE_Abstract* se_obj)
{
    auto& self = CScriptStoryIDManager::GetInstance();
    {
        if (auto Casted = smart_cast<CSE_ALifeDynamicObject*>(se_obj); Casted && Casted->m_script_story_ID.size())
        {
            self.Register(se_obj->ID, Casted->m_script_story_ID);
            return;
        }
    }
    auto& ini = se_obj->spawn_ini();
    if (ini.section_exist("story_object"))
    {
        LPCSTR key;
        LPCSTR value;
        if (!ini.r_line("story_object", 0, &key, &value) || !key)
        {
            R_ASSERT3(false, "There is no 'story_id' field in [story_object] section :object", se_obj->name());
        }
        if (!value)
        {
            R_ASSERT3(false, "Field 'story_id' in [story_object] section got no value :object", se_obj->name());
        }
        self.Register(se_obj->ID, value);
        //if (auto Casted = smart_cast<CSE_ALifeDynamicObject*>(se_obj); Casted)
        //{
        //    Casted->m_script_story_ID = value;
        //}
        return;
    }
    auto story_id = READ_IF_EXISTS(pSettings, r_string, se_obj->name_replace(), "story_id", nullptr);
    if (story_id)
    {
        self.Register(se_obj->ID, story_id);
        //if (auto Casted = smart_cast<CSE_ALifeDynamicObject*>(se_obj); Casted)
        //{
        //    Casted->m_script_story_ID = story_id;
        //}
    }
}

namespace ScriptStoryIDManager
{

    ALife::_OBJECT_ID get(CScriptStoryIDManager& manager, LPCSTR story_id)
    {
       return manager.GetID(story_id);
    }

    LPCSTR get_story_id(CScriptStoryIDManager& manager, ALife::_OBJECT_ID id)
    {
       return manager.GetID(id);
    }
}

void CScriptStoryIDManager::script_register(lua_State* L)
{
    using namespace luabind;
    
    module(L, "story_objects")[
        class_<CScriptStoryIDManager>("CScriptStoryIDManager")
            .def("register",	&CScriptStoryIDManager::Register)
            .def("unregister_by_id", (void(CScriptStoryIDManager::*)(ALife::_OBJECT_ID))&CScriptStoryIDManager::Unregister)
            .def("unregister_by_story_id", (void(CScriptStoryIDManager::*)(LPCSTR))&CScriptStoryIDManager::Unregister)
            .def("get", &ScriptStoryIDManager::get)
            .def("get_story_id", &ScriptStoryIDManager::get_story_id),
            def("get_story_objects_registry", &CScriptStoryIDManager::GetInstance),
            def("check_spawn_ini_for_story_id", &CScriptStoryIDManager::VerifiedRegisterObject)
            ];
}

void CScriptStoryIDManager::Register(ALife::_OBJECT_ID obj_id, shared_str script_story_id)
{
    SContainer* container = new SContainer(obj_id, script_story_id);
    auto ByIDIt = m_containers_by_id.find(container);
    auto ByScriptStoryIDIt = m_containers_by_script_story_id.find(container);
    if (ByScriptStoryIDIt != m_containers_by_script_story_id.end() && (*ByScriptStoryIDIt)->m_obj_id != obj_id)
    {
        xr_string message = "You are trying to spawn two or more objects with the same story_id:[";
        message.append(script_story_id.c_str());
        message.append("] --> [");
        auto ExistName = ai().alife().objects().object((*ByScriptStoryIDIt)->m_obj_id)->name();
        message.append(ExistName);
        message.append("] try to add:[");
        auto NewName = ai().alife().objects().object(obj_id)->name();
        message.append("]");
        R_ASSERT2(ByIDIt == m_containers_by_script_story_id.end(), message.c_str());
    }
    if (ByIDIt != m_containers_by_id.end()){
        xr_string message = "Object [";
        message.append(script_story_id.c_str());
        message.append("] is already in story_objects_registry with story_id[");
        message.append((*ByIDIt)->m_script_story_id.c_str());
        R_ASSERT2(ByScriptStoryIDIt != m_containers_by_script_story_id.end(), message.c_str());
    }
    m_containers_by_id.insert(container);
    m_containers_by_script_story_id.insert(container);
}

void CScriptStoryIDManager::Unregister(ALife::_OBJECT_ID obj_id)
{
    SContainer cont;
    cont.m_obj_id = obj_id;
    VERIFY(m_containers_by_id.contains(&cont));
    if (m_containers_by_id.contains(&cont)){
        auto elem = *m_containers_by_id.find(&cont);
        m_containers_by_id.erase(elem);
        m_containers_by_script_story_id.erase(elem);
        xr_delete(elem);
    }
}

void CScriptStoryIDManager::Unregister(LPCSTR script_story_id)
{
    SContainer cont;
    cont.m_script_story_id = script_story_id;
    VERIFY(m_containers_by_script_story_id.contains(&cont));
    if (m_containers_by_script_story_id.contains(&cont)){
        auto elem = *m_containers_by_script_story_id.find(&cont);
        m_containers_by_id.erase(elem);
        m_containers_by_script_story_id.erase(elem);
        xr_delete(elem);
    }
}

ALife::_OBJECT_ID CScriptStoryIDManager::GetID(LPCSTR script_story_id) const
{
    SContainer cont;
    cont.m_script_story_id = script_story_id;
    return m_containers_by_script_story_id.contains(&cont) ?
        (*m_containers_by_script_story_id.find(&cont))->m_obj_id : ALife::_OBJECT_ID(-1);
    //R_ASSERT(m_containers_by_script_story_id.contains(&cont), "Unable to find obj ID from script story ID", script_story_id);
    //return (*m_containers_by_script_story_id.find(&cont))->m_obj_id;
}

LPCSTR CScriptStoryIDManager::GetID(ALife::_OBJECT_ID obj_id) const
{
    SContainer cont;
    cont.m_obj_id = obj_id;
    //return m_containers_by_id.contains(&cont) ?
    //    (*m_containers_by_id.find(&cont))->m_script_story_id.c_str() : nullptr;
    R_ASSERT3(m_containers_by_id.contains(&cont), "Unable to find script story ID from obj ID", std::to_string(obj_id).c_str());
    return m_containers_by_id.contains(&cont) ? (*m_containers_by_id.find(&cont))->m_script_story_id.c_str() : nullptr;
}
