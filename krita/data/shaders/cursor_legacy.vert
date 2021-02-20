/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

uniform mat4 modelViewProjection;

attribute highp vec4 a_vertexPosition;

void main()
{
    gl_Position = modelViewProjection * a_vertexPosition;
}

