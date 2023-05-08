precision highp float;

uniform mat4 mvp_matrix;

attribute vec3 in_coord;

varying mediump vec3 ex_cubeTexCoord;

void main(void)
{
    gl_Position = mvp_matrix * vec4(in_coord, 1.0);
    ex_cubeTexCoord = -in_coord;
}

