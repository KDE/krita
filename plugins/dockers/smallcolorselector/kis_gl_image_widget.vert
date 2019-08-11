/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
