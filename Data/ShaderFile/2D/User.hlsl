#include "../Header.hlsli"
#include "Header2D.hlsli"

struct VS_OUT_GAUSSIAN
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : TEXCOORD1;
};

VS_OUT_GAUSSIAN GaussianFilterXVS(VS_IN_TEX vs_in)
{
	VS_OUT_GAUSSIAN vs_out = (VS_OUT_GAUSSIAN)0;
	vs_out.pos = float4(vs_in.pos.xyz, 1.0f);
	vs_out.tex = vs_in.tex;
	vs_out.color = vs_in.col;
	return vs_out;
}
VS_OUT_GAUSSIAN GaussianFilterYVS(VS_IN_TEX vs_in)
{
	VS_OUT_GAUSSIAN vs_out = (VS_OUT_GAUSSIAN)0;
	vs_out.pos = float4(vs_in.pos.xyz, 1.0f);
	vs_out.tex = vs_in.tex;
	vs_out.color = vs_in.col;
	return vs_out;
}

typedef VS_OUT_GAUSSIAN PS_IN;
#define LOOP 5

inline float4 LoopGaussX(PS_IN ps_in)
{
	float4 color = txDiffuse.Sample(samLinear, ps_in.tex) * 0.20;
	//[unroll]
	for (int i = 0; i < LOOP; i++)
	{
		float weight = (1.0 * 0.5f) / LOOP;
		weight += (i - 1) * -0.02f;
		float2 texelm = ps_in.tex + float2(-i / width, 0.0f);
		float2 texelp = ps_in.tex + float2(-i / width, 0.0f);
		color += txDiffuse.Sample(samLinear, texelm) * weight;
		color += txDiffuse.Sample(samLinear, texelp) * weight;
	}
	return color;
}
inline float4 NoLoopGaussX(PS_IN ps_in)
{
	float2 texel0 = ps_in.tex + float2(-1.0f / width, 0.0f);
	float2 texel1 = ps_in.tex + float2(-2.0f / width, 0.0f);
	float2 texel2 = ps_in.tex + float2(-3.0f / width, 0.0f);
	float2 texel3 = ps_in.tex + float2(-4.0f / width, 0.0f);
	float2 texel4 = ps_in.tex + float2(-5.0f / width, 0.0f);
	float2 texel5 = ps_in.tex + float2(1.0f / width, 0.0f);
	float2 texel6 = ps_in.tex + float2(2.0f / width, 0.0f);
	float2 texel7 = ps_in.tex + float2(3.0f / width, 0.0f);
	float2 texel8 = ps_in.tex + float2(4.0f / width, 0.0f);
	float2 texel9 = ps_in.tex + float2(5.0f / width, 0.0f);

	float4 color = txDiffuse.Sample(samLinear, ps_in.tex) * 0.20;

	float4 color0 = txDiffuse.Sample(samLinear, texel0) * 0.12;
	float4 color1 = txDiffuse.Sample(samLinear, texel1) * 0.10f;
	float4 color2 = txDiffuse.Sample(samLinear, texel2) * 0.08f;
	float4 color3 = txDiffuse.Sample(samLinear, texel3) * 0.06f;
	float4 color4 = txDiffuse.Sample(samLinear, texel4) * 0.04f;

	float4 color5 = txDiffuse.Sample(samLinear, texel5) * 0.12f;
	float4 color6 = txDiffuse.Sample(samLinear, texel6) * 0.10f;
	float4 color7 = txDiffuse.Sample(samLinear, texel7) * 0.08f;
	float4 color8 = txDiffuse.Sample(samLinear, texel8) * 0.06;
	float4 color9 = txDiffuse.Sample(samLinear, texel9) * 0.04;

	return (color + color0 + color1 + color2 + color3 + color4 + color5 + color6 + color7 + color8 + color9) * ps_in.color;
}

