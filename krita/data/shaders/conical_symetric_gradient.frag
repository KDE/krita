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
uniform float gradientVectorAngle;

const float PI = 3.1415926535;

void main(void)
{
    vec2 p = gl_TexCoord[0].st - gradientVectorStart;
    
    float angle = atan(p.y, p.x) + PI;
    angle -= gradientVectorAngle;

    if (angle < 0.0) {
        angle += 2.0 * PI;
    }

    float t;

    if (angle < PI) {
        t = angle / PI;
    }
    else {
        t = 1.0 - ((angle - PI) / PI);
    }

    gl_FragColor = texture1D(gradientColors, t);
}

