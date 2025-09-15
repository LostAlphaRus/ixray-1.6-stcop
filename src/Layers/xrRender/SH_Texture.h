#ifndef SH_TEXTURE_H
#define SH_TEXTURE_H
#pragma once

#include "../../xrCore/xr_resource.h"
#include "../../xrCore/xr_rtree.h"

#include <memory_resource>

class  ENGINE_API CAviPlayerCustom;
class  CTheoraSurface;

class  ECORE_API CTexture : public xr_resource_named
{
public:
	//	Since DX10 allows up to 128 unique textures, 
	//	distance between enum values should be at leas 128
	enum ResourceShaderType	//	Don't change this since it's hardware-dependent
	{
		rstPixel = 0,	//	Default texture offset
		rstVertex = D3DVERTEXTEXTURESAMPLER0,
		rstGeometry = rstVertex + 256,
		rstHull = rstGeometry + 256,
		rstDomain = rstHull + 256,
		rstCompute = rstDomain + 256,
		rstInvalid = rstCompute + 256
	};

public:
	void								apply_load(u32	stage);
	void								apply_theora(u32	stage);
	void								apply_avi(u32	stage);
	void								apply_seq(u32	stage);
	void								apply_normal(u32	stage);

	void								Preload();
	void								Load();
	/// @brief just creates resources but without uploading and filling
	void								CreateEmpty(u32 w, u32 h);
	void								PostLoad();
	void								Unload(void);
	//	void								Apply			(u32 dwStage);

	void								surface_set(ID3DBaseTexture* surf);
	ID3DBaseTexture* surface_get();

	IC BOOL								isUser() { return flags.bUser; }
	IC u32								get_Width() { desc_enshure(); return desc.Width; }
	IC u32								get_Height() { desc_enshure(); return desc.Height; }

#ifdef USE_DX11
	IC DXGI_FORMAT						get_Format() { desc_enshure(); return desc.Format; }
#endif

	void								video_Sync(u32 _time) { m_play_time = _time; }
	void								video_Play(BOOL looped, u32 _time = 0xFFFFFFFF);
	void								video_Pause(BOOL state);
	void								video_Stop();
	BOOL								video_IsPlaying();

	CTexture();
	virtual ~CTexture();

#ifdef USE_DX11
	ID3DShaderResourceView* get_SRView() { return m_pSRView; }
#endif //USE_DX11

private:
	IC BOOL								desc_valid() { return pSurface == desc_cache; }
	IC void								desc_enshure() { if (!desc_valid()) desc_update(); }
	void								desc_update();
#ifdef USE_DX11
	void								Apply(u32 dwStage);
	void								ProcessStaging();
	D3D_USAGE							GetUsage();
#endif //USE_DX11

	//	Class data
public:	//	Public class members (must be encapsulated furthur)
	struct
	{
		u32					bLoaded : 1;
		u32					bUser : 1;
		u32					seqCycles : 1;
		u32					MemoryUsage : 28;
#ifdef USE_DX11
		u32					bLoadedAsStaging : 1;
#endif //USE_DX11
	}									flags;
	xr_delegate<void(u32)> bind;


	CAviPlayerCustom* pAVI;
	CTheoraSurface* pTheora;
	float								m_material;
	shared_str							m_bumpmap;

	union {
		u32								m_play_time;		// sync theora time
		u32								seqMSPF;			// Sequence data milliseconds per frame
	};

	ID3DBaseTexture* pSurface;
	// create from CreateEmpty and supposed to be manually created/allocated
	bool can_unload;
	void setDebugName(const char* pName);
private:
	// Sequence data
	xr_vector<ID3DBaseTexture*>			seqDATA;

	// Description
	ID3DBaseTexture* desc_cache;
	D3D_TEXTURE2D_DESC					desc;

#ifdef USE_DX11
	ID3DShaderResourceView* m_pSRView;
	// Sequence view data
	xr_vector<ID3DShaderResourceView*>m_seqSRView;
#endif //USE_DX11
};
struct 		resptrcode_texture : public resptr_base<CTexture>
{
	ECORE_API void		create(LPCSTR	_name);
	void				destroy() { _set(NULL); }
	shared_str			bump_get() { return _get()->m_bumpmap; }
	bool				bump_exist() { return 0 != bump_get().size(); }
};
typedef	resptr_core<CTexture, resptrcode_texture >
ref_texture;

constexpr unsigned char _kRenderBackend_DebugTextureAtlasNameLength = 16;
constexpr unsigned char _kRenderBackend_SVGStorageSizeInitial = 2;
constexpr u32 _kRenderBackend_TextureAtlasInvalidID = u32(-1);
constexpr u32 _kRenderBackend_TextureAtlasPreallocatedItems = 64;
constexpr u32 _kRenderBackend_TextureAtlasPreallocatedDimensions = 8;

