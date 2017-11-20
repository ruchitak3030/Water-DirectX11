//cbuffer externalData : register(b0)
//{
//	matrix world;
//	matrix view;
//	matrix projection;
//	matrix reflection;
//};
//
//cbuffer CamNormBuffer
//{
//	float3 CameraPosition;
//	float2 normalMapTiling;
//};
//
//struct VertexShaderInput
//{
//	
//	float3 position		: POSITION;     // XYZ position
//	//float4 color		: COLOR;        // RGBA color
//	float2 uv			: TEXCOORD0;
//};
//
//struct VertexToPixel
//{
//	
//	float4 position				: SV_POSITION;	// XYZW position (System Value Position)
//	//float2 uv					: TEXCOORD0;        // RGBA color
//	float4 reflectionPosition	: TEXCOORD0;
//	float4 refractionPosition	: TEXCOORD1;
//
//	float3 viewDirection		: TEXCOORD2;
//	float2 uv1					: TEXCOORD3;
//	float2 uv2					: TEXCOORD4;
//	float3 worldPos				: POSITION;
//};
//
//VertexToPixel main(VertexShaderInput input)
//{
//	VertexToPixel output;
//	
//	matrix worldViewProj = mul(mul(world, view), projection);
//	output.position = mul(float4(input.position, 1.0f), worldViewProj);
//
//	//output.color = input.color;
//
//	
//
//	//Create reflection proejction world matrix
//	matrix reflectProjWorld = mul(reflection, projection);
//	reflectProjWorld = mul(world, reflectProjWorld);
//
//	// Calculate the input position against the reflectProjectWorld matrix.
//	output.reflectionPosition = mul(input.position, reflectProjWorld);
//
//	//Create the view projection world matrix for refraction
//	matrix viewProjWorld = mul(view, projection);
//	viewProjWorld = mul(world, viewProjWorld);
//
//	// Calculate the input position against the viewProjectWorld matrix.
//	output.refractionPosition = mul(input.position, viewProjWorld);
//
//	// Calculate the position of the vertex in the world.
//	float4 worldPosition = mul(input.position, world);
//
//	// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
//	output.viewDirection = CameraPosition.xyz - worldPosition.xyz;
//
//	// Normalize the viewing direction vector.
//	output.viewDirection = normalize(output.viewDirection);
//
//	//Store the texture coordinates for the pixel shader
//	output.uv1 = input.uv/ normalMapTiling.x;
//	output.uv2 = input.uv / normalMapTiling.y;
//
//	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;
//
//
//
//
//	return output;
//}

cbuffer externalData	: register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix reflection;
}

struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float4 worldPos		: POSITION;
	float2 uv			: TEXCOORD;
	float3 reflWorldPos	: POSITION1;
};

VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;

	// Calculate output position
	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	// Get the normal to the pixel shader
	output.normal = mul(input.normal, (float3x3)world); // ASSUMING UNIFORM SCALE HERE!!!  If not, use inverse transpose of world matrix
	output.tangent = mul(input.tangent, (float3x3)world); // Needed for normal mapping

	// Get world position of vertex
	output.worldPos = mul(float4(input.position, 1.0f), world);

	matrix worldReflProj = mul(mul(world, reflection), projection);
	output.reflWorldPos = mul(float4(input.position, 1.0f), worldReflProj);

	// Pass through the uv
	output.uv = input.uv;

	return output;
}