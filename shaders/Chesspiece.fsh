precision highp float;

uniform lowp sampler2D texture;
uniform lowp samplerCube env_cube_map;

varying mediump vec2 ex_texCoord;
varying mediump vec3 ex_normal; // Surface normal (in object space)
varying vec3 ex_reflect; // Reflection vector (in object space)
varying vec3 ex_eyeDir; // Direction to the eye
varying float ex_blur; // Blur factor in range [0, 1]

// Specular lighting properties
const float shininess = 64.0;
const vec4 specular_color = vec4(1.0, 1.0, 1.0, 1.0);

// Diffuse lighting properties
const float diffRange = 0.7;
const float diffMin = 1.0 - diffRange;

void main(void)
{
    vec4 tex_color = texture2D(texture, ex_texCoord);
    vec4 ref_color = textureCube(env_cube_map, ex_reflect);
    vec4 mix_color = mix(tex_color, ref_color, 0.65);

    vec3 light_dir = normalize(ex_eyeDir);
    vec3 normal = normalize(ex_normal);

    float dot_product = clamp(dot(normal, light_dir), 0.0, 1.0);
    float diffuseI = dot_product * diffRange + diffMin;
    // Since E == L, we'll fake phong highlight this way
    float specularI = pow(dot_product, shininess);

    vec3 diffuse_color = vec3(diffuseI * mix_color.rgb);
    vec3 color = vec3(diffuse_color + (specular_color.rgb * specularI));

    gl_FragColor = vec4(color, ex_blur);
}
