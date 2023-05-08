precision highp float;

uniform lowp sampler2D unblurred_texture;
uniform lowp sampler2D blurred_texture;

varying mediump vec2 ex_texCoord;

void main(void)
{
    vec4 unblurredColor = texture2D(unblurred_texture, ex_texCoord);
    vec4 blurredColor = texture2D(blurred_texture, ex_texCoord);

    vec3 color = mix(unblurredColor.rgb, blurredColor.rgb, unblurredColor.a);

    gl_FragColor = vec4(color, 1.0);
}
