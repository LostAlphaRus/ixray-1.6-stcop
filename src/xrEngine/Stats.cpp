#include "stdafx.h"
#include "GameFont.h"


#include "../xrCore/Collision/ISpatial.h"
#include "IGame_Persistent.h"
#include "Render.h"
#include "xr_object.h"

#include "../Include/xrRender/DrawUtils.h"

int		g_ErrorLineCount	= 15;
Flags32 g_stats_flags		= {0};

// stats
DECLARE_RP(Stats);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BOOL			g_bDisableRedText	= FALSE;
CStats::CStats	()
{
	fFPS				= 30.f;
	fRFPS				= 30.f;
	fTPS				= 0;
	pFont				= 0;
	fMem_calls			= 0;
	RenderDUMP_DT_Count = 0;
	Device.seqRender.Add		(this,REG_PRIORITY_LOW-1000);
}

CStats::~CStats()
{
	Device.seqRender.Remove		(this);
	xr_delete		(pFont);
}

void _draw_cam_pos(CGameFont* pFont)
{
	float sz		= pFont->GetHeight();
	pFont->SetColor	(0xffffffff);
	pFont->Out		(10, 600, "CAMERA POSITION:  [%3.2f,%3.2f,%3.2f]",VPUSH(Device.vCameraPosition));
	pFont->SetHeight(sz);
	pFont->OnRender	();
}

#define FONT_SIZE 15.f
#define SKIP_SIZE 0.25f

void drawStatParam(CGameFont* F, LPCSTR text)
{
	F->SetColor(color_rgba(128, 128, 192, 255));
	F->OutNext(text);
	F->OutSkip(SKIP_SIZE);
}


void drawStatParamBy(CGameFont* F, CStats* stats, LPCSTR text, u32 calls)
{
	F->SetColor(color_rgba(0, 255, 0, 255));
	F->OutNext(text, calls);
	F->OutSkip(SKIP_SIZE);
};

void drawStatParamByMS(float MAX_Stat, CGameFont* F, CStats* stats, LPCSTR text, float stat)
{
	if (stat < MAX_Stat)
		F->SetColor(color_rgba(128, 128, 192, 255));
	else
		if (stat > MAX_Stat && stat < MAX_Stat * 2)
			F->SetColor(color_rgba(255, 0, 128, 255));
		else
			F->SetColor(color_rgba(255, 0, 0, 255));

	F->OutNext(text, stat);
	F->OutSkip(SKIP_SIZE);
};


void drawStatParamByMS(float MAX_Stat, CGameFont* F, CStats* stats, LPCSTR text, float stat, float stat2)
{
	if (stat < MAX_Stat)
		F->SetColor(color_rgba(128, 128, 192, 255));
	else
		if (stat > MAX_Stat && stat < MAX_Stat * 2)
			F->SetColor(color_rgba(255, 0, 128, 255));
		else
			F->SetColor(color_rgba(255, 0, 0, 255));

	F->OutNext(text, stat, stat2);
	F->OutSkip(SKIP_SIZE);
};

void drawStatParamByMS(float MAX_Stat, CGameFont* F, CStats* stats, LPCSTR text, float stat, u32 stat2, float stat3)
{
	if (stat < MAX_Stat)
		F->SetColor(color_rgba(128, 128, 192, 255));
	else
		if (stat > MAX_Stat && stat < MAX_Stat * 2)
			F->SetColor(color_rgba(255, 0, 128, 255));
		else
			F->SetColor(color_rgba(255, 0, 0, 255));

	F->OutNext(text, stat, stat2, stat3);
	F->OutSkip(SKIP_SIZE);
};


CGameFont* pFontGame = 0;


