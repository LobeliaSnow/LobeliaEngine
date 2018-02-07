#include "../Header.hlsli"
#include "Header2D.hlsli"

PS_IN_TEX Main2D(VS_IN_TEX vs_in)
{
    PS_IN_TEX output;
    output.pos = vs_in.pos;
    output.col = vs_in.col;
    output.tex = vs_in.tex;
    return output;
}

PS_IN_TEX Main2DInst(VS_IN_INSTANCING_TEX vs_in)
{
    PS_IN_TEX vs_out;

    vs_out.pos = mul(vs_in.pos, vs_in.ndcTrans); //ワールド変換
    vs_out.col = vs_in.col;
    vs_out.tex = vs_in.tex * vs_in.uvTrans.zw + vs_in.uvTrans.xy;
	
    return vs_out;
}

//instancing renderer
PS_IN_TEX_RENDERER Main2DInstRenderer(VS_IN_INSTANCING_TEX_RENDERER vs_in)
{
    PS_IN_TEX_RENDERER vs_out;

    vs_out.pos = mul(vs_in.pos, vs_in.ndcTrans); //ワールド変換
    vs_out.col = vs_in.col;
    vs_out.tex = vs_in.tex * vs_in.uvTrans.zw + vs_in.uvTrans.xy;
    vs_out.textureSlot = vs_in.textureSlot;

    return vs_out;
}