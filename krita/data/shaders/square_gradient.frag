/*
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
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