void CStats::Show()
{
	// calc FPS & TPS
	if (Device.fTimeDelta > EPS_S)
	{
		float fps = 1.f / Device.fTimeDelta;
		//if (Engine.External.tune_enabled)	vtune.update	(fps);
		float fOne = 0.3f;
		float fInv = 1.f - fOne;
		fFPS = fInv * fFPS + fOne * fps;
		FPS = fFPS;
		if (RenderTOTAL.result > EPS_S)
		{
			u32	rendered_polies = Device.m_pRender->GetCacheStatPolys();
			fTPS = fInv * fTPS + fOne * float(rendered_polies) / (RenderTOTAL.result * 1000.f);
			//fTPS = fInv*fTPS + fOne*float(RCache.stat.polys)/(RenderTOTAL.result*1000.f);
			fRFPS = fInv * fRFPS + fOne * 1000.f / RenderTOTAL.result;
		}
	}

	{
		float mem_count = float(Memory.stat_calls);
		if (mem_count > fMem_calls)	fMem_calls = mem_count;
		else						fMem_calls = .9f * fMem_calls + .1f * mem_count;
		Memory.stat_calls = 0;
	}

	////////////////////////////////////////////////
	if (g_dedicated_server) return;
	////////////////////////////////////////////////


	// Stop timers
	if (true)
	{
		for (auto& stat : FrameTicks)
			stat.second.FrameEnd();

		EngineFrame.FrameEnd();
		EngineMTFrame.FrameEnd();
		EngineMTFrameSCore.FrameEnd();

		Sheduler.FrameEnd();
		ShedulerLow.FrameEnd();

		UpdateClient.FrameEnd();
		Physics.FrameEnd();
		ph_collision.FrameEnd();
		ph_core.FrameEnd();
		Animation.FrameEnd();
		AI_Think.FrameEnd();
		AI_Range.FrameEnd();
		AI_Path.FrameEnd();
		AI_Node.FrameEnd();
		AI_Vis.FrameEnd();
		AI_Vis_Query.FrameEnd();
		AI_Vis_RayTests.FrameEnd();

		RenderTOTAL.FrameEnd();
		RenderCALC.FrameEnd();
		RenderCALC_HOM.FrameEnd();

		RenderDUMP.FrameEnd();
		RenderDUMP_Second.FrameEnd();

		RenderDUMP_RT.FrameEnd();

		RenderDUMP_SKIN.FrameEnd();
		RenderDUMP_Wait.FrameEnd();
		RenderDUMP_Wait_S.FrameEnd();
		RenderDUMP_HUD.FrameEnd();
		RenderDUMP_Glows.FrameEnd();
		RenderDUMP_Lights.FrameEnd();
		RenderDUMP_WM.FrameEnd();
		RenderDUMP_DT_VIS.FrameEnd();
		RenderDUMP_DT_Render.FrameEnd();
		RenderDUMP_DT_Cache.FrameEnd();
		RenderDUMP_Pcalc.FrameEnd();
		RenderDUMP_Scalc.FrameEnd();
		RenderDUMP_Srender.FrameEnd();

		Sound.FrameEnd();
		Input.FrameEnd();
		clRAY.FrameEnd();
		clBOX.FrameEnd();
		clFRUSTUM.FrameEnd();

		netClient1.FrameEnd();
		netClient2.FrameEnd();
		netServer.FrameEnd();

		netClientCompressor.FrameEnd();
		netServerCompressor.FrameEnd();

		TEST0.FrameEnd();
		TEST1.FrameEnd();
		TEST2.FrameEnd();
		TEST3.FrameEnd();

		g_SpatialSpace->stat_insert.FrameEnd();
		g_SpatialSpace->stat_remove.FrameEnd();
		g_SpatialSpacePhysic->stat_insert.FrameEnd();
		g_SpatialSpacePhysic->stat_remove.FrameEnd();

		RenderTOTAL_Real.FrameEnd();
		RenderMain.FrameEnd();
		RenderMain_Calcualte.FrameEnd();

		RenderMainVIS_Static.FrameEnd();
		RenderMainVIS_Dynamic.FrameEnd();
		RenderSun.FrameEnd();
		RenderLights.FrameEnd();
		Render_dsgHUD_UI.FrameEnd();
		Render_postprocess.FrameEnd();

		NetworkSpawnCreate_xrEngine.FrameEnd();
		NetworkSpawn.FrameEnd();
		NetworkSpawn_ProcessCSE.FrameEnd();
		NetworkRelcase.FrameEnd();


		Particles_update_Time.FrameEnd();
		Particles_render_Time.FrameEnd();


		ThreadEngine.FrameEnd();
		ThreadParticles.FrameEnd();
		ThreadSecond.FrameEnd();


		OnFrame1.FrameEnd();
		OnFrame2.FrameEnd();
		OnFrame3.FrameEnd();
		OnFrame4.FrameEnd();

		// UpdateCL DATA !!!!
		UpdateClientPH.FrameEnd();
		UpdateClientUnsorted.FrameEnd();
		UpdateClientA.FrameEnd();
		UpdateClientAI_mutant.FrameEnd();
		UpdateClientAI.FrameEnd();
		UpdateClientInv.FrameEnd();

		RenderMainVIS_StaticTraverce.FrameEnd();
	}

	///////////////////////////////////////////////////
	/////////////// TIMERS END
	//////////////////////////////////////////////////

	if (!pFontGame)
	{
		Msg("!pFont");
		pFontGame = new CGameFont("ui_font_graffiti22_russian", CGameFont::fsDeviceIndependent);
	}
	 

	CGameFont& F = *pFontGame;
 	pFontGame->OutSet(100, 40);

	bool any_flag = psDeviceFlags.test(rsStatistic);

	if (any_flag)
	{
		pFontGame->SetHeight(FONT_SIZE);
		F.SetColor(color_rgba(128, 128, 192, 255));
		F.OutSet(1550, 20);
	}

	if (psDeviceFlags.test(rsStatistic))
	{
  		F.SetColor(color_rgba(128, 128, 192, 255));
		F.OutNext("FPS:   %3.0f", fFPS);

  		drawStatParam(pFontGame, "----------------");

		drawStatParamByMS(10, pFontGame, this, "EngineFrame:		%2.4fms", EngineFrame.result);

		// drawStatParam(pFontGame, "----------------");
		// drawStatParamByMS(pFontGame, this, "uUpdateCL:			%2.4fms | %2.4fms(relcase)", UpdateClient.result, NetworkRelcase.result);
		// drawStatParamByMS(pFontGame, this, "uShedule:			%2.4fms | %2.4fms(low)", Sheduler.result, ShedulerLow.result);

		drawStatParam(pFontGame, "----------------");
		drawStatParamByMS(6, pFontGame, this, "Render:				%2.4fms", RenderTOTAL_Real.result);

		drawStatParam(pFontGame, "----------------");
		drawStatParamByMS(1, pFontGame, this, "Render Wait Gpus:	%2.4fms", RenderDUMP_Wait_S.result);

		// drawStatParamByMS(1, pFontGame, this, "Render Build Total:		%2.4fms", RenderMain.result);
		drawStatParamByMS(1, pFontGame, this, "Render Build Static:		%2.4fms", RenderMainVIS_Static.result);
		drawStatParamByMS(1, pFontGame, this, "Render Build Dynamic:	%2.4fms", RenderMainVIS_Dynamic.result);
 
 		drawStatParamByMS(3, pFontGame, this, "Render GPU Draw:	%2.4fms",	  RenderDUMP.result);
		drawStatParamByMS(1, pFontGame, this, "Render GPU DrawFwd:	%2.4fms", RenderDUMP_Second.result);

		drawStatParam(pFontGame, "----------------");
		drawStatParamByMS(2, pFontGame, this, "R_Main_Sun:			%2.4fms", RenderSun.result);
		drawStatParamByMS(2, pFontGame, this, "R_Main_Lights:		%2.4fms", RenderLights.result);
		drawStatParamByMS(2, pFontGame, this, "R_Postprocess:		%2.4fms", Render_postprocess.result);

		drawStatParamByMS(1, pFontGame, this, "RDT_Ren:   %2.4fms", RenderDUMP_DT_Render.result);
		drawStatParamByMS(1, pFontGame, this, "RDT_Vis:   %2.4fms", RenderDUMP_DT_VIS.result);
		drawStatParamByMS(1, pFontGame, this, "RDT_Cache: %2.4fms", RenderDUMP_DT_Cache.result);


		drawStatParam(pFontGame, "----------------");
		u32 dcalls; u32 verts; u32 polys;
		m_pRender->DrawCalls(dcalls); m_pRender->DrawVerticy(verts); m_pRender->DrawPoly(polys);

		drawStatParamBy(pFontGame, this, "Draw Calls(DPI):		%u", dcalls);
		drawStatParamBy(pFontGame, this, "Draw Vertex:			%u", verts);
		drawStatParamBy(pFontGame, this, "Draw Pollys:			%u", polys);

		// drawStatParam(pFontGame, "----------------");
		// pFontGame->SetColor(color_rgba(0, 255, 0, 200));
		// pFontGame->OutNext("UpdateCL by Objects:");
		// pFontGame->OutNext("PH:   %.2fms, NPC:   %.2fms, Monster: %.2fms", UpdateClientPH.result, UpdateClientAI.result, UpdateClientAI_mutant.result);
		// pFontGame->OutNext("Item: %.2fms, Actor: %.2fms, Other:   %.2fms", UpdateClientInv.result, UpdateClientA.result, UpdateClientUnsorted.result);
	}
	else
	if (psDeviceFlags.test(rsStatistic))
	{
  		F.SetColor(color_rgba(128, 128, 192, 255));

		F.OutNext("FPS:   %3.0f", fFPS);

		drawStatParamByMS(10, pFontGame, this, "ThreadMain:			 %2.4fms", ThreadEngine.result);
		drawStatParamByMS(10, pFontGame, this, "ThreadSecond:		 %2.4fms", ThreadSecond.result);
		drawStatParamByMS(10, pFontGame, this, "ThreadParticles:	 %2.4fms", ThreadParticles.result);

		drawStatParamByMS(10, pFontGame, this, "EngineMTFrame:	 %2.4fms", EngineMTFrame.result);
		drawStatParamByMS(10, pFontGame, this, "EngineFrame:	 %2.4fms", EngineFrame.result);

		drawStatParamByMS(3, pFontGame, this, "uUpdateCL:		 %2.4fms", UpdateClient.result);
		drawStatParamByMS(2, pFontGame, this, "uShedule:		 %2.4fms", Sheduler.result);
		drawStatParamByMS(2, pFontGame, this, "uSheduleLov:		 %2.4fms", ShedulerLow.result);

		drawStatParamByMS(1, pFontGame, this, "Physics:		 %2.2fms", Physics.result);
		drawStatParamByMS(1, pFontGame, this, "  collider:	 %2.2fms", ph_collision.result);
		drawStatParamByMS(1, pFontGame, this, "  solver:		 %2.2fms", ph_core.result);

		m_pRender->OutDetails(*pFontGame);

		drawStatParamByMS(6, pFontGame, this, "Render:			 %2.4fms", RenderTOTAL_Real.result);
		drawStatParamByMS(2, pFontGame, this, "Particles_update: %2.4fms", Particles_update_Time.result);
		drawStatParamByMS(2, pFontGame, this, "Particles_render: %2.4fms", Particles_render_Time.result);

		//drawStatParamByMS(pFontGame, this, "Memory:      %2.2fa", fMem_calls);
		//drawStatParamByMS(pFontGame, this, "Network_xrEngine: %2.2f ms", NetworkSpawnCreate_xrEngine.result);
		//drawStatParamByMS(pFontGame, this, "Network_Spawn:    %2.2f ms", NetworkSpawn.result);
		//drawStatParamByMS(pFontGame, this, "Network_CSE:      %2.2f ms", NetworkSpawn_ProcessCSE.result);		
		//drawStatParamByMS(pFontGame, this, "Network_Relcase	  %2.2f ms", NetworkRelcase.result);
		//drawStatParamByMS	(pFontGame, this, "aiThink:     %2.2fms, %d",AI_Think.result, AI_Think.count);	
		//drawStatParamByMS	(pFontGame, this, "  aiRange:   %2.2fms, %d",AI_Range.result, AI_Range.count);
		//drawStatParamByMS	(pFontGame, this, "  aiPath:    %2.2fms, %d",AI_Path.result,  AI_Path.count);
		//drawStatParamByMS	(pFontGame, this, "  aiNode:    %2.2fms, %d",AI_Node.result,  AI_Node.count);
		//drawStatParamByMS	(pFontGame, this, "aiVision:    %2.2fms, %d",AI_Vis.result,  AI_Vis.count);
		//drawStatParamByMS	(pFontGame, this, "  Query:     %2.2fms",	AI_Vis_Query.result);
		//drawStatParamByMS	(pFontGame, this, "  RayCast:   %2.2fms",	AI_Vis_RayTests.result);
		//drawStatParamByMS	(pFontGame, this, "netClientRecv:   %2.2fms, %d",	netClient1.result, netClient1.count);
		//drawStatParamByMS	(pFontGame, this, "netClientSend:   %2.2fms, %d",	netClient2.result, netClient2.count);
	}
 
	if (psDeviceFlags.test(rsCameraPos))
 		_draw_cam_pos(pFontGame);

	if (any_flag)
		pFontGame->OnRender();


	if (true)
	{
		for (auto& stat : FrameTicks)
			stat.second.FrameStart();


		EngineFrame.FrameStart();
		EngineMTFrame.FrameStart();
		EngineMTFrameSCore.FrameStart();

		Sheduler.FrameStart();
		ShedulerLow.FrameStart();
		UpdateClient.FrameStart();
		Physics.FrameStart();
		ph_collision.FrameStart();
		ph_core.FrameStart();
		Animation.FrameStart();
		AI_Think.FrameStart();
		AI_Range.FrameStart();
		AI_Path.FrameStart();
		AI_Node.FrameStart();
		AI_Vis.FrameStart();
		AI_Vis_Query.FrameStart();
		AI_Vis_RayTests.FrameStart();

		RenderTOTAL.FrameStart();
		RenderCALC.FrameStart();
		RenderCALC_HOM.FrameStart();

		RenderDUMP.FrameStart();
		RenderDUMP_Second.FrameStart();
		RenderDUMP_RT.FrameStart();

		RenderDUMP_SKIN.FrameStart();
		RenderDUMP_Wait.FrameStart();
		RenderDUMP_Wait_S.FrameStart();
		RenderDUMP_HUD.FrameStart();
		RenderDUMP_Glows.FrameStart();
		RenderDUMP_Lights.FrameStart();
		RenderDUMP_WM.FrameStart();
		RenderDUMP_DT_VIS.FrameStart();
		RenderDUMP_DT_Render.FrameStart();
		RenderDUMP_DT_Cache.FrameStart();
		RenderDUMP_Pcalc.FrameStart();
		RenderDUMP_Scalc.FrameStart();
		RenderDUMP_Srender.FrameStart();

		Sound.FrameStart();
		Input.FrameStart();
		clRAY.FrameStart();
		clBOX.FrameStart();
		clFRUSTUM.FrameStart();

		netClient1.FrameStart();
		netClient2.FrameStart();
		netServer.FrameStart();
		netClientCompressor.FrameStart();
		netServerCompressor.FrameStart();

		TEST0.FrameStart();
		TEST1.FrameStart();
		TEST2.FrameStart();
		TEST3.FrameStart();

		g_SpatialSpace->stat_insert.FrameStart();
		g_SpatialSpace->stat_remove.FrameStart();

		g_SpatialSpacePhysic->stat_insert.FrameStart();
		g_SpatialSpacePhysic->stat_remove.FrameStart();

		RenderTOTAL_Real.FrameStart();
		RenderMain.FrameStart();
		RenderMain_Calcualte.FrameStart();

		RenderMainVIS_Static.FrameStart();
		RenderMainVIS_Dynamic.FrameStart();

		RenderSun.FrameStart();
		RenderLights.FrameStart();


		Render_dsgHUD_UI.FrameStart();
		Render_postprocess.FrameStart();

		NetworkSpawnCreate_xrEngine.FrameStart();
		NetworkSpawn.FrameStart();
		NetworkSpawn_ProcessCSE.FrameStart();
		NetworkRelcase.FrameStart();

		Particles_update_Time.FrameStart();
		Particles_render_Time.FrameStart();

		ThreadEngine.FrameStart();
		ThreadParticles.FrameStart();
		ThreadSecond.FrameStart();

		OnFrame1.FrameStart();
		OnFrame2.FrameStart();
		OnFrame3.FrameStart();
		OnFrame4.FrameStart();


		UpdateClientPH.FrameStart();
		UpdateClientUnsorted.FrameStart();
		UpdateClientA.FrameStart();
		UpdateClientAI_mutant.FrameStart();
		UpdateClientAI.FrameStart();
		UpdateClientInv.FrameStart();


		RenderMainVIS_StaticTraverce.FrameStart();
	}


	dwSND_Played = dwSND_Allocated = 0;
	Particles_starting = Particles_active = Particles_destroy = 0;
}


