precision highp float;

uniform mat4 mvp_matrix;
uniform highp vec3 light_pos; // In object space
uniform highp vec3 eye_pos; // In object space
uniform mediump mat4 shadow_matrix;

attribute vec3 in_coord;
attribute vec2 in_texCoord;
attribute vec3 in_normal;
attribute vec4 in_tangent;

varying mediump vec2 ex_texCoord;
varying mediump vec3 ex_normal;
varying mediump vec3 ex_lightDir; // In tangent space
varying mediump vec3 ex_eyeDir; // In tangent space
varying mediump vec4 ex_shadowCoord;

void main(void)
{
    gl_Position = mvp_matrix * vec4(in_coord, 1.0);
    ex_texCoord = in_texCoord;
    ex_shadowCoord = shadow_matrix * vec4(in_coord, 1.0);

    // Calculate the direction to the light from the current vertex
    vec3 lightDir = normalize(light_pos - in_coord);

    // Calculate the direction to the eye (camera) from the current vertex
    vec3 eyeDir = normalize(eye_pos - in_coord);

    // Calculate the object space -> tangent space matrix (TNB)
    vec3 n = normalize(in_normal);
    vec3 t = normalize(in_tangent.xyz);
    vec3 b = cross(n, t) * in_tangent.w; // w = handedness, 1.0 / -1.0

    // Transform the light direction vector by the tangent space basis
    vec3 l;
    l.x = dot(lightDir, t);
    l.y = dot(lightDir, b);
    l.z = dot(lightDir, n);
    ex_lightDir = l; 

    // Transform the eye direction vector by the tangent space basis
    vec3 e;
    e.x = dot(eyeDir, t);
    e.y = dot(eyeDir, b);
    e.z = dot(eyeDir, n);
    ex_eyeDir = e;
}

