/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

uniform vec4 fragColor;

void main(void)
{
    gl_FragColor = vec4(fragColor.x, fragColor.y, fragColor.z, fragColor.w);
}
