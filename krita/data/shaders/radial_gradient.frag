/*
 *  SPDX-FileCopyrightText: 2007 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

uniform sampler1D gradientColors;

uniform vec2 gradientVectorStart;

void main(void)
{
    float t = distance(gl_TexCoord[0].st, gradientVectorStart);

    gl_FragColor = texture1D(gradientColors, t);
}
