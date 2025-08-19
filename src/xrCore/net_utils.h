#pragma once
#include "client_id.h"

#pragma pack(push,1)

struct XRCORE_API IIniFileStream
{
	virtual void	 __stdcall 	move_begin		()							= 0;

	virtual void	 __stdcall 	w_float			( float a)					= 0;
	virtual void 	 __stdcall 	w_vec3			( const Fvector& a)			= 0;
	virtual void 	 __stdcall 	w_vec4			( const Fvector4& a)		= 0;
	virtual void 	 __stdcall 	w_u64			( u64 a)					= 0;
	virtual void 	 __stdcall 	w_s64			( s64 a)					= 0;
	virtual void 	 __stdcall 	w_u32			( u32 a)					= 0;
	virtual void 	 __stdcall 	w_s32			( s32 a)					= 0;
	virtual void 	 __stdcall 	w_u16			( u16 a)					= 0;
	virtual void 	__stdcall	w_s16			( s16 a)					= 0;
	virtual void	__stdcall	w_u8			( u8 a)						= 0;
	virtual void	__stdcall	w_s8			( s8 a)						= 0;
	virtual void	__stdcall	w_stringZ		( LPCSTR S)					= 0;
					
	virtual void	__stdcall	r_vec3			(Fvector&)					= 0;
	virtual void	__stdcall	r_vec4			(Fvector4&)					= 0;
	virtual void	__stdcall	r_float			(float&)					= 0;
	virtual void	__stdcall	r_u8			(u8&)						= 0;
	virtual void	__stdcall	r_u16			(u16&)						= 0;
	virtual void	__stdcall	r_u32			(u32&)						= 0;
	virtual void	__stdcall	r_u64			(u64&)						= 0;
	virtual void	__stdcall	r_s8			(s8&)						= 0;
	virtual void	__stdcall	r_s16			(s16&)						= 0;
	virtual void	__stdcall	r_s32			(s32&)						= 0;
	virtual void	__stdcall	r_s64			(s64&)						= 0;

	virtual void	__stdcall	r_string		(LPSTR dest, u32 dest_size)	= 0;
	virtual void	__stdcall	skip_stringZ	()							= 0;
};


constexpr size_t NET_PacketSizeLimit = 16 * 1024;
constexpr size_t NET_PacketSizeLight = 8 * 1024;

#define INI_W(what_to_do)\
if(inistream)\
{\
	inistream->what_to_do;\
}