inline float4 LoopGaussY(PS_IN ps_in)
{
	float4 color = txDiffuse.Sample(samLinear, ps_in.tex) * 0.20;
	//[unroll]
	for (int i = 0; i < LOOP; i++)
	{
		float weight = (1.0 * 0.5f) / LOOP;
		weight += (i - 1) * -0.02f;
		float2 texelm = ps_in.tex + float2(0.0f, -i / height);
		float2 texelp = ps_in.tex + float2(0.0f, i / height);
		color += txDiffuse.Sample(samLinear, texelm) * weight;
		color += txDiffuse.Sample(samLinear, texelp) * weight;
	}
	return color;

}
inline float4 NoLoopGaussY(PS_IN ps_in)
{
	float2 texel0 = ps_in.tex + float2(0.0f, -1.0f / height);
	float2 texel1 = ps_in.tex + float2(0.0f, -2.0f / height);
	float2 texel2 = ps_in.tex + float2(0.0f, -3.0f / height);
	float2 texel3 = ps_in.tex + float2(0.0f, -4.0f / height);
	float2 texel4 = ps_in.tex + float2(0.0f, -5.0f / height);
	float2 texel5 = ps_in.tex + float2(0.0f, 1.0f / height);
	float2 texel6 = ps_in.tex + float2(0.0f, 2.0f / height);
	float2 texel7 = ps_in.tex + float2(0.0f, 3.0f / height);
	float2 texel8 = ps_in.tex + float2(0.0f, 4.0f / height);
	float2 texel9 = ps_in.tex + float2(0.0f, 5.0f / height);

	float4 color = txDiffuse.Sample(samLinear, ps_in.tex) * 0.20;
	float4 color0 = txDiffuse.Sample(samLinear, texel0) * 0.12;
	float4 color1 = txDiffuse.Sample(samLinear, texel1) * 0.10f;
	float4 color2 = txDiffuse.Sample(samLinear, texel2) * 0.08f;
	float4 color3 = txDiffuse.Sample(samLinear, texel3) * 0.06f;
	float4 color4 = txDiffuse.Sample(samLinear, texel4) * 0.04f;

	float4 color5 = txDiffuse.Sample(samLinear, texel5) * 0.12f;
	float4 color6 = txDiffuse.Sample(samLinear, texel6) * 0.10f;
	float4 color7 = txDiffuse.Sample(samLinear, texel7) * 0.08f;
	float4 color8 = txDiffuse.Sample(samLinear, texel8) * 0.06;
	float4 color9 = txDiffuse.Sample(samLinear, texel9) * 0.04;

	return (color + color0 + color1 + color2 + color3 + color4 + color5 + color6 + color7 + color8 + color9) * ps_in.color;
}


float4 GaussianFilterXPS(PS_IN ps_in) : SV_Target
{
	return LoopGaussX(ps_in);
//   float4 color = txDiffuse.Sample(samLinear, ps_in.tex) * 0.20;
   ////[unroll]
//   for (int i = 0; i < LOOP; i++)
//   {
//       float weight = (1.0 * 0.5f) / LOOP;
//       weight += (i - 1) * -0.02f;
//       float2 texelm = ps_in.tex + float2(-i / width, 0.0f);
//       float2 texelp = ps_in.tex + float2(-i / width, 0.0f);
//       color += txDiffuse.Sample(samLinear, texelm) * weight;
//       color += txDiffuse.Sample(samLinear, texelp) * weight;
//   }
//   return color;
}

float4 GaussianFilterYPS(PS_IN ps_in) : SV_Target
{
	return LoopGaussY(ps_in);
//   float4 color = txDiffuse.Sample(samLinear, ps_in.tex) * 0.20;
   ////[unroll]
//   for (int i = 0; i < LOOP; i++)
//   {
//       float weight = (1.0 * 0.5f) / LOOP;
//       weight += (i - 1) * -0.02f;
//       float2 texelm = ps_in.tex + float2(0.0f, -i / height);
//       float2 texelp = ps_in.tex + float2(0.0f, i / height);
//       color += txDiffuse.Sample(samLinear, texelm) * weight;
//       color += txDiffuse.Sample(samLinear, texelp) * weight;
//   }
//   return color;

}



