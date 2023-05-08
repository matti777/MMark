precision highp float;

uniform mat4 mvp_matrix;

attribute vec3 in_coord;
attribute vec2 in_texCoord;

varying mediump vec2 ex_texCoord;

void main()
{
   gl_Position = mvp_matrix * vec4(in_coord, 1.0);
   ex_texCoord = in_texCoord;
}
