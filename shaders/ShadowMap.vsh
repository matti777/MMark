precision highp float;

uniform mat4 mvp_matrix;

attribute vec3 in_coord;

void main()
{
   gl_Position = mvp_matrix * vec4(in_coord, 1.0);
}

