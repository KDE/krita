/*
 *  SPDX-FileCopyrightText: 2007 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
