precision highp float;

uniform lowp sampler2D texture;

varying mediump vec2 ex_texCoord;
varying mediump float ex_lighting;

void main(void)
{
    vec4 color = texture2D(texture, ex_texCoord);
    gl_FragColor = vec4(color.rgb * ex_lighting, color.a);
}
