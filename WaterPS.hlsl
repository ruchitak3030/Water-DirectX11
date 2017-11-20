////Texture2D reflectionTexture			: register(t0);
//TextureCube reflectionTexture		: register(t0);
//Texture2D refractionTexture			: register(t1);
//Texture2D normalTexture				: register(t2);
//SamplerState SampleType				: register(s0);
//
//
//cbuffer waterData			: register(b0)
//{
//	float4 refractionTint;
//	//float3 lightDirection;
//	float3 CameraPosition;
//	float WaterTranslation;
//	float reflectRefractScale;
//	float specularShininess;
//
//};
//
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
//
//float4 main(VertexToPixel input) : SV_TARGET
//{
//
//
//	float4 normalMap1;
//	float4 normalMap2;
//	float3 normal1;
//	float3 normal2;
//	float3 normal;
//	float2 reflectTexCoord;
//	float2 refractTexCoord;
//	float3 heightView;
//
//	// Move the position the water normal is sampled from to simulate moving water.	
//	input.uv1.y += WaterTranslation;
//	input.uv2.y += WaterTranslation;
//
//	//Sample the normal from normal map texture using two different tilled and translated coordinates
//	normalMap1 = normalTexture.Sample(SampleType, input.uv1);
//	normalMap2 = normalTexture.Sample(SampleType, input.uv2);
//
//	// Expand the range of the normal from (0,1) to (-1,+1).
//	normal1 = (normalMap1.rgb * 2.0f) - 1.0f;
//	normal2 = (normalMap2.rgb * 2.0f) - 1.0f;
//
//	// Combine the normals to add the normal maps together.
//	normal = normalize(normal1 + normal2);
//	
//	//Calculate the projected reflection texture coordinates.
//	reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
//	reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;
//
//
//	// Calculate the projected refraction texture coordinates.
//	refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
//	refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;
//	
//	
//	// Re-position the texture coordinate sampling position by the normal map value to simulate the rippling wave effect.
//	reflectTexCoord = reflectTexCoord + (normal.xy * reflectRefractScale);
//	refractTexCoord = refractTexCoord + (normal.xy * reflectRefractScale);
//
//	// Sample the texture pixels from the textures using the updated texture coordinates.
//	float4 reflectionColor = reflectionTexture.Sample(SampleType, float3(reflectTexCoord, 1.0f));
//	float4 refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);
//	
//	// Combine the reflection and refraction results for the final color.
//	// Combine the tint with the refraction color.
//	refractionColor = saturate(refractionColor * refractionTint);
//
//	// Get a modified viewing direction of the camera that only takes into account height.
//	heightView.x = input.viewDirection.y;
//	heightView.y = input.viewDirection.y;
//	heightView.z = input.viewDirection.y;
//
//	// Now calculate the fresnel term based solely on height.
//	float r = (1.2f - 1.0f) / (1.2f + 1.0f);
//	float fresnelFactor = max(0.0f, min(1.0f, r + (1.0f - r) * pow(1.0f - dot(normal, heightView), 2)));
//
//	// Combine the reflection and refraction results for the final color using the fresnel factor.
//	float4 color = lerp(reflectionColor, refractionColor, fresnelFactor);
//
//	// Calculate the reflection vector using the normal and the direction of the light.
//	//float3 reflection = -reflect(normalize(lightDirection), normal);
//	float3 reflection = -reflect(normalize(CameraPosition - input.worldPos), normal);
//	 
//
//	// Calculate the specular light based on the reflection and the camera position.
//	float specular = dot(normalize(reflection), normalize(input.viewDirection));
//
//	// Check to make sure the specular was positive so we aren't adding black spots to the water.
//	if (specular > 0.0f)
//	{
//		// Increase the specular light by the shininess value.
//		specular = pow(specular, specularShininess);
//
//		// Add the specular to the final color.
//		color = saturate(color + specular);
//	}
//
//	return color;
//
//}

cbuffer lightData : register(b0)
{
	float4 DirLightColor;
	float3 DirLightDirection;

	float4 PointLightColor;
	float3 PointLightPosition;

	float3 CameraPosition;
}

cbuffer waterData	: register(b1)
{
	float waterTranslation;
}

//External texture-related data
TextureCube reflectionTexture		: register(t0);
Texture2D refractionTexture			: register(t1);
Texture2D NormalMap					: register(t2);
TextureCube Sky						: register(t3);
SamplerState Sampler				: register(s0);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float4 worldPos		: POSITION;
	float2 uv			: TEXCOORD;
	float3 reflWorldPos	: POSITION1;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	// Move the position the water normal is sampled from to simulate moving water.	
	input.uv.x += waterTranslation;


	input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);

	//Normal Mapping
	float3 normalFromMap = NormalMap.Sample(Sampler, input.uv).rgb * 2 - 1;

	//Calculate my TBN matrix to get the normal into world space
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	input.normal = normalize(mul(normalFromMap, TBN));

	//DirectionalLight calculation
	float dirLightAmount = saturate(dot(input.normal, -normalize(DirLightDirection)));

	//Point light calculation
	float3 dirToPointLight = normalize(PointLightPosition - input.worldPos);
	float pointLightAmount = saturate(dot(input.normal, dirToPointLight));

	//Specular
	float3 toCamera = normalize(CameraPosition - input.worldPos);
	float3 refl = reflect(-dirToPointLight, input.normal);
	float spec = pow(max(dot(refl, toCamera), 0), 32);

	float3 toCamera1 = normalize(CameraPosition - input.reflWorldPos);
	//Sample the texture
	float4 reflTextureColor = reflectionTexture.Sample(Sampler,reflect(-toCamera1, input.normal));


	//calculate the projected refraction coordinates
	float2 refractionTexCoord;
	refractionTexCoord.x = input.worldPos.x / input.worldPos.w / 2.0 + 0.5f;
	refractionTexCoord.y = -input.worldPos.y / input.worldPos.w / 2.0f + 0.5f;

	// Re-position the texture coordinate sampling position by the normal map value to simulate the rippling wave effect.
	refractionTexCoord = refractionTexCoord + (normalFromMap.xy * 0.03);


	//Sample refraction texture
	float4 refractionTextureColor = refractionTexture.Sample(Sampler, refractionTexCoord);

	//Calculate the reflection vector from the camera
	float4 skyColor = Sky.Sample(Sampler, reflect(-toCamera1, input.normal));

	//Calculation of fresnal factor
	float r = (1.2f - 1.0f) / (1.2f + 1.0f);
	float fresnelFactor = max(0.0f, min(1.0f, r + (1.0f - r) * pow(1.0f - dot(normalFromMap,toCamera), 2)));

	//return lerp(reflTextureColor, skyColor, 0.6f);
	float4 color = lerp(reflTextureColor, refractionTextureColor,0.5);

	return color;
}
