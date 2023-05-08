precision highp float;

uniform lowp sampler2D texture;
uniform lowp sampler2D normalspec_texture;

varying mediump vec2 ex_texCoord;
varying mediump vec2 ex_normalmap_texCoord;
varying mediump vec2 ex_specmap_texCoord;
varying mediump vec3 ex_lightDir;

// Constant eye direction
const vec3 E = vec3(0.0, 0.0, 1.0);

// Shininess
const float Shininess = 16.0;

void main(void)
{
    vec3 texColor = texture2D(texture, ex_texCoord).rgb;

    // Read normal from normal map and normalize from [0,1] to [-1,1]
    vec3 n = texture2D(normalspec_texture, ex_normalmap_texCoord).xyz;
    vec3 N = normalize(n * 2.0 - 1.0);

    // Calculate diffuse lighting
    vec3 L = normalize(ex_lightDir);
    float NdotL = max(0.0, dot(N, L)) * 0.8 + 0.2;

    // Calculate specular lighting
    vec3 R = reflect(-L, N);
    float RdotE = max(0.0, dot(R, E));
    float specular = pow(RdotE, Shininess);
    vec3 specularColor = texture2D(normalspec_texture, ex_specmap_texCoord).xyz;

    vec3 color = mix(texColor * NdotL, specularColor, specular);

    gl_FragColor = vec4(color, 1.0);
}
