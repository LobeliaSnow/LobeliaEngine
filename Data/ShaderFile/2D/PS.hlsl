#include "../Header.hlsli"
#include "Header2D.hlsli"

float4 NegativeReversal(float4 color)
{
    return float4(1.0f - color.rgb, color.a);
}
float4 Grayscale(float4 color)
{
    static const float4 mono = float4(0.298912, 0.586611, 0.114478, 0.0);
    color.rgb = dot(color.rgb, mono.rgb);
    return color;
}
float4 Sepia(float4 color)
{
    static const float4 sepiaTone = float4(0.941176f, 0.784313f, 0.293103f, 1.0f);
    return Grayscale(color) * sepiaTone;
}
interface PSInterface
{
    float4 GetColor(PS_IN_TEX ps_in);
};

//引数もシェーダーリンケージのinterfaceすれば変えれる説浮上
class TextureColor : PSInterface
{
    float4 GetColor(PS_IN_TEX ps_in)
    {
        return txDiffuse.Sample(samLinear, ps_in.tex);
    }
};

class VertexColor : PSInterface
{
    float4 GetColor(PS_IN_TEX ps_in)
    {
        return ps_in.col;
    }
};
class InvertTextureColor : PSInterface
{
    float4 GetColor(PS_IN_TEX ps_in)
    {
        return NegativeReversal(txDiffuse.Sample(samLinear, ps_in.tex));
    }
};
class GrayscaleTextureColor : PSInterface
{
    float4 GetColor(PS_IN_TEX ps_in)
    {
        return Grayscale(txDiffuse.Sample(samLinear, ps_in.tex));
    }
};
class SepiaTextureColor : PSInterface
{
    float4 GetColor(PS_IN_TEX ps_in)
    {
        return Sepia(txDiffuse.Sample(samLinear, ps_in.tex));
    }
};

//まんまとはまったのでメモ
//シェーダーリンケージする際に、interfaceに対してclassが一つの場合
//それ以外に呼び出される可能性は0なので最適化でインライン展開されてしまい
//このインターフェースのインスタンスも消える。
PSInterface ps;

float4 Main2D(PS_IN_TEX ps_in) : SV_Target
{
    return ps.GetColor(ps_in) * ps_in.col;
}


//instancing renderer
float4 FetchTextureColor(int slot, float2 uv)
{
    switch (slot)
    {
        case 0:
            return txDiffuse0.Sample(samLinear, uv);
        case 1:
            return txDiffuse1.Sample(samLinear, uv);
        case 2:
            return txDiffuse2.Sample(samLinear, uv);
        case 3:
            return txDiffuse3.Sample(samLinear, uv);
        case 4:
            return txDiffuse4.Sample(samLinear, uv);
        case 5:
            return txDiffuse5.Sample(samLinear, uv);
        case 6:
            return txDiffuse6.Sample(samLinear, uv);
        case 7:
            return txDiffuse7.Sample(samLinear, uv);
    }
    return float4(1, 1, 1, 1);
}

float4 Main2DRenderer(PS_IN_TEX_RENDERER ps_in) : SV_Target
{
    return FetchTextureColor(ps_in.textureSlot, ps_in.tex) * ps_in.col;
}