//--------------------------------------------------------------------------------------
// File: SkinningShaders.fx
//
// Copyright (c) Microsoft Corporation.
//--------------------------------------------------------------------------------------
#define NUM_LIGHTS (2)

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
static const unsigned int MAX_NUM_BONES = 256u;
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnCameraMovement

  Summary:  Constant buffer used for view transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnCameraMovement : register( b0 )
{
	matrix View;
	float4 CameraPosition;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangeOnResize

  Summary:  Constant buffer used for projection transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangeOnResize : register( b1 )
{
	matrix Projection;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbChangesEveryFrame

  Summary:  Constant buffer used for world transformation
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbChangesEveryFrame : register( b2 )
{
	matrix World;
    float4 OutputColor;
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbLights

  Summary:  Constant buffer used for shading
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbLights : register( b3 )
{
    float4 LightPositions[NUM_LIGHTS];
    float4 LightColors[NUM_LIGHTS];
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Cbuffer:  cbSkinning

  Summary:  Constant buffer used for skinning
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
cbuffer cbSkinning : register( b4 )
{
    matrix BoneTransforms[MAX_NUM_BONES];
}

//--------------------------------------------------------------------------------------
/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   VS_INPUT

  Summary:  Used as the input to the vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct VS_INPUT
{
	float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
    float3 Norm : NORMAL;
    uint4 BoneIndices : BONEINDICES;
    float4 BoneWeights : BONEWEIGHTS; 
};

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Struct:   PS_PHONG_INPUT

  Summary:  Used as the input to the pixel shader, output of the 
            vertex shader
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
struct PS_PHONG_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float3 Norm : NORMAL;
	float4 WorldPos : POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_PHONG_INPUT VSPhong(VS_INPUT input)
{
    PS_PHONG_INPUT output = (PS_PHONG_INPUT)0;
    
    matrix skinTransform = (matrix)0;
    skinTransform += BoneTransforms[input.BoneIndices.x] * input.BoneWeights.x;
    skinTransform += BoneTransforms[input.BoneIndices.y] * input.BoneWeights.y;
    skinTransform += BoneTransforms[input.BoneIndices.z] * input.BoneWeights.z;
    skinTransform += BoneTransforms[input.BoneIndices.w] * input.BoneWeights.w;

    output.Pos = mul(input.Pos, skinTransform);
    output.Pos = mul(output.Pos, World);
    output.WorldPos = output.Pos;
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

    output.Norm = normalize(mul(float4(input.Norm, 0), World).xyz);
    output.Norm = normalize(mul(float4(output.Norm, 0), skinTransform).xyz);

    output.Tex = input.Tex;
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSPhong(PS_PHONG_INPUT input) : SV_TARGET
{
    input.Norm = normalize(input.Norm);
    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 lightDirection = float3(0.0f, 0.0f, 0.0f);
   
    for (uint i = 0; i < NUM_LIGHTS; ++i)
    {
        lightDirection = normalize(LightPositions[i].xyz - input.WorldPos);
        ambient += float3(0.1f, 0.1f, 0.1f) * LightColors[i].xyz;
        diffuse += saturate(dot(input.Norm, lightDirection)) * LightColors[i];
    }

    return float4(diffuse + ambient, 1.0f) * txDiffuse.Sample(samLinear, input.Tex);
}