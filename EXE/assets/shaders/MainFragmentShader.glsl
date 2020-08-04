#version 420
#define BlendMultiply(base, blend) 		(base * blend)
#define BlendAverage(base, blend) 		((base + blend) / 2.0)
#define BlendSoftLightf(base, blend) 	((blend < 0.5) ? (2.0 * base * blend + base * base * (1.0 - 2.0 * blend)) : (sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend)))

in vec4 fColour;	
in vec4 fVertWorldLocation;
in vec4 fNormal;
in vec4 fUVx2;
in mat3 TBN;
in vec4 FragPosLightSpace;

uniform vec4 eyeLocation;
uniform vec4 diffuseColour;				
uniform vec4 specularColour;
uniform vec4 wireframeColour;		// Used to draw debug (or unlit) objects

uniform bool bWireframeMode;
uniform bool bIsSkyBox;
uniform bool bIsImposter;
uniform bool bNormalPassMode;
uniform bool bAlbedoPassMode;
uniform bool bUseTransparentMap;
uniform bool bNoTexture;
uniform bool bBaseTexture;
uniform bool bDiffuseColourBlend;
uniform bool bColourBlend;
uniform bool bMultiplyBlend;
uniform bool bAverageBlend;
uniform bool bSoftlightBlend;
uniform bool bNoLighting;
uniform bool bFullscreenQuadPass;
uniform bool bIsChrome;

// Texture samplers
uniform sampler2D textSamp00;
uniform sampler2D textSamp01;
uniform sampler2D textSamp02;
uniform sampler2D textSamp03;
uniform samplerCube skyBox;
uniform sampler2D secondPassColourTexture;
uniform sampler2D fullscreenQuadTexture;
uniform sampler2D shadowMap;


uniform vec4 tex_0_3_tile;		// x = 0, y = 1, z = 2, w = 3, Repeat texture

uniform vec2 groundOffset;
uniform float instanceTransparency[150];		// For particles instancing
flat in uint instanceID;

uniform int passNumber;

layout (location = 0) out vec4 pixelColour;			// RGB A   (0 to 1) 
layout (location = 1) out vec4 BrightColour;


struct sLight
{
	vec4 position;			
	vec4 diffuse;	
	vec4 specular;	// rgb = highlight colour, w = power
	vec4 ambient;
	vec4 atten;		// x = constant, y = linear, z = quadratic, w = DistanceCutOff
	vec4 direction;	// Spot, directional lights
	vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
	                // 0 = pointlight
					// 1 = spot light
					// 2 = directional light
	vec4 param2;	// x = 0 for off, 1 for on
};

const int POINT_LIGHT_TYPE = 0;
const int SPOT_LIGHT_TYPE = 1;
const int DIRECTIONAL_LIGHT_TYPE = 2;

const int NUMBEROFLIGHTS = 1;		// REMEMBER TO CHANGE THIS VALUE WHILE MODIFYING LIGHT CONFIG
uniform sLight theLights[NUMBEROFLIGHTS];  	// 80 uniforms

const float	brightThreshold = 0.6;

vec4 calcualteLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, vec3 vertexWorldPos, vec4 vertexSpecular );

vec3 RGBToHSL(vec3 color);
vec3 HSLToRGB(vec3 hsl);
float HueToRGB(float f1, float f2, float hue);
vec3 BlendColor(vec3 base, vec3 blend);

const float offset = 1.0 / 500.0;

vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
		);

		float blurKernel[9] = float[](
			1.0 / 16, 2.0 / 16, 1.0 / 16,
			2.0 / 16, 4.0 / 16, 2.0 / 16,
			1.0 / 16, 2.0 / 16, 1.0 / 16  
		);

		float edgeDetectionKernel[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
		);

