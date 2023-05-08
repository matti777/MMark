precision highp float;

uniform mediump float sample_height;

attribute vec3 in_coord;

varying mediump vec2 ex_texCoordU3;
varying mediump vec2 ex_texCoordU2;
varying mediump vec2 ex_texCoordU1;
varying mediump vec2 ex_texCoord0;
varying mediump vec2 ex_texCoordD1;
varying mediump vec2 ex_texCoordD2;
varying mediump vec2 ex_texCoordD3;

void main(void)
{
    // We are passed coordinates already in the unit cube and we want them
    // to remain identity
    gl_Position = vec4(in_coord, 1.0);

    // We'll also use the in_coord to calculate the texture coordinates by
    // simply making transform from unit cube [-1, 1] to [0, 1]
    ex_texCoord0 = in_coord.xy * 0.5 + 0.5;

    // Set up the blur texture lookup coords; 3 pixels up, 3 down
    ex_texCoordU1 = vec2(ex_texCoord0.x, ex_texCoord0.y - sample_height);
    ex_texCoordU2 = vec2(ex_texCoord0.x, ex_texCoord0.y - sample_height*2.0);
    ex_texCoordU3 = vec2(ex_texCoord0.x, ex_texCoord0.y - sample_height*3.0);
    ex_texCoordD1 = vec2(ex_texCoord0.x, ex_texCoord0.y + sample_height);
    ex_texCoordD2 = vec2(ex_texCoord0.x, ex_texCoord0.y + sample_height*2.0);
    ex_texCoordD3 = vec2(ex_texCoord0.x, ex_texCoord0.y + sample_height*3.0);
}
