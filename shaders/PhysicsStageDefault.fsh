precision highp float;

uniform lowp sampler2D texture;
uniform lowp sampler2D shadow_texture;

varying mediump vec2 ex_texCoord;
varying mediump vec3 ex_normal;
varying mediump vec3 ex_lightDir;
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
float myShadowProj(vec4 coord)
{
  highp float shadowDepth = texture2D(shadow_texture, coord.st).z + ZFix;
  return step(coord.z, shadowDepth);
}

void main(void)
{
    vec3 N = normalize(ex_normal);
    vec3 L = normalize(ex_lightDir);
    
    // Calculate diffuse lighting
    float NdotL = dot(N, L);
    float diffuse = (DiffuseScale * max(NdotL, 0.0)) + DiffuseAdd;

    // Calculate shadowing. If ex_shadowCoord.w < 0.0, we're behing the
    // light source frustum and thus will not apply any shadow.
    float step = step(ex_shadowCoord.w, 0.0);
    highp vec4 unitCoord = ex_shadowCoord / ex_shadowCoord.w;
    diffuse = max(diffuse * max(myShadowProj(unitCoord), step), 0.3);

    // Adjust the color by the diffuse and specular components
    vec4 texColor = texture2D(texture, ex_texCoord);
    vec3 color = texColor.rgb * diffuse;

    gl_FragColor =  vec4(color, texColor.a);
}

