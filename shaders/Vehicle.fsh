precision highp float;

uniform lowp sampler2D texture;
uniform lowp sampler2D normalMap;
uniform lowp sampler2D shadow_texture;
uniform mediump float shininess;
uniform mediump vec3 specularColor;

varying mediump vec2 ex_texCoord;
varying mediump vec3 ex_lightDir; // In tangent space
varying mediump vec3 ex_eyeDir; // In tangent space
varying mediump vec4 ex_shadowCoord;

// Diffuse lighting parameters
const float DiffuseScale = 0.7;
const float DiffuseAdd = 1.0 - DiffuseScale;

// Amount of ambient light
const float AmbientLight = 0.3;

// Constant to add all Z in shadow testing - adjust to match z-range.
// This will remove backface shadowing if needed.
const float ZFix = 0.01;

// This function performs a lookup to the shadow texture and compares the
// depth of the pixel (in the light's space) to it. Returns 0.0 if the
// current pixel is in shadow, or 1.0 if it is not.
float myShadowProj(vec4 shadowCoord)
{
    // If shadowCoord.w < 0.0, we're behing the
    // light source frustum and thus will not apply any shadow.
    float shadowStep = step(shadowCoord.w, 0.0);

    // Perspective project into light space unit coordinates
    highp vec4 unitCoord = shadowCoord / shadowCoord.w;

    // Read the corresponding stored shadow depth from the shadow texture
    highp float shadowDepth = texture2D(shadow_texture, unitCoord.st).z + ZFix;

    // Perform the depth comparison to determine whether in shadow or not
    shadowStep = max(shadowStep, step(unitCoord.z, shadowDepth));

    return shadowStep;
}

void main(void)
{
    vec3 E = normalize(ex_eyeDir);
    vec3 L = normalize(ex_lightDir);

    // Read the normal from the normal map and transform components
    // from [0,1] to [-1,1]
    vec3 N = normalize(texture2D(normalMap, ex_texCoord).xyz * 2.0 - 1.0);

    // Calculate diffuse lighting
    float NdotL = max(0.0, dot(N, L));
    float diffuse = (DiffuseScale * NdotL) + DiffuseAdd;

    // Calculate specular lighting
    vec3 R = reflect(-L, N);
    float RdotE = max(0.0, dot(R, E));
    float specular = pow(RdotE, shininess);

    // Apply shadowing
    float shadowStep = myShadowProj(ex_shadowCoord);
    diffuse = max((diffuse * shadowStep), AmbientLight);
    specular = (specular * shadowStep);

    // Adjust the color by the diffuse and specular components
    vec4 texColor = texture2D(texture, ex_texCoord);
    vec3 color = (texColor.rgb * diffuse) + (specularColor * specular);

    gl_FragColor = vec4(color, texColor.a);
}
