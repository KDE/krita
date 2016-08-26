#version 150 core

/*
 * Vertex shader for handling scaling
 */
uniform mat4 modelViewProjection;
uniform mat4 textureMatrix;

in vec4 in_vertexPosition;
in vec4 in_textureCoordinate;

out vec4 out_textureCoordinate;

void main()
{
    gl_Position = modelViewProjection * in_textureCoordinate;
    out_textureCoordinate = textureMatrix * in_textureCoordinate;
}
