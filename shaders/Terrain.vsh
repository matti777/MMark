precision highp float;

uniform mediump mat4 mvp_matrix;
uniform mediump mat4 mv_matrix;
uniform mediump mat4 shadow_matrix;

attribute vec3 in_coord;
attribute vec2 in_texCoord;
attribute vec3 in_normal;

varying mediump vec2 ex_texCoord;
varying mediump vec3 ex_normal;
varying mediump float ex_textureMix;
varying mediump vec4 ex_shadowCoord;
varying mediump float ex_fog;

// Distance fog parameters
const float DistanceFogStart = 150.0;
const float DistanceFogDensity = 0.01;

float calculateFog(float distance)
{
    // introduce a fog-free zone
    distance = max(0.0, distance - DistanceFogStart);
    return 1.0 - clamp(exp(-DistanceFogDensity * distance), 0.0, 1.0);
}

void main(void)
{
    gl_Position = mvp_matrix * vec4(in_coord, 1.0);
    ex_texCoord = in_texCoord;
    ex_normal = in_normal;
    ex_shadowCoord = shadow_matrix * vec4(in_coord, 1.0);

    // Calculate fog at this vertex
    vec4 eyeSpaceCoord = mv_matrix * vec4(in_coord, 1.0);
    ex_fog = calculateFog(length(eyeSpaceCoord));

    // Calculate the mixing ratio between texture & texture2 from the
    // y component of the normal
    mediump float y = 1.0 - abs(in_normal.y);
    ex_textureMix = smoothstep(0.3, 0.31, y);
}

