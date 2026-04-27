#version 440 core

in vec3 fragmentPosition;
in vec3 fragmentVertexNormal;
in vec2 fragmentTextureCoordinate;

out vec4 outFragmentColor;

uniform bool bUseTexture = false;
uniform bool bUseLighting = false;
uniform vec4 objectColor = vec4(1.0f);
uniform sampler2D objectTexture;
uniform vec2 UVscale = vec2(1.0f, 1.0f);

// View position for specular calculations
uniform vec3 viewPosition;

// Light source 1 uniforms - main overhead light
uniform vec3 lightPos1;
uniform vec3 lightColor1;

// Light source 2 uniforms - colored fill light
uniform vec3 lightPos2;
uniform vec3 lightColor2;

// Per-object material properties set from SceneManager
uniform float materialAmbientStrength = 0.3;
uniform float materialSpecularStrength = 0.5;
uniform float materialShininess = 32.0;

// Calculates Phong lighting contribution from one light source.
// Combines ambient + diffuse + specular components.
vec3 CalcLight(vec3 lightPosition, vec3 lightClr, vec3 normal, vec3 viewDir, vec3 baseClr)
{
    // Ambient - prevents surfaces from being completely black
    vec3 ambient = materialAmbientStrength * lightClr;

    // Diffuse - brightness based on surface angle relative to light
    vec3 lightDir = normalize(lightPosition - fragmentPosition);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightClr;

    // Specular - bright reflection highlights on shiny surfaces
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    vec3 specular = materialSpecularStrength * spec * lightClr;

    return (ambient + diffuse + specular) * baseClr;
}

void main()
{
    // 1. Determine Base Color (Texture or Solid Color)
    vec4 baseColor;
    if(bUseTexture)
    {
        baseColor = texture(objectTexture, fragmentTextureCoordinate * UVscale);
    }
    else
    {
        baseColor = objectColor;
    }

    // 2. Apply Lighting if enabled
    if(bUseLighting)
    {
        vec3 norm = normalize(fragmentVertexNormal);
        vec3 viewDir = normalize(viewPosition - fragmentPosition);

        // Sum contributions from both light sources
        vec3 result1 = CalcLight(lightPos1, lightColor1, norm, viewDir, baseColor.rgb);
        vec3 result2 = CalcLight(lightPos2, lightColor2, norm, viewDir, baseColor.rgb);

        // Clamp to prevent over-bright blown-out areas
        vec3 finalColor = min(result1 + result2, vec3(1.0));
        outFragmentColor = vec4(finalColor, baseColor.a);
    }
    else
    {
        outFragmentColor = baseColor;
    }
}
