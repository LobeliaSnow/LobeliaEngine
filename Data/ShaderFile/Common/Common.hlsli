#include "../Header.hlsli"

bool IsFrustumRange(float3 pos, float radius)
{
    for (int i = 0; i < 6; i++)
    {
        float3 dir = pos - frustum.center[i].xyz;
        float dotValue = dot(dir, frustum.normal[i].xyz);
        if (dotValue < -radius)return false;
    }
    return true;
}