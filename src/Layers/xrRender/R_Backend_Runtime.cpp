#include "stdafx.h"


#include <DirectXMath.h>
using namespace DirectX;

#include "../../xrCore/Collision/Frustum.h"

#ifdef USE_DX11
#include "../xrRenderDX10/StateManager/dx10StateManager.h"
#include "../xrRenderDX10/StateManager/dx10ShaderResourceStateCache.h"
#endif //USE_DX11

#include "dxRenderDeviceRender.h"

#include "smol-atlas.h"

#ifdef DEBUG
#ifdef IXR_WINDOWS
// for setting debug names for DirectX resources due to GUID definitions
#pragma comment(lib, "dxguid.lib")
#endif
#endif

void CBackend::OnFrameEnd	()
{
//#ifndef DEDICATED_SERVER
#ifndef _EDITOR
	if (!g_dedicated_server)
#endif    
	{
#ifdef USE_DX11
		Invalidate			();
#else //USE_DX11

		for (u32 stage=0; stage<Caps.raster.dwStages; stage++)
			CHK_DX(RDevice->SetTexture(0,0));
		CHK_DX				(RDevice->SetStreamSource	(0,0,0,0));
		CHK_DX				(RDevice->SetIndices			(0));
		CHK_DX				(RDevice->SetVertexShader	(0));
		CHK_DX				(RDevice->SetPixelShader		(0));
		Invalidate			();
#endif
	}
//#endif
}

void CBackend::OnFrameBegin	()
{
//#ifndef DEDICATED_SERVER
#ifndef _EDITOR
	if (!g_dedicated_server)
#endif    
	{
		PGO					(Msg("PGO:*****frame[%d]*****",RDEVICE.dwFrame));
#ifdef USE_DX11
		Invalidate();
		//	DX9 sets base rt nd base zb by default
		RImplementation.rmNormal();
		set_RT				(RImplementation.Target->rt_BackbufferLUT->pRT);
		set_ZB				(nullptr);
#endif //USE_DX11
		Memory.mem_fill		(&stat,0,sizeof(stat));
		Vertex.Flush		();
		Index.Flush			();
		set_Stencil			(FALSE);
	}
//#endif
}

void CBackend::Invalidate	()
{
	pRT[0]						= nullptr;
	pRT[1]						= nullptr;
	pRT[2]						= nullptr;
	pRT[3]						= nullptr;
	pZB							= nullptr;

	decl						= nullptr;
	vb							= nullptr;
	ib							= nullptr;
	vb_stride					= 0;

	state						= nullptr;
	ps							= nullptr;
	vs							= nullptr;
DX10_ONLY(gs					= nullptr);
#ifdef USE_DX11
	hs = 0;
	ds = 0;
	cs = 0;
#endif //USE_DX11
	ctable						= nullptr;

	T							= nullptr;
	M							= nullptr;
	C							= nullptr;

	stencil_enable=u32(-1);
	stencil_func=u32(-1);
	stencil_ref=u32(-1);
	stencil_mask=u32(-1);
	stencil_writemask=u32(-1);
	stencil_fail=u32(-1);
	stencil_pass=u32(-1);
	stencil_zfail=u32(-1);
	cull_mode=u32(-1);
	z_enable=u32(-1);
	z_func=u32(-1);
	alpha_ref=u32(-1);
	colorwrite_mask				= u32(-1);

	//	Since constant buffers are unmapped (for DirecX 10)
	//	transform setting handlers should be unmapped too.
	xforms.unmap	();

#ifdef USE_DX11
	m_pInputLayout				= nullptr;
	m_PrimitiveTopology			= D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	m_bChangedRTorZB			= false;
	m_pInputSignature			= nullptr;
	for (int i=0; i<MaxCBuffers; ++i)
	{
		m_aPixelConstants[i] = 0;
		m_aVertexConstants[i] = 0;
		m_aGeometryConstants[i] = 0;
		m_aHullConstants[i] = 0;
		m_aDomainConstants[i] = 0;
		m_aComputeConstants[i] = 0;
	}
	StateManager.Reset();
	//	Redundant call. Just no note that we need to unmap const
	//	if we create dedicated class.
	StateManager.UnmapConstants();
	SSManager.ResetDeviceState();
	SRVSManager.ResetDeviceState();

	for (u32 gs_it =0; gs_it < mtMaxGeometryShaderTextures;)	textures_gs	[gs_it++]	= 0;
	for (u32 hs_it =0; hs_it < mtMaxHullShaderTextures;)	textures_hs	[hs_it++]	= 0;
	for (u32 ds_it =0; ds_it < mtMaxDomainShaderTextures;)	textures_ds	[ds_it++]	= 0;
	for (u32 cs_it =0; cs_it < mtMaxComputeShaderTextures;)	textures_cs	[cs_it++]	= 0;
#endif //USE_DX11

	for (u32 ps_it =0; ps_it < mtMaxPixelShaderTextures;)	textures_ps	[ps_it++]	= 0;
	for (u32 vs_it =0; vs_it < mtMaxVertexShaderTextures;)	textures_vs	[vs_it++]	= 0;
#ifdef _EDITOR
	for (u32 m_it =0; m_it< 8;)		matrices	[m_it++]	= 0;
#endif
}

