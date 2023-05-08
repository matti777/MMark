precision highp float;

uniform mediump float sample_width;

attribute vec3 in_coord;

varying mediump vec2 ex_texCoordL3;
varying mediump vec2 ex_texCoordL2;
varying mediump vec2 ex_texCoordL1;
varying mediump vec2 ex_texCoord0;
varying mediump vec2 ex_texCoordR1;
varying mediump vec2 ex_texCoordR2;
varying mediump vec2 ex_texCoordR3;

void main(void)
{
    // We are passed coordinates already in the unit cube and we want them
    // to remain identity
    gl_Position = vec4(in_coord, 1.0);

    // We'll also use the in_coord to calculate the texture coordinates by
    // simply making transform from unit cube [-1, 1] to [0, 1]
    ex_texCoord0 = in_coord.xy * 0.5 + 0.5;

    // Set up the blur texture lookup coords; 3 pixels to left, 3 to the right
    ex_texCoordL1 = vec2(ex_texCoord0.x - sample_width, ex_texCoord0.y);
    ex_texCoordL2 = vec2(ex_texCoord0.x - sample_width*2.0, ex_texCoord0.y);
    ex_texCoordL3 = vec2(ex_texCoord0.x - sample_width*3.0, ex_texCoord0.y);
    ex_texCoordR1 = vec2(ex_texCoord0.x + sample_width, ex_texCoord0.y);
    ex_texCoordR2 = vec2(ex_texCoord0.x + sample_width*2.0, ex_texCoord0.y);
    ex_texCoordR3 = vec2(ex_texCoord0.x + sample_width*3.0, ex_texCoord0.y);
}

