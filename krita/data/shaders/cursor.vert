#version 150
uniform mat4 modelViewProjection;

in highp vec4 a_vertexPosition;

void main()
{
    gl_Position = modelViewProjection * a_vertexPosition;
}