void main()  
{

	if(bFullscreenQuadPass)
	{
		int screenWidth = 1920;
		int screenHeight = 1080; 
		vec2 textCoords = vec2( gl_FragCoord.x / float(screenWidth), gl_FragCoord.y / float(screenHeight) );
		vec3 fullscreenCol = texture(fullscreenQuadTexture, textCoords.st).rgb;
		//vec3 fullscreenCol = texture(fullscreenQuadTexture, fUVx2.st).rgb;
		vec3 sampleTex[9];
		for(int i = 0; i < 9; i++)
		{
			//sampleTex[i] = vec3(texture(fullscreenQuadTexture, fUVx2.st + offsets[i]));
			sampleTex[i] = vec3(texture(fullscreenQuadTexture, textCoords.st + offsets[i]));
		}
		vec3 col = vec3(0.0);
		for(int i = 0; i < 9; i++)
			col += sampleTex[i] * edgeDetectionKernel[i];		
		
		vec3 invCol = 1.0 - col;		
		vec3 multiplyCol = BlendMultiply( fullscreenCol, invCol );		
		vec3 vignette = texture( textSamp00, fUVx2.st * tex_0_3_tile.x ).rgb;
		multiplyCol = BlendMultiply( multiplyCol, vignette );
		pixelColour = vec4(multiplyCol, 1.0);

		return;
	}

	if ( passNumber == 1 )
	{
		vec2 textCoords = vec2( 1.0f - fUVx2.s, fUVx2.t );
		vec3 sampleTex[9];
		for(int i = 0; i < 9; i++)
		{
			sampleTex[i] = vec3(texture(secondPassColourTexture, textCoords.st + offsets[i]));			
		}
		vec3 col = vec3(0.0);
		for(int i = 0; i < 9; i++)
			col += sampleTex[i] * blurKernel[i];

		vec3 yellowPaper = texture( textSamp00, fUVx2.st * tex_0_3_tile.x ).rgb;
		vec3 outCol = BlendMultiply( col, yellowPaper );
		vec3 greenScreen = texture( textSamp01, fUVx2.st * tex_0_3_tile.x ).rgb;
		outCol = BlendAverage( outCol, greenScreen );
		pixelColour = vec4(outCol, 1.0);		

		return;
	}

	if ( bWireframeMode )
	{
		pixelColour.rgb = wireframeColour.rgb;
		pixelColour.a = 1.0f;				// NOT transparent
		return;
	}
	
	if ( bNormalPassMode )
	{
		pixelColour.rgb = fNormal.xyz;
		pixelColour.a = 1.0f;				// NOT transparent
		return;
	}
	
	if ( bIsSkyBox )
	{
		// sample the skybox using the normal from the surface
		vec3 skyColour = texture( skyBox, fNormal.xyz ).rgb;
		pixelColour.rgb = skyColour.rgb * 1.5f;
		pixelColour.a = 1.0f;				// NOT transparent		
		return;
	}

	if ( bAlbedoPassMode )
	{
		vec3 texAlbedo_RGB = texture( textSamp00, fUVx2.st * tex_0_3_tile.x ).rgb;
		pixelColour.rgb = texAlbedo_RGB;
		pixelColour.a = 1.0f;				// NOT transparent
		return;
	}	

	if ( bNoTexture )
	{	
		vec4 outColour = calcualteLightContrib( diffuseColour.rgb, fNormal.xyz, fVertWorldLocation.xyz, specularColour );
		pixelColour = outColour;
		pixelColour.a = diffuseColour.a;

		float brightness = dot(outColour.rgb, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > brightThreshold)
			BrightColour = outColour;
		else
			BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
		
		return;
	}

	if ( bIsImposter )
	{
		vec3 texRGB = texture( textSamp00, fUVx2.st ).rgb;
		float grey = (texRGB.r + texRGB.g + texRGB.b)/3.0f;
		if ( grey < 0.05 ) 	{	discard;	}

		pixelColour.rgb = texRGB.rgb;		
		//pixelColour.a = diffuseColour.a;		
		pixelColour.a = instanceTransparency[instanceID];

		float brightness = dot(pixelColour.rgb, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > brightThreshold)
			BrightColour = pixelColour;			
		else
			BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
			BrightColour.a = pixelColour.a;

		return;
	}	

	// The 3rd texture as cutout map
	vec3 tex2_RGB = texture( textSamp02, fUVx2.st * tex_0_3_tile.z).rgb;
	float tex2_grey = (tex2_RGB.r + tex2_RGB.g + tex2_RGB.b)/3.0f;
	if ( tex2_grey < 0.1f )	{	discard;	}		// Drop black pixel

	// The 1st texture as base Albedo
	vec3 tex0_RGB = texture( textSamp00, (fUVx2.st + groundOffset) * tex_0_3_tile.x ).rgb;

	// The 2nd texture as noraml map or blend map	
	vec3 tex1_RGB = texture( textSamp01, (fUVx2.st + groundOffset) * tex_0_3_tile.y).rgb;
	
	vec3 normalForLight;
	vec3 texRGB;
	
	if ( bBaseTexture )
	{
		texRGB = tex0_RGB;
		tex1_RGB = tex1_RGB * 2.0 - 1.0;		  // this normal is in tangent space
		normalForLight = TBN * tex1_RGB;		
	}

	if ( bDiffuseColourBlend )
	{
		texRGB = BlendColor( tex0_RGB, diffuseColour.rgb );
		normalForLight = fNormal.xyz;
	}	

	if ( bColourBlend )
	{
		texRGB = BlendColor( tex0_RGB, tex1_RGB );
		normalForLight = fNormal.xyz;
	}
	
	if ( bMultiplyBlend )
	{
		texRGB = BlendMultiply( tex0_RGB, tex1_RGB );
		normalForLight = fNormal.xyz;
	}
	
	if ( bAverageBlend )
	{
		texRGB = BlendAverage( tex0_RGB, tex1_RGB );
		normalForLight = fNormal.xyz;
	}
	
	if ( bSoftlightBlend )
	{
		texRGB.r = BlendSoftLightf( tex0_RGB.r, tex1_RGB.r );
		texRGB.g = BlendSoftLightf( tex0_RGB.g, tex1_RGB.g );
		texRGB.b = BlendSoftLightf( tex0_RGB.b, tex1_RGB.b );
		normalForLight = fNormal.xyz;
	}
	
	if(bIsChrome)
	{
		vec3 eyeVector = normalize(fVertWorldLocation.xyz - eyeLocation.xyz);
		vec3 reflectVector = reflect(eyeVector, normalize(fNormal.xyz));
		vec3 reflectColour = texture( skyBox, reflectVector.xyz ).rgb;

		float ratio = 1.0f / 1.52f;
		vec3 refractVector = refract( eyeVector, fNormal.xyz, ratio );
		vec3 refractColour = texture( skyBox, refractVector.xyz ).rgb;

		texRGB = BlendAverage(reflectColour, refractColour) * 2.0f;

		normalForLight = fNormal.xyz;
	}

	// Lighting computation	
	vec4 outColour = calcualteLightContrib( texRGB.rgb, normalForLight, fVertWorldLocation.xyz, specularColour );

	if ( bNoLighting )
	{
		pixelColour.rgb = texRGB.rgb;

		float brightness = dot(texRGB.rgb, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > brightThreshold)
			BrightColour = outColour;
		else
			BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
			BrightColour.a = pixelColour.a;
	}
	else
	{
		pixelColour = outColour;

		float brightness = dot(outColour.rgb, vec3(0.2126, 0.7152, 0.0722));
		if(brightness > brightThreshold)
			BrightColour = outColour;
		else
			BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
			BrightColour.a = pixelColour.a;
	}	

	// The 4th texture as transparent map
	if ( bUseTransparentMap )
	{
		vec3 tex3_RGB = texture( textSamp03, fUVx2.st * tex_0_3_tile.w).rgb;
		float tex3_grey = (tex3_RGB.r + tex3_RGB.g + tex3_RGB.b)/3.0f;
		pixelColour.a = tex3_grey;
	}
	else
	{
		pixelColour.a = diffuseColour.a;
	}
	
	

}