void	CBackend::set_ClipPlanes	(u32 _enable, Fplane*	_planes /*=nullptr */, u32 count/* =0*/)
{
#ifdef USE_DX11
	//	TODO: DX10: Implement in the corresponding vertex shaders
	//	Use this to set up location, were shader setup code will get data
	//VERIFY(!"CBackend::set_ClipPlanes not implemented!");
	return;
#else //USE_DX11
	if (0==Caps.geometry.dwClipPlanes)	return;
	if (!_enable)	{
		CHK_DX	(RDevice->SetRenderState(D3DRS_CLIPPLANEENABLE,FALSE));
		return;
	}
	
	// Enable and setup planes
	VERIFY	(_planes && count);
	if		(count>Caps.geometry.dwClipPlanes)	count=Caps.geometry.dwClipPlanes;

	auto worldToClipMatrixIT = XMMatrixInverse(nullptr, XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&Device.mFullTransform)));
	worldToClipMatrixIT = XMMatrixTranspose(worldToClipMatrixIT);
	XMFLOAT4 planeClip{};
	XMVECTOR planeWorld{};

	for (u32 it = 0; it < count; it++) {
		Fplane& P = _planes[it];
		planeWorld = XMPlaneNormalize(XMVectorSet(-P.n.x, -P.n.y, -P.n.z, -P.d));
		XMStoreFloat4(&planeClip, XMPlaneTransform(planeWorld, worldToClipMatrixIT));
		CHK_DX(RDevice->SetClipPlane(it, reinterpret_cast<float*>(&planeClip)));
	}

	// Enable them
	u32		e_mask	= (1<<count)-1;
	CHK_DX	(RDevice->SetRenderState(D3DRS_CLIPPLANEENABLE,e_mask));
#endif
}

#ifndef DEDICATED_SREVER
void	CBackend::set_ClipPlanes	(u32 _enable, Fmatrix*	_xform  /*=nullptr */, u32 fmask/* =0xff */)
{
	if (!_enable)	{
#ifdef USE_DX11
		//	TODO: DX10: Implement in the corresponding vertex shaders
		//	Use this to set up location, were shader setup code will get data
		//VERIFY(!"CBackend::set_ClipPlanes not implemented!");
#else //USE_DX11
		CHK_DX	(RDevice->SetRenderState(D3DRS_CLIPPLANEENABLE,FALSE));
#endif
		return;
	}
	VERIFY		(_xform && fmask);
	CFrustum	F;
	F.CreateFromMatrix	(*_xform,fmask);
	set_ClipPlanes		(_enable,F.planes,F.p_count);
}

