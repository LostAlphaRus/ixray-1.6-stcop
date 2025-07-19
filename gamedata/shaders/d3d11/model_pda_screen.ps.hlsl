#include "common.hlsli"
#include "sload.hlsli"

Texture2D 	s_vp2;
Texture2D 	s_load;

uniform	float4 		m_affects;

float get_noise(float2 co)
{
      return (frac(sin(dot(co.xy ,float2(12.9898,78.233))) * 43758.5453))*0.5;
};

// TODO: gbuffer someday
#if 0
float4 problems_main( p_bumped_new I )
{
	// узкая полоска искажений
	float problems = frac( timers.z * 5*(1 + 2 * m_affects.x) );	
	I.tcdh.x+= ( m_affects.x > 0.09 && I.tcdh.y > problems-0.01 && I.tcdh.y < problems) ? sin((I.tcdh.y-problems)*5*m_affects.y)  : 0;

	// широкая полоска искажений	
	problems = cos( ( frac( timers.z *2 ) - 0.5 ) * 3.1416 )*2 - 0.8;
	float AMPL = 0.13;
	I.tcdh.x -= ( m_affects.x > 0.15 && I.tcdh.y > problems-AMPL && I.tcdh.y < problems+AMPL) ? cos(4.71*(I.tcdh.y-problems)/AMPL) * sin( frac(timers.z)*6.2831*90 )  * 0.02 * (AMPL-abs(I.tcdh.y-problems))/AMPL : 0;		
	
	// тряска влево-вправо в финальной стадии
	I.tcdh.x += ( m_affects.x > 0.38 ) ? (m_affects.y - 0.5) * 0.04 : 0;	
	
	float4	t_vp2	 = (m_affects.x < 0.27) ? s_vp2.Sample	( smp_base, I.tcdh.xy) : s_base.Sample	( smp_base, I.tcdh.xy) ;  	
	
	// Шум при выбросе
	float noise	= get_noise(I.tcdh.xy*timers.z) *  m_affects.x * m_affects.x * 20;		
	t_vp2.r += noise;
	t_vp2.g += noise;
	t_vp2.b += noise;
	
	//отключение экрана
	t_vp2.rgb = (m_affects.x > 0.41) ? 0 : t_vp2.rgb;
	
	return  float4	(t_vp2.r, t_vp2.g, t_vp2.b, 1);
}


float4 loading_main( p_bumped_new I )
{
	float4 t_load = s_load.Sample ( smp_base, I.tcdh.xy);
	return  float4	(t_load.r, t_load.g, t_load.b, 1);
}

void main(p_bumped_new I, out IXrayGbufferPack O)
{
    IXrayMaterial M;
    M.Depth = I.position.z;
	
#ifdef USE_CLIP_NEAR_PLANE
	clip(I.hpos_curr.z - I.hpos_curr.w * 0.02f);
#endif

    M.Sun = I.tcdh.w;
    M.Hemi = I.tcdh.z;
    M.Point = I.position.xyz;

    SloadNew(I, M);

#ifdef USE_AREF
    #if defined(USE_HASHED_AREF) && !defined(DETAIL_SHADOW_PASS)
		    clip(M.Color.w - hashed_alpha_test(M.Point));
    #else
		    clip(M.Color.w - def_aref);
    #endif
    #ifdef USE_DXT1_HACK
	      M.Color.xyz *= rcp(max(0.0001f, M.Color.w));
    #endif
#endif

#if defined(USE_BUMP) || defined(USE_TDETAIL_BUMP)
    M.Normal = mul(float3x3(I.M1, I.M2, I.M3), M.Normal);
#else
	M.Normal = float3(I.M1.z, I.M2.z, I.M3.z);
#endif

    M.Normal = normalize(M.Normal);

#ifdef USE_LM_HEMI
    float4 lm = s_hemi.Sample(smp_rtlinear, I.tcdh.zw);

    M.Sun = get_sun(lm);
    M.Hemi = get_hemi(lm);
#endif

#ifdef USE_LEGACY_LIGHT
    #ifndef USE_PBR
		M.Metalness = L_material.w;
    #else
		M.Color.xyz *= M.AO;
		M.AO = 1.0f;
		float Specular = M.Metalness * dot(M.Color.xyz, LUMINANCE_VECTOR);
		M.Color.xyz = lerp(M.Color.xyz, 0.04f, M.Metalness);
		M.Metalness = 0.5f - M.Roughness * M.Roughness * 0.5f;
		M.Roughness = Specular;
    #endif
#endif

#if defined(USE_AREF) && defined(USE_TREEWAVE)
    M.SSS = 1.0f;
#endif

	float4 t_base = (m_affects.a > 0 && m_affects.x >= 0.08 ) ? loading_main(I) : problems_main(I);
	M.Color.xyz = detonemap(t_base.xyz);

    O.Velocity = I.hpos_curr.xy / I.hpos_curr.w - I.hpos_old.xy / I.hpos_old.w;
    GbufferPack(O, M);
}

#endif

struct 	v2p
{
 	float2 	tc0: 		TEXCOORD0;	// base
 	float3 	tc1: 		TEXCOORD1;	// environment
  	float4	c0:			COLOR0;		// sun.(fog*fog)
};

float4 problems_main( v2p I )
{
	// узкая полоска искажений
	float problems = frac( timers.z * 5*(1 + 2 * m_affects.x) );	
	I.tc0.x+= ( m_affects.x > 0.09 && I.tc0.y > problems-0.01 && I.tc0.y < problems) ? sin((I.tc0.y-problems)*5*m_affects.y)  : 0;

	// широкая полоска искажений	
	problems = cos( ( frac( timers.z *2 ) - 0.5 ) * 3.1416 )*2 - 0.8;
	float AMPL = 0.13;
	I.tc0.x -= ( m_affects.x > 0.15 && I.tc0.y > problems-AMPL && I.tc0.y < problems+AMPL) ? cos(4.71*(I.tc0.y-problems)/AMPL) * sin( frac(timers.z)*6.2831*90 )  * 0.02 * (AMPL-abs(I.tc0.y-problems))/AMPL : 0;		
	
	// тряска влево-вправо в финальной стадии
	I.tc0.x += ( m_affects.x > 0.38 ) ? (m_affects.y - 0.5) * 0.04 : 0;	
	
	float4	t_vp2	 = (m_affects.x < 0.27) ? s_vp2.Sample	( smp_base, I.tc0) : s_base.Sample	( smp_base, I.tc0) ;  	
	
	// Шум при выбросе
	float noise	= get_noise(I.tc0*timers.z) *  m_affects.x * m_affects.x * 20;		
	t_vp2.r += noise;
	t_vp2.g += noise;
	t_vp2.b += noise;
	
	//отключение экрана
	t_vp2.rgb = (m_affects.x > 0.41) ? 0 : t_vp2.rgb;
	
	return  float4	(t_vp2.r, t_vp2.g, t_vp2.b, 1);
}


float4 loading_main( v2p I )
{
	float4 t_load = s_load.Sample ( smp_base, I.tc0);
	return  float4	(t_load.r, t_load.g, t_load.b, 1);
}

float4 main(p_bumped_new I) : SV_Target
{
	v2p vp;
	vp.tc0 = I.tcdh.xy;

	float4 t_base = (m_affects.a > 0 && m_affects.x >= 0.08 ) ? loading_main(vp) : problems_main(vp);
	t_base.xyz = detonemap(t_base.xyz);
	t_base.xyz *= 0.45f; // TODO: vsrati tonemap, hozar fix pozalyista, dakyu
	return t_base;
}