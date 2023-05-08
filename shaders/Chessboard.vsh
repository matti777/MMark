precision highp float;

uniform mat4 mvp_matrix;
uniform mat4 mv_matrix;
uniform vec3 eye_pos; // Eye position (in object space)
uniform vec2 dof_params; // x = focal distance, y = focal range

attribute vec3 in_coord;
attribute vec2 in_texCoord;
attribute vec3 in_normal;

varying mediump vec2 ex_texCoord;
varying vec3 ex_reflect; // Reflection vector (in object space)
varying float ex_blur; // Blur factor in range [0, 1]

float computeBlur(float depth)
{
    return clamp((depth - dof_params.x) / dof_params.y, 0.0, 1.0);
}

void main(void)
{
    gl_Position = mvp_matrix * vec4(in_coord, 1.0);
    vec4 eyeSpaceCoord = mv_matrix * vec4(in_coord, 1.0);
    ex_blur = computeBlur(-eyeSpaceCoord.z);
    ex_texCoord = in_texCoord;
    vec3 ex_eyeDir = eye_pos - in_coord;
    ex_reflect = reflect(ex_eyeDir, in_normal);
}

