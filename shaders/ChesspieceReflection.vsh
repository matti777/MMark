precision highp float;

uniform mat4 mvp_matrix;
uniform vec3 eye_pos; // Eye position (in object space)

attribute vec3 in_coord;
attribute vec2 in_texCoord;
attribute vec3 in_normal;

varying mediump vec2 ex_texCoord;
varying vec3 ex_normal; // Surface normal (in object space)
varying vec3 ex_eyeDir; // Direction to the eye

void main(void)
{
    gl_Position = mvp_matrix * vec4(in_coord, 1.0);
    ex_texCoord = in_texCoord;
    ex_normal = in_normal;
    ex_eyeDir = eye_pos - in_coord;
}

