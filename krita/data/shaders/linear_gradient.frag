/*
 *  SPDX-FileCopyrightText: 2007 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

uniform sampler1D gradientColors;

uniform vec2 gradientVectorStart;
uniform vec2 normalisedGradientVector;

void main(void)
{
    vec2 v = gl_TexCoord[0].st - gradientVectorStart;

    /* Project the vector onto the normalised gradient vector. */
    float t = dot(v, normalisedGradientVector);

    gl_FragColor = texture1D(gradientColors, t);
}