void CBackend::set_Textures			(STextureList* _T)
{
	PROF_EVENT("set_Textures");
	if (T == _T)	return;
	T				= _T;
	//	If resources weren't set at all we should clear from resource #0.
	int _last_ps	= -1;
	int _last_vs	= -1;
#ifdef USE_DX11
	int _last_gs	= -1;
	int _last_hs	= -1;
	int _last_ds	= -1;
	int _last_cs	= -1;
#endif //USE_DX11
	STextureList::iterator	_it		= _T->begin	();
	STextureList::iterator	_end	= _T->end	();

	for (; _it!=_end; _it++)
	{
		std::pair<u32,ref_texture>&		loader	=	*_it;
		u32			load_id		= loader.first		;
		CTexture*	load_surf	= &*loader.second	;
//		if (load_id < 256)		{
		if (load_id < CTexture::rstVertex)
		{
			//	Set up pixel shader resources
			VERIFY(load_id<mtMaxPixelShaderTextures);
			// ordinary pixel surface
			if ((int)load_id>_last_ps)		_last_ps	=	load_id;
			if (textures_ps[load_id]!=load_surf)	
			{
				textures_ps[load_id]	= load_surf			;
#ifdef DEBUG
				stat.textures			++;
#endif
				if (load_surf)			
				{
					PGO					(Msg("PGO:tex%d:%s",load_id,load_surf->cName.c_str()));
					load_surf->bind		(load_id);
//					load_surf->Apply	(load_id);
				}
			}
		} else 
#ifdef USE_DX11
		if (load_id < CTexture::rstGeometry)
#endif	//	UDE_DX10
		{
			//	Set up pixel shader resources
			VERIFY(load_id < CTexture::rstVertex+mtMaxVertexShaderTextures);

			// vertex only //d-map or vertex	
			u32		load_id_remapped	= load_id - CTexture::rstVertex;
			if ((int)load_id_remapped>_last_vs)	_last_vs	=	load_id_remapped;
			if (textures_vs[load_id_remapped]!=load_surf)	
			{
				textures_vs[load_id_remapped]	= load_surf;
#ifdef DEBUG
				stat.textures	++;
#endif
				if (load_surf)
				{
					PGO					(Msg("PGO:tex%d:%s",load_id,load_surf->cName.c_str()));
					load_surf->bind		(load_id);
//					load_surf->Apply	(load_id);
				}
			}
		}
#ifdef USE_DX11
		else if (load_id < CTexture::rstHull)
		{
			//	Set up pixel shader resources
			VERIFY(load_id < CTexture::rstGeometry+mtMaxGeometryShaderTextures);

			// vertex only //d-map or vertex	
			u32		load_id_remapped	= load_id - CTexture::rstGeometry;
			if ((int)load_id_remapped>_last_gs)	_last_gs	=	load_id_remapped;
			if (textures_gs[load_id_remapped]!=load_surf)	
			{
				textures_gs[load_id_remapped]	= load_surf;
#ifdef DEBUG
				stat.textures	++;
#endif
				if (load_surf)
				{
					PGO					(Msg("PGO:tex%d:%s",load_id,load_surf->cName.c_str()));
					load_surf->bind		(load_id);
					//					load_surf->Apply	(load_id);
				}
			}
		}
#ifdef USE_DX11
		else if (load_id < CTexture::rstDomain)
		{
			//	Set up pixel shader resources
			VERIFY(load_id < CTexture::rstHull+mtMaxHullShaderTextures);

			// vertex only //d-map or vertex	
			u32		load_id_remapped	= load_id - CTexture::rstHull;
			if ((int)load_id_remapped>_last_hs)	_last_hs	=	load_id_remapped;
			if (textures_hs[load_id_remapped]!=load_surf)	
			{
				textures_hs[load_id_remapped]	= load_surf;
#ifdef DEBUG
				stat.textures	++;
#endif
				if (load_surf)
				{
					PGO					(Msg("PGO:tex%d:%s",load_id,load_surf->cName.c_str()));
					load_surf->bind		(load_id);
					//					load_surf->Apply	(load_id);
				}
			}
		}
		else if (load_id < CTexture::rstCompute)
		{
			//	Set up pixel shader resources
			VERIFY(load_id < CTexture::rstDomain+mtMaxDomainShaderTextures);

			// vertex only //d-map or vertex	
			u32		load_id_remapped	= load_id - CTexture::rstDomain;
			if ((int)load_id_remapped>_last_ds)	_last_ds	=	load_id_remapped;
			if (textures_ds[load_id_remapped]!=load_surf)	
			{
				textures_ds[load_id_remapped]	= load_surf;
#ifdef DEBUG
				stat.textures	++;
#endif
				if (load_surf)
				{
					PGO					(Msg("PGO:tex%d:%s",load_id,load_surf->cName.c_str()));
					load_surf->bind		(load_id);
					//					load_surf->Apply	(load_id);
				}
			}
		}
		else if (load_id < CTexture::rstInvalid)
		{
			//	Set up pixel shader resources
			VERIFY(load_id < CTexture::rstCompute+mtMaxComputeShaderTextures);

			// vertex only //d-map or vertex	
			u32		load_id_remapped	= load_id - CTexture::rstCompute;
			if ((int)load_id_remapped>_last_cs)	_last_cs	=	load_id_remapped;
			if (textures_cs[load_id_remapped]!=load_surf)	
			{
				textures_cs[load_id_remapped]	= load_surf;
#ifdef DEBUG
				stat.textures	++;
#endif
				if (load_surf)
				{
					PGO					(Msg("PGO:tex%d:%s",load_id,load_surf->cName.c_str()));
					load_surf->bind		(load_id);
					//					load_surf->Apply	(load_id);
				}
			}
		}
#endif
		else
			VERIFY("Invalid enum");
#endif	//	UDE_DX10
	}


	// clear remaining stages (PS)
	for (++_last_ps; _last_ps<mtMaxPixelShaderTextures; _last_ps++)
	{
		if (!textures_ps[_last_ps])
			continue;

		textures_ps[_last_ps]			= 0;
#ifdef USE_DX11
		//	TODO: DX10: Optimise: set all resources at once
		ID3DShaderResourceView	*pRes = 0;
		//RDevice->PSSetShaderResources(_last_ps, 1, &pRes);
		SRVSManager.SetPSResource(_last_ps, pRes);
#else //USE_DX11
		CHK_DX							(RDevice->SetTexture(_last_ps,nullptr));
#endif
	}
	// clear remaining stages (VS)
	for (++_last_vs; _last_vs<mtMaxVertexShaderTextures; _last_vs++)		
	{
		if (!textures_vs[_last_vs])
			continue;

		textures_vs[_last_vs]			= 0;
#ifdef USE_DX11
		//	TODO: DX10: Optimise: set all resources at once
		ID3DShaderResourceView	*pRes = 0;
		//RDevice->VSSetShaderResources(_last_vs, 1, &pRes);
		SRVSManager.SetVSResource(_last_vs, pRes);
#else //USE_DX11
		CHK_DX							(RDevice->SetTexture(_last_vs+CTexture::rstVertex,nullptr));
#endif
	}

#ifdef USE_DX11
	// clear remaining stages (VS)
	for (++_last_gs; _last_gs<mtMaxGeometryShaderTextures; _last_gs++)
	{
		if (!textures_gs[_last_gs])
			continue;

		textures_gs[_last_gs]			= 0;

		//	TODO: DX10: Optimise: set all resources at once
		ID3DShaderResourceView	*pRes = 0;
		//RDevice->GSSetShaderResources(_last_gs, 1, &pRes);
		SRVSManager.SetGSResource(_last_gs, pRes);
	}

	for (++_last_hs; _last_hs<mtMaxHullShaderTextures; _last_hs++)
	{
		if (!textures_hs[_last_hs])
			continue;

		textures_hs[_last_hs]			= 0;

		//	TODO: DX10: Optimise: set all resources at once
		ID3DShaderResourceView	*pRes = 0;
		SRVSManager.SetHSResource(_last_hs, pRes);
	}
	for (++_last_ds; _last_ds<mtMaxDomainShaderTextures; _last_ds++)
	{
		if (!textures_ds[_last_ds])
			continue;

		textures_ds[_last_ds]			= 0;

		//	TODO: DX10: Optimise: set all resources at once
		ID3DShaderResourceView	*pRes = 0;
		SRVSManager.SetDSResource(_last_ds, pRes);
	}
	for (++_last_cs; _last_cs<mtMaxComputeShaderTextures; _last_cs++)
	{
		if (!textures_cs[_last_cs])
			continue;

		textures_cs[_last_cs]			= 0;

		//	TODO: DX10: Optimise: set all resources at once
		ID3DShaderResourceView	*pRes = 0;
		SRVSManager.SetCSResource(_last_cs, pRes);
	}

#endif //USE_DX11
}
#else

