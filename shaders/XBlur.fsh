precision highp float;

uniform lowp sampler2D texture;

varying mediump vec2 ex_texCoordL3;
varying mediump vec2 ex_texCoordL2;
varying mediump vec2 ex_texCoordL1;
varying mediump vec2 ex_texCoord0;
varying mediump vec2 ex_texCoordR1;
varying mediump vec2 ex_texCoordR2;
varying mediump vec2 ex_texCoordR3;

// Constant for 1/7 to average the 7 separate pixel colors
const float Avg = 1.0 / 7.0;

void main(void)
{
    vec3 c0 = texture2D(texture, ex_texCoordL3).rgb;
    vec3 c1 = texture2D(texture, ex_texCoordL2).rgb;
    vec3 c2 = texture2D(texture, ex_texCoordL1).rgb;
    vec3 c3 = texture2D(texture, ex_texCoord0).rgb;
    vec3 c4 = texture2D(texture, ex_texCoordR1).rgb;
    vec3 c5 = texture2D(texture, ex_texCoordR2).rgb;
    vec3 c6 = texture2D(texture, ex_texCoordR3).rgb;
    vec3 color = (c0 + c1 + c2 + c3 + c4 + c5 + c6) * Avg;

    gl_FragColor = vec4(color, 1.0);
}

