#version 130

/*
 * Vertex shader for handling scaling
 */
uniform mat4 modelViewProjection;
uniform mat4 textureMatrix;

in vec4 a_vertexPosition;
in vec4 a_textureCoordinate;

out vec4 v_textureCoordinate;

void main()
{
    gl_Position = modelViewProjection * a_vertexPosition;
    v_textureCoordinate = textureMatrix * a_textureCoordinate;
}
