struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
};

Texture2D _ShadowMap : register(t3);
SamplerState _Sampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float depth = _ShadowMap.Sample(_Sampler, input.uv).r;
	return float4(depth, depth, depth, 1.0);
}