#version 150 core
uniform mat4 modelViewProjection;

in vec4 in_vertexPosition;

void main()
{
    gl_Position = modelViewProjection * in_vertexPosition;
}