void	CBackend::set_ClipPlanes	(u32 _enable, Fmatrix*	_xform  /*=nullptr */, u32 fmask/* =0xff */) {}
void CBackend::set_Textures			(STextureList* _T) {}

#endif


CTextureAtlas::CTextureAtlas() :
#ifdef DEBUG
	init_was_called{},
	m_name{},
#endif
	m_width{},
	m_height{},
	m_id{ _kRenderBackend_TextureAtlasInvalidID },
	m_p_atlas{},
	m_p_texture{},
	static_atlas_items_storage{},
	sais_wrapper{ &static_atlas_items_storage, sizeof(static_atlas_items_storage) },
	m_atlas_items{ std::pmr::polymorphic_allocator<CTextureAtlasItem>{&sais_wrapper} }
{
	m_atlas_items.reserve(_kRenderBackend_TextureAtlasPreallocatedItems);
}

CTextureAtlas::CTextureAtlas(CTextureAtlas&& other) noexcept :
#ifdef DEBUG
	init_was_called{ other.init_was_called },
	m_name{},
#endif
	m_width{ other.m_width }, m_height{ other.m_height }, m_id{ other.m_id }, m_p_atlas{ other.m_p_atlas }, m_p_texture{ other.m_p_texture }, static_atlas_items_storage{}, sais_wrapper{ &static_atlas_items_storage, sizeof(static_atlas_items_storage) }, m_atlas_items{ std::pmr::polymorphic_allocator<CTextureAtlasItem>{&sais_wrapper} }
{
	other.m_p_atlas = nullptr;
	other.m_p_texture = nullptr;
	other.m_width = 0;
	other.m_height = 0;
	other.m_id = _kRenderBackend_TextureAtlasInvalidID;

	for (CTextureAtlasItem& item : other.m_atlas_items)
	{
		m_atlas_items.push_back(std::move(item));
	}

	other.m_atlas_items.clear();

#ifdef DEBUG
	if (other.m_name[0] != '\0')
	{
		std::memcpy(m_name, other.m_name, strlen(other.m_name));
	}

	other.m_name[0] = '\0';
#endif
}

