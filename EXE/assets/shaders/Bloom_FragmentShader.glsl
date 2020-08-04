#version 420

out vec4 FragColour;

in vec2 fUV;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom = true;
uniform float exposure = 1.0;

void main()
{  
    const float gamma = 2.2;
    vec3 hdrColour = texture(scene, fUV).rgb;    
    vec3 bloomColour = texture(bloomBlur, fUV).rgb;
    
    if(bloom)	hdrColour += bloomColour;

    // Tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColour * exposure);

    // Gamma correct    
    result = pow(result, vec3(1.0 / gamma));
    
    FragColour = vec4(result, 1.0);    
    
}