//struct VS_OUT_GAUSSIAN
//{
//	float4 pos : SV_POSITION;
//	float2 texel0 : TEXCOORD0; // テクセル
//	float2 texel1 : TEXCOORD1; // テクセル
//	float2 texel2 : TEXCOORD2; // テクセル
//	float2 texel3 : TEXCOORD3; // テクセル
//	float2 texel4 : TEXCOORD4; // テクセル
//	float2 texel5 : TEXCOORD5; // テクセル
//	float2 texel6 : TEXCOORD6; // テクセル
//	float2 texel7 : TEXCOORD7; // テクセル
//	float4 color : TEXCOORD8;
//};
//
//VS_OUT_GAUSSIAN GaussianFilterXVS(VS_IN_TEX vs_in)
//{
//	VS_OUT_GAUSSIAN vs_out = (VS_OUT_GAUSSIAN)0;
//	vs_out.pos = float4(vs_in.pos.xyz, 1.0f);
//	vs_out.texel0 = vs_in.tex + float2(-1.0f / width, 0.0f);
//	vs_out.texel1 = vs_in.tex + float2(-3.0f / width, 0.0f);
//	vs_out.texel2 = vs_in.tex + float2(-5.0f / width, 0.0f);
//	vs_out.texel3 = vs_in.tex + float2(-7.0f / width, 0.0f);
//	vs_out.texel4 = vs_in.tex + float2(-9.0f / width, 0.0f);
//	vs_out.texel5 = vs_in.tex + float2(-11.0f / width, 0.0f);
//	vs_out.texel6 = vs_in.tex + float2(-13.0f / width, 0.0f);
//	vs_out.texel7 = vs_in.tex + float2(-15.0f / width, 0.0f);
//	vs_out.color = vs_in.col;
//	return vs_out;
//}
//VS_OUT_GAUSSIAN GaussianFilterYVS(VS_IN_TEX vs_in)
//{
//	VS_OUT_GAUSSIAN vs_out = (VS_OUT_GAUSSIAN)0;
//	vs_out.pos = float4(vs_in.pos.xyz, 1.0f);
//	vs_out.texel0 = vs_in.tex + float2(0.0f, -1.0f / height);
//	vs_out.texel1 = vs_in.tex + float2(0.0f, -3.0f / height);
//	vs_out.texel2 = vs_in.tex + float2(0.0f, -5.0f / height);
//	vs_out.texel3 = vs_in.tex + float2(0.0f, -7.0f / height);
//	vs_out.texel4 = vs_in.tex + float2(0.0f, -9.0f / height);
//	vs_out.texel5 = vs_in.tex + float2(0.0f, -11.0f / height);
//	vs_out.texel6 = vs_in.tex + float2(0.0f, -13.0f / height);
//	vs_out.texel7 = vs_in.tex + float2(0.0f, -15.0f / height);
//	vs_out.color = vs_in.col;
//	return vs_out;
//}
//
//typedef VS_OUT_GAUSSIAN PS_IN;
//
//float4 GaussianFilterXPS(PS_IN ps_in) : SV_Target
//{
//	float4 color = 0.0f;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel0) + txDiffuse.Sample(samLinear, float2(ps_in.texel7.x + offset_x, ps_in.texel7.y))) * weight0;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel1) + txDiffuse.Sample(samLinear, float2(ps_in.texel6.x + offset_x, ps_in.texel6.y))) * weight1;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel2) + txDiffuse.Sample(samLinear, float2(ps_in.texel5.x + offset_x, ps_in.texel5.y))) * weight2;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel3) + txDiffuse.Sample(samLinear, float2(ps_in.texel4.x + offset_x, ps_in.texel4.y))) * weight3;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel4) + txDiffuse.Sample(samLinear, float2(ps_in.texel3.x + offset_x, ps_in.texel3.y))) * weight4;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel5) + txDiffuse.Sample(samLinear, float2(ps_in.texel2.x + offset_x, ps_in.texel2.y))) * weight5;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel6) + txDiffuse.Sample(samLinear, float2(ps_in.texel1.x + offset_x, ps_in.texel1.y))) * weight6;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel7) + txDiffuse.Sample(samLinear, float2(ps_in.texel0.x + offset_x, ps_in.texel0.y))) * weight7;
//	return color * ps_in.color;
//}
//
//float4 GaussianFilterYPS(PS_IN ps_in) : SV_Target
//{
//	float4 color = 0.0f;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel0) + txDiffuse.Sample(samLinear, float2(ps_in.texel7.x, ps_in.texel7.y + offset_y))) * weight0;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel1) + txDiffuse.Sample(samLinear, float2(ps_in.texel6.x, ps_in.texel6.y + offset_y))) * weight1;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel2) + txDiffuse.Sample(samLinear, float2(ps_in.texel5.x, ps_in.texel5.y + offset_y))) * weight2;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel3) + txDiffuse.Sample(samLinear, float2(ps_in.texel4.x, ps_in.texel4.y + offset_y))) * weight3;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel4) + txDiffuse.Sample(samLinear, float2(ps_in.texel3.x, ps_in.texel3.y + offset_y))) * weight4;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel5) + txDiffuse.Sample(samLinear, float2(ps_in.texel2.x, ps_in.texel2.y + offset_y))) * weight5;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel6) + txDiffuse.Sample(samLinear, float2(ps_in.texel1.x, ps_in.texel1.y + offset_y))) * weight6;
//	color += (txDiffuse.Sample(samLinear, ps_in.texel7) + txDiffuse.Sample(samLinear, float2(ps_in.texel0.x, ps_in.texel0.y + offset_y))) * weight7;
//	return color * ps_in.color;
//}
