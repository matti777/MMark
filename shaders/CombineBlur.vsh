precision highp float;

attribute vec3 in_coord;

varying mediump vec2 ex_texCoord;

void main(void)
{
    // We are passed coordinates already in the unit cube and we want them
    // to remain identity
    gl_Position = vec4(in_coord, 1.0);

    // We'll also use the in_coord to calculate the texture coordinates by
    // simply making transform from unit cube [-1, 1] to [0, 1]
    ex_texCoord = in_coord.xy * 0.5 + 0.5;
}

