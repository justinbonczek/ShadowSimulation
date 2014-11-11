#include "Lighting.hlsli"

cbuffer perFrame : register(b0)
{
	DirectionalLight dLight;
	PointLight pLight;
	SpotLight sLight;
	matrix view;
	matrix projection;
	float3 eyePos;
	float time;
	float4 fogColor;
	float fogStart;
	float fogRange;
	float pad[2];
};

cbuffer perObject : register(b1)
{
	matrix world;
	matrix worldInverseTranspose;
	LightMaterial lightMat;
	float tileX;
	float tileZ;
	float padO[2];
};

cbuffer shadow : register(b2)
{
	matrix sView;
	matrix sProj;
	float resolution;
	float padS[3];
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 worldpos		: POSITION;
	float4 color		: COLOR;
	float2 uv			: TEXCOORD0;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float4 shadowpos	: TEXCOORD1;
};

Texture2D _Texture : register(t0);
Texture2D _Normal  : register(t1);
Texture2D _ShadowMap : register(t3);
SamplerState _Sampler : register(s0);
SamplerComparisonState _CmpSampler : register(s1);

float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	// Get the normal values from the map and unpack them
	float3 normalT = _Normal.Sample(_Sampler, float2(input.uv.x * tileX, input.uv.y * tileZ));
	normalT = 2.0 * normalT - 1.0;
	
	// TBN matrix calculation and bumped normal calculation
	float3 N = input.normal;
	float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	float3 bumpedNormal = mul(normalT, TBN);
	bumpedNormal = normalize(bumpedNormal);

	// Calculate relation to camera for specularity and camera based effects
	float distToEye = length(eyePos - input.worldpos);
	float3 toEye = normalize(eyePos - input.worldpos);

	// Create light values and set them to zero
	float4 ambient = float4(0, 0, 0, 0);
	float4 diffuse = float4(0, 0, 0, 0);
	float4 spec = float4(0, 0, 0, 0);

	float4 A, D, S;

	///
	// Lighting Calculations (found in lighting.hlsli)
	///
	ComputeDirectionalLight(lightMat, dLight, bumpedNormal, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	spec	+= S;
	
	ComputePointLight(lightMat, pLight, input.worldpos, bumpedNormal, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	spec	+= S;
	
	ComputeSpotLight(lightMat, sLight, input.worldpos, bumpedNormal, toEye, A, D, S);
	ambient += A;
	diffuse += D;
	spec	+= S;

	// Complete projection (if using perspective projection)
	input.shadowpos.xyz /= input.shadowpos.w;

	// Calculate distance between each pixel
	float dx = 1.0 / resolution;

	// PCF Filtering
	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
	};

	// Calculate Shadows
	float lightDepth = input.shadowpos.z;
	for (int i = 0; i < 9; i++)
	{
		percentLit += _ShadowMap.SampleCmpLevelZero(_CmpSampler, input.shadowpos.xy + offsets[i], lightDepth - 0.0005).r;
	}

	percentLit /= 9.0;

	// Sample texture(s)
	float4 texColor = _Texture.Sample(_Sampler, float2(input.uv.x * tileX, input.uv.y * tileZ));

	// Calculate lit color based on lighting and shadow calculations
	float4 litColor = texColor * (ambient + diffuse * percentLit) + spec * percentLit;

	// Fog calculations
	float fogLerp = saturate((distToEye - fogStart) / fogRange);
	litColor = lerp(litColor, fogColor, fogLerp);
	
	// Pass through alpha values
	litColor.a = lightMat.diffuse.a;

	return litColor;
}