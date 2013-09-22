/*
 * Vertex shader for handling scaling
 */
uniform mat4 modelViewProjection;
uniform mat4 textureMatrix;

attribute highp vec4 a_vertexPosition;
attribute mediump vec4 a_textureCoordinate;

varying vec4 v_textureCoordinate;

void main()
{
    gl_Position = modelViewProjection * a_vertexPosition;
    v_textureCoordinate = textureMatrix * a_textureCoordinate;
}
