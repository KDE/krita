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
#define DECLARE_OUT_VAR out vec4 f_fragColor;
#define OUT_VAR f_fragColor
#define highp
#define texture2D texture
#else
#define INATTR varying
#define DECLARE_OUT_VAR
#define OUT_VAR gl_FragColor
#endif
// vertices datas
INATTR highp vec4 textureCoordinates;
uniform sampler2D f_tileTexture;
DECLARE_OUT_VAR

void main()
{
    // get the fragment color from the tile texture
    highp vec4 color = texture2D(f_tileTexture, textureCoordinates.st);
    OUT_VAR = vec4(color);  
}
