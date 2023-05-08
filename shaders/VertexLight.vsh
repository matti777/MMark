precision highp float;

uniform mat4 mvp_matrix;
uniform highp vec3 light_pos; // In object space

attribute vec3 in_coord;
attribute vec3 in_normal;
attribute vec2 in_texCoord;

varying mediump vec2 ex_texCoord;
varying mediump float ex_lighting;

void main(void)
{
    gl_Position = mvp_matrix * vec4(in_coord, 1.0);
    ex_texCoord = in_texCoord;
    
    vec3 N = normalize(in_normal);
    
    // Calculate the direction to the light from the current vertex
    vec3 L = normalize(light_pos - in_coord);
    
    // Calculate the Lambert term for lighting at this vertex
    ex_lighting = max(0.0, dot(L, N)) * 0.7 + 0.3;
}

