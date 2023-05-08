precision highp float;

uniform lowp sampler2D texture;
uniform lowp sampler2D texture2;
uniform lowp sampler2D shadow_texture;

varying mediump vec2 ex_texCoord;
varying mediump vec3 ex_normal;
varying mediump float ex_textureMix;
varying mediump vec4 ex_shadowCoord;
varying mediump float ex_fog;

// Amount of ambient light
const float ambient = 0.3;

// Position of the sun in world coordinates
const vec3 sunPos = normalize(vec3(-550.0, 470.0, -500.0));

// Constant to add all Z in shadow testing - adjust to match z-range.
// This will remove backface shadowing if needed.
const float ZFix = 0.0001;

// Diffuse lighting parameters
const float DiffuseScale = 0.8;
const float DiffuseAdd = 1.0 - DiffuseScale;

// Color of the distance fog
const vec3 FogColor = vec3(215.0 / 255.0, 227.0 / 255.0, 239.0 / 255.0);

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
    vec3 L = sunPos;

    // Calculate lighting based on the light dir
    float NdotL = max(0.0, dot(N, L));
    float diffuse = (DiffuseScale * NdotL) + DiffuseAdd;

    // Normal texture color
    vec4 texColor = texture2D(texture, ex_texCoord);

    // Texture2 color; present on cliff sides
    vec4 tex2Color = texture2D(texture2, ex_texCoord);

    // Mix them together
    vec3 color = mix(tex2Color.rgb, texColor.rgb, ex_textureMix);

    // Calculate shadowing. If ex_shadowCoord.w < 0.0, we're behing the
    // near plane of the light source frustum and thus will not apply any
    // shadow. If ex_shadowCoord.w > farclip, we're behind the far plane and
    // will not apply shadow either.
    float shadowStep = step(ex_shadowCoord.w, 0.0) + step(865.0, ex_shadowCoord.w);

    highp vec4 unitCoord = ex_shadowCoord / ex_shadowCoord.w;
    diffuse = max(diffuse * max(myShadowProj(unitCoord), shadowStep), 0.3);

    // Add some ambient light and apply diffuse lighting
    color = (color + ambient) * diffuse;

    // Apply distance fog
    color = mix(color, FogColor, ex_fog);

    gl_FragColor = vec4(color, texColor.a);
}
