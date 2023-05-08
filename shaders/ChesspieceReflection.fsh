precision highp float;

uniform lowp sampler2D texture;
uniform lowp float fade;

varying mediump vec2 ex_texCoord;
varying mediump vec3 ex_normal; // Surface normal (in object space)
varying vec3 ex_eyeDir; // Direction to the eye

// Specular lighting properties
const float shininess = 64.0;
const vec4 specular_color = vec4(1.0, 1.0, 1.0, 1.0);

// Diffuse lighting properties
const float diffRange = 0.7;
const float diffMin = 1.0 - diffRange;

void main(void)
{
    vec4 tex_color = texture2D(texture, ex_texCoord);

    vec3 L = normalize(ex_eyeDir);
    vec3 N = normalize(ex_normal);

    float dot_product = clamp(dot(N, L), 0.0, 1.0);
    float diffuseI = dot_product * diffRange + diffMin;
    float specularI = pow(dot_product, shininess);

    vec3 diffuse_color = vec3(diffuseI * tex_color.rgb);
    vec3 color = vec3(diffuse_color + (specular_color.rgb * specularI));

    gl_FragColor = vec4(color, fade);
}
