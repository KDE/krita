/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef USE_OPENGLES
#define INATTR in
#define OUTATTR out
#define highp
#else
#define INATTR attribute
#define OUTATTR varying
#endif
uniform mat4 viewProjectionMatrix;
INATTR highp vec3 vertexPosition;
INATTR highp vec2 texturePosition;
OUTATTR highp vec4 textureCoordinates;
void main()
{
   textureCoordinates = vec4(texturePosition.x, texturePosition.y, 0.0, 1.0);
   gl_Position = viewProjectionMatrix * vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0);
}
