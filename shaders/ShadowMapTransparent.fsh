precision highp float;

uniform lowp sampler2D texture;

varying mediump vec2 ex_texCoord;

// Threshold for alpha; if pixel alpha < this, discard z-write
const float AlphaThreshold = 0.01;

void main()
{
    // If current pixel's alpha is less than threshold, skip writing depth
    vec4 texColor = texture2D(texture, ex_texCoord);
    if ( texColor.a < AlphaThreshold )
    {
        discard;
    }
}
