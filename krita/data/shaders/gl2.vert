/*
 * Vertex shader for handling scaling
 */
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

attribute vec4 vertex;
attribute vec2 uv0;

varying vec2 out_uv0;

void main()
{
    gl_Position = projectionMatrix * viewMatrix *modelMatrix * vertex;
    out_uv0 = uv0;
}
