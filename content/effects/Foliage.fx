static const float PI = 3.141592654f;

cbuffer CBufferPerObject
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
}

cbuffer CBufferPerFrame
{
    float4 SunDirection;
    float4 SunColor;
    float4 AmbientColor;
    float4 ShadowTexelSize;
    float4 ShadowCascadeDistances;
    float RotateToCamera;
    float Time;
    float WindFrequency;
    float WindStrength;
    float WindGustDistance;
    float4 WindDirection;
}

SamplerState ColorSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
};

Texture2D albedoTexture;

struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TextureCoordinates : TEXCOORD0;
    row_major float4x4 World : WORLD;
    float3 Color : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 TextureCoordinates : TEXCOORD0;
    float3 Color : TEXCOORD1;
};

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT) 0;

    float4 localPos = IN.Position;
    
    float vertexHeight = 0.5f;
    
    //if (RotateToCamera > 0.0f)
    //{
    //    float scaleX = sqrt(IN.World[0][0] * IN.World[0][0] + IN.World[0][1] * IN.World[0][1] + IN.World[0][2] * IN.World[0][2]);
    //    //float scaleY = sqrt(IN.World[1][0] * IN.World[1][0] + IN.World[1][1] * IN.World[1][1] + IN.World[1][2] * IN.World[1][2]);
    //    //float scaleZ = sqrt(IN.World[2][0] * IN.World[2][0] + IN.World[2][1] * IN.World[2][1] + IN.World[2][2] * IN.World[2][2]);
    //
    //    float4x4 modelView = mul(IN.World, View);
    //
    //    //cylindrical rotation to camera trick from https://www.geeks3d.com/20140807/billboarding-vertex-shader-glsl/
    //    modelView[0][0] = scaleX;
    //    modelView[0][1] = 0.0f;
    //    modelView[0][2] = 0.0f;
    //
    //    //modelView[1][0] = 0.0f; //uncomment for spherical rotation
    //    //modelView[1][1] = 1.0f; //uncomment for spherical rotation
    //    //modelView[1][2] = 0.0f; //uncomment for spherical rotation
    //
    //    modelView[2][0] = 0.0f;
    //    modelView[2][1] = 0.0f;
    //    modelView[2][2] = 1.0f; //dont care bout z scale in a billboard
    //
    //    OUT.Position = mul(IN.Position, modelView);
    //}
    //else
    {
        OUT.Position = mul(IN.Position, IN.World);
        if (localPos.y > vertexHeight)
        {
            OUT.Position.x += sin(Time * WindFrequency + OUT.Position.x * WindGustDistance) * vertexHeight * WindStrength * WindDirection.x;
            OUT.Position.z += sin(Time * WindFrequency + OUT.Position.z * WindGustDistance) * vertexHeight * WindStrength * WindDirection.z;
        }
        OUT.Position = mul(OUT.Position, View);
    }

    OUT.Position = mul(OUT.Position, Projection);
    OUT.TextureCoordinates = IN.TextureCoordinates;
    OUT.Color = IN.Color;
    
    return OUT;
}
float4 pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float4 albedo = albedoTexture.Sample(ColorSampler, IN.TextureCoordinates);

    float4 color = albedo * SunColor * float4(0.8f, 0.8f, 0.8, 1.0f);
    
    // Saturate the final color result.
    color = saturate(color);

    return color;
}

/************* Techniques *************/
technique11 main
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, pixel_shader()));

    }
}