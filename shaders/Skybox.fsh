precision highp float;

uniform lowp samplerCube skybox;

varying mediump vec3 ex_cubeTexCoord;

// Use value of 254 (8bit) for alpha to provide a way to detect the skybox
// via glReadPixels() for lens flare occlusion testing for example
const float SkyboxAlpha = 254.0 / 255.0;

void main(void)
{
    vec4 color = textureCube(skybox, ex_cubeTexCoord);
    gl_FragColor = vec4(color.rgb, SkyboxAlpha);
}