#ifdef SHIPPING_BUILD
#define INI_ASSERT(what_to_do)
#else
#define INI_ASSERT(what_to_do)\
{\
if(inistream)\
	R_ASSERT3(0,#what_to_do,"not implemented");\
}
#endif

class XRCORE_API NET_PacketBase
{
public:
	IIniFileStream* inistream = nullptr;

	void construct(const void* data, size_t size)
    {
		memcpy(GetBuffer(), data, size);
		SetBufferSize(size);
    }

	virtual u8* GetBuffer() = 0;
	virtual const size_t GetBufferSize() const = 0;
	virtual void SetBufferSize(size_t Sise) = 0;

	u32 r_pos = 0;
	u32 timeReceive = 0;
	bool w_allow = true;

public:
	// writing - main
	IC void write_start	()				{ SetBufferSize(0); INI_W(move_begin());}
	IC void	w_begin		( u16 type	)	{ SetBufferSize(0); w_u16(type);}

	struct W_guard
	{
		bool* guarded;
		W_guard(bool* b) : guarded(b) { *b = true; }
		~W_guard() { *guarded = false; }
	};

	IC void	w(const void* p, u32 count)
	{
		R_ASSERT(inistream == nullptr || w_allow);
		VERIFY(p && count);
		VERIFY(GetBufferSize() + count < NET_PacketSizeLimit);
		CopyMemory(&GetBuffer()[GetBufferSize()], p, count);
		SetBufferSize(GetBufferSize() + count);
		VERIFY(GetBufferSize() < NET_PacketSizeLimit);
	}

	IC void r(void* p, u32 count)
	{
		R_ASSERT(inistream == nullptr);
		VERIFY(p && count);
		CopyMemory(p, &GetBuffer()[r_pos], count);
		r_pos += count;
		VERIFY(r_pos <= GetBufferSize());
	}

	// read/write operators
	template <typename T>
	void operator<<(const T& value)
	{
		w(&value, (u32)sizeof(T));
	}

	template <typename T>
	void operator>>(T& value)
	{
		r(&value, (u32)sizeof(T));
	}

	IC void w_seek(u32 pos, const void* p, u32 count);
	IC u32	w_tell() { return (u32)GetBufferSize(); }

	// writing - utilities
	IC void	w_float		(float a       )	{ W_guard g(&w_allow);  w(&a,4);				INI_W(w_float(a)); }
	IC void w_vec3		(const Fvector& a)  { W_guard g(&w_allow);  w(&a,sizeof(Fvector));	INI_W(w_vec3(a));  }
	IC void w_vec4		(const Fvector4& a) { W_guard g(&w_allow);  w(&a,sizeof(Fvector4));	INI_W(w_vec4(a));  }
	IC void w_u64		(u64 a			)	{ W_guard g(&w_allow);  w(&a,8);				INI_W(w_u64(a));   }
	IC void w_s64		(s64 a			)	{ W_guard g(&w_allow);  w(&a,8);				INI_W(w_s64(a));   }
	IC void w_u32		(u32 a			)	{ W_guard g(&w_allow);  w(&a,4);				INI_W(w_u32(a));   }
	IC void w_s32		(s32 a			)	{ W_guard g(&w_allow);  w(&a,4);				INI_W(w_s32(a));   }
	IC void w_u16		(u16 a			)	{ W_guard g(&w_allow);  w(&a,2);				INI_W(w_u16(a));   }
	IC void w_s16		(s16 a			)	{ W_guard g(&w_allow);  w(&a,2);				INI_W(w_s16(a));   }
	IC void	w_u8		(u8 a			)	{ W_guard g(&w_allow);  w(&a,1);				INI_W(w_u8(a));    }
	IC void	w_s8		(s8 a			)	{ W_guard g(&w_allow);  w(&a,1);				INI_W(w_s8(a));    }

	IC void w_float_q16	( float a, float min, float max)
	{
		VERIFY		(a>=min && a<=max);
		float q		= (a-min)/(max-min);
		w_u16( u16(iFloor(q*65535.f+0.5f)));
	}
	IC void w_float_q8	( float a, float min, float max)
	{
		VERIFY		(a>=min && a<=max);
		float q		= (a-min)/(max-min);
		w_u8( u8(iFloor(q*255.f+0.5f)));
	}
	IC void w_angle16	( float a		)	{	w_float_q16	(angle_normalize(a),0,PI_MUL_2);}
	IC void w_angle8	( float a		)	{w_float_q8	(angle_normalize(a),0,PI_MUL_2);	}
	IC void w_dir		( const Fvector& D) {w_u16(pvCompress(D));							}
	IC void w_sdir		( const Fvector& D) {
		Fvector C;
		float mag		= D.magnitude();
		if (mag>EPS_S)	{
			C.div		(D,mag);
		} else {
			C.set		(0,0,1);
			mag			= 0;
		}
		w_dir	(C);
		w_float (mag);
	}
	IC void w_stringZ			( LPCSTR S )	{ W_guard g(&w_allow); w(S,(u32)xr_strlen(S)+1);	INI_W(w_stringZ(S));		}
	IC void w_stringZ			( const shared_str& p)
	{
		W_guard g(&w_allow); 
    	if (*p)	
			w(*p,p.size()+1);
		else{
			IIniFileStream* tmp = inistream;
			inistream = NULL;
			w_u8(0);
			inistream = tmp; //hack -(
		}

		INI_W		(w_stringZ(p.c_str()));
	}
	IC void w_matrix			(Fmatrix& M)
	{
		w_vec3		(M.i);
		w_vec3		(M.j);
		w_vec3		(M.k);
		w_vec3		(M.c);
	}
	
	IC void w_clientID			(ClientID& C) {		w_u32(C.value());	}
	
	IC void	w_chunk_open8		(u32& position)
	{
		position	= w_tell();
		w_u8		(0);
		INI_ASSERT	(w_chunk_open8)
	}
	
	IC void w_chunk_close8		(u32 position)
	{
		u32 size	= u32(w_tell() - position) - sizeof(u8);
		VERIFY		(size<256	);
		u8			_size = (u8)size;
		w_seek		(position,&_size,sizeof(_size));
		INI_ASSERT	(w_chunk_close8)
	}

	IC void	w_chunk_open16		(u32& position)
	{
		position	= w_tell	();
		w_u16		(0);
		INI_ASSERT	(w_chunk_open16)
	}

	IC void w_chunk_close16		(u32 position)
	{
		u32 size	= u32(w_tell() - position) - sizeof(u16);
		VERIFY		(size < 65536);
		u16			_size = (u16)size;
		w_seek		(position,&_size,sizeof(_size));
		INI_ASSERT	(w_chunk_close16)
	}

	// reading
	void		read_start		();
	u32			r_begin			( u16& type	);
	void		r_seek			(u32 pos);
	u32			r_tell			();

	BOOL		r_eof			();
	u32			r_elapsed		();
	void		r_advance		(u32 size);

	// reading - utilities
	void		r_vec3			(Fvector& A);
	void		r_vec4			(Fvector4& A);
	void		r_float			(float& A );
	void 		r_u64			(u64& A);
	void 		r_s64			(s64& A);
	void 		r_u32			(u32& A);
	void		r_s32			(s32& A);
	void		r_u16			(u16& A);
	void		r_s16			(s16& A);
	void		r_u8			(u8&  A);
	void		r_s8			(s8&  A);

	// IReader compatibility
	Fvector		r_vec3			();
	Fvector4	r_vec4			();
	float		r_float_q8		(float min,float max);
	float		r_float_q16		(float min, float max);
	float		r_float			();
	u64 		r_u64			();
	s64 		r_s64			();
	u32 		r_u32			();
	s32			r_s32			();
	u16			r_u16			();
	s16			r_s16			();
	u8			r_u8			();
	s8			r_s8			();

	void		r_float_q16		(float& A, float min, float max);
	void		r_float_q8		(float& A, float min, float max);
	void		r_angle16		(float& A);
	void		r_angle8		(float& A);
	void		r_dir			(Fvector& A);

	void		r_sdir			(Fvector& A);
	void		r_stringZ		(LPSTR S );
	void		r_stringZ		(xr_string& dest );
	void 		r_stringZ		(shared_str& dest);
	
	void		skip_stringZ	();
	
	void		r_stringZ_s		(LPSTR string, u32 size);

	template <u32 size>
	inline void	r_stringZ_s		(char (&string)[size])
	{
		r_stringZ_s	(string, size);
	}

	void		r_matrix		(Fmatrix& M);
	void		r_clientID		(ClientID& C);
};

template <size_t SSize>
struct NET_Buffer
{
	u8 data[SSize] = {};
	u32 count = 0;
};

class XRCORE_API NET_Packet :
	public NET_PacketBase
{
	NET_Buffer<NET_PacketSizeLimit> B;

public:
	NET_Packet() = default;

	virtual u8* GetBuffer()
	{
		return B.data;
	}
	
	virtual const u8* GetBuffer() const
	{
		return B.data;
	}

	virtual const size_t GetBufferSize() const
	{
		return B.count;
	}

	virtual void SetBufferSize(size_t Sise) override
	{
		B.count = Sise;
	};
};

class XRCORE_API NET_PacketLight :
	public NET_PacketBase
{
	NET_Buffer<NET_PacketSizeLight> B;

public:
	NET_PacketLight() = default;

	virtual u8* GetBuffer()
	{
		return B.data;
	}

	virtual const size_t GetBufferSize() const
	{
		return B.count;
	}

	virtual void SetBufferSize(size_t Sise) override
	{
		B.count = Sise;
	};
};

#pragma pack(pop)