CTextureAtlas::~CTextureAtlas()
{
#ifdef DEBUG
	R_ASSERT2(!init_was_called, "you forgot to call uninit or destroy this instance!");
#endif
}

CTextureAtlas& CTextureAtlas::operator=(CTextureAtlas&& other) noexcept
{
	if (this != &other)
	{
		uninit();

		this->m_width = other.m_width;
		this->m_height = other.m_height;
		this->m_id = other.m_id;
		this->m_p_atlas = other.m_p_atlas;

		this->m_p_texture = other.m_p_texture;

		R_ASSERT(this->m_atlas_items.capacity() != 0 && "it MUST be initialized through ctor otherwise something is broken or memory corruption!");

		for (CTextureAtlasItem& item : other.m_atlas_items)
		{
			this->m_atlas_items.push_back(std::move(item));
		}

		other.m_p_atlas = nullptr;
		other.m_p_texture = nullptr;
		other.m_width = 0;
		other.m_height = 0;
		other.m_id = _kRenderBackend_TextureAtlasInvalidID;
		other.m_atlas_items.clear();
#ifdef DEBUG
		init_was_called = other.init_was_called;
		if (other.m_name[0] != '\0')
		{
			std::memcpy(m_name, other.m_name, strlen(other.m_name));
		}

		other.m_name[0] = '\0';
#endif
	}

	return *this;
}

void CTextureAtlas::init(ID3DDevice* p_device, int width, int height, const char* pName)
{
	R_ASSERT2(p_device, "you must pass a valid device!");

	R_ASSERT(width > 0 && "must be valid");
	R_ASSERT(height > 0 && "must be valid!");
	R_ASSERT(!this->m_p_atlas && "must be not initialized otherwise you forgot to call uninit!");
	R_ASSERT(DEV && "early calling?");

	if (!this->m_p_atlas)
	{
		this->m_p_atlas = sma_atlas_create(width, height);

		R_ASSERT(this->m_p_atlas && "failed to create logical layout atlas!");
	}

	this->setName(pName);
	this->m_p_texture = DEV->_CreateEmptyTexture(this->m_name, width, height);
	R_ASSERT(this->m_p_texture && "must be created a valid texture from resource manager, failed to create!");
}

