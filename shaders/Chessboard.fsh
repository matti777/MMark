precision highp float;

uniform lowp sampler2D texture;
uniform lowp samplerCube env_cube_map;

varying mediump vec2 ex_texCoord;
varying vec3 ex_reflect; // Reflection vector (in object space)
varying float ex_blur; // Blur factor in range [0, 1]

void main(void)
{
    vec4 ref_color = textureCube(env_cube_map, ex_reflect);
    vec4 tex_color = texture2D(texture, ex_texCoord);

    gl_FragColor = vec4(mix(tex_color, ref_color, 0.5).rgb, ex_blur);
}
