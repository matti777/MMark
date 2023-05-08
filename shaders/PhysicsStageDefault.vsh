precision highp float;

uniform mat4 mvp_matrix;
uniform highp vec3 light_pos; // In object space
uniform mediump mat4 shadow_matrix;

attribute vec3 in_coord;
attribute vec2 in_texCoord;
attribute vec3 in_normal;

varying mediump vec2 ex_texCoord;
varying mediump vec3 ex_normal;
varying mediump vec3 ex_lightDir;
varying mediump vec4 ex_shadowCoord;

void main(void)
{
    gl_Position = mvp_matrix * vec4(in_coord, 1.0);
    ex_texCoord = in_texCoord;
    ex_normal = in_normal;
    ex_shadowCoord = shadow_matrix * vec4(in_coord, 1.0);

    // Calculate the direction to the light from the current vertex
    ex_lightDir = normalize(light_pos - in_coord);
}