void	_LogCallback(LPCSTR string)
{
	if (string && '!' == string[0] && ' ' == string[1])
		Device.Statistic->errors.emplace_back(string);
}

void CStats::OnDeviceCreate			()
{
	g_bDisableRedText = Core.ParamsData.test(ECoreParams::xclsx);

	if (!g_dedicated_server) {
		pFont = g_FontManager->GetFont("stat_font", CGameFont::fsDeviceIndependent);// new CGameFont("stat_font", CGameFont::fsDeviceIndependent);
	}
	
	if(!pSettings->section_exist("evaluation")
		||!pSettings->line_exist("evaluation","line1")
		||!pSettings->line_exist("evaluation","line2")
		||!pSettings->line_exist("evaluation","line3") )
		FATAL	("");

	eval_line_1 = pSettings->r_string_wb("evaluation","line1");
	eval_line_2 = pSettings->r_string_wb("evaluation","line2");
	eval_line_3 = pSettings->r_string_wb("evaluation","line3");

	// 
#ifdef DEBUG
	if (!g_bDisableRedText)
		xrLogger::AddLogCallback(_LogCallback);
#endif
}

void CStats::OnDeviceDestroy		()
{
	xrLogger::RemoveLogCallback(_LogCallback);
	xr_delete	(pFont);
}

