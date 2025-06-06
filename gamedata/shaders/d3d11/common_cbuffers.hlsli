#ifndef common_cbuffers_h_included
#define common_cbuffers_h_included

//	Used by dynamic lights and volumetric effects
cbuffer dynamic_light
{
    float4 Ldynamic_color; // dynamic light color (rgb1)	- spot/point/sun
    float4 Ldynamic_pos; // dynamic light pos+1/range(w)	- spot/point
    float4 Ldynamic_dir; // dynamic light direction		- sun
	float4 m_lmap[2];
	int Ldynamic_hud;
	float4x4 m_texgen;
	float4x4 mVPTexgen;
}

cbuffer postprocess_data
{
	float4 b_params;
	float4 weight[2];
	float4 MiddleGray;
	float4 screen_res; // Screen resolution (x-Width,y-Height, zw - 1/resolution)
	float4 scaled_screen_res;
	float4 c_brightness;
	float4 c_colormap;
}

cbuffer unsorted_data
{
	float4 tfactor;
	float3 eye_direction;
	float3 water_intensity;
}

cbuffer game_data
{
	float4 m_affects;
	float4 m_timearrow;
	float4 m_timearrow2;
	float4 m_digiclock;
	float4 m_actor_params;
	float4 m_hud_params;
	float4 m_zoom_deviation;
}

cbuffer shadow_data
{
	float4x4 m_shadow;
	float3x4 m_xform;
	float3x4 m_xform_v;
	float4 dir2D;
    float4 dir2D_old;
	float4 consts; // {1/quant,1/quant,???,???}
	float4 c_scale, c_bias, wind, wave;
	float4 consts_old;
    float4 wave_old;
    float4 wind_old;
    float2 c_sun;
}

cbuffer VolumetricLights
{
    float3 vMinBounds;
    float3 vMaxBounds;
    float4 FrustumClipPlane[6];
}

cbuffer DetailConstants
{
    float2x4 array[61];
};

//-----------------------------------------------
// Fluid

cbuffer FluidSimConfig
{
    float textureHeight;
    float textureWidth;
    float textureDepth;

    float modulate = 1.0;
    float epsilon;
    float timestep;
    float forward = 1.0;
    float4 floatVolumeDim; //	Actually float3. We don't support float3 and float2
}

cbuffer AABBBounds
{
    float4 boxLBDcorner; //	float3
    float4 boxRTUcorner; //	float3
}

cbuffer EmitterParams
{
    float size;
    float4 center; //	Actually float3. We don't support float3 and float2
    float4 splatColor;
}

cbuffer OOBBClipPlanes
{
    float4 OOBBClipPlane[6];
    //	0 - Top
    //	1 - Bottom
}

cbuffer DynOOBBData
{
    float3x4 WorldToLocal; //	World to local of fog volume
    float3x4 LocalToWorld; //	Local of fog volume to world
    float4 MassCenter; //	Center for angular velocity
    float4 OOBBWorldAngularVelocity;
    float4 OOBBWorldTranslationVelocity;
}


//	Set once per volume
//	Use for all rendering passes
cbuffer FluidRenderConfig
{
    float RTWidth;
    float RTHeight;

    float4 DiffuseLight;
	float4 DepthUnpack;

    float4x4 WorldViewProjection;
    float4x4 InvWorldViewProjection;

    float ZNear;
    float ZFar;

    float4 gridDim; //	float3
    float4 recGridDim; //	float3
    float maxGridDim;
    float gridScaleFactor = 1.0;
    float4 eyeOnGrid; //	float3
}

//-----------------------------------------------

#define MAX_BONES_COUNT 128

cbuffer SkinConstants
{
	float4 sbones_array[MAX_BONES_COUNT * 3];
	
#ifndef DISABLE_VELOCITY
	float4 sbones_array_old[MAX_BONES_COUNT * 3];
#endif
}


#endif //	common_cbuffers_h_included