float ShadowCalculation(float dotLightNormal)
{
	// perform perspective divide
    //vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    vec3 projCoords = FragPosLightSpace.xyz * 0.5 + 0.5;
	if(projCoords.z > 1.0)	projCoords.z = 1.0;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    //float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float bias = max(0.05 * (1.0 - dotLightNormal), 0.001);  

    //float shadow = projCoords.z > closestDepth + bias ? 1.0 : 0.0;
	// percentage-closer filtering
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += projCoords.z > pcfDepth + bias ? 1.0 : 0.0;        
        }    
    }
    
    return shadow /= 9.0;    
}

vec4 calcualteLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, vec3 vertexWorldPos, vec4 vertexSpecular )
{
	vec3 norm = normalize(vertexNormal);
	
	vec4 finalObjectColour = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	
	for ( int index = 0; index < NUMBEROFLIGHTS; index++ )
	{		
		if ( theLights[index].param2.x == 0.0f )		continue;		// it's off
		
		// Cast to an int (note with c'tor)
		int intLightType = int(theLights[index].param1.x);		

		// Ambient
		vec3 ambient = theLights[index].ambient.rgb;

		vec3 toEyeVector = normalize(eyeLocation.xyz - vertexWorldPos.xyz);
		//vec3 halfwayDir = normalize(-theLights[index].direction.xyz + toEyeVector);
		if ( intLightType == DIRECTIONAL_LIGHT_TYPE )		// = 2
		{	
			// Diffuse
			float diff = max( 0.0f, dot( -theLights[index].direction.xyz, norm ) );		// 0 to 1	
			vec3 diffuse = diff * theLights[index].diffuse.rgb;

			// Specular 			
			//vec3 reflectDir = reflect( theLights[index].direction.xyz, norm );			
			//vec3 toEyeVector = normalize(eyeLocation.xyz - vertexWorldPos.xyz);
			vec3 halfwayDir = normalize(-theLights[index].direction.xyz + toEyeVector);
			//vec3 lightSpecularContrib = pow( max(0.0f, dot(toEyeVector, reflectDir)), theLights[index].specular.w )
			vec3 specular = pow( max(0.0f, dot(norm, halfwayDir)), theLights[index].specular.w )
			                   * theLights[index].specular.rgb;
			
			float shadow = ShadowCalculation(diff);

			finalObjectColour.rgb += ((vertexMaterialColour * diffuse) + (vertexSpecular.rgb  * specular )) * (1.0 - shadow)
									+ (vertexMaterialColour * ambient);
			continue;
		}
		
		// Assume it's a point light, intLightType = 0
		// Diffuse
		vec3 lightDir = vertexWorldPos.xyz - theLights[index].position.xyz;
		float lightDist = length(lightDir);		
		float diff = max( 0.0f, dot(-normalize(lightDir), norm) );		
		vec3 diffuse = diff * theLights[index].diffuse.rgb;

		// Specular		
		//vec3 reflectDir = reflect( normalize(lightDir), norm );		
		//vec3 toEyeVector = normalize(eyeLocation.xyz - vertexWorldPos.xyz);
		vec3 halfwayDir = normalize(-normalize(lightDir) + toEyeVector);
		vec3 specular = pow( max(0.0f, dot(norm, halfwayDir) ), theLights[index].specular.w )
			                   * theLights[index].specular.rgb;
		
		// Attenuation
		float attenuation = 1.0f / 
				( theLights[index].atten.x + 										
				  theLights[index].atten.y * lightDist +						
				  theLights[index].atten.z * lightDist*lightDist );  	
				  
		// total light contribution
		diffuse *= attenuation;
		specular *= attenuation;
		ambient *= attenuation;		
		
		if ( intLightType == SPOT_LIGHT_TYPE )
		{					
			float currentLightRayAngle = max(0.0f, dot( normalize(lightDir), theLights[index].direction.xyz ));			
			float outerConeAngleCos = cos(radians(theLights[index].param1.z));
			float innerConeAngleCos = cos(radians(theLights[index].param1.y));							
			
			if ( currentLightRayAngle < outerConeAngleCos )		// Outside of the spot
			{
				diffuse = vec3(0.0f, 0.0f, 0.0f);
				specular = vec3(0.0f, 0.0f, 0.0f);
			}
			else if ( currentLightRayAngle < innerConeAngleCos )	// Angle is between the inner and outer cone				
			{				
				// This blends the brightness from full brightness (near the inner cone) to black (near the outter cone)
				float penumbraRatio = (currentLightRayAngle - outerConeAngleCos) / 
									  (innerConeAngleCos - outerConeAngleCos);									  
				diffuse *= penumbraRatio;
				specular *= penumbraRatio;
			}
						
		}// if ( intLightType == 1 )		
					
		finalObjectColour.rgb += (vertexMaterialColour.rgb * diffuse.rgb)
								  + (vertexSpecular.rgb  * specular.rgb )
								  + (vertexMaterialColour.rgb * ambient);
	}//for(intindex=0...
	
	finalObjectColour.a = 1.0f;	
	return finalObjectColour;
}