inline constexpr size_t calculate_reserve_count(size_t bytes, size_t amount)
{
#ifndef DEBUG
	return std::bit_ceil(bytes * amount);
#else
	return std::bit_ceil(bytes * amount) * 2;
#endif
}

struct smol_atlas_t;
struct smol_atlas_item_t;

/// @brief author: wh1t3lord
class ECORE_API CTextureAtlas
{
public:
	using element_lookupid_type = char;

	struct ECORE_API CTextureAtlasElement
	{
		smol_atlas_item_t* p_placement = nullptr;

		float x() const;
		float y() const;
		float w() const;
		float h() const;


		float u0(u32 atlas_width) const;
		float v0(u32 atlas_height) const;
		float u1(u32 atlas_width) const;
		float v1(u32 atlas_height) const;
	};

	/// @brief CTextureAtlasElement spatial indexing feature using morton codes, contains stable lookup_id for accessing element itself 
	struct ECORE_API CTAESpatialIndex
	{
		element_lookupid_type lookup_id = element_lookupid_type(-1);
	};

	using storage_type = std::pmr::vector<CTextureAtlasElement>;
	using spatial_storage_type = std::pmr::vector<CTAESpatialIndex>;
	using storage_allocator = std::pmr::polymorphic_allocator<storage_type::value_type>;
	using spatial_storage_allocator = std::pmr::polymorphic_allocator<spatial_storage_type::value_type>;

public:
	CTextureAtlas();
	CTextureAtlas(CTextureAtlas&& other) noexcept;
	CTextureAtlas(const CTextureAtlas&) = delete;
	CTextureAtlas& operator=(const CTextureAtlas&) = delete;
	~CTextureAtlas();

	CTextureAtlas& operator=(CTextureAtlas&& other) noexcept;

	void init(ID3DDevice* p_device, int width, int height, const char* pName);
	void uninit();

	bool addRegion(ID3DDevice* p_device, ID3DDeviceContext* p_context, const xr_string_view& icon_subpath_name, u32 w, u32 h, const void* pData, u32 pitch = 0);

	// if was successful immediately call addData after that method
	bool tryAddRegion(const xr_string_view& icon_subpath_name, u32 w, u32 h);

	bool addData(ID3DDevice* p_device, ID3DDeviceContext* p_context, u32 w, u32 h, const void* pData, u32 pitch);

	void getRegion(const xr_string_view& icon_subpath_name, u32& w, u32& h);

	void* getResource();
	void* getResource() const;

	void saveOnDisk();

	u32 getID() const;
	void setID(u32);

	u32 getWidth(void) const;
	u32 getHeight(void) const;

	const storage_type& getElements(void) const;

	CTextureAtlasElement* findNearest(float w, float h);
	const CTextureAtlasElement* findNearest(float w, float h) const;

	bool removeElement(float w, float h);
	bool removeElement(element_lookupid_type lookup_id);

private:
	element_lookupid_type findNearestSpatialIndex(float w, float h) const;

	// for older GAPI < DX11
	void addRegion(ID3DDevice* p_device, u32 x, u32 y, u32 w, u32 h, const void* pData, u32 pitch);

	// for newer GAPI >= DX11
	bool addRegion(ID3DDevice* p_device, ID3DDeviceContext* p_context, u32 x, u32 y, u32 w, u32 h, const void* pData, u32 pitch);
private:
#ifdef DEBUG
	bool init_was_called;
#endif
	mutable bool m_is_storage_dirty;
	u32 m_id;

	// logical layout placement 
	smol_atlas_t* m_p_atlas;

	// returned from resource manager and resource manager stores this texture (because later user will need to SetShader calling and for building we need to compile "blender" for that we need to obtain our texture from resource manager otherwise we can't use original way of rendering svg)
	CTexture* m_p_texture;


	// be very careful, change it only when it is needed by sense 
	// otherwise we can't provide find as const
	std::pmr::monotonic_buffer_resource sais_wrapper;
	std::pmr::monotonic_buffer_resource saissi_wrapper;
	mutable storage_type m_atlas_items;
	mutable spatial_storage_type m_atlas_items_spatial_indexing;
	unsigned char static_atlas_items_storage_spatial_indexing[calculate_reserve_count(sizeof(spatial_storage_type::value_type), _kRenderBackend_TextureAtlasPreallocatedItems)];
	unsigned char static_atlas_items_storage[calculate_reserve_count(sizeof(storage_type::value_type), _kRenderBackend_TextureAtlasPreallocatedItems)];
};

#endif