void CTextureAtlas::uninit()
{
	if (this->m_p_texture)
	{
		DEV->_DeleteTexture(this->m_p_texture);
		this->m_p_texture->Unload();
		this->m_p_texture = nullptr;
	}

	this->m_width = 0;
	this->m_height = 0;

	if (this->m_p_atlas)
	{
		for (CTextureAtlasItem& item : this->m_atlas_items)
		{
			R_ASSERT(item.p_placement && "must be valid otherwise you didn't remove item from vector properly");
			if (item.p_placement)
			{
				sma_item_remove(this->m_p_atlas, item.p_placement);
			}
		}

		sma_atlas_destroy(this->m_p_atlas);

		this->m_atlas_items.clear();
		this->m_p_atlas = nullptr;
	}

#ifdef DEBUG
	init_was_called = false;
#endif
}

void CTextureAtlas::addRegion(ID3DDevice* p_device, ID3DDeviceContext* p_context, u32 w, u32 h, const void* pData, u32 pitch)
{
	R_ASSERT(this->m_p_atlas && "must be initialized before calling this method!");
	R_ASSERT(this->m_p_texture && "you forgot to call init because texture wasn't initialized!");

	if (this->m_p_atlas && this->m_p_texture)
	{
		smol_atlas_item_t* p_current_placement = sma_item_add(this->m_p_atlas, w, h);
		R_ASSERT(p_current_placement && "failed to create logical placement item");
		if (p_current_placement)
		{
			u32 x = static_cast<u32>(sma_item_x(p_current_placement));
			u32 y = static_cast<u32>(sma_item_y(p_current_placement));

			CTextureAtlasItem item;
			item.p_placement = p_current_placement;

			item.u0 = float(x) / float(this->m_width);
			item.v0 = float(y) / float(this->m_height);
			item.u1 = float(x + w) / float(this->m_width);
			item.v1 = float(y + h) / float(this->m_height);

			this->m_atlas_items.push_back(item);

			if (pitch == 0)
				pitch = w * 4;

			addRegion(p_device, p_context, x, y, w, h, pData, pitch);
		}
	}
}

void CTextureAtlas::addRegion(ID3DDevice* p_device, u32 x, u32 y, u32 w, u32 h, const void* pData, u32 pitch)
{
	R_ASSERT2(p_device, "you must pass a valid device!");

	R_ASSERT(m_p_texture && "must be valid!");
	R_ASSERT(m_p_texture->pSurface && "must be valid!");
	R_ASSERT(dynamic_cast<ID3DTexture2D*>(m_p_texture->pSurface) && "must be casted to ID3DTexture2D!");

	ID3DTexture2D* pCasted = static_cast<ID3DTexture2D*>(m_p_texture->pSurface);

#ifdef IXR_WINDOWS
#if defined(D3D10_SDK_VERSION)
#elif defined(DIRECT3D_VERSION) && DIRECT3D_VERSION >= 0x0900

	D3DLOCKED_RECT lr = {};
	HRESULT hr = pCasted->LockRect(
		0,
		&lr,
		nullptr,
		D3DLOCK_NOOVERWRITE
	);

	R_ASSERT(SUCCEEDED(hr) && "failed to lockrect");

	// Copy row by row
	BYTE* destBase = reinterpret_cast<BYTE*>(lr.pBits);
	for (UINT row = 0; row < h; ++row)
	{
		BYTE* destRow = destBase
			+ (y + row) * lr.Pitch
			+ (x * 4);
		const BYTE* srcRow = reinterpret_cast<const BYTE*>(pData)
			+ row * pitch;

		std::memcpy(destRow, srcRow, w * 4);
	}

	pCasted->UnlockRect(0);

#else
#error provide sdk 
#endif
#endif
}

