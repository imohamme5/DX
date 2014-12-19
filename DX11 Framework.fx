Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);


cbuffer cbData
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;

	float4 gDiffuseMtrl;
	float4 gAmbientMtrl;
	float4 gDiffuseLight;
	float4 gAmbientLight;
	float4 gLightVecW;

	float4 gSpecularMtrl;
	float4 gSpecularLight;
	float gSpecularPower;
	float3 gEyePosW;

};

struct VS_IN
{
	float4 posL   : POSITION;
	float3 normalL : NORMAL;
	float2 Tex : TEXCOORD0;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float4 PosW : POSITION;
	float4 Col : COLOR;
	float2 Tex : TEXCOORD0;

};

VS_OUT VS(VS_IN vIn)
{
	VS_OUT output = (VS_OUT)0;
	output.Pos = mul(vIn.posL, World);
	output.PosW = output.Pos;
	// Compute the vector from the vertex to the eye position.
	// output.Pos is currently the position in world space

	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	// Convert from local to world normal
	float3 normalW = mul(float4(vIn.normalL, 0.0f), World).xyz;
		normalW = normalize(normalW);
	output.Norm = normalW;
	// Compute Colour
	// Compute the reflection vector.

	//Texture
	output.Tex = vIn.Tex;

	return output;
}


float4 PS(VS_OUT pIn) : SV_Target
{
	float4 textureColour = txDiffuse.Sample(samLinear, pIn.Tex);
	clip(textureColour.a < 0.25f ? -1 : 1);

	float3 lightvec = normalize(gLightVecW);
		float3 r = reflect(-lightvec, pIn.Norm);
		// Determine how much (if any) specular light makes it
		// into the eye.
		float3 toEye = normalize(gEyePosW - pIn.PosW.xyz);
		float t = pow(max(dot(r, toEye), 0.0f), gSpecularPower);
	// Determine the diffuse light intensity that strikes the vertex.
	float s = max(dot(lightvec, pIn.Norm), 0.0f);
	if (s <= 0.0f)
		t = 0.0f;
	// Compute the ambient, diffuse, and specular terms separately.
	float3 spec = t*(gSpecularMtrl*gSpecularLight).rgb;
		float3 diffuse = s*(gDiffuseMtrl*gDiffuseLight).rgb;
		float3 ambient = gAmbientMtrl * gAmbientLight;
		// Sum all the terms together and copy over the diffuse alpha.

		pIn.Col.rgb = (textureColour * (ambient + diffuse)) + spec;
	pIn.Col.a = gDiffuseMtrl.a;




	return pIn.Col;
}

technique11 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VS())); SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}
