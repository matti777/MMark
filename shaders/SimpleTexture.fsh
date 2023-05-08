precision highp float;

uniform lowp sampler2D texture;

varying mediump vec2 ex_texCoord;

void main(void)
{
    gl_FragColor = texture2D(texture, ex_texCoord);
}