void CTextureAtlas::addRegion(ID3DDevice* p_device, ID3DDeviceContext* p_context, u32 x, u32 y, u32 w, u32 h, const void* pData, u32 pitch)
{
	R_ASSERT2(p_device, "you must pass a valid device!");


#ifdef IXR_WINDOWS
#if defined(D3D12_SDK_VERSION)
	R_ASSERT2(p_context, "you must pass a valid context! For D3D11 device context, for D3D12 command list!");

#elif defined(D3D11_SDK_VERSION)
	R_ASSERT2(p_context, "you must pass a valid context! For D3D11 device context, for D3D12 command list!");
	R_ASSERT(m_p_texture && "must be valid!");
	R_ASSERT(m_p_texture->pSurface && "must be valid!");
	R_ASSERT(dynamic_cast<ID3DTexture2D*>(m_p_texture->pSurface) && "must be casted to ID3DTexture2D!");

	ID3DTexture2D* pResourceTexture = static_cast<ID3DTexture2D*>(m_p_texture->pSurface);

	D3D11_BOX destBox;
	destBox.left = x;
	destBox.top = y;
	destBox.front = 0;
	destBox.right = x + w;
	destBox.bottom = y + h;
	destBox.back = 1;

	// rowPitch = bytes per row of your source image
	UINT rowPitch = pitch;

	// context is your ID3D11DeviceContext*
	// atlasTex is your ID3D11Texture2D*
	p_context->UpdateSubresource(
		pResourceTexture,         // Destination resource
		0,                // Mip slice
		&destBox,         // Subresource region to update
		pData,          // Pointer to the raw texel data
		rowPitch,         // Row pitch in bytes
		0                 // Depth pitch (unused for 2D)
	);

#else
	if (!p_context)
	{
		addRegion(p_device, x, y, w, h, pData, pitch);
	}
#endif
#endif

}

std::string_view CTextureAtlas::getName(void) const
{
	return std::string_view(m_name);	
}

void CTextureAtlas::setName(const char* pName)
{
	R_ASSERT(pName && "you must pass a valid name!");
	R_ASSERT(pName[0] != '\0' && "you must pass a valid name!");

	if (pName && pName[0] != '\0')
		std::memcpy(m_name, pName, sizeof(m_name));
}

void* CTextureAtlas::getResource()
{
	return nullptr;
}

void CTextureAtlas::saveOnDisk()
{
#ifdef DEBUG

#endif
}

u32 CTextureAtlas::getID()
{
	return this->m_id;
}

void CTextureAtlas::setID(u32 id)
{
	this->m_id = id;
}


CSVGStorage::CSVGStorage(u32 flags) :

#ifdef DEBUG
	init_was_called{},
#endif
	static_storage{},
	ss_wrapper{ &static_storage, sizeof(static_storage), flags & eSVGStorageFlags::kFeatureSVGStorage_Static_Allocation ? std::pmr::null_memory_resource() : std::pmr::get_default_resource() },
	storage{ std::pmr::polymorphic_allocator<CTextureAtlas>{&ss_wrapper} }
{
	R_ASSERT(!(flags & eSVGStorageFlags::kFeatureSVGStorage_Static_Allocation && flags & eSVGStorageFlags::kFeatureSVGStorage_Dynamic_Allocation) && "invalid flags");

	// if allocation size is changed in static mode you will get throw bad_alloc due to fact that required allocation formula was changed so in such case you have to change the size of static_storage field please
	storage.reserve(_kRenderBackend_SVGStorageSizeInitial);
}


CSVGStorage::~CSVGStorage()
{

}

void CSVGStorage::init()
{
}

void CSVGStorage::uninit() {}

// returns preallocated size that was specified initially (but it doesn't show current size)
constexpr unsigned char CSVGStorage::get_static_size() const
{
	return _kRenderBackend_SVGStorageSizeInitial;
}

// returns current size of storage
unsigned int CSVGStorage::get_size() const
{
	return this->storage.size();
}

// if returns u32(-1) means it is failed to add atlas
// see allocation policies that defined in eSVGStorageFlags
u32 CSVGStorage::add_atlas()
{
	return u32();
}

CTextureAtlas* CSVGStorage::get_atlas(u32 id)
{
	return nullptr;
}

const CTextureAtlas* CSVGStorage::get_atlas(u32 id) const
{
	return nullptr;
}

void CSVGStorage::delete_atlas(u32 id)
{

}

void CSVGStorage::cache_atlases()
{

}

// make it optional field that will check should we cache
void CSVGStorage::load_cache()
{

}