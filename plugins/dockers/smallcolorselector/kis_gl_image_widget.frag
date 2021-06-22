/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
// vertices data
INATTR highp vec4 textureCoordinates;
uniform sampler2D f_tileTexture;
DECLARE_OUT_VAR

void main()
{
    // get the fragment color from the tile texture
    highp vec4 color = texture2D(f_tileTexture, textureCoordinates.st);
    OUT_VAR = vec4(color);  
}
