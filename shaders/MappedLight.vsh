precision highp float;

uniform mat4 mvp_matrix;
uniform highp vec3 light_pos; // In object space

attribute vec3 in_coord;
attribute vec3 in_normal;
attribute vec2 in_texCoord;

varying mediump vec2 ex_texCoord;
varying mediump vec2 ex_normalmap_texCoord;
varying mediump vec2 ex_specmap_texCoord;
varying mediump vec3 ex_lightDir;

void main(void)
{
    gl_Position = mvp_matrix * vec4(in_coord, 1.0);
    ex_texCoord = in_texCoord;

    // normal/spec maps are side by side in their texture; adjust texture
    // coordinates for both
    ex_normalmap_texCoord = vec2(ex_texCoord.s * 0.5, ex_texCoord.t);
    ex_specmap_texCoord = vec2(ex_texCoord.s * 0.5 + 0.5, ex_texCoord.t);

    // Calculate the direction to the light from the current vertex
    ex_lightDir = normalize(light_pos - in_coord);
}

