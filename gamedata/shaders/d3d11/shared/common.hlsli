//////////////////////////////////////////////////
//  All comments by Nivenhbro are preceded by !
/////////////////////////////////////////////////

#ifndef SHARED_COMMON_H
#define SHARED_COMMON_H

//	Used by VS
cbuffer dynamic_transforms
{
    float4x4 m_WVP;
    float3x4 m_WV;
    float3x4 m_W;

    float4x4 m_WVP_old;
    float3x4 m_WV_old;
    float3x4 m_W_old;

    float4x4 m_P_hud;

    float4 L_material;
    float4 hemi_cube_pos_faces;
    float4 hemi_cube_neg_faces;
    float4 dt_params;
}

cbuffer shader_params
{
    float m_AlphaRef;
}

cbuffer static_globals
{
    float3x4 m_invV;

    float3x4 m_V;
    float4x4 m_P;
    float4x4 m_VP;

    float3x4 m_V_old;
    float4x4 m_P_old;
    float4x4 m_VP_old;

    float4 timers;

    float4 fog_plane;
    float4 fog_params;
    float4 fog_color;

    float4 L_ambient;
    float3 L_sun_color;
    float3 L_sun_dir_w;
    float4 L_sky_color;
    float4 L_hemi_color;

    float3 eye_position;

    float4 pos_decompression_params;
    float4 pos_decompression_params2;
    float4 pos_decompression_params_hud;
    float4 depth_unpack;
    float def_aref;
    float4 parallax;

    float4 m_taa_jitter;
}

float calc_cyclic(float x)
{
    float f = 1.4142f * sin(x * 3.14159f);
    return f * f - 1.0f;
}

float2 calc_xz_wave(float2 dir2D, float frac)
{
    return dir2D * frac;
}
#endif