// Color Mode keeps the brightness of the base color and applies both the hue and saturation of the blend color.
vec3 BlendColor(vec3 base, vec3 blend)
{
	vec3 blendHSL = RGBToHSL(blend);
	return HSLToRGB(vec3(blendHSL.r, blendHSL.g, RGBToHSL(base).b));
}

vec3 RGBToHSL(vec3 color)
{
	vec3 hsl; // init to 0 to avoid warnings ? (and reverse if + remove first part)
	
	float fmin = min(min(color.r, color.g), color.b);    //Min. value of RGB
	float fmax = max(max(color.r, color.g), color.b);    //Max. value of RGB
	float delta = fmax - fmin;             //Delta RGB value

	hsl.z = (fmax + fmin) / 2.0; // Luminance

	if (delta == 0.0)		//This is a gray, no chroma...
	{
		hsl.x = 0.0;	// Hue
		hsl.y = 0.0;	// Saturation
	}
	else                                    //Chromatic data...
	{
		if (hsl.z < 0.5)
			hsl.y = delta / (fmax + fmin); // Saturation
		else
			hsl.y = delta / (2.0 - fmax - fmin); // Saturation
		
		float deltaR = (((fmax - color.r) / 6.0) + (delta / 2.0)) / delta;
		float deltaG = (((fmax - color.g) / 6.0) + (delta / 2.0)) / delta;
		float deltaB = (((fmax - color.b) / 6.0) + (delta / 2.0)) / delta;

		if (color.r == fmax )
			hsl.x = deltaB - deltaG; // Hue
		else if (color.g == fmax)
			hsl.x = (1.0 / 3.0) + deltaR - deltaB; // Hue
		else if (color.b == fmax)
			hsl.x = (2.0 / 3.0) + deltaG - deltaR; // Hue

		if (hsl.x < 0.0)
			hsl.x += 1.0; // Hue
		else if (hsl.x > 1.0)
			hsl.x -= 1.0; // Hue
	}

	return hsl;
}

vec3 HSLToRGB(vec3 hsl)
{
	vec3 rgb;
	
	if (hsl.y == 0.0)
		rgb = vec3(hsl.z); // Luminance
	else
	{
		float f2;
		
		if (hsl.z < 0.5)
			f2 = hsl.z * (1.0 + hsl.y);
		else
			f2 = (hsl.z + hsl.y) - (hsl.y * hsl.z);
			
		float f1 = 2.0 * hsl.z - f2;
		
		rgb.r = HueToRGB(f1, f2, hsl.x + (1.0/3.0));
		rgb.g = HueToRGB(f1, f2, hsl.x);
		rgb.b= HueToRGB(f1, f2, hsl.x - (1.0/3.0));
	}
	
	return rgb;
}

float HueToRGB(float f1, float f2, float hue)
{
	if (hue < 0.0)
		hue += 1.0;
	else if (hue > 1.0)
		hue -= 1.0;
	float res;
	if ((6.0 * hue) < 1.0)
		res = f1 + (f2 - f1) * 6.0 * hue;
	else if ((2.0 * hue) < 1.0)
		res = f2;
	else if ((3.0 * hue) < 2.0)
		res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
	else
		res = f1;
	return res;
}
