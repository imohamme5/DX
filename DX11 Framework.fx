Texture2D tcDiffuse : register(t0);
SamplerState	samLinear :	register (s0);

cbuffer cbData
{
	float4x4 World; 
	float4x4 View; 
	float4x4 Projection;

	float4 gDiffuseMtrl;  // just a color of the material
	float4 gDiffuseLight; 

	float4 gAmbientMtrl;
	float4 gAmbientLight;

	float4 gLightVecW; //reflection

	float4 gSpecularMtrl;
	float4 gSpecularLight;
	float gSpecularPower;
	float3 gEyePosW;
};

struct VS_IN
{
	float4 posL   : POSITION;
	float3 normalL : NORMAL;
	float2 Tex	:TEXCOORD0; // Added colour to vertex shader
};

struct VS_OUT
{
	float4 Pos    : SV_POSITION;
	float3 Norm	  : NORMAL;
	float4 PosW	  : POSITION;
	float2 Tex  : TEXCOORD0;
};

VS_OUT VS(VS_IN vIn)
{	
	
	VS_OUT output = (VS_OUT)0;
	
	float4 textureColour = txDiffuse.Sample(samLinear, vIn.Tex);

	output.Pos = mul(vIn.posL, World);
	output.PosW = output.Pos;
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	
	// Convert from local to world normal
	float3 normalW = mul(float4(vIn.posL.rgb, 0.0f), World).xyz;
	normalW = normalize(normalW);

	output.Tex = input.Tex;
	output.Norm = normalW;
	
	
	output.Col = vIn.Col;  // This is the line to pass to pixel shader
	return output;
}

float4 PS(VS_OUT pIn) : SV_Target
{
	float3 normalW = normalize(pIn.Norm);
	float3 lightVec = normalize(gLightVecW);
	float3 toEye = normalize(gEyePosW - pIn.PosW.xyz);


	// Compute Colour
	// Compute the reflection vector.
	float3 r = reflect(-lightVec, normalW);
	// Determine how much (if any) specular light makes it
	// into the eye.
	float t = pow(max(dot(r, toEye), 0.0f), gSpecularPower);
	// Determine the diffuse light intensity that strikes the vertex.
	float s = max(dot(gLightVecW, normalW), 0.0f);

	if (s <= 0.0f)
	{
		t = 0.0f;
	}

	// Compute the ambient, diffuse, and specular terms separately.
	float3 spec = t*(gSpecularMtrl*gSpecularLight).rgb;
		float3 diffuse = s*(gDiffuseMtrl*gDiffuseLight).rgb;
		float3 ambient = gAmbientMtrl * gAmbientLight;
		// Sum all the terms together and copy over the diffuse alpha.
		pIn.Col.rgb = ambient + diffuse + spec;
	pIn.Col.a = gDiffuseMtrl.a;
	float3 texColour = g_texture.Sample(TexSamplerWrap, a_input.Tex);

		//return pIn.Col;// float4(ambient, 1.0f);//pIn.Col;
		return texColour;
}

technique11 Render
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) ); SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
	}
}
