precision highp float;

uniform lowp sampler2D texture;
uniform mediump vec3 highlight;

varying mediump vec2 ex_texCoord;

void main(void)
{
    vec4 texColor = texture2D(texture, ex_texCoord);
    vec3 adjustedColor = texColor.rgb + highlight;
    gl_FragColor = vec4(adjustedColor, texColor.a);
}