void CStats::OnRender				()
{
#ifdef DEBUG_DRAW
	if (g_stats_flags.is(st_sound)){
		CSound_stats_ext				snd_stat_ext;
		::Sound->statistic				(0,&snd_stat_ext);
		CSound_stats_ext::item_vec_it	_I = snd_stat_ext.items.begin();
		CSound_stats_ext::item_vec_it	_E = snd_stat_ext.items.end();
		for (;_I!=_E;_I++){
			const CSound_stats_ext::SItem& item = *_I;
			if (item._3D)
			{
				m_pRender->SetDrawParams(&*Device.m_pRender);
				//RCache.set_xform_world(Fidentity);
				//RCache.set_Shader		(Device.m_SelectionShader);
				//RCache.set_c			("tfactor",1,1,1,1);
				DU->DrawCross			(item.params.position, 0.5f, 0xFF0000FF, true );
				if (g_stats_flags.is(st_sound_min_dist))
					DU->DrawSphere		(Fidentity, item.params.position, item.params.min_distance, 0x400000FF,	0xFF0000FF, true, true);
				if (g_stats_flags.is(st_sound_max_dist))
					DU->DrawSphere		(Fidentity, item.params.position, item.params.max_distance, 0x4000FF00,	0xFF008000, true, true);
				
				xr_string out_txt		= (out_txt.size() && g_stats_flags.is(st_sound_info_name)) ? item.name.c_str():"";

				if (item.game_object)
				{
					if (g_stats_flags.is(st_sound_ai_dist))
						DU->DrawSphere	(Fidentity, item.params.position, item.params.max_ai_distance, 0x80FF0000,0xFF800000,true,true);
					if (g_stats_flags.is(st_sound_info_object)){
						out_txt			+= "  (";
						out_txt			+= item.game_object->cNameSect().c_str();
						out_txt			+= ")";
					}
				}
				if (g_stats_flags.is_any(st_sound_info_name|st_sound_info_object) && item.name.size())
					DU->OutText			(item.params.position, out_txt.c_str(),0xFFFFFFFF,0xFF000000);
			}
		}
	}
#endif
}
