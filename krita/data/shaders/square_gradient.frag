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
    vec2 p = gl_TexCoord[0].st - gradientVectorStart;

    /* Point to line distance is:
     * distance = ((l0.y() - l1.y()) * p.x() + (l1.x() - l0.x()) * p.y() + l0.x() * l1.y() - l1.x() * l0.y())
     *
     * Here l0 = (0, 0) and |l1 - l0| = 1
     */

    float distance1 = -normalisedGradientVector.y * p.x + normalisedGradientVector.x * p.y;
    distance1 = abs(distance1);

    /* Rotate point by 90 degrees and get the distance to the perpendicular */
    float distance2 = -normalisedGradientVector.y * -p.y + normalisedGradientVector.x * p.x;
    distance2 = abs(distance2);

    float t = max(distance1, distance2);

    gl_FragColor = texture1D(gradientColors, t);
}
