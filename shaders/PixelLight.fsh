precision highp float;

uniform lowp sampler2D texture;

varying mediump vec2 ex_texCoord;
varying mediump vec3 ex_normal;
varying mediump vec3 ex_lightDir;

void main(void)
{
    vec3 L = normalize(ex_lightDir);
    vec3 N = normalize(ex_normal);

    // Calculate diffuse lighting
    float NdotL = max(0.0, dot(N, L)) * 0.8 + 0.2;

    vec4 color = texture2D(texture, ex_texCoord);
    gl_FragColor = vec4(color.rgb * NdotL, color.a);
